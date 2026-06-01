export module scs.persistence;

import std;
import scs.domain;

export namespace scs {

enum class AppErrorCode {
    EmptyName,
    InvalidId,
    InvalidScore,
    StudentAlreadyExists,
    TeacherAlreadyExists,
    CourseAlreadyExists,
    StudentNotFound,
    TeacherNotFound,
    CourseNotFound,
    CourseAlreadySelected,
    CourseNotSelected,
    DatabaseOpenFailed,
    SqlExecutionFailed,
    UnsupportedSchemaVersion,
    InvalidData
};

struct AppError {
    AppErrorCode code;
    std::string message;
};

[[nodiscard]] constexpr std::string_view toString(AppErrorCode code) noexcept {
    switch (code) {
    case AppErrorCode::EmptyName:
        return "EmptyName";
    case AppErrorCode::InvalidId:
        return "InvalidId";
    case AppErrorCode::InvalidScore:
        return "InvalidScore";
    case AppErrorCode::StudentAlreadyExists:
        return "StudentAlreadyExists";
    case AppErrorCode::TeacherAlreadyExists:
        return "TeacherAlreadyExists";
    case AppErrorCode::CourseAlreadyExists:
        return "CourseAlreadyExists";
    case AppErrorCode::StudentNotFound:
        return "StudentNotFound";
    case AppErrorCode::TeacherNotFound:
        return "TeacherNotFound";
    case AppErrorCode::CourseNotFound:
        return "CourseNotFound";
    case AppErrorCode::CourseAlreadySelected:
        return "CourseAlreadySelected";
    case AppErrorCode::CourseNotSelected:
        return "CourseNotSelected";
    case AppErrorCode::DatabaseOpenFailed:
        return "DatabaseOpenFailed";
    case AppErrorCode::SqlExecutionFailed:
        return "SqlExecutionFailed";
    case AppErrorCode::UnsupportedSchemaVersion:
        return "UnsupportedSchemaVersion";
    case AppErrorCode::InvalidData:
        return "InvalidData";
    }
    return "Unknown";
}

struct StorageSnapshot {
    std::vector<Student> students;
    std::vector<Teacher> teachers;
    std::vector<Course> courses;
};

class SqliteStorage {
public:
    [[nodiscard]] static std::expected<SqliteStorage, AppError> open(const std::filesystem::path& databasePath);

    ~SqliteStorage();
    SqliteStorage(SqliteStorage&&) noexcept;
    SqliteStorage& operator=(SqliteStorage&&) noexcept;

    SqliteStorage(const SqliteStorage&) = delete;
    SqliteStorage& operator=(const SqliteStorage&) = delete;

    [[nodiscard]] const std::filesystem::path& databasePath() const noexcept;
    [[nodiscard]] std::expected<StorageSnapshot, AppError> load() const;

    [[nodiscard]] std::expected<void, AppError> insertStudent(std::string_view name, int id) const;
    [[nodiscard]] std::expected<void, AppError> insertTeacher(std::string_view name) const;
    [[nodiscard]] std::expected<void, AppError> insertCourse(
        std::string_view name,
        int id,
        std::string_view description) const;
    [[nodiscard]] std::expected<void, AppError> insertEnrollment(int studentId, int courseId) const;
    [[nodiscard]] std::expected<void, AppError> updateScore(int courseId, int studentId, int score) const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;

    explicit SqliteStorage(std::unique_ptr<Impl> impl) noexcept;
};

} // namespace scs
