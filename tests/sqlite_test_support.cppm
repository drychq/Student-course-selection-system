export module scs.test.sqlite_support;

import std;

export namespace scs::test {

[[nodiscard]] bool setSchemaVersion(const std::filesystem::path& databasePath, int version);

} // namespace scs::test
