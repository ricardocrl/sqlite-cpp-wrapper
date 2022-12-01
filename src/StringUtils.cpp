#include "StringUtils.hpp"

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <type_traits>

const char StringUtils::backslash_char    = '\\';
const char StringUtils::double_quote_char = '"';
const char StringUtils::plus_char         = '+';
const char StringUtils::quote_char        = '\'';
const char StringUtils::slash_char        = '/';
const char StringUtils::whitespace_char   = ' ';

const std::string StringUtils::backslash{"\\"};
const std::string StringUtils::colon{":"};
const std::string StringUtils::comma_whitespace{", "};
const std::string StringUtils::empty{""};
const std::string StringUtils::minus{"-"};
const std::string StringUtils::slash{"/"};
const std::string StringUtils::underscore{"_"};
const std::string StringUtils::whitespace{" "};

template<typename T>
std::string StringUtils::Join(const T& container, const std::string& sep)
{
    if constexpr (std::is_same<char, typename T::value_type>::value)
    {
        std::vector<std::string> copy;
        std::for_each(container.cbegin(), container.cend(), [&copy](const char& ch) { copy.emplace_back(1, ch); });
        return Join(copy, sep);
    }
    else
    {
        return boost::algorithm::join(container, sep);
    }
}

void StringUtils::Quote(std::string& str, const char& ch)
{
    const std::string q(1, ch);
    str = q + str + q;
}

std::string StringUtils::Quote(const std::string& str, const char& ch)
{
    const std::string q(1, ch);
    return q + str + q;
}

// explicit instantiations
//
template std::string StringUtils::Join(const std::list<std::string>&, const std::string&);
template std::string StringUtils::Join(const std::set<std::string>&, const std::string&);
template std::string StringUtils::Join(const std::vector<char>&, const std::string&);
template std::string StringUtils::Join(const std::vector<std::string>&, const std::string&);
template std::string StringUtils::Join(const std::set<char>&, const std::string&);
//
