#include "SqliteConnection.hpp"

#include "gtest/gtest.h"

#include <memory>
#include <thread>
#include <vector>

using namespace base_services;
using namespace ::testing;

const std::chrono::milliseconds kSleepBetweenIterations{20};

struct TestSqliteConcurrency : public Test
{
    static inline const std::string DBPath = "test_db.db";
    static inline const char* TestTable    = "test_table";

    void SetUp() override
    {
        sqlite::sqlite_config config;
        config.flags        = sqlite::OpenFlags::READWRITE | sqlite::OpenFlags::CREATE;
        sqlite::database db = sqlite::database(DBPath, config);

        db << "DROP TABLE IF EXISTS test_table;";
        db << "CREATE TABLE test_table (number INTEGER, string TEXT);";
    }

    void init(int connectionCount)
    {
        for (auto i = 0; i < connectionCount; ++i)
        {
            auto& sqliteConnection = mConnections.emplace_back(std::make_unique<db::SqliteConnection>(DBPath));
            sqliteConnection->open();
        }
    }

    void defaultFillTable()
    {
        auto keys = mConnections[0]->insert(TestTable,
                                            db::Rows{{"0", "zero"},
                                                     {"1", "one"},
                                                     {"2", "two"},
                                                     {"3", "three"},
                                                     {"4", "four"},
                                                     {"5", "five"},
                                                     {"6", "six"},
                                                     {"7", "seven"},
                                                     {"8", "eight"},
                                                     {"9", "nine"}},
                                            false);
        EXPECT_EQ(keys.size(), 10);
    }

    void testSelects(int threadId, int connectionId, int count, bool transaction = false)
    {
        for (auto i = 0; i < count / 3; ++i)
        {
            (void)threadId;
            // std::cout << "Thread " + std::to_string(threadId) << ": " << std::to_string(i) << std::endl;

            if (transaction)
            {
                mConnections[connectionId]->beginTransaction(true);
            }

            db::Rows rows;

            rows = mConnections[connectionId]->select(TestTable, db::KeyValues{{"number", "3"}});
            EXPECT_EQ(rows[0], (db::Row{"3", "three"}));

            rows = mConnections[connectionId]->select(TestTable, "string", db::KeyValues{{"number", "9"}});
            EXPECT_EQ(rows[0][0].value(), "nine");

            rows = mConnections[connectionId]->select(TestTable, "number", db::KeyValues{{"number", "10"}});
            EXPECT_TRUE(rows.empty());

            if (transaction)
                mConnections[connectionId]->commitTransaction();
        }
    }

    void testUpdatesAndSelects(int threadId, int connectionId, int count, bool transaction = false)
    {
        for (auto i = 0; i < count / 2; ++i)
        {
            (void)threadId;
            //            std::cout << "Thread " + std::to_string(threadId) << ": " << std::to_string(i) << std::endl;
            if (transaction)
                mConnections[connectionId]->beginTransaction(true);

            auto threadSpecific = ((threadId == 0) ? "zero" : "one");
            auto numberStr1     = std::to_string(rand());
            auto numberStr2     = std::to_string(rand());

            // Testing different column updates between threads
            mConnections[connectionId]->update(
                TestTable, {{"number", numberStr1}}, {{"string", threadSpecific}}, transaction);
            auto val1
                = mConnections[connectionId]->select(TestTable, "number", db::KeyValues{{"string", threadSpecific}});

            // Testing same column updates between threads
            mConnections[connectionId]->update(TestTable, {{"number", numberStr2}}, {{"string", "two"}}, transaction);
            auto val2 = mConnections[connectionId]->select(TestTable, "number", db::KeyValues{{"string", "two"}});
            EXPECT_FALSE(val1.empty());
            EXPECT_EQ(val1[0][0].value(), numberStr1);

            if (transaction)
                mConnections[connectionId]->commitTransaction();
        }
    }

    void testInsertOrReplaces(int threadId, int connectionId, int newEntries, bool transaction = false)
    {
        for (auto i = 0; i < newEntries; ++i)
        {
            (void)threadId;
            //            std::cout << "Thread " + std::to_string(threadId) << ": " << std::to_string(i) << std::endl;
            if (transaction)
                mConnections[connectionId]->beginTransaction(true);

            auto numberStr = std::to_string(rand());
            auto pk        = mConnections[connectionId]->insertOrReplace(
                TestTable, db::Rows{{numberStr, "randomNumber" + numberStr}}, transaction);

            auto numberStrReplace = std::to_string(stoi(numberStr) + 1);
            mConnections[connectionId]->insertOrReplace(TestTable,
                                                        db::KeyValues{{"rowid", std::to_string(pk[0])},
                                                                      {"number", numberStr},
                                                                      {"string", "randomNumber" + numberStrReplace}},
                                                        transaction);

            auto rows = mConnections[connectionId]->select(TestTable, "string", {{"number", numberStr}});
            EXPECT_FALSE(rows[0].empty());
            EXPECT_EQ("randomNumber" + numberStrReplace, rows[0][0].value());

            if (transaction)
                mConnections[connectionId]->commitTransaction();
        }
    }

