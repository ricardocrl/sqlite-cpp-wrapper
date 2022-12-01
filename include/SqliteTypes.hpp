#pragma once

#include <chrono>
#include <list>
#include <optional>
#include <string>
#include <vector>

namespace sqlite_wrapper
{

using PrimaryKey  = int64_t;
using PrimaryKeys = std::vector<PrimaryKey>;

using Value = std::optional<std::string>;

using Row  = std::vector<Value>;
using Rows = std::vector<Row>;

class KeyValue
{
public:
    template<typename T>
    KeyValue(const std::string& key, const T& value)
        : mKey{key}
        , mValue{ToString(value)}
    {
    }

    template<typename T>
    KeyValue(const std::string& key, const std::optional<T>& value = std::nullopt)
        : mKey{key}
    {
        if (value)
        {
            mValue = ToString(value.value());
        }
    }

    const std::string& key() const
    {
        return mKey;
    }

    const std::optional<std::string>& value() const
    {
        return mValue;
    }

private:
    static std::string ToString(const std::string& value)
    {
        return value;
    }

    static std::string ToString(const char* const value)
    {
        return value;
    }

    static std::string ToString(bool value)
    {
        return value ? "1" : "0";
    }

    template<typename T>
    static std::string ToString(const T& value)
    {
        return std::to_string(value);
    }

    template<typename Rep, typename Per>
    static std::string ToString(const std::chrono::duration<Rep, Per>& value)
    {
        return std::to_string(value.count());
    }

    template<typename TClock>
    static std::string ToString(const std::chrono::time_point<TClock>& value)
    {
        return ToString(std::chrono::duration_cast<std::chrono::seconds>(value.time_since_epoch()));
    }

    std::string mKey;
    std::optional<std::string> mValue;
};

using KeyValues = std::list<KeyValue>;

} // namespace sqlite_wrapper
