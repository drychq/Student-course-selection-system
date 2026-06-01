#include <filesystem>
#include <print>
#include <utility>
import scs.frontend;
import scs.persistence;
import scs.service;

int main(int argc, char* argv[]) {
    const std::filesystem::path databasePath =
        argc > 1 ? std::filesystem::path{argv[1]} : std::filesystem::path{"data/student_course_selection.db"};

    auto storageResult = scs::SqliteStorage::open(databasePath);
    if (!storageResult) {
        std::println("数据库初始化失败：{}", storageResult.error().message);
        return 1;
    }
    auto storage = std::move(*storageResult);

    auto serviceResult = scs::CourseSelectionService::create(storage);
    if (!serviceResult) {
        std::println("数据库加载失败：{}", serviceResult.error().message);
        return 1;
    }
    auto service = std::move(*serviceResult);

    scs::CliApp app(service);
    app.run();
    return 0;
}
