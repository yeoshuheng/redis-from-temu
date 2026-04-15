//
// Created by Yeo Shu Heng on 15/4/26.
//

#ifndef HEARTBEAT_HPP
#define HEARTBEAT_HPP
#include <atomic>

#include "include/core/core.hpp"
#include "include/core/lru_cache.hpp"

namespace commons {
struct ThreadHeartBeatState {
    std::atomic<core::Clock::time_point> core_heartbeat;
    std::atomic<core::Clock::time_point> disk_heartbeat;
};
using heartbeat_state = std::shared_ptr<ThreadHeartBeatState>;
class ThreadHeartBeat {
  public:
    explicit ThreadHeartBeat(const heartbeat_state& hb_state, const uint32_t beat_interval_ms = 100,
        const uint32_t check_interval_ms = 100)
        : hb_state(hb_state), beat_interval_ms(beat_interval_ms),
          check_interval_ms(check_interval_ms) {};
    virtual ~ThreadHeartBeat() = default;

  protected:
    heartbeat_state hb_state;
    uint32_t beat_interval_ms;
    uint32_t check_interval_ms;

    virtual boost::asio::awaitable<void> beat_loop() = 0;
    virtual boost::asio::awaitable<void> check_loop(std::chrono::milliseconds timeout) = 0;
};
} // namespace commons

#endif // HEARTBEAT_HPP
