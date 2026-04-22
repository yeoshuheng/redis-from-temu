//
// Created by Yeo Shu Heng on 17/4/26.
//

#ifndef STATE_HPP
#define STATE_HPP

namespace disk {
enum class DiskManagerState { RUNNING = 0, STOP_REQUESTED = 1, STOPPED = 2 };
}

#endif // STATE_HPP