    void testInserts(int threadId, int connectionId, int count, bool transaction = false)
    {
        for (auto i = 0; i < count / 2; ++i)
        {
            (void)threadId;
            //            std::cout << "Thread " + std::to_string(threadId) << ": " << std::to_string(i) << std::endl;
            if (transaction)
                mConnections[connectionId]->beginTransaction(true);

            auto numberStr = std::to_string(rand());
            mConnections[connectionId]->insert(
                TestTable, db::Rows{{numberStr, "randomNumber" + numberStr}}, transaction);

            numberStr = std::to_string(rand());
            mConnections[connectionId]->insert(
                TestTable, db::KeyValues{{"number", numberStr}, {"string", "randomNumber" + numberStr}}, transaction);

            if (transaction)
                mConnections[connectionId]->commitTransaction();
        }
    }

    void testInsertsAndSelects(int threadId, int connectionId, int count, bool transaction = false)
    {
        for (auto i = 0; i < count; ++i)
        {
            (void)threadId;
            //            std::cout << "Thread " + std::to_string(threadId) << ": " << std::to_string(i) << std::endl;
            if (transaction)
                mConnections[connectionId]->beginTransaction(true);

            auto numberStr = std::to_string(rand());
            mConnections[connectionId]->insert(
                TestTable, db::Rows{{numberStr, "randomNumber" + numberStr}}, transaction);

            auto rows = mConnections[connectionId]->select(TestTable, {{"number", numberStr}});
            EXPECT_EQ(rows[0][1].value(), "randomNumber" + rows[0][0].value());

            if (transaction)
                mConnections[connectionId]->commitTransaction();
        }
    }

    void testInsertsAndDeletes(int threadId, int connectionId, int count, bool transaction = false)
    {
        for (auto i = 0; i < count; ++i)
        {
            (void)threadId;
            //            std::cout << "Thread " + std::to_string(threadId) << ": " << std::to_string(i) << std::endl;
            if (transaction)
                mConnections[connectionId]->beginTransaction(true);

            auto numberStr = std::to_string(rand());
            mConnections[connectionId]->insert(
                TestTable, db::Rows{{numberStr, "randomNumber" + numberStr}}, transaction);
            mConnections[connectionId]->deleteRows(TestTable, db::KeyValues{{"number", numberStr}}, transaction);

            if (transaction)
                mConnections[connectionId]->commitTransaction();
        }
    };

    std::vector<std::unique_ptr<db::SqliteConnection>> mConnections;
};

TEST_F(TestSqliteConcurrency, SingleConnection_ParallelSelects_Works)
{
    init(1);
    defaultFillTable();
    std::thread t1([&] { testSelects(0, 0, 40); });
    std::thread t2([&] { testSelects(1, 0, 40); });
    t1.join();
    t2.join();
}

TEST_F(TestSqliteConcurrency, SingleConnection_ParallelUpdatesAndSelects_Works)
{
    init(1);
    defaultFillTable();
    std::thread t1([&] { testUpdatesAndSelects(0, 0, 20); });
    std::thread t2([&] { testUpdatesAndSelects(1, 0, 20); });
    t1.join();
    t2.join();
}

TEST_F(TestSqliteConcurrency, SingleConnection_ParallelInserts_Works)
{
    init(1);
    std::thread t1([&] { testInserts(0, 0, 10); });
    std::thread t2([&] { testInserts(1, 0, 10); });
    t1.join();
    t2.join();

    auto rows = mConnections[0]->select(TestTable, {});
    EXPECT_EQ(rows.size(), 20);

    for (const auto entry : rows)
    {
        EXPECT_EQ(entry[1].value(), "randomNumber" + entry[0].value());
    }
}

TEST_F(TestSqliteConcurrency, SingleConnection_ParallelInsertsAndSelects_Works)
{
    init(1);
    std::thread t1([&] { testInsertsAndSelects(0, 0, 10); });
    std::thread t2([&] { testInsertsAndSelects(1, 0, 10); });
    t1.join();
    t2.join();

    auto rows = mConnections[0]->select(TestTable, {});
    EXPECT_EQ(rows.size(), 20);
}

TEST_F(TestSqliteConcurrency, SingleConnection_ParallelDeletes_Works)
{
    init(1);
    std::thread t1([&] { testInsertsAndDeletes(0, 0, 10); });
    std::thread t2([&] { testInsertsAndDeletes(1, 0, 10); });
    t1.join();
    t2.join();

    EXPECT_EQ(mConnections[0]->count(TestTable, {}), 0);
}

