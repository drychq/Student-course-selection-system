module;

#include <sqlite3.h>

module scs.persistence;

import std;
import scs.domain;

namespace scs {

namespace {

constexpr int currentSchemaVersion = 1;

[[nodiscard]] AppError makeSqlError(sqlite3* database, std::string_view operation) {
    return AppError{
        .code = AppErrorCode::SqlExecutionFailed,
        .message = std::string{operation} + "失败: " + sqlite3_errmsg(database)};
}

[[nodiscard]] AppError makeInvalidDataError(std::string message) {
    return AppError{.code = AppErrorCode::InvalidData, .message = std::move(message)};
}

[[nodiscard]] std::expected<void, AppError> execute(sqlite3* database, std::string_view sql) {
    char* errorMessage = nullptr;
    const int result = sqlite3_exec(database, sql.data(), nullptr, nullptr, &errorMessage);
    if (result == SQLITE_OK) {
        return {};
    }

    std::string message = errorMessage == nullptr ? sqlite3_errmsg(database) : errorMessage;
    sqlite3_free(errorMessage);
    return std::unexpected(AppError{
        .code = AppErrorCode::SqlExecutionFailed,
        .message = "执行 SQL 失败: " + message});
}

class Statement {
public:
    [[nodiscard]] static std::expected<Statement, AppError> prepare(sqlite3* database, std::string_view sql) {
        sqlite3_stmt* statement = nullptr;
        const int result = sqlite3_prepare_v2(
            database,
            sql.data(),
            static_cast<int>(sql.size()),
            &statement,
            nullptr);
        if (result != SQLITE_OK) {
            return std::unexpected(makeSqlError(database, "准备 SQL"));
        }
        return Statement{database, statement};
    }

    ~Statement() {
        sqlite3_finalize(m_statement);
    }

    Statement(Statement&& other) noexcept
        : m_database(std::exchange(other.m_database, nullptr)),
          m_statement(std::exchange(other.m_statement, nullptr)) {
    }

    Statement& operator=(Statement&& other) noexcept {
        if (this != &other) {
            sqlite3_finalize(m_statement);
            m_database = std::exchange(other.m_database, nullptr);
            m_statement = std::exchange(other.m_statement, nullptr);
        }
        return *this;
    }

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    [[nodiscard]] sqlite3_stmt* get() const noexcept {
        return m_statement;
    }

    [[nodiscard]] sqlite3* database() const noexcept {
        return m_database;
    }

private:
    sqlite3* m_database;
    sqlite3_stmt* m_statement;

    Statement(sqlite3* database, sqlite3_stmt* statement) noexcept
        : m_database(database), m_statement(statement) {
    }
};

class Transaction {
public:
    [[nodiscard]] static std::expected<Transaction, AppError> begin(sqlite3* database) {
        if (auto result = execute(database, "BEGIN IMMEDIATE"); !result) {
            return std::unexpected(result.error());
        }
        return Transaction{database};
    }

    ~Transaction() {
        if (!m_finished) {
            static_cast<void>(execute(m_database, "ROLLBACK"));
        }
    }

    Transaction(Transaction&& other) noexcept
        : m_database(std::exchange(other.m_database, nullptr)),
          m_finished(std::exchange(other.m_finished, true)) {
    }

    Transaction& operator=(Transaction&&) = delete;
    Transaction(const Transaction&) = delete;
    Transaction& operator=(const Transaction&) = delete;

    [[nodiscard]] std::expected<void, AppError> commit() {
        auto result = execute(m_database, "COMMIT");
        if (result) {
            m_finished = true;
        }
        return result;
    }

private:
    sqlite3* m_database;
    bool m_finished = false;

    explicit Transaction(sqlite3* database) noexcept
        : m_database(database) {
    }
};

[[nodiscard]] std::expected<void, AppError> bindText(Statement& statement, int index, std::string_view value) {
    const int result = sqlite3_bind_text(
        statement.get(),
        index,
        value.data(),
        static_cast<int>(value.size()),
        SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
        return std::unexpected(makeSqlError(statement.database(), "绑定文本参数"));
    }
    return {};
}

[[nodiscard]] std::expected<void, AppError> bindInt(Statement& statement, int index, int value) {
    if (sqlite3_bind_int(statement.get(), index, value) != SQLITE_OK) {
        return std::unexpected(makeSqlError(statement.database(), "绑定整数参数"));
    }
    return {};
}

[[nodiscard]] std::expected<void, AppError> executeStatement(Statement& statement) {
    if (sqlite3_step(statement.get()) != SQLITE_DONE) {
        return std::unexpected(makeSqlError(statement.database(), "执行 SQL"));
    }
    return {};
}

[[nodiscard]] std::expected<int, AppError> readSchemaVersion(sqlite3* database) {
    auto statement = Statement::prepare(database, "PRAGMA user_version");
    if (!statement) {
        return std::unexpected(statement.error());
    }
    if (sqlite3_step(statement->get()) != SQLITE_ROW) {
        return std::unexpected(makeSqlError(database, "读取数据库版本"));
    }
    return sqlite3_column_int(statement->get(), 0);
}

[[nodiscard]] std::string columnText(sqlite3_stmt* statement, int index) {
    const auto* text = sqlite3_column_text(statement, index);
    if (text == nullptr) {
        return {};
    }
    return reinterpret_cast<const char*>(text);
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

} // namespace

struct SqliteStorage::Impl {
    sqlite3* database;
    std::filesystem::path path;

