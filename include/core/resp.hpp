//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef RESP_HPP
#define RESP_HPP

namespace core {
struct CoreResp {
    enum class RespType { OK = 0, VALUE = 1, NIL = 2, ERROR = 3 };
    RespType type;
    std::optional<stored_value> value;
    std::string message;
};
} // namespace core

#endif // RESP_HPP
