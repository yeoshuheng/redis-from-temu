//
// Created by Yeo Shu Heng on 19/4/26.
//
#include "../../include/runtime/executor.hpp"

namespace runtime {
DBCoreExecutor::DBCoreExecutor(const core_ptr& core, const boost::asio::any_io_executor& executor)
    : core(core), strand(executor) {};

boost::asio::awaitable<response::CoreResp> DBCoreExecutor::execute(command::Command cmd) const {
    co_return co_await boost::asio::co_spawn(
        strand,
        [this, cmd = std::move(cmd)]() mutable -> boost::asio::awaitable<response::CoreResp> {
            co_return core->execute(cmd);
        },
        boost::asio::use_awaitable);
};
} // namespace runtime