# SQLite C++ Wrapper Library

## Overview

This repository includes a C++ SQLite library, built on top of the open-source project:
https://github.com/SqliteModernCpp/sqlite_modern_cpp

This project is added as a git submodule, in directory external/.
It also requires library sqlite3.

The main goal of this repo is to provide a SqliteConnection class, which represents a single SQLite connection.

This implementation allows read and write access to a DB file, concurrently.
An instance of this class instantiates a single DB connection. It can be used concurrently from multiple threads for both reads, writes and transactions.
Multiple instances of this class also allow concurrent read  and write access.

Write access is allowed using two mechanisms:
- Transactions and individual writes in the same connection from multiple threads are possible by protecting any writes (individual operations and full-transaction) with the same mutex.
- Between connections where mutex instance are not shared, the concurrency handling is achieved using sqlite3_busy_timeout().

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
