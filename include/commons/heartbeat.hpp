//
// Created by Yeo Shu Heng on 15/4/26.
//

#ifndef HEARTBEAT_HPP
#define HEARTBEAT_HPP
#include <atomic>

#include "include/core/core.hpp"
#include "include/core/lru_cache.hpp"

namespace commons {
enum class FailureState { OK = 0, SUSPECTED = 1, FAILED = 2 };

struct ThreadHeartBeatState {
    std::atomic<uint64_t> core_heartbeat;
    std::atomic<uint64_t> disk_heartbeat;
};
using heartbeat_state = std::shared_ptr<ThreadHeartBeatState>;
class ThreadHeartBeat {
  public:
    explicit ThreadHeartBeat(const heartbeat_state& hb_state, const uint32_t beat_interval_ms = 100)
        : hb_state(hb_state), beat_interval_ms(beat_interval_ms) {};
    virtual ~ThreadHeartBeat() = default;

  protected:
    heartbeat_state hb_state;
    uint32_t beat_interval_ms;

    virtual boost::asio::awaitable<void> beat_loop() = 0;
};
} // namespace commons

#endif // HEARTBEAT_HPP
