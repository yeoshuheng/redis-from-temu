//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

namespace utils {

inline uint32_t read_u32(const char*& ptr) {
    uint32_t v;
    std::memcpy(&v, ptr, sizeof(v));
    ptr += sizeof(v);
    return v;
};

inline void write_u32(std::string& out, const uint32_t v) {
    out.append(reinterpret_cast<const char*>(&v), sizeof(v));
};

inline bool fast_str_equals(const std::string& s, const char* l) {
    // faster way to compare 2 strings agnostic to casing.
    size_t i = 0;
    for (; l[i]; ++i) {
        if (i >= s.size()) {
            return false;
        }
        // bitwise OR with 0x20 converts upper to lower and keeps lower
        if ((s[i] | 0x20) != (l[i] | 0x20)) {
            return false;
        }
    }
    return i == s.size();
};
} // namespace utils

#endif // UTILS_HPP
