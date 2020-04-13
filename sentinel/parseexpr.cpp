#include "parseexpr.hpp"

#include <cctype>
#include <cstdio>

#include <utility>
#include <string_view>

namespace {

    std::string_view lex(char const* expr, char const*& begin, char const*& end) {
        bool quote_escaped       = false;
        bool char_escaped        = false;

        for (begin = expr; std::isspace(static_cast<unsigned char>(*begin)); ++begin) ;
        quote_escaped = *begin == '\"';

        for (end = begin + int(quote_escaped); *end; ++end) {
            char const ch = *end;
            bool const is_space = std::isspace(static_cast<unsigned char>(ch));

            if (is_space) {
                if (!quote_escaped) break;
            } else if (char_escaped) {
                char_escaped = false;
            } else if (ch == '\\') {
                char_escaped = true;
            } else if (ch == '\"') {
                ++end;
                break;
            } else {
                /* CONTINUE AS NORMAL */
            }
        }

        return std::string_view(begin, end - begin);
    }

    std::string unescape(std::string_view const& e) {
        std::string result;
        result.reserve(e.size() + 1u);

        bool is_escaping = false;
        bool is_initial  = true;
        for (auto&& ch : e) {
            if (is_initial && ch == '\"') {
                /* DO NOTHING */
            } else if (is_escaping) {
                result += ch;
                is_escaping = false;
            } else if (ch == '\\') {
                is_escaping = true;
            } else if (ch != '\"') {
                result += ch;
            }

            is_initial = false;
        }

        return result;
    }

}

std::vector<std::string> ParseExpr(char const* expr) {
    std::vector<std::string> args;
    args.reserve(4);

    char const* begin = expr;
    char const* end = expr;
    do {
        begin = end;
        auto token = lex(begin, begin, end);

        if (begin != end)
            args.push_back(unescape(token));
    } while (begin != end);

    return args;
}
