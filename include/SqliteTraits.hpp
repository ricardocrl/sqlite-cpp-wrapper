#pragma once

#include "SqliteTypes.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace base_services
{
namespace db
{

class SqliteTraits
{
public:
    static std::string SqlSelect(const std::string& table, const std::string& col, const KeyValues& filters = {});
    static std::string SqlInsert(const std::string& table, const KeyValues& keyValues, bool replace);
    static std::string SqlUpdate(const std::string& table, const KeyValues& keyValues, const KeyValues& filters = {});
    static std::string SqlDelete(const std::string& table, const KeyValues& filters = {});

    static std::string SqlCount(const std::string& table, const std::string& col, const KeyValues& filters = {});
    static std::string SqlSum(const std::string& table, const std::string& col, const KeyValues& filters = {});
    static std::string SqlAvg(const std::string& table, const std::string& col, const KeyValues& filters = {});

    static std::string SqlInsertWithPlaceholders(const std::string& table, const std::size_t& count, bool replace);

    SqliteTraits()                     = delete;
    SqliteTraits(const SqliteTraits&)  = delete;
    SqliteTraits(const SqliteTraits&&) = delete;
    ~SqliteTraits()                    = delete;

private:
    using Tokens = std::vector<std::string>;

    static std::string SqlSelectFunction(const std::string& function,
                                         const std::string& col,
                                         const std::string& table,
                                         const KeyValues& filters);

    static std::string SqlFilters(const KeyValues& keyValues);
    static std::string SqlFilter(const KeyValue& kv);

    static std::string SqlAssignments(const KeyValues& keyValues);
    static std::string SqlAssignment(const KeyValue& kv);
};

} // namespace db
} // namespace base_services
