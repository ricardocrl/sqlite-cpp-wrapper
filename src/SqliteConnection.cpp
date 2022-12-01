#include "SqliteConnection.hpp"

#include "SqliteTraits.hpp"

#ifndef DEBUG
#define DEBUG 0
#endif

namespace base_services
{
namespace db
{

SqliteConnection::SqliteConnection(const std::string& databasePath)
    : mDatabasePath{databasePath}
    , mDatabase{std::shared_ptr<sqlite3>(nullptr)}
    , mInTransaction{false}
{
}

const std::string& SqliteConnection::getDatabasePath() const
{
    return mDatabasePath;
}

bool SqliteConnection::open()
{
    sqlite::sqlite_config config;
    config.flags = sqlite::OpenFlags::READWRITE | sqlite::OpenFlags::CREATE | sqlite::OpenFlags::FULLMUTEX;

    try
    {
        mDatabase = sqlite::database(mDatabasePath, config);
    }
    catch (...)
    {
        std::cerr << "File could not be opened: " << mDatabasePath << std::endl;
        return false;
    }

    connectionHook();
    return true;
}

bool SqliteConnection::isOpen() const
{
    return mDatabase.connection() != nullptr;
}

void SqliteConnection::applySql(const std::string& sql)
{
    mDatabase << sql;
}

bool SqliteConnection::tableExists(const std::string& table)
{
    // source: https://stackoverflow.com/a/1604121/5010785
    auto rows = select("sqlite_master", "name", {{"type", "table"}, {"name", table}});

    if (rows.empty() || rows[0].empty())
    {
        return false;
    }

    return rows[0].front().has_value();
}

void SqliteConnection::beginTransaction(bool enableForeignKeys)
{
    mWriteMutex.lock();
    mInTransaction = true;

    if (enableForeignKeys)
    {
        applySql("PRAGMA foreign_keys=ON;");
    }
    else
    {
        applySql("PRAGMA foreign_keys=OFF;");
    }

    std::string query = "begin;";
#if DEBUG
    std::cout << "Built SQL: " << query << std::endl;
#endif
    mDatabase << query;
}

void SqliteConnection::commitTransaction()
{
#if DEBUG
    std::cout << "Built SQL: commit;" << std::endl;
#endif

    mDatabase << "commit;";
    mInTransaction = false;
    mWriteMutex.unlock();
}

void SqliteConnection::rollbackTransaction()
{
#if DEBUG
    std::cout << "Built SQL: rollback;" << std::endl;
#endif

    mDatabase << "rollback;";
    mInTransaction = false;
    mWriteMutex.unlock();
}

Rows SqliteConnection::select(const std::string& table, const KeyValues& filters)
{
    Rows rows; // will represent an array of size N-by-M

    auto&& sql = SqliteTraits::SqlSelect(table, "*", filters);

    auto dbConnection  = mDatabase.connection().get();
    sqlite3_stmt* stmt = nullptr;
    auto hresult       = sqlite3_prepare_v2(dbConnection, sql.c_str(), -1, &stmt, nullptr);
    if (hresult != SQLITE_OK)
    {
        std::cerr << "select(), could not prepare statement, got error code: " << hresult << std::endl;
    }

    const auto numberOfColumns = sqlite3_column_count(stmt);
    while ((hresult = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        Row row;
        for (int i = 0; i < numberOfColumns; ++i)
        {
            auto columnValue = sqlite3_column_text(stmt, i);
            if (columnValue == nullptr)
            {
                row.emplace_back(std::nullopt);
                continue;
            }

            std::string value = std::string{reinterpret_cast<const char*>(columnValue)}; // NOLINT
            row.emplace_back(value);
        }

        rows.emplace_back(row);
    }

    sqlite3_finalize(stmt);

    return rows;
}

Rows SqliteConnection::select(const std::string& table, const std::string& col, const KeyValues& filters)
{
    Rows rows; // will represent an array of size N-by-1

    auto&& sql = SqliteTraits::SqlSelect(table, col, filters);
    std::vector<std::optional<std::string>> columnValues;

    mDatabase << sql >> [&](std::unique_ptr<std::string> value) {
        Row row{value ? std::optional<std::string>(*value) : std::nullopt};
        rows.emplace_back(row);
    };

    return rows;
}

PrimaryKey SqliteConnection::insert(const std::string& table, const KeyValues& keyValues, bool transaction)
{
    return insertRow(table, keyValues, transaction, false);
}

PrimaryKeys SqliteConnection::insert(const std::string& table, const Rows& rows, bool transaction)
{
    return insertRows(table, rows, transaction, false);
}

PrimaryKey SqliteConnection::insertOrReplace(const std::string& table, const KeyValues& keyValues, bool transaction)
{
    return insertRow(table, keyValues, transaction, true);
}

PrimaryKeys SqliteConnection::insertOrReplace(const std::string& table, const Rows& rows, bool transaction)
{
    return insertRows(table, rows, transaction, true);
}

void SqliteConnection::update(const std::string& table,
                              const KeyValues& keyValues,
                              const KeyValues& filters,
                              bool transaction)
{
    lockWriteAccess(transaction);

    auto&& sql = SqliteTraits::SqlUpdate(table, keyValues, filters);
    mDatabase << sql;

    unlockWriteAccess(transaction);
}

void SqliteConnection::deleteRows(const std::string& table, const KeyValues& filters, bool transaction)
{
    lockWriteAccess(transaction);

    auto&& sql = SqliteTraits::SqlDelete(table, filters);
    mDatabase << sql;

    unlockWriteAccess(transaction);
}

std::size_t SqliteConnection::count(const std::string& table, const KeyValues& filters)
{
    return count(table, "*", filters);
}

std::size_t SqliteConnection::count(const std::string& table, const std::string& col, const KeyValues& filters)
{
    std::size_t result{0};
    auto sql = SqliteTraits::SqlCount(table, col, filters);
    mDatabase << sql >> result;
    return result;
}

double SqliteConnection::sum(const std::string& table, const std::string& col, const KeyValues& filters)
{
    double sum{0.0};
    auto sql = SqliteTraits::SqlSum(table, col, filters);
    mDatabase << sql >> sum;
    return sum;
}

double SqliteConnection::average(const std::string& table, const std::string& col, const KeyValues& filters)
{
    double average{0.0};
    auto sql = SqliteTraits::SqlAvg(table, col, filters);
    mDatabase << sql >> average;
    return average;
}

void SqliteConnection::connectionHook()
{
    sqlite3_busy_timeout(mDatabase.connection().get(), kBusyTimeoutMs);
}

void SqliteConnection::lockWriteAccess(bool partOfTransaction)
{
    if (!mInTransaction || (mInTransaction && !partOfTransaction))
    {
        mWriteMutex.lock();
    }
}

void SqliteConnection::unlockWriteAccess(bool partOfTransaction)
{
    if (!partOfTransaction)
    {
        mWriteMutex.unlock();
    }
}

PrimaryKey
SqliteConnection::insertRow(const std::string& table, const KeyValues& keyValues, bool transaction, bool replace)
{
    lockWriteAccess(transaction);

    auto sql = SqliteTraits::SqlInsert(table, keyValues, replace);
    mDatabase << sql;
    PrimaryKey key = mDatabase.last_insert_rowid();

    unlockWriteAccess(transaction);
    return key;
}

PrimaryKeys SqliteConnection::insertRows(const std::string& table, const Rows& rows, bool transaction, bool replace)
{
    PrimaryKeys primaryKeys;

    if (rows.empty())
    {
        return primaryKeys;
    }

    lockWriteAccess(transaction);

    auto sql = SqliteTraits::SqlInsertWithPlaceholders(table, rows[0].size(), replace);
    auto pps = mDatabase << sql;

    for (const auto& row : rows)
    {
        primaryKeys.emplace_back(executePPS(pps, row));
    }

    unlockWriteAccess(transaction);
    return primaryKeys;
}

PrimaryKey SqliteConnection::executePPS(sqlite::database_binder& pps, const Row& row)
{
    PrimaryKey primaryKey;

    for (size_t i = 0; i < row.size(); ++i)
    {
        if (row[i])
        {
            pps << row[i].value();
        }
        else
        {
            pps << nullptr;
        }
    }

    pps.execute();

    primaryKey = mDatabase.last_insert_rowid();
    return primaryKey;
}

} // namespace db
} // namespace base_services
