module;

#include <SQLiteCpp/SQLiteCpp.h>
#include <sqlite3.h>

module scs.persistence;

import std;
import scs.domain;

namespace scs {

namespace {

constexpr int busyTimeoutMilliseconds = 5'000;
constexpr std::int64_t maximumDomainId = std::numeric_limits<int>::max();

class StorageFailure final : public std::runtime_error {
public:
    StorageFailure(AppErrorCode code, std::string message)
        : std::runtime_error{std::move(message)}, m_code{code} {
    }

    [[nodiscard]] AppErrorCode code() const noexcept {
        return m_code;
    }

private:
    AppErrorCode m_code;
};

class SchemaResetRequired final : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

[[noreturn]] void fail(AppErrorCode code, std::string message) {
    throw StorageFailure{code, std::move(message)};
}

[[noreturn]] void invalidData(std::string message) {
    fail(AppErrorCode::InvalidData, std::move(message));
}

[[noreturn]] void requireSchemaReset(std::string message) {
    throw SchemaResetRequired{std::move(message)};
}

[[nodiscard]] AppError makeStorageError(const StorageFailure& error) {
    return AppError{.code = error.code(), .message = error.what()};
}

[[nodiscard]] int primaryErrorCode(const SQLite::Exception& error) noexcept {
    const int code = error.getErrorCode();
    return code < 0 ? code : code & 0xff;
}

[[nodiscard]] std::string_view errorCategory(const SQLite::Exception& error) noexcept {
    switch (primaryErrorCode(error)) {
    case SQLITE_CONSTRAINT:
        return "constraint";
    case SQLITE_BUSY:
    case SQLITE_LOCKED:
        return "busy/locked";
    case SQLITE_IOERR:
    case SQLITE_FULL:
    case SQLITE_CANTOPEN:
    case SQLITE_READONLY:
        return "I/O";
    case SQLITE_CORRUPT:
    case SQLITE_NOTADB:
        return "corrupt/not-a-database";
    default:
        return "other";
    }
}

[[nodiscard]] AppError makeSqlError(
    AppErrorCode code,
    std::string_view operation,
    const SQLite::Exception& error) {
    return AppError{
        .code = code,
        .message = std::format(
            "{}失败 [category={}, primary={}, extended={}]: {}",
            operation,
            errorCategory(error),
            primaryErrorCode(error),
            error.getExtendedErrorCode(),
            error.what())};
}

[[nodiscard]] std::string lowercase(std::string value) {
    std::ranges::transform(value, value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

[[nodiscard]] int readIntegerPragma(SQLite::Database& database, const char* sql) {
    const std::int64_t value = database.execAndGet(sql).getInt64();
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) {
        invalidData("数据库 PRAGMA 返回了超出范围的整数。");
    }
    return static_cast<int>(value);
}

[[nodiscard]] std::string readTextPragma(SQLite::Database& database, const char* sql) {
    return database.execAndGet(sql).getString();
}

void configureConnection(SQLite::Database& database) {
    if (readIntegerPragma(database, "PRAGMA busy_timeout") != busyTimeoutMilliseconds) {
        fail(AppErrorCode::DatabaseOpenFailed, "无法确认数据库 busy timeout 配置。");
    }

    const std::string journalMode = lowercase(readTextPragma(database, "PRAGMA journal_mode = DELETE"));
    if (journalMode != "delete") {
        fail(
            AppErrorCode::DatabaseOpenFailed,
            "无法启用 DELETE journal mode，数据库返回: " + journalMode);
    }

    database.exec("PRAGMA foreign_keys = ON");
    if (readIntegerPragma(database, "PRAGMA foreign_keys") != 1) {
        fail(AppErrorCode::DatabaseOpenFailed, "无法启用 SQLite 外键约束。");
    }

    const std::string lockingMode = lowercase(readTextPragma(database, "PRAGMA locking_mode = EXCLUSIVE"));
    if (lockingMode != "exclusive") {
        fail(
            AppErrorCode::DatabaseOpenFailed,
            "无法启用 SQLite 独占连接模式，数据库返回: " + lockingMode);
    }

    SQLite::Transaction exclusiveLock{database, SQLite::TransactionBehavior::EXCLUSIVE};
    exclusiveLock.commit();
}

[[nodiscard]] bool hasApplicationObjects(SQLite::Database& database) {
    SQLite::Statement query{
        database,
        R"sql(
            SELECT 1
            FROM sqlite_schema
            WHERE name IN ('students', 'teachers', 'courses', 'enrollments', 'idx_enrollments_course_id')
            LIMIT 1
        )sql"};
    return query.executeStep();
}

struct ColumnSpec {
    std::string_view name;
    std::string_view type;
    int notNull;
    int primaryKeyPosition;
};

constexpr std::array studentColumns{
    ColumnSpec{"id", "INTEGER", 1, 1},
    ColumnSpec{"name", "TEXT", 1, 0},
};
constexpr std::array teacherColumns{
    ColumnSpec{"name", "TEXT", 1, 1},
};
constexpr std::array courseColumns{
    ColumnSpec{"id", "INTEGER", 1, 1},
    ColumnSpec{"name", "TEXT", 1, 0},
    ColumnSpec{"description", "TEXT", 1, 0},
};
constexpr std::array enrollmentColumns{
    ColumnSpec{"student_id", "INTEGER", 1, 1},
    ColumnSpec{"course_id", "INTEGER", 1, 2},
    ColumnSpec{"score", "INTEGER", 0, 0},
};

void validateTable(
    SQLite::Database& database,
    std::string_view tableName,
    std::span<const ColumnSpec> expectedColumns,
    bool expectedStrict) {
    const std::string table{tableName};
    SQLite::Statement columns{database, "PRAGMA table_info(" + table + ")"};

    std::size_t index = 0;
    while (columns.executeStep()) {
        if (index >= expectedColumns.size()) {
            requireSchemaReset("表 " + table + " 包含非预期列。");
        }

        const auto& expected = expectedColumns[index];
        const std::string actualName = columns.getColumn(1).getString();
        const std::string actualType = columns.getColumn(2).getString();
        const int actualNotNull = columns.getColumn(3).getInt();
        const int actualPrimaryKeyPosition = columns.getColumn(5).getInt();
        if (actualName != expected.name || actualType != expected.type
            || actualNotNull != expected.notNull
            || actualPrimaryKeyPosition != expected.primaryKeyPosition) {
            requireSchemaReset(std::format("表 {} 的列 {} 定义不符合预期。", table, actualName));
        }
        ++index;
    }

    if (index != expectedColumns.size()) {
        requireSchemaReset("表 " + table + " 缺少必要列。");
    }

    SQLite::Statement tableMetadata{
        database,
        "SELECT strict FROM pragma_table_list "
        "WHERE schema = 'main' AND name = ? AND type = 'table'"};
    tableMetadata.bind(1, table);
    if (!tableMetadata.executeStep()) {
        requireSchemaReset("数据库缺少必要表: " + table);
    }
    const bool actualStrict = tableMetadata.getColumn(0).getInt() != 0;
    if (actualStrict != expectedStrict) {
        requireSchemaReset("表 " + table + " 的 STRICT 属性不符合预期。");
    }
    if (tableMetadata.executeStep()) {
        requireSchemaReset("数据库中存在重复的表元数据: " + table);
    }
}

void validateForeignKeys(SQLite::Database& database) {
    using ForeignKey = std::tuple<std::string, std::string, std::string, std::string, std::string>;
    std::vector<ForeignKey> actual;
    SQLite::Statement foreignKeys{database, "PRAGMA foreign_key_list(enrollments)"};
    while (foreignKeys.executeStep()) {
        actual.emplace_back(
            foreignKeys.getColumn(3).getString(),
            foreignKeys.getColumn(2).getString(),
            foreignKeys.getColumn(4).getString(),
            foreignKeys.getColumn(5).getString(),
            foreignKeys.getColumn(6).getString());
    }
    std::ranges::sort(actual);

    std::vector<ForeignKey> expected{
        ForeignKey{"course_id", "courses", "id", "NO ACTION", "CASCADE"},
        ForeignKey{"student_id", "students", "id", "NO ACTION", "CASCADE"},
    };
    std::ranges::sort(expected);
    if (actual != expected) {
        requireSchemaReset("enrollments 表的外键定义不符合预期。");
    }
}

void validateManagedObjects(SQLite::Database& database) {
    using ManagedObject = std::pair<std::string, std::string>;
    std::vector<ManagedObject> objects;
    SQLite::Statement query{
        database,
        R"sql(
            SELECT type, name
            FROM sqlite_schema
            WHERE tbl_name IN ('students', 'teachers', 'courses', 'enrollments')
              AND type IN ('index', 'trigger')
              AND sql IS NOT NULL
            ORDER BY type, name
        )sql"};
    while (query.executeStep()) {
        objects.emplace_back(query.getColumn(0).getString(), query.getColumn(1).getString());
    }

    const std::vector<ManagedObject> expected{{"index", "idx_enrollments_course_id"}};
    if (objects != expected) {
        requireSchemaReset("应用表包含非预期的索引或触发器。");
    }
}

void validateCourseIndex(SQLite::Database& database) {
    bool found = false;
    SQLite::Statement indexes{database, "PRAGMA index_list(enrollments)"};
    while (indexes.executeStep()) {
        if (indexes.getColumn(1).getString() != "idx_enrollments_course_id") {
            continue;
        }
        found = true;
        if (indexes.getColumn(2).getInt() != 0 || indexes.getColumn(4).getInt() != 0) {
            requireSchemaReset("idx_enrollments_course_id 索引属性不符合预期。");
        }
    }
    if (!found) {
        requireSchemaReset("数据库缺少 idx_enrollments_course_id 索引。");
    }

    SQLite::Statement columns{database, "PRAGMA index_info(idx_enrollments_course_id)"};
    if (!columns.executeStep() || columns.getColumn(2).getString() != "course_id") {
        requireSchemaReset("idx_enrollments_course_id 索引列不符合预期。");
    }
    if (columns.executeStep()) {
        requireSchemaReset("idx_enrollments_course_id 包含非预期列。");
    }
}

[[nodiscard]] std::string normalizeSql(std::string sql) {
    std::string normalized;
    normalized.reserve(sql.size());
    for (const unsigned char character : sql) {
        if (!std::isspace(character)) {
            normalized.push_back(static_cast<char>(std::tolower(character)));
        }
    }
    return normalized;
}

void requireSchemaFragments(
    SQLite::Database& database,
    std::string_view tableName,
    std::initializer_list<std::string_view> fragments) {
    SQLite::Statement query{
        database,
        "SELECT sql FROM sqlite_schema WHERE type = 'table' AND name = ?"};
    const std::string table{tableName};
    query.bind(1, table);
    if (!query.executeStep() || query.getColumn(0).isNull()) {
        requireSchemaReset("无法读取表定义: " + table);
    }

    const std::string normalized = normalizeSql(query.getColumn(0).getString());
    for (const std::string_view fragment : fragments) {
        if (!normalized.contains(fragment)) {
            requireSchemaReset("表 " + table + " 缺少必要约束: " + std::string{fragment});
        }
    }
}

void requireNoRows(SQLite::Database& database, const char* sql, std::string_view message) {
    SQLite::Statement query{database, sql};
    if (query.executeStep()) {
        requireSchemaReset(std::string{message});
    }
}

void validateStoredData(SQLite::Database& database) {
    requireNoRows(
        database,
        "SELECT 1 FROM students "
        "WHERE typeof(id) <> 'integer' OR id < 0 OR id > 2147483647 "
        "   OR typeof(name) <> 'text' OR length(trim(name)) = 0 LIMIT 1",
        "students 表包含无效数据。");
    requireNoRows(
        database,
        "SELECT 1 FROM teachers "
        "WHERE typeof(name) <> 'text' OR length(trim(name)) = 0 LIMIT 1",
        "teachers 表包含无效数据。");
    requireNoRows(
        database,
        "SELECT 1 FROM courses "
        "WHERE typeof(id) <> 'integer' OR id < 0 OR id > 2147483647 "
        "   OR typeof(name) <> 'text' OR length(trim(name)) = 0 "
        "   OR typeof(description) <> 'text' LIMIT 1",
        "courses 表包含无效数据。");
    requireNoRows(
        database,
        "SELECT 1 FROM enrollments "
        "WHERE typeof(student_id) <> 'integer' OR student_id < 0 OR student_id > 2147483647 "
        "   OR typeof(course_id) <> 'integer' OR course_id < 0 OR course_id > 2147483647 "
        "   OR (score IS NOT NULL AND (typeof(score) <> 'integer' OR score < 0 OR score > 100)) "
        "LIMIT 1",
        "enrollments 表包含无效数据。");
}

void validatePhysicalIntegrity(SQLite::Database& database) {
    SQLite::Statement quickCheck{database, "PRAGMA quick_check"};
    bool receivedResult = false;
    while (quickCheck.executeStep()) {
        receivedResult = true;
        if (quickCheck.getColumn(0).getString() != "ok") {
            invalidData("SQLite quick_check 失败: " + quickCheck.getColumn(0).getString());
        }
    }
    if (!receivedResult) {
        invalidData("SQLite quick_check 未返回结果。");
    }
}

void validateIntegrity(SQLite::Database& database) {
    validatePhysicalIntegrity(database);
    SQLite::Statement foreignKeyCheck{database, "PRAGMA foreign_key_check"};
    if (foreignKeyCheck.executeStep()) {
        requireSchemaReset("SQLite foreign_key_check 发现无效外键关联。");
    }
}

void validateSchema(SQLite::Database& database) {
    validateTable(database, "students", studentColumns, true);
    validateTable(database, "teachers", teacherColumns, true);
    validateTable(database, "courses", courseColumns, true);
    validateTable(database, "enrollments", enrollmentColumns, true);
    validateForeignKeys(database);
    validateManagedObjects(database);
    validateCourseIndex(database);
    requireSchemaFragments(
        database,
        "students",
        {"check(idbetween0and2147483647)", "check(length(trim(name))>0)"});
    requireSchemaFragments(
        database,
        "teachers",
        {"check(length(trim(name))>0)"});
    requireSchemaFragments(
        database,
        "courses",
        {"check(idbetween0and2147483647)", "check(length(trim(name))>0)"});
    requireSchemaFragments(
        database,
        "enrollments",
        {
            "check(student_idbetween0and2147483647)",
            "check(course_idbetween0and2147483647)",
            "check(scorebetween0and100)",
        });
    validateStoredData(database);
    validateIntegrity(database);
    if (readIntegerPragma(database, "PRAGMA foreign_keys") != 1) {
        invalidData("数据库外键约束未启用。");
    }
}

constexpr std::string_view schemaSql = R"sql(
    CREATE TABLE students (
        id INTEGER PRIMARY KEY NOT NULL CHECK (id BETWEEN 0 AND 2147483647),
        name TEXT NOT NULL CHECK (length(trim(name)) > 0)
    ) STRICT;
    CREATE TABLE teachers (
        name TEXT PRIMARY KEY NOT NULL CHECK (length(trim(name)) > 0)
    ) STRICT;
    CREATE TABLE courses (
        id INTEGER PRIMARY KEY NOT NULL CHECK (id BETWEEN 0 AND 2147483647),
        name TEXT NOT NULL CHECK (length(trim(name)) > 0),
        description TEXT NOT NULL
    ) STRICT;
    CREATE TABLE enrollments (
        student_id INTEGER NOT NULL CHECK (student_id BETWEEN 0 AND 2147483647)
            REFERENCES students(id) ON DELETE CASCADE,
        course_id INTEGER NOT NULL CHECK (course_id BETWEEN 0 AND 2147483647)
            REFERENCES courses(id) ON DELETE CASCADE,
        score INTEGER CHECK (score BETWEEN 0 AND 100),
        PRIMARY KEY (student_id, course_id)
    ) STRICT;
    CREATE INDEX idx_enrollments_course_id ON enrollments(course_id);
)sql";

void createSchema(SQLite::Database& database) {
    SQLite::Transaction transaction{database, SQLite::TransactionBehavior::EXCLUSIVE};
    database.exec(std::string{schemaSql});
    validateSchema(database);
    transaction.commit();
}

constexpr std::string_view resetSchemaSql = R"sql(
    DROP TABLE IF EXISTS enrollments;
    DROP TABLE IF EXISTS courses;
    DROP TABLE IF EXISTS students;
    DROP TABLE IF EXISTS teachers;
)sql";

void resetSchema(SQLite::Database& database) {
    SQLite::Transaction transaction{database, SQLite::TransactionBehavior::EXCLUSIVE};
    database.exec(std::string{resetSchemaSql});
    database.exec(std::string{schemaSql});
    validateSchema(database);
    transaction.commit();
}

void ensureCurrentSchema(SQLite::Database& database) {
    if (!hasApplicationObjects(database)) {
        createSchema(database);
        return;
    }

    try {
        validateSchema(database);
    } catch (const SchemaResetRequired&) {
        resetSchema(database);
    }
}

[[nodiscard]] auto findStudent(std::vector<Student>& students, int id) {
    return std::ranges::find_if(students, [id](const Student& student) {
        return student.id() == id;
    });
}

[[nodiscard]] auto findCourse(std::vector<Course>& courses, int id) {
    return std::ranges::find_if(courses, [id](const Course& course) {
        return course.id() == id;
    });
}

[[nodiscard]] int readDomainId(const SQLite::Column& column, std::string_view fieldName) {
    if (!column.isInteger()) {
        invalidData(std::string{fieldName} + " 不是整数。");
    }
    const std::int64_t value = column.getInt64();
    if (value < 0 || value > maximumDomainId) {
        invalidData(std::string{fieldName} + " 超出 C++ 领域模型范围。");
    }
    return static_cast<int>(value);
}

[[nodiscard]] std::string readText(const SQLite::Column& column, std::string_view fieldName) {
    if (!column.isText()) {
        invalidData(std::string{fieldName} + " 不是文本。");
    }
    return column.getString();
}

[[nodiscard]] std::optional<int> readScore(const SQLite::Column& column) {
    if (column.isNull()) {
        return std::nullopt;
    }
    if (!column.isInteger()) {
        invalidData("数据库中的成绩不是整数。");
    }

    const std::int64_t score = column.getInt64();
    if (score < 0 || score > 100) {
        invalidData("数据库中的成绩超出有效范围。");
    }
    return static_cast<int>(score);
}

void loadEnrollment(StorageSnapshot& snapshot, SQLite::Statement& row) {
    const int studentId = readDomainId(row.getColumn(0), "选课学生 ID");
    const int courseId = readDomainId(row.getColumn(1), "选课课程 ID");
    auto student = findStudent(snapshot.students, studentId);
    auto course = findCourse(snapshot.courses, courseId);
    if (student == snapshot.students.end() || course == snapshot.courses.end()) {
        invalidData("数据库中存在无效的选课关联。");
    }
    if (!student->enroll(courseId) || !course->enrollStudent(studentId)) {
        invalidData("数据库中存在重复的选课关联。");
    }

    const std::optional<int> score = readScore(row.getColumn(2));
    if (score.has_value() && !course->setScore(studentId, *score)) {
        invalidData("数据库中存在无效的成绩关联。");
    }
}

template <typename Operation>
[[nodiscard]] std::expected<void, AppError> runStorageOperation(
    std::string_view operationName,
    Operation&& operation) {
    try {
        std::invoke(std::forward<Operation>(operation));
        return {};
    } catch (const StorageFailure& error) {
        return std::unexpected(makeStorageError(error));
    } catch (const SQLite::Exception& error) {
        return std::unexpected(makeSqlError(AppErrorCode::SqlExecutionFailed, operationName, error));
    }
}

} // namespace

