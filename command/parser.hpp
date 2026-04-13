//
// Created by Yeo Shu Heng on 13/4/26.
//

#ifndef PARSER_HPP
#define PARSER_HPP

#include <optional>
#include <vector>

#include "command.hpp"
#include "parsed_results.hpp"

namespace command {

using ResultOption = std::optional<ParsedResult>;
using ParsedCommandOption = std::optional<Command>;

class Parser {
   public:
    void feed(const char* data, size_t len);
    [[nodiscard]] ParsedCommandOption next_msg();

   private:
    std::vector<char> buffer;
    size_t offset = 0;
    [[nodiscard]] ResultOption parse_array(size_t start);
    [[nodiscard]] ResultOption parse_bulk(size_t start);
    [[nodiscard]] std::optional<size_t> find_terminal_idx(size_t start) const;
};
}  // namespace command

#endif  // PARSER_HPP
