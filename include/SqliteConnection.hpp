#pragma once

#include "SqliteDb.hpp"

#include "sqlite_modern_cpp.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace base_services
{
namespace db
{

/**
 * @class SqliteConnection
 * @brief Implements @c SqliteDb using [sqlite_modern_cpp](https://github.com/SqliteModernCpp/sqlite_modern_cpp.)
 *
 * This implementation allows read and write access to a DB file, concurrently.
 * An instance of this class instantiates a single DB connection. It can be used
 * concurrently from multiple threads for both reads, writes and transactions.
 * Multiple instances of this class also allow concurrent read  and write access.
 *
 * Write access is allowed using two mechanisms:
 * - Transactions and individual writes in the same connection from multiple threads are possible
 *   by protecting any writes (individual operations and full-transaction) with the same mutex.
 * - Between connections where mutex instance are not shared, the concurrency handling is
 *   achieved using sqlite3_busy_timeout().
 */
class SqliteConnection : public SqliteDb
{
public:
    SqliteConnection(const std::string& databasePath);

    const std::string& getDatabasePath() const override;
    bool open() override;
    bool isOpen() const override;
    void applySql(const std::string& sql) override;
    bool tableExists(const std::string& table) override;
    void beginTransaction(bool enableForeignKeys) override;
    void commitTransaction() override;
    void rollbackTransaction() override;
    Rows select(const std::string& table, const KeyValues& filters) override;
    Rows select(const std::string& table, const std::string& col, const KeyValues& filters) override;
    PrimaryKey insert(const std::string& table, const KeyValues& keyValues, bool transaction) override;
    PrimaryKeys insert(const std::string& table, const Rows& rows, bool transaction) override;
    PrimaryKey insertOrReplace(const std::string& table, const KeyValues& keyValues, bool transaction) override;
    PrimaryKeys insertOrReplace(const std::string& table, const Rows& rows, bool transaction) override;
    void
    update(const std::string& table, const KeyValues& keyValues, const KeyValues& filters, bool transaction) override;
    void deleteRows(const std::string& table, const KeyValues& filters, bool transaction) override;
    std::size_t count(const std::string& table, const KeyValues& filters) override;
    std::size_t count(const std::string& table, const std::string& col, const KeyValues& filters) override;
    double sum(const std::string& table, const std::string& col, const KeyValues& filters) override;
    double average(const std::string& table, const std::string& col, const KeyValues& filters) override;

private:
    void connectionHook();

    void lockWriteAccess(bool partOfTransaction);
    void unlockWriteAccess(bool partOfTransaction);

    PrimaryKey insertRow(const std::string& table, const KeyValues& keyValues, bool transaction, bool replace);
    PrimaryKeys insertRows(const std::string& table, const Rows& rows, bool transaction, bool replace);
    PrimaryKey executePPS(sqlite::database_binder& pps, const Row& row);

    static const int kBusyTimeoutMs = 60000;

    std::string mDatabasePath;
    sqlite::database mDatabase;
    std::mutex mWriteMutex;
    std::atomic<bool> mInTransaction;
};

} // namespace db
} // namespace base_services
