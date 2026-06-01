module;

#include <sqlite3.h>

module scs.test.sqlite_support;

import std;

namespace scs::test {

bool setSchemaVersion(const std::filesystem::path& databasePath, int version) {
    sqlite3* database = nullptr;
    const auto utf8Path = databasePath.u8string();
    if (sqlite3_open_v2(
            reinterpret_cast<const char*>(utf8Path.c_str()),
            &database,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
            nullptr)
        != SQLITE_OK) {
        sqlite3_close(database);
        return false;
    }

    const std::string sql = "PRAGMA user_version = " + std::to_string(version);
    const bool succeeded = sqlite3_exec(database, sql.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK;
    return sqlite3_close(database) == SQLITE_OK && succeeded;
}

} // namespace scs::test