    ~Impl() {
        sqlite3_close(database);
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

    sqlite3* rawDatabase = nullptr;
    const auto utf8Path = databasePath.u8string();
    const int openResult = sqlite3_open_v2(
        reinterpret_cast<const char*>(utf8Path.c_str()),
        &rawDatabase,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
        nullptr);
    if (openResult != SQLITE_OK) {
        const std::string message = rawDatabase == nullptr ? "未知错误" : sqlite3_errmsg(rawDatabase);
        sqlite3_close(rawDatabase);
        return std::unexpected(AppError{
            .code = AppErrorCode::DatabaseOpenFailed,
            .message = "无法打开数据库: " + message});
    }

    auto impl = std::make_unique<Impl>(rawDatabase, databasePath);
    if (sqlite3_busy_timeout(rawDatabase, 5'000) != SQLITE_OK) {
        return std::unexpected(makeSqlError(rawDatabase, "配置数据库超时"));
    }
    if (auto foreignKeys = execute(rawDatabase, "PRAGMA foreign_keys = ON"); !foreignKeys) {
        return std::unexpected(foreignKeys.error());
    }

    auto schemaVersion = readSchemaVersion(rawDatabase);
    if (!schemaVersion) {
        return std::unexpected(schemaVersion.error());
    }

    if (*schemaVersion == 0) {
        constexpr std::string_view schemaSql = R"sql(
            CREATE TABLE IF NOT EXISTS students (
                id INTEGER PRIMARY KEY CHECK (id >= 0),
                name TEXT NOT NULL CHECK (length(trim(name)) > 0)
            );
            CREATE TABLE IF NOT EXISTS teachers (
                name TEXT PRIMARY KEY CHECK (length(trim(name)) > 0)
            );
            CREATE TABLE IF NOT EXISTS courses (
                id INTEGER PRIMARY KEY CHECK (id >= 0),
                name TEXT NOT NULL CHECK (length(trim(name)) > 0),
                description TEXT NOT NULL
            );
            CREATE TABLE IF NOT EXISTS enrollments (
                student_id INTEGER NOT NULL REFERENCES students(id) ON DELETE CASCADE,
                course_id INTEGER NOT NULL REFERENCES courses(id) ON DELETE CASCADE,
                score INTEGER CHECK (score BETWEEN 0 AND 100),
                PRIMARY KEY (student_id, course_id)
            );
            PRAGMA user_version = 1;
        )sql";
        auto transaction = Transaction::begin(rawDatabase);
        if (!transaction) {
            return std::unexpected(transaction.error());
        }
        if (auto schema = execute(rawDatabase, schemaSql); !schema) {
            return std::unexpected(schema.error());
        }
        if (auto committed = transaction->commit(); !committed) {
            return std::unexpected(committed.error());
        }
    } else if (*schemaVersion != currentSchemaVersion) {
        return std::unexpected(AppError{
            .code = AppErrorCode::UnsupportedSchemaVersion,
            .message = "不支持的数据库 schema 版本: " + std::to_string(*schemaVersion)});
    }

    return SqliteStorage{std::move(impl)};
}

const std::filesystem::path& SqliteStorage::databasePath() const noexcept {
    return m_impl->path;
}

std::expected<StorageSnapshot, AppError> SqliteStorage::load() const {
    StorageSnapshot snapshot;

    auto studentsStatement = Statement::prepare(m_impl->database, "SELECT id, name FROM students ORDER BY id");
    if (!studentsStatement) {
        return std::unexpected(studentsStatement.error());
    }
    for (int result = sqlite3_step(studentsStatement->get()); result != SQLITE_DONE; result = sqlite3_step(studentsStatement->get())) {
        if (result != SQLITE_ROW) {
            return std::unexpected(makeSqlError(m_impl->database, "读取学生"));
        }
        snapshot.students.emplace_back(columnText(studentsStatement->get(), 1), sqlite3_column_int(studentsStatement->get(), 0));
    }

    auto teachersStatement = Statement::prepare(m_impl->database, "SELECT name FROM teachers ORDER BY name");
    if (!teachersStatement) {
        return std::unexpected(teachersStatement.error());
    }
    for (int result = sqlite3_step(teachersStatement->get()); result != SQLITE_DONE; result = sqlite3_step(teachersStatement->get())) {
        if (result != SQLITE_ROW) {
            return std::unexpected(makeSqlError(m_impl->database, "读取教师"));
        }
        snapshot.teachers.emplace_back(columnText(teachersStatement->get(), 0));
    }

    auto coursesStatement = Statement::prepare(m_impl->database, "SELECT id, name, description FROM courses ORDER BY id");
    if (!coursesStatement) {
        return std::unexpected(coursesStatement.error());
    }
    for (int result = sqlite3_step(coursesStatement->get()); result != SQLITE_DONE; result = sqlite3_step(coursesStatement->get())) {
        if (result != SQLITE_ROW) {
            return std::unexpected(makeSqlError(m_impl->database, "读取课程"));
        }
        snapshot.courses.emplace_back(
            columnText(coursesStatement->get(), 1),
            sqlite3_column_int(coursesStatement->get(), 0),
            columnText(coursesStatement->get(), 2));
    }

    auto enrollmentsStatement = Statement::prepare(
        m_impl->database,
        "SELECT student_id, course_id, score FROM enrollments ORDER BY student_id, course_id");
    if (!enrollmentsStatement) {
        return std::unexpected(enrollmentsStatement.error());
    }
    for (int result = sqlite3_step(enrollmentsStatement->get()); result != SQLITE_DONE; result = sqlite3_step(enrollmentsStatement->get())) {
        if (result != SQLITE_ROW) {
            return std::unexpected(makeSqlError(m_impl->database, "读取选课记录"));
        }

        const int studentId = sqlite3_column_int(enrollmentsStatement->get(), 0);
        const int courseId = sqlite3_column_int(enrollmentsStatement->get(), 1);
        auto student = findStudent(snapshot.students, studentId);
        auto course = findCourse(snapshot.courses, courseId);
        if (student == snapshot.students.end() || course == snapshot.courses.end()) {
            return std::unexpected(makeInvalidDataError("数据库中存在无效的选课关联。"));
        }
        if (!student->enroll(courseId) || !course->enrollStudent(studentId)) {
            return std::unexpected(makeInvalidDataError("数据库中存在重复的选课关联。"));
        }
        if (sqlite3_column_type(enrollmentsStatement->get(), 2) != SQLITE_NULL
            && !course->setScore(studentId, sqlite3_column_int(enrollmentsStatement->get(), 2))) {
            return std::unexpected(makeInvalidDataError("数据库中存在无效的成绩关联。"));
        }
    }

    return snapshot;
}

std::expected<void, AppError> SqliteStorage::insertStudent(std::string_view name, int id) const {
    auto statement = Statement::prepare(m_impl->database, "INSERT INTO students (id, name) VALUES (?, ?)");
    if (!statement) {
        return std::unexpected(statement.error());
    }
    if (auto result = bindInt(*statement, 1, id); !result) {
        return result;
    }
    if (auto result = bindText(*statement, 2, name); !result) {
        return result;
    }
    return executeStatement(*statement);
}

std::expected<void, AppError> SqliteStorage::insertTeacher(std::string_view name) const {
    auto statement = Statement::prepare(m_impl->database, "INSERT INTO teachers (name) VALUES (?)");
    if (!statement) {
        return std::unexpected(statement.error());
    }
    if (auto result = bindText(*statement, 1, name); !result) {
        return result;
    }
    return executeStatement(*statement);
}

std::expected<void, AppError> SqliteStorage::insertCourse(
    std::string_view name,
    int id,
    std::string_view description) const {
    auto statement = Statement::prepare(
        m_impl->database,
        "INSERT INTO courses (id, name, description) VALUES (?, ?, ?)");
    if (!statement) {
        return std::unexpected(statement.error());
    }
    if (auto result = bindInt(*statement, 1, id); !result) {
        return result;
    }
    if (auto result = bindText(*statement, 2, name); !result) {
        return result;
    }
    if (auto result = bindText(*statement, 3, description); !result) {
        return result;
    }
    return executeStatement(*statement);
}

std::expected<void, AppError> SqliteStorage::insertEnrollment(int studentId, int courseId) const {
    auto statement = Statement::prepare(
        m_impl->database,
        "INSERT INTO enrollments (student_id, course_id) VALUES (?, ?)");
    if (!statement) {
        return std::unexpected(statement.error());
    }
    if (auto result = bindInt(*statement, 1, studentId); !result) {
        return result;
    }
    if (auto result = bindInt(*statement, 2, courseId); !result) {
        return result;
    }
    return executeStatement(*statement);
}

std::expected<void, AppError> SqliteStorage::updateScore(int courseId, int studentId, int score) const {
    auto statement = Statement::prepare(
        m_impl->database,
        "UPDATE enrollments SET score = ? WHERE student_id = ? AND course_id = ?");
    if (!statement) {
        return std::unexpected(statement.error());
    }
    if (auto result = bindInt(*statement, 1, score); !result) {
        return result;
    }
    if (auto result = bindInt(*statement, 2, studentId); !result) {
        return result;
    }
    if (auto result = bindInt(*statement, 3, courseId); !result) {
        return result;
    }
    if (auto result = executeStatement(*statement); !result) {
        return result;
    }
    if (sqlite3_changes(m_impl->database) != 1) {
        return std::unexpected(makeInvalidDataError("数据库中不存在要更新的选课记录。"));
    }
    return {};
}

} // namespace scs