struct SqliteStorage::Impl {
    SQLite::Database database;
    std::filesystem::path path;

    explicit Impl(const std::filesystem::path& databasePath)
        : database{
              databasePath,
              SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE,
              busyTimeoutMilliseconds},
          path{databasePath} {
    }
};

SqliteStorage::SqliteStorage(std::unique_ptr<Impl> impl) noexcept
    : m_impl(std::move(impl)) {
}

SqliteStorage::~SqliteStorage() = default;

SqliteStorage::SqliteStorage(SqliteStorage&&) noexcept = default;

SqliteStorage& SqliteStorage::operator=(SqliteStorage&&) noexcept = default;

std::expected<SqliteStorage, AppError> SqliteStorage::open(const std::filesystem::path& databasePath) {
    std::error_code errorCode;
    const auto parentPath = databasePath.parent_path();
    if (!parentPath.empty()) {
        std::filesystem::create_directories(parentPath, errorCode);
        if (errorCode) {
            return std::unexpected(AppError{
                .code = AppErrorCode::DatabaseOpenFailed,
                .message = "无法创建数据库目录: " + parentPath.string()});
        }
    }

    AppErrorCode sqliteErrorCode = AppErrorCode::DatabaseOpenFailed;
    std::string_view operation = "打开数据库";
    try {
        auto impl = std::make_unique<Impl>(databasePath);

        operation = "配置并锁定数据库";
        configureConnection(impl->database);

        sqliteErrorCode = AppErrorCode::SqlExecutionFailed;
        operation = "检查数据库物理完整性";
        validatePhysicalIntegrity(impl->database);

        operation = "建立当前数据库 schema";
        ensureCurrentSchema(impl->database);

        operation = "校验数据库 schema";
        validateSchema(impl->database);
        return SqliteStorage{std::move(impl)};
    } catch (const SchemaResetRequired& error) {
        return std::unexpected(AppError{
            .code = AppErrorCode::InvalidData,
            .message = "无法建立当前数据库 schema: " + std::string{error.what()}});
    } catch (const StorageFailure& error) {
        return std::unexpected(makeStorageError(error));
    } catch (const SQLite::Exception& error) {
        return std::unexpected(makeSqlError(sqliteErrorCode, operation, error));
    }
}

