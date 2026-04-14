//
// Created by Yeo Shu Heng on 14/4/26.
//

#ifndef PARSER_RESULTS_HPP
#define PARSER_RESULTS_HPP

#include <string>
#include <vector>

namespace command {
struct ParsedResult {
    size_t consumed;
    std::vector<std::string> value;
};
} // namespace command

#endif // PARSER_RESULTS_HPP
