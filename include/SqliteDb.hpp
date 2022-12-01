#pragma once

#include "SqliteTypes.hpp"

#include <cstddef>

namespace base_services
{
namespace db
{

/**
 * @interface SqliteDb
 * @brief Provides access to an on-disk SQLite database.
 */
class SqliteDb
{
public:
    virtual ~SqliteDb() = default;

    /**
     * @brief Get the file path to the on-disk database this @c SqliteDb is binded to.
     * @return The file path to the database file.
     */
    virtual const std::string& getDatabasePath() const = 0;

    /**
     * @brief Open a connection to the on-disk database this @c SqliteDb is binded to.
     * @return True if successful, false otherwise.
     */
    virtual bool open() = 0;

    /**
     * @brief Check whether the connection to the database is open.
     * @return True if connection is open, false otherwise.
     */
    virtual bool isOpen() const = 0;

    /**
     * @brief Apply an SQL statement.
     * @param sql The statement to be applied.
     */
    virtual void applySql(const std::string& sql) = 0;

    /**
     * @brief Check whether the specified table exists.
     * @param table The target table.
     * @return True if @arg table exists, false otherwise.
     */
    virtual bool tableExists(const std::string& table) = 0;

    /**
     * @brief Begin a transaction.
     * @param enableForeignKeys Whether to enable foreign key constraints (https://www.sqlite.org/foreignkeys.html).
     *
     * When renaming or removing columns, SQLite forces you to DROP tables.
     * In such cases, @arg enableForeignKeys must be false to prevent tables
     * being dropped on cascade (by mistake).
     */
    virtual void beginTransaction(bool enableForeignKeys = true) = 0;

    /**
     * @brief Commit an active transaction.
     */
    virtual void commitTransaction() = 0;

    /**
     * @brief Rollback an active transaction.
     */
    virtual void rollbackTransaction() = 0;

    /**
     * @brief Select all columns of all rows from the specified table.
     * @param table The target table.
     * @param filters The target filters, if any.
     * @return All matching rows.
     */
    virtual Rows select(const std::string& table, const KeyValues& filters = {}) = 0;

    /**
     * @brief Select a column of all rows from the specified table.
     * @param table The target table.
     * @param col The target column.
     * @param filters The target filters, if any.
     * @return All matching rows.
     */
    virtual Rows select(const std::string& table, const std::string& col, const KeyValues& filters = {}) = 0;

    /**
     * @brief Create a row in a table using the specified key-value pairs.
     * @param table The target table.
     * @param keyValues The key-value pairs.
     * @param transaction Whether the operation is part of an active transaction.
     * @return The @c PrimaryKey of the created row.
     */
    virtual PrimaryKey insert(const std::string& table, const KeyValues& keyValues, bool transaction = false) = 0;

    /**
     * @brief Create one or multiple rows in a table.
     * @param table The target table.
     * @param rows The @c Rows to be inserted in @arg table.
     * @param transaction Whether the SQLite statement is being run inside a transaction or not.
     * @return The @c PrimaryKeys of the created rows.
     */
    virtual PrimaryKeys insert(const std::string& table, const Rows& rows, bool transaction = false) = 0;

    /**
     * @brief Create or replace a row in a table using the specified key-value pairs.
     * @param table The target table.
     * @param keyValues The key-value pairs.
     * @param transaction Whether the operation is part of an active transaction.
     * @return The @c PrimaryKey of the created or replaced row.
     */
    virtual PrimaryKey insertOrReplace(const std::string& table, const KeyValues& keyValues, bool transaction = false)
        = 0;

    /**
     * @brief Create or replace one or multiple rows in a table.
     * @param table The target table.
     * @param rows The @c Rows to be inserted in @arg table.
     * @param transaction Whether the SQLite statement is being run inside a transaction or not.
     * @return The @c PrimaryKeys of the created rows.
     */
    virtual PrimaryKeys insertOrReplace(const std::string& table, const Rows& rows, bool transaction = false) = 0;

    /**
     * @brief Update rows in a table with the specified key-value pairs.
     * @param table The target table.
     * @param keyValues The key-value pairs.
     * @param filters The target filters, if any.
     * @param transaction Whether the operation is part of an active transaction.
     *
     * If @arg filters is empty, all rows in the target table will be updated.
     */
    virtual void update(const std::string& table,
                        const KeyValues& keyValues,
                        const KeyValues& filters = {},
                        bool transaction         = false)
        = 0;

    /**
     * @brief Delete rows from the specified table.
     * @param table The target table.
     * @param filters The target filters, if any.
     * @param transaction Whether the operation is part of an active transaction.
     *
     * If @arg filters is empty, all rows in the target table will be deleted.
     */
    virtual void deleteRows(const std::string& table, const KeyValues& filters = {}, bool transaction = false) = 0;

    /**
     * @brief Count the number of rows in the specified table.
     * @param table The target table.
     * @param filters The target filters, if any.
     * @return The counted number of rows.
     */
    virtual std::size_t count(const std::string& table, const KeyValues& filters = {}) = 0;

    /**
     * @brief Count the number of rows in a table for which the specified column is not NULL.
     * @param table The target table.
     * @param col The target column.
     * @param filters The target filters, if any.
     * @return The end result.
     */
    virtual std::size_t count(const std::string& table, const std::string& col, const KeyValues& filters = {}) = 0;

    /**
     * @brief Sum all non-NULL values from a column in the specified table.
     * @param table The target table.
     * @param col The target column.
     * @param filters The target filters, if any.
     * @return The end result.
     *
     * If all values all NULL, or no rows are found, this function returns 0.0.
     */
    virtual double sum(const std::string& table, const std::string& col, const KeyValues& filters = {}) = 0;

    /**
     * @brief Get the average of all non-NULL values from a column in the specified table.
     * @param table The target table.
     * @param col The target column.
     * @param filters The target filters, if any.
     * @return The end result.
     *
     * If all values all NULL, or no rows are found, this function returns 0.0.
     */
    virtual double average(const std::string& table, const std::string& col, const KeyValues& filters = {}) = 0;
};

} // namespace db
} // namespace base_services
