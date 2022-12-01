#pragma once

#include <list>
#include <set>
#include <string>
#include <vector>

class StringUtils
{
public:
    /**
     * @brief Construct a string from a container.
     * @param container A container of @c std::string or @c char objects.
     * @param sep Separator to be used (can be empty); defaults to @c COMMA_WHITESPACE.
     * @return A new string.
     *
     * Example(s):
     *
     *    std::vector<std::string> in {"ab", "cd"};
     *    Join(in);          // returns "ab, cd"
     *    Join(in, SLASH);   // returns "ac/cd"
     *    Join(in, EMPTY);   // returns "abcd"
     *
     *    std::vector<std::string> in {"", "aa", "", "", "bb"};
     *    Join(in, SLASH);   // returns "/aa///bb"
     */
    template<typename T>
    static std::string Join(const T& container, const std::string& sep = comma_whitespace);

    static void Quote(std::string& str, const char& ch = quote_char);
    static std::string Quote(const std::string& str, const char& ch = quote_char);

    static const char backslash_char;
    static const char double_quote_char;
    static const char plus_char;
    static const char quote_char;
    static const char slash_char;
    static const char whitespace_char;

    static const std::string backslash;
    static const std::string colon;
    static const std::string comma_whitespace;
    static const std::string empty;
    static const std::string minus;
    static const std::string slash;
    static const std::string underscore;
    static const std::string whitespace;

    StringUtils()                    = delete;
    StringUtils(const StringUtils&)  = delete;
    StringUtils(const StringUtils&&) = delete;
    ~StringUtils()                   = delete;
};

// forward declare explicit instantiations
//
extern template std::string StringUtils::Join(const std::list<std::string>&, const std::string&);
extern template std::string StringUtils::Join(const std::set<std::string>&, const std::string&);
extern template std::string StringUtils::Join(const std::vector<char>&, const std::string&);
extern template std::string StringUtils::Join(const std::vector<std::string>&, const std::string&);
extern template std::string StringUtils::Join(const std::set<char>&, const std::string&);
//
