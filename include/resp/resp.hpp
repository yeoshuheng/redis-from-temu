//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef RESP_HPP
#define RESP_HPP

#include "../core/object.hpp"
#include <optional>

namespace response {
using core::stored_value;
struct CoreResp {
    enum class RespType { OK = 0, VALUE = 1, NIL = 2, ERROR = 3 };
    RespType type;
    std::optional<stored_value> value;
    std::string message;
};
} // namespace response

#endif // RESP_HPP
