//
// Created by Yeo Shu Heng on 22/4/26.
//
#include "../../include/runtime/engine.hpp"

#include <ranges>

namespace runtime {
DBEngine::DBEngine(const std::string& host, uint8_t port, std::unique_ptr<core::DBCore> core,
    std::unique_ptr<disk::DiskManager> disk_manager)
    : accept(core_ctx, {boost::asio::ip::make_address(std::move(host)), port}),
      core(std::move(core)), disk_manager(std::move(disk_manager)) {};

void DBEngine::run() {
    if (auto expected = EngineState::STOPPED;
        !state.compare_exchange_strong(expected, EngineState::RUNNING)) {
        return;
    }
    disk_manager->start();
    accept_loop();
    core_ctx.run();
};

void DBEngine::close() {
    if (auto expected = EngineState::RUNNING;
        !state.compare_exchange_strong(expected, EngineState::STOP_REQUESTED)) {
        return;
    }
    boost::system::error_code ec;
    accept.close(ec);
    for (const auto& session : sessions | std::views::values) {
        end_session(session->id);
    }
    core_ctx.stop();
    disk_manager->shutdown();
    state.store(EngineState::STOPPED, std::memory_order_release);
}

void DBEngine::accept_loop() {
    if (state.load(std::memory_order_acquire) != EngineState::RUNNING) {
        return;
    }
    auto sock = std::make_shared<boost::asio::ip::tcp::socket>(core_ctx);
    accept.async_accept(*sock, [this, sock](const boost::system::error_code& ec) {
        const auto id = curr_id.fetch_add(1);
        if (!ec) {
            auto session = std::make_shared<DBSession>(id, sock);
            sessions.emplace(id, std::move(session));
            start_read(id);
        } else {
            spdlog::error("failed to accept connection, {}", ec.message());
            if (const auto it = sessions.find(id); it != sessions.end()) {
                it->second->socket->close();
                sessions.erase(it);
            }
        }
        accept_loop();
    });
}

void DBEngine::end_session(const session_id id) {
    const auto it = sessions.find(id);
    if (it == sessions.end()) {
        return;
    }
    const auto& session = it->second;
    if (auto expected = SessionState::ACTIVE;
        !session->state.compare_exchange_strong(expected, SessionState::CLOSING)) {
        return;
    }
    boost::system::error_code ec;
    session->state.store(SessionState::CLOSING, std::memory_order_release);
    session->socket->cancel(ec);
    session->socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    session->socket->close(ec);
    session->state.store(SessionState::CLOSED, std::memory_order_release);
    sessions.erase(it);
}

void DBEngine::start_write(session_id id) {
    if (state.load(std::memory_order_acquire) != EngineState::RUNNING) {
        return;
    }
    const auto it = sessions.find(id);
    if (it == sessions.end()) {
        return;
    }
    const auto& session = it->second;
    if (session->state.load(std::memory_order_acquire) != SessionState::ACTIVE) {
        return;
    }
    const auto data = std::make_shared<std::string>(std::move(session->write_buffer));
    boost::asio::async_write(*session->socket, boost::asio::buffer(*data),
        [this, id, data](const boost::system::error_code& ec, std::size_t) {
            if (ec) {
                if (ec != boost::asio::error::operation_aborted && ec != boost::asio::error::eof) {
                    spdlog::error("failed to write to session with id={} with unexpected error={}",
                        id, ec.message());
                }
                end_session(id);
                return;
            }
            const auto itr = sessions.find(id);
            if (itr == sessions.end()) {
                return;
            }
            const auto& s = itr->second;
            s->write_buffer.clear();
        });
};

void DBEngine::start_read(session_id id) {
    if (state.load(std::memory_order_acquire) != EngineState::RUNNING) {
        return;
    }
    const auto it = sessions.find(id);
    if (it == sessions.end()) {
        return;
    }
    const auto& session = it->second;
    if (session->state.load(std::memory_order_acquire) != SessionState::ACTIVE) {
        return;
    }
    session->socket->async_read_some(boost::asio::buffer(session->read_buffer),
        [this, id](const boost::system::error_code& ec, const std::size_t bytes_transferred) {
            if (ec) {
                if (ec != boost::asio::error::operation_aborted && ec != boost::asio::error::eof) {
                    spdlog::error("failed to read from session with id={} with unexpected error={}",
                        id, ec.message());
                }
                end_session(id);
                return;
            }
            const auto itr = sessions.find(id);
            if (itr == sessions.end()) {
                return;
            }
            const auto& s = itr->second;
            s->parser.feed(s->read_buffer.data(), bytes_transferred);
            while (s->parser.has_next_msg()) {
                const auto cmd_opt = s->parser.next_msg();
                if (!cmd_opt.has_value()) {
                    continue;
                }
                command::Command cmd = cmd_opt.value();
                const auto result = core->execute(cmd);
                s->write_buffer.append(std::move(serializer.serialize(result)));
            }
            if (!s->write_buffer.empty()) {
                start_write(id);
            } else {
                start_read(id);
            }
        });
}
} // namespace runtime