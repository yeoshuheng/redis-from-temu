//
// Created by Yeo Shu Heng on 19/4/26.
//

#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP
#include <memory>

#include "include/core/core.hpp"

namespace runtime {
using core_ptr = std::shared_ptr<core::DBCore>;
class DBCoreExecutor {
    core_ptr core;
    boost::asio::strand<boost::asio::any_io_executor>
        strand; // serialized wrapper for core executions
  public:
    DBCoreExecutor(const core_ptr& core, const boost::asio::any_io_executor& executor);
    boost::asio::awaitable<response::CoreResp> execute(command::Command cmd) const;
};
} // namespace runtime

#endif // EXECUTOR_HPP
