# SQLite C++ Wrapper Library

## Overview

C++ SQLite library that wraps the open-source project:
https://github.com/SqliteModernCpp/sqlite_modern_cpp

This implementation offers **thread-safe DB file read/write access, through class `sqlite_wrapper::Connection`**.

An instance of this class instantiates a single DB connection. It can be used concurrently from multiple threads for both reads, writes and transactions.
Multiple instances of this class are also allowed to be used concurrently.

Write access is allowed using two mechanisms:
- Transactions and individual writes in the same connection from multiple threads are possible by protecting any writes (individual operations and full-transaction) with the same mutex.
- Between connections where mutex instance are not shared, the concurrency handling is achieved using sqlite3_busy_timeout().

For a comprehensive usage of the library under multi-threading context, refer to the unit-tests.

## Using the Library

```cpp
// Setting up DB table - sqlite_modern_cpp only

const char* testTable = "test_table";
sqlite::sqlite_config config;
config.flags        = sqlite::OpenFlags::READWRITE | sqlite::OpenFlags::CREATE;

sqlite::database db = sqlite::database("test.db", config);
db << "CREATE TABLE test_table (number INTEGER, string TEXT);";

// Using SQLite C++ wrapper

// Create connection
auto connection = db::SqliteConnection("test.db");

// Insert N rows
auto keys = connection->insert(testTable, db::Rows{{"0", "zero"}, {"1", "one"}, {"2", "two"}, {"3", "three"}, {"4", "four"}}, false);

// Insert 1 row
auto key = connection->insert(testTable, db::KeyValues{{"number", 5}, {"string", "five"}}, false);

// Select example
rows = connection->select(testTable, db::KeyValues{{"number", 3}});

// Transaction example
bool foreignKeys = true;
connections->beginTransaction(foreignKeys);
connections->update(testTable, {{"number", 3}}, {{"string", "trois"}}, true);
connections->commitTransaction();
```

## Requirements

- Compiler with C++17 support (tested with g++ 9.4.0).
- Built under Linux, Ubuntu 20.04, but should work cross-platform.
- Requires sqlite3 library.

## Building Library and Running Unit-tests

Tested under Ubuntu 20.04.

Compile:

    $ ./build.sh

Clean and compile:

    $ ./build.sh --clean
