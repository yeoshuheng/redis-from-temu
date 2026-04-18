//
// Created by Yeo Shu Heng on 18/4/26.
//

#ifndef SERIALIZER_HPP
#define SERIALIZER_HPP

#include "include/core/object.hpp"
#include "resp.hpp"
#include <string>

namespace response {
using core::stored_value;
class Serializer {
    static std::string serialize_value(const stored_value& value);

  public:
    static std::string serialize(const CoreResp& resp);
};
} // namespace response

#endif // SERIALIZER_HPP
