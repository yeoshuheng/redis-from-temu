//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef OBJECT_HPP
#define OBJECT_HPP

namespace core {
using stored_value = std::variant<int64_t, double, float, std::string>;
struct LRUObject {
    stored_value val;
};
} // namespace core

#endif // OBJECT_HPP
