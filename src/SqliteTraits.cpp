#include "SqliteTraits.hpp"

#include "StringUtils.hpp"

#include <algorithm>

namespace sqlite_wrapper
{

std::string SqliteTraits::SqlSelect(const std::string& table, const std::string& col, const KeyValues& filters)
{
    // SQL statement:
    //     SELECT <col> FROM <table> <filters>;

    Tokens tokens{"SELECT ", col, " FROM ", table, SqlFilters(filters), ";"};
    return StringUtils::Join(tokens, StringUtils::empty);
}

std::string SqliteTraits::SqlInsert(const std::string& table, const KeyValues& keyValues, bool replace)
{
    // SQL statement:
    //     INSERT [OR REPLACE] INTO <table> (<keys>) VALUES (<values>);

    if (keyValues.empty())
    {
        return std::string();
    }

    static auto getVal = [](const KeyValue& kv) { return kv.value() ? StringUtils::Quote(*kv.value()) : "NULL"; };

    std::string keys = keyValues.front().key();
    for (auto it = ++keyValues.begin(); keyValues.end() != it; ++it)
    {
        keys += ", " + it->key();
    }

    std::string values = getVal(keyValues.front());
    for (auto it = ++keyValues.begin(); keyValues.end() != it; ++it)
    {
        values += ", " + getVal(*it);
    }

    Tokens tokens{"INSERT ", replace ? "OR REPLACE" : "", " INTO ", table, "(", keys, ") VALUES (", values, ");"};
    return StringUtils::Join(tokens, StringUtils::empty);
}

std::string SqliteTraits::SqlUpdate(const std::string& table, const KeyValues& keyValues, const KeyValues& filters)
{
    // SQL statement:
    //     UPDATE <table> SET <key-value pairs> <filters>;

    Tokens tokens{"UPDATE ", table, " SET ", SqlAssignments(keyValues), SqlFilters(filters), ";"};
    return StringUtils::Join(tokens, StringUtils::empty);
}

std::string SqliteTraits::SqlDelete(const std::string& table, const KeyValues& filters)
{
    // SQL statement:
    //     DELETE FROM <table> <filters>;

    Tokens tokens{"DELETE FROM ", table, SqlFilters(filters), ";"};
    return StringUtils::Join(tokens, StringUtils::empty);
}

std::string SqliteTraits::SqlInsertWithPlaceholders(const std::string& table, const size_t& count, bool replace)
{
    // SQL statement:
    //     INSERT [OR REPLACE] INTO <table> VALUES (<placeholders>);

    std::vector<char> params(count);
    std::fill(params.begin(), params.end(), '?');
    auto placeholders = StringUtils::Join(params, ", ");

    Tokens tokens{"INSERT ", replace ? "OR REPLACE" : "", " INTO ", table, " VALUES (", placeholders, ");"};
    return StringUtils::Join(tokens, StringUtils::empty);
}

std::string SqliteTraits::SqlCount(const std::string& table, const std::string& col, const KeyValues& filters)
{
    // SQL statement:
    //     SELECT COUNT(<column>) FROM <table> <filters>;

    return SqlSelectFunction("COUNT", col, table, filters);
}

std::string SqliteTraits::SqlSum(const std::string& table, const std::string& col, const KeyValues& filters)
{
    // SQL statement:
    //     SELECT SUM(<column>) FROM <table> <filters>;

    return SqlSelectFunction("SUM", col, table, filters);
}

std::string SqliteTraits::SqlAvg(const std::string& table, const std::string& col, const KeyValues& filters)
{
    // SQL statement:
    //     SELECT AVG(<column>) FROM <table> <filters>;

    return SqlSelectFunction("AVG", col, table, filters);
}

std::string SqliteTraits::SqlSelectFunction(const std::string& function,
                                            const std::string& col,
                                            const std::string& table,
                                            const KeyValues& filters)
{
    // SQL statement:
    //     SELECT <function>(<column>) FROM <table> <filters>;

    Tokens tokens{"SELECT ", function, "(", col, ")", " FROM ", table, SqlFilters(filters), ";"};
    return StringUtils::Join(tokens, StringUtils::empty);
}

std::string SqliteTraits::SqlFilters(const KeyValues& keyValues)
{
    std::string sql;

    if (!keyValues.empty())
    {
        sql += " WHERE " + SqlFilter(keyValues.front());
        for (auto it = ++keyValues.begin(); keyValues.end() != it; ++it)
        {
            sql += " AND " + SqlFilter(*it);
        }
    }

    return sql;
}

std::string SqliteTraits::SqlFilter(const KeyValue& keyValue)
{
    return keyValue.key() + (keyValue.value() ? "=" + StringUtils::Quote(*keyValue.value()) : " IS NULL");
}

std::string SqliteTraits::SqlAssignments(const KeyValues& keyValues)
{
    std::string sql;

    if (!keyValues.empty())
    {
        sql += " " + SqlAssignment(keyValues.front());
        for (auto it = ++keyValues.begin(); keyValues.end() != it; ++it)
        {
            sql += ", " + SqlAssignment(*it);
        }
    }

    return sql;
}

std::string SqliteTraits::SqlAssignment(const KeyValue& keyValue)
{
    return keyValue.key() + "=" + (keyValue.value() ? StringUtils::Quote(*keyValue.value()) : "NULL");
}

} // namespace sqlite_wrapper
