#include "persistence/file_storage.h"

#include <charconv>
#include <fstream>
#include <sstream>
#include <system_error>
#include <utility>

namespace scs {

namespace {

constexpr auto studentsFileName = "students.txt";
constexpr auto teachersFileName = "teachers.txt";
constexpr auto coursesFileName = "courses.txt";
constexpr auto studentCoursesFileName = "student_courses.txt";
constexpr auto scoresFileName = "scores.txt";

} // namespace

std::expected<void, AppError> FileStorage::save(
    const CourseSelectionService& service,
    const std::filesystem::path& directory) const {
    std::error_code errorCode;
    std::filesystem::create_directories(directory, errorCode);
    if (errorCode) {
        return std::unexpected(AppError{
            .code = AppErrorCode::FileOpenFailed,
            .message = "无法创建数据目录: " + directory.string()});
    }

    {
        const auto file = directory / studentsFileName;
        std::ofstream output(file);
        if (!output) {
            return std::unexpected(makeFileError(file, "写入"));
        }
        for (const auto& student : service.students()) {
            output << student.name() << ',' << student.id() << '\n';
        }
    }

    {
        const auto file = directory / teachersFileName;
        std::ofstream output(file);
        if (!output) {
            return std::unexpected(makeFileError(file, "写入"));
        }
        for (const auto& teacher : service.teachers()) {
            output << teacher.name() << '\n';
        }
    }

    {
        const auto file = directory / coursesFileName;
        std::ofstream output(file);
        if (!output) {
            return std::unexpected(makeFileError(file, "写入"));
        }
        for (const auto& course : service.courses()) {
            output << course.name() << ',' << course.id() << ',' << course.description() << '\n';
        }
    }

    {
        const auto file = directory / studentCoursesFileName;
        std::ofstream output(file);
        if (!output) {
            return std::unexpected(makeFileError(file, "写入"));
        }
        for (const auto& student : service.students()) {
            for (int courseId : student.enrolledCourseIds()) {
                output << student.id() << ',' << courseId << '\n';
            }
        }
    }

    {
        const auto file = directory / scoresFileName;
        std::ofstream output(file);
        if (!output) {
            return std::unexpected(makeFileError(file, "写入"));
        }
        for (const auto& course : service.courses()) {
            for (const auto& [studentId, score] : course.scoreBook()) {
                if (score.has_value()) {
                    output << course.id() << ',' << studentId << ',' << *score << '\n';
                }
            }
        }
    }

    return {};
}

std::expected<void, AppError> FileStorage::load(
    CourseSelectionService& service,
    const std::filesystem::path& directory) const {
    CourseSelectionService restored;

    {
        std::ifstream input(directory / studentsFileName);
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }
            auto fields = splitCsvLine(line);
            if (fields.size() < 2) {
                continue;
            }
            auto id = parseInt(fields[1]);
            if (id) {
                (void)restored.addStudent(fields[0], *id);
            }
        }
    }

    {
        std::ifstream input(directory / teachersFileName);
        std::string line;
        while (std::getline(input, line)) {
            if (!line.empty()) {
                (void)restored.addTeacher(trim(line));
            }
        }
    }

    {
        std::ifstream input(directory / coursesFileName);
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }
            auto fields = splitCsvLine(line);
            if (fields.size() < 3) {
                continue;
            }
            auto id = parseInt(fields[1]);
            if (id) {
                (void)restored.addCourse(fields[0], *id, fields[2]);
            }
        }
    }

    {
        std::ifstream input(directory / studentCoursesFileName);
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }
            auto fields = splitCsvLine(line);
            if (fields.size() < 2) {
                continue;
            }
            auto studentId = parseInt(fields[0]);
            auto courseId = parseInt(fields[1]);
            if (studentId && courseId) {
                (void)restored.selectCourse(*studentId, *courseId);
            }
        }
    }

    {
        std::ifstream input(directory / scoresFileName);
        std::string line;
        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }
            auto fields = splitCsvLine(line);
            if (fields.size() < 3) {
                continue;
            }
            auto courseId = parseInt(fields[0]);
            auto studentId = parseInt(fields[1]);
            auto score = parseInt(fields[2]);
            if (courseId && studentId && score) {
                (void)restored.setScore(*courseId, *studentId, *score);
            }
        }
    }

    service = std::move(restored);
    return {};
}

std::vector<std::string> FileStorage::splitCsvLine(std::string_view line) {
    std::vector<std::string> fields;
    std::stringstream stream(std::string{line});
    std::string field;
    while (std::getline(stream, field, ',')) {
        fields.push_back(std::string{trim(field)});
    }
    return fields;
}

std::string_view FileStorage::trim(std::string_view value) {
    while (!value.empty() && static_cast<unsigned char>(value.front()) <= ' ') {
        value.remove_prefix(1);
    }
    while (!value.empty() && static_cast<unsigned char>(value.back()) <= ' ') {
        value.remove_suffix(1);
    }
    return value;
}

std::expected<int, AppError> FileStorage::parseInt(std::string_view value) {
    value = trim(value);
    int result = 0;
    auto [ptr, error] = std::from_chars(value.data(), value.data() + value.size(), result);
    if (error != std::errc{} || ptr != value.data() + value.size()) {
        return std::unexpected(AppError{
            .code = AppErrorCode::InvalidData,
            .message = "数据文件中存在无法解析的整数: " + std::string{value}});
    }
    return result;
}

AppError FileStorage::makeFileError(const std::filesystem::path& file, std::string_view operation) {
    return AppError{
        .code = AppErrorCode::FileOpenFailed,
        .message = "无法打开文件进行" + std::string{operation} + ": " + file.string()};
}

} // namespace scs
