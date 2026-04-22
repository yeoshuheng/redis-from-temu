//
// Created by Yeo Shu Heng on 22/4/26.
//

#ifndef ENGINE_STATE_HPP
#define ENGINE_STATE_HPP

namespace runtime {

enum struct SessionState { ACTIVE = 0, CLOSING = 1, CLOSED = 2 };

enum struct EngineState { STOPPED = 0, RUNNING = 1, STOP_REQUESTED = 2 };
} // namespace runtime

#endif // ENGINE_STATE_HPP
