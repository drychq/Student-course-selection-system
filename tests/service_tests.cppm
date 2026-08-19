import std;
import scs.persistence;
import scs.service;

using scs::AppErrorCode;
using scs::CourseSelectionService;
using scs::SqliteStorage;

namespace {

[[noreturn]] void fail(std::string_view message) {
    throw std::runtime_error{std::string{message}};
}

void require(bool condition, std::string_view message) {
    if (!condition) {
        fail(message);
    }
}

void requireSuccess(const auto& result, std::string_view operation) {
    if (!result) {
        fail(std::format("{}: {} ({})", operation, result.error().message, scs::toString(result.error().code)));
    }
}

void requireErrorCode(const auto& result, AppErrorCode code) {
    require(!result, "操作应当失败。");
    require(result.error().code == code, "错误码与预期不一致。");
}

struct TempDatabase {
    std::filesystem::path path;

    explicit TempDatabase(std::string_view name)
        : path(std::filesystem::current_path() / "build" / "service-test-data" / name) {
        std::error_code errorCode;
        std::filesystem::create_directories(path.parent_path(), errorCode);
        require(!errorCode, "无法创建测试目录。");
        removeFiles();
    }

    ~TempDatabase() {
        removeFiles();
    }

    void removeFiles() const {
        std::error_code errorCode;
        std::filesystem::remove(path, errorCode);
        std::filesystem::remove(path.string() + "-journal", errorCode);
        std::filesystem::remove(path.string() + "-wal", errorCode);
        std::filesystem::remove(path.string() + "-shm", errorCode);
    }
};

[[nodiscard]] SqliteStorage openStorage(const std::filesystem::path& path) {
    auto storage = SqliteStorage::open(path);
    requireSuccess(storage, "打开数据库");
    return std::move(*storage);
}

[[nodiscard]] CourseSelectionService createService(SqliteStorage& storage) {
    auto service = CourseSelectionService::create(storage);
    requireSuccess(service, "创建服务");
    return std::move(*service);
}

void testEntityManagement() {
    TempDatabase database{"entity-management.db"};
    auto storage = openStorage(database.path);
    auto service = createService(storage);

    requireSuccess(service.addStudent("Alice", 1), "添加学生");
    requireSuccess(service.addTeacher("ProfLi"), "添加教师");
    requireSuccess(service.addCourse("Math", 101, "Algebra"), "添加课程");

    requireErrorCode(service.addStudent("Bob", 1), AppErrorCode::StudentAlreadyExists);
    requireErrorCode(service.addTeacher("ProfLi"), AppErrorCode::TeacherAlreadyExists);
    requireErrorCode(service.addCourse("Physics", 101, "Mechanics"), AppErrorCode::CourseAlreadyExists);
}

void testEnrollmentAndScores() {
    TempDatabase database{"enrollment-and-scores.db"};
    auto storage = openStorage(database.path);
    auto service = createService(storage);

    requireSuccess(service.addStudent("Alice", 1), "添加学生");
    requireSuccess(service.addTeacher("ProfLi"), "添加教师");
    requireSuccess(service.addCourse("Math", 101, "Algebra"), "添加数学课程");
    requireSuccess(service.addCourse("Physics", 102, "Mechanics"), "添加物理课程");

    requireSuccess(service.selectCourse(1, 101), "选择课程");
    requireErrorCode(service.selectCourse(1, 101), AppErrorCode::CourseAlreadySelected);
    requireErrorCode(service.selectCourse(2, 101), AppErrorCode::StudentNotFound);
    requireErrorCode(service.selectCourse(1, 999), AppErrorCode::CourseNotFound);

    requireSuccess(service.importScore("ProfLi", 101, 1, 95), "录入成绩");
    requireErrorCode(service.importScore("ProfLi", 102, 1, 88), AppErrorCode::CourseNotSelected);
    requireErrorCode(service.importScore("ProfLi", 101, 1, 101), AppErrorCode::InvalidScore);
    requireErrorCode(service.importScore("Missing", 101, 1, 90), AppErrorCode::TeacherNotFound);

    auto scores = service.studentScores(1);
    requireSuccess(scores, "查询成绩");
    require(scores->scores.size() == 1, "成绩数量错误。");
    require(scores->scores[0].score == 95, "成绩内容错误。");
}

void testCourseStatistics() {
    TempDatabase database{"course-statistics.db"};
    auto storage = openStorage(database.path);
    auto service = createService(storage);

    requireSuccess(service.addStudent("Alice", 1), "添加学生 Alice");
    requireSuccess(service.addStudent("Bob", 2), "添加学生 Bob");
    requireSuccess(service.addTeacher("ProfLi"), "添加教师");
    requireSuccess(service.addCourse("Math", 101, "Algebra"), "添加课程");
    requireSuccess(service.selectCourse(1, 101), "Alice 选课");
    requireSuccess(service.selectCourse(2, 101), "Bob 选课");
    requireSuccess(service.importScore("ProfLi", 101, 1, 80), "录入 Alice 成绩");
    requireSuccess(service.importScore("ProfLi", 101, 2, 100), "录入 Bob 成绩");

    auto stats = service.courseStatistics(101);
    requireSuccess(stats, "查询统计");
    require(stats->enrolledCount == 2, "选课人数错误。");
    require(stats->gradedCount == 2, "评分人数错误。");
    require(stats->averageScore == 90.0, "平均分错误。");
    require(stats->highestScore == 100, "最高分错误。");
    require(stats->lowestScore == 80, "最低分错误。");
}

void testRealtimePersistence() {
    TempDatabase database{"realtime-persistence.db"};
    {
        auto storage = openStorage(database.path);
        auto service = createService(storage);
        requireSuccess(service.addStudent("Alice", 1), "添加学生");
        requireSuccess(service.addTeacher("ProfLi"), "添加教师");
        requireSuccess(service.addCourse("Math", 101, "Algebra"), "添加课程");
        requireSuccess(service.selectCourse(1, 101), "选择课程");
        requireSuccess(service.importScore("ProfLi", 101, 1, 93), "录入成绩");
    }

    auto storage = openStorage(database.path);
    auto service = createService(storage);
    requireSuccess(service.findStudent(1), "恢复学生");
    requireSuccess(service.findTeacher("ProfLi"), "恢复教师");
    requireSuccess(service.findCourse(101), "恢复课程");

    auto scores = service.studentScores(1);
    requireSuccess(scores, "恢复成绩");
    require(scores->scores.size() == 1, "恢复后的成绩数量错误。");
    require(scores->scores[0].score == 93, "恢复后的成绩错误。");
}

void testForeignKeyConstraint() {
    TempDatabase database{"foreign-key.db"};
    auto storage = openStorage(database.path);
    requireErrorCode(storage.insertEnrollment(999, 999), AppErrorCode::SqlExecutionFailed);
}

void testDatabasePathError() {
    const auto blocker = std::filesystem::current_path() / "build" / "service-test-data" / "not-a-directory";
    std::error_code errorCode;
    std::filesystem::create_directories(blocker.parent_path(), errorCode);
    require(!errorCode, "无法创建路径错误测试目录。");
    std::filesystem::remove(blocker, errorCode);

    {
        std::ofstream output(blocker);
        require(static_cast<bool>(output), "无法创建路径占位文件。");
        output << "blocker";
    }

    requireErrorCode(SqliteStorage::open(blocker / "database.db"), AppErrorCode::DatabaseOpenFailed);
    std::filesystem::remove(blocker, errorCode);
}

void testWriteFailureDoesNotMutateMemory() {
    TempDatabase database{"write-failure.db"};
    auto storage = openStorage(database.path);
    auto service = createService(storage);

    requireSuccess(storage.insertStudent("External", 7), "直接写入学生");
    requireErrorCode(service.addStudent("External", 7), AppErrorCode::SqlExecutionFailed);
    requireErrorCode(service.findStudent(7), AppErrorCode::StudentNotFound);
    require(service.students().empty(), "数据库写入失败后不应修改内存状态。");
}

} // namespace

int main() {
    try {
        testEntityManagement();
        testEnrollmentAndScores();
        testCourseStatistics();
        testRealtimePersistence();
        testForeignKeyConstraint();
        testDatabasePathError();
        testWriteFailureDoesNotMutateMemory();
        std::println("所有测试通过。");
        return 0;
    } catch (const std::exception& error) {
        std::println("测试失败：{}", error.what());
        return 1;
    }
}
