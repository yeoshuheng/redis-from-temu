//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef OBJECT_HPP
#define OBJECT_HPP

namespace storage {
using stored_value = std::variant<int64_t, std::string>;
struct StoredObject {
    stored_value val;
};
}  // namespace storage

#endif  // OBJECT_HPP