const std::filesystem::path& SqliteStorage::databasePath() const noexcept {
    return m_impl->path;
}

std::expected<StorageSnapshot, AppError> SqliteStorage::load() const {
    try {
        SQLite::Transaction transaction{
            m_impl->database,
            SQLite::TransactionBehavior::DEFERRED};
        StorageSnapshot snapshot;

        SQLite::Statement students{
            m_impl->database,
            "SELECT id, name FROM students ORDER BY id"};
        while (students.executeStep()) {
            snapshot.students.emplace_back(
                readText(students.getColumn(1), "学生姓名"),
                readDomainId(students.getColumn(0), "学生 ID"));
        }

        SQLite::Statement teachers{
            m_impl->database,
            "SELECT name FROM teachers ORDER BY name"};
        while (teachers.executeStep()) {
            snapshot.teachers.emplace_back(readText(teachers.getColumn(0), "教师姓名"));
        }

        SQLite::Statement courses{
            m_impl->database,
            "SELECT id, name, description FROM courses ORDER BY id"};
        while (courses.executeStep()) {
            snapshot.courses.emplace_back(
                readText(courses.getColumn(1), "课程名称"),
                readDomainId(courses.getColumn(0), "课程 ID"),
                readText(courses.getColumn(2), "课程描述"));
        }

        SQLite::Statement enrollments{
            m_impl->database,
            "SELECT student_id, course_id, score "
            "FROM enrollments ORDER BY student_id, course_id"};
        while (enrollments.executeStep()) {
            loadEnrollment(snapshot, enrollments);
        }

        transaction.commit();
        return snapshot;
    } catch (const StorageFailure& error) {
        return std::unexpected(makeStorageError(error));
    } catch (const SQLite::Exception& error) {
        return std::unexpected(makeSqlError(AppErrorCode::SqlExecutionFailed, "加载数据库快照", error));
    }
}