TEST_F(TestSqliteConcurrency, SingleConnections_ParallelInsertsOrUpdates_Works)
{
    init(1);
    std::thread t1([&] { testInsertOrReplaces(0, 0, 10); });
    std::thread t2([&] { testInsertOrReplaces(1, 0, 10); });
    t1.join();
    t2.join();

    EXPECT_EQ(mConnections[0]->count(TestTable, {}), 20);
}

TEST_F(TestSqliteConcurrency, MultipleConnections_ParallelSelects_Works)
{
    init(2);
    defaultFillTable();
    std::thread t1([&] { testSelects(0, 0, 40); });
    std::thread t2([&] { testSelects(1, 1, 40); });
    t1.join();
    t2.join();
}

TEST_F(TestSqliteConcurrency, MultipleConnections_ParallelInserts_Works)
{
    init(2);
    std::thread t1([&] { testInserts(0, 0, 10); });
    std::thread t2([&] { testInserts(1, 1, 10); });
    t1.join();
    t2.join();

    auto rows = mConnections[0]->select(TestTable, {});
    EXPECT_EQ(rows.size(), 20);

    for (const auto entry : rows)
    {
        EXPECT_EQ(entry[1].value(), "randomNumber" + entry[0].value());
    }
}

TEST_F(TestSqliteConcurrency, MultipleConnections_ParallelInsertsAndSelects_Works)
{
    init(2);
    std::thread t1([&] { testInsertsAndSelects(0, 0, 10); });
    std::thread t2([&] { testInsertsAndSelects(1, 1, 10); });
    t1.join();
    t2.join();

    auto rows = mConnections[0]->select(TestTable, {});
    EXPECT_EQ(rows.size(), 20);
}

TEST_F(TestSqliteConcurrency, MultipleConnection_ParallelUpdatesAndSelects_Works)
{
    init(2);
    defaultFillTable();
    std::thread t1([&] { testUpdatesAndSelects(0, 0, 10); });
    std::thread t2([&] { testUpdatesAndSelects(1, 1, 10); });
    t1.join();
    t2.join();
}

TEST_F(TestSqliteConcurrency, MultipleConnections_ParallelDeletes_Works)
{
    init(2);
    std::thread t1([&] { testInsertsAndDeletes(0, 0, 10); });
    std::thread t2([&] { testInsertsAndDeletes(1, 1, 10); });
    t1.join();
    t2.join();

    EXPECT_EQ(mConnections[0]->count(TestTable, {}), 0);
}

TEST_F(TestSqliteConcurrency, MultipleConnections_ParallelInsertsOrUpdates_Works)
{
    init(2);
    std::thread t1([&] { testInsertOrReplaces(0, 0, 10); });
    std::thread t2([&] { testInsertOrReplaces(1, 1, 10); });
    t1.join();
    t2.join();

    EXPECT_EQ(mConnections[0]->count(TestTable, {}), 20);
}

TEST_F(TestSqliteConcurrency, SingleConnection_ParallelTransactions_Works)
{
    init(1);
    defaultFillTable();

    std::thread t1([&] { testInserts(0, 0, 10, true); });
    std::thread t2([&] { testSelects(1, 0, 20, true); });
    std::thread t3([&] { testUpdatesAndSelects(2, 0, 10, true); });

    // Let's mix transactions with regular individual operations
    std::thread t4([&] { testInsertsAndSelects(3, 0, 10); });
    std::thread t5([&] { testInsertsAndDeletes(4, 0, 10); });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    /* 200 + 10 from defaultFillTable() */
    EXPECT_EQ(mConnections[0]->count(TestTable, {}), 30);
}

TEST_F(TestSqliteConcurrency, MultipleConnections_ParallelTransactions_Works)
{
    init(5);
    defaultFillTable();

    std::thread t1([&] { testInserts(0, 0, 10, true); });
    std::thread t2([&] { testSelects(1, 1, 20, true); });
    std::thread t3([&] { testUpdatesAndSelects(2, 2, 10, true); });

    // Let's mix transactions with regular individual operations
    std::thread t4([&] { testInsertsAndSelects(3, 3, 10); });
    std::thread t5([&] { testInsertsAndDeletes(4, 4, 10); });

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    /* 200 + 10 from defaultFillTable() */
    EXPECT_EQ(mConnections[0]->count(TestTable, {}), 30);
}

TEST_F(TestSqliteConcurrency, MultipleConnections_StressTest_Works)
{
    init(5);
    defaultFillTable();
    std::thread t1([&] { testSelects(0, 0, 2000); });
    std::thread t2([&] { testInsertsAndDeletes(1, 1, 500); });
    std::thread t3([&] { testUpdatesAndSelects(2, 2, 500); });
    std::thread t4([&] { testInserts(3, 3, 500); });
    std::thread t5([&] { testInsertsAndSelects(4, 4, 500); });
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
}
