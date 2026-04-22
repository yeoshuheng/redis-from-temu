//
// Created by Yeo Shu Heng on 22/4/26.
//

#ifndef ENGINE_STATE_HPP
#define ENGINE_STATE_HPP

namespace runtime {
enum class EngineState {
    STOPPED = 0,
    RUNNING = 1,
    STOP_REQUESTED = 2,
};
}

#endif // ENGINE_STATE_HPP