std::expected<void, AppError> SqliteStorage::insertStudent(std::string_view name, int id) const {
    return runStorageOperation("插入学生", [&] {
        SQLite::Statement statement{
            m_impl->database,
            "INSERT INTO students (id, name) VALUES (?, ?)"};
        const std::string ownedName{name};
        statement.bind(1, id);
        statement.bind(2, ownedName);
        statement.exec();
    });
}

std::expected<void, AppError> SqliteStorage::insertTeacher(std::string_view name) const {
    return runStorageOperation("插入教师", [&] {
        SQLite::Statement statement{
            m_impl->database,
            "INSERT INTO teachers (name) VALUES (?)"};
        const std::string ownedName{name};
        statement.bind(1, ownedName);
        statement.exec();
    });
}

std::expected<void, AppError> SqliteStorage::insertCourse(
    std::string_view name,
    int id,
    std::string_view description) const {
    return runStorageOperation("插入课程", [&] {
        SQLite::Statement statement{
            m_impl->database,
            "INSERT INTO courses (id, name, description) VALUES (?, ?, ?)"};
        const std::string ownedName{name};
        const std::string ownedDescription{description};
        statement.bind(1, id);
        statement.bind(2, ownedName);
        statement.bind(3, ownedDescription);
        statement.exec();
    });
}

std::expected<void, AppError> SqliteStorage::insertEnrollment(int studentId, int courseId) const {
    return runStorageOperation("插入选课记录", [&] {
        SQLite::Statement statement{
            m_impl->database,
            "INSERT INTO enrollments (student_id, course_id) VALUES (?, ?)"};
        statement.bind(1, studentId);
        statement.bind(2, courseId);
        statement.exec();
    });
}

std::expected<void, AppError> SqliteStorage::updateScore(
    int courseId,
    int studentId,
    int score) const {
    return runStorageOperation("更新成绩", [&] {
        SQLite::Statement statement{
            m_impl->database,
            "UPDATE enrollments SET score = ? WHERE student_id = ? AND course_id = ?"};
        statement.bind(1, score);
        statement.bind(2, studentId);
        statement.bind(3, courseId);
        statement.exec();
        if (statement.getChanges() != 1) {
            invalidData("数据库中不存在要更新的选课记录。");
        }
    });
}

} // namespace scs
