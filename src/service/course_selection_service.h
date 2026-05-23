#pragma once

#include "domain/course.h"
#include "domain/student.h"
#include "domain/teacher.h"
#include "service/app_error.h"

#include <expected>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace scs {

struct CourseSummary {
    int id;
    std::string name;
    std::string description;
    std::size_t studentCount;
};

struct StudentCourseInfo {
    int courseId;
    std::string courseName;
    std::string description;
};

struct StudentCoursesView {
    int studentId;
    std::string studentName;
    std::vector<StudentCourseInfo> courses;
};

struct StudentScoreInfo {
    int courseId;
    std::string courseName;
    std::optional<int> score;
};

struct StudentScoresView {
    int studentId;
    std::string studentName;
    std::vector<StudentScoreInfo> scores;
};

struct CourseStatistics {
    int courseId;
    std::string courseName;
    std::size_t enrolledCount;
    std::size_t gradedCount;
    double averageScore;
    std::optional<int> highestScore;
    std::optional<int> lowestScore;
};

class CourseSelectionService {
public:
    std::expected<void, AppError> addStudent(std::string_view name, int id);
    std::expected<void, AppError> addTeacher(std::string_view name);
    std::expected<void, AppError> addCourse(std::string_view name, int id, std::string_view description);
    std::expected<void, AppError> selectCourse(int studentId, int courseId);
    std::expected<void, AppError> importScore(std::string_view teacherName, int courseId, int studentId, int score);
    std::expected<void, AppError> setScore(int courseId, int studentId, int score);

    std::expected<std::reference_wrapper<const Student>, AppError> findStudent(int id) const;
    std::expected<std::reference_wrapper<const Teacher>, AppError> findTeacher(std::string_view name) const;
    std::expected<std::reference_wrapper<const Course>, AppError> findCourse(int id) const;

    [[nodiscard]] const std::vector<Student>& students() const noexcept;
    [[nodiscard]] const std::vector<Teacher>& teachers() const noexcept;
    [[nodiscard]] const std::vector<Course>& courses() const noexcept;
    [[nodiscard]] std::vector<CourseSummary> listCourses() const;

    std::expected<StudentCoursesView, AppError> studentCourses(int studentId) const;
    std::expected<StudentScoresView, AppError> studentScores(int studentId) const;
    std::expected<CourseStatistics, AppError> courseStatistics(int courseId) const;

    void clear() noexcept;

private:
    std::vector<Student> m_students;
    std::vector<Teacher> m_teachers;
    std::vector<Course> m_courses;

    std::vector<Student>::iterator findStudentIt(int id);
    std::vector<Student>::const_iterator findStudentIt(int id) const;
    std::vector<Teacher>::const_iterator findTeacherIt(std::string_view name) const;
    std::vector<Course>::iterator findCourseIt(int id);
    std::vector<Course>::const_iterator findCourseIt(int id) const;

    static bool isBlank(std::string_view value);
    static AppError makeError(AppErrorCode code, std::string message);
};

} // namespace scs
