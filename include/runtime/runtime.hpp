//
// Created by Yeo Shu Heng on 18/4/26.
//

#ifndef SESSION_HPP
#define SESSION_HPP
#include "include/core/core.hpp"
#include "include/wal/wal.hpp"

namespace runtime {
    using wal_ptr = std::shared_ptr<wal::WAL>;

    class RedisRuntime final: commons::ThreadHeartBeat {
        wal_ptr wal;
        core::RedisCore core;


    };
}

#endif //SESSION_HPP
