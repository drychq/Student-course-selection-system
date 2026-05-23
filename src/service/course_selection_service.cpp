#include "service/course_selection_service.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <utility>

namespace scs {

std::expected<void, AppError> CourseSelectionService::addStudent(std::string_view name, int id) {
    if (isBlank(name)) {
        return std::unexpected(makeError(AppErrorCode::EmptyName, "学生姓名不能为空。"));
    }
    if (id < 0) {
        return std::unexpected(makeError(AppErrorCode::InvalidId, "学生ID不能为负数。"));
    }
    if (findStudentIt(id) != m_students.end()) {
        return std::unexpected(makeError(AppErrorCode::StudentAlreadyExists, "学生ID已存在。"));
    }

    m_students.emplace_back(std::string{name}, id);
    return {};
}

std::expected<void, AppError> CourseSelectionService::addTeacher(std::string_view name) {
    if (isBlank(name)) {
        return std::unexpected(makeError(AppErrorCode::EmptyName, "教师姓名不能为空。"));
    }
    if (findTeacherIt(name) != m_teachers.end()) {
        return std::unexpected(makeError(AppErrorCode::TeacherAlreadyExists, "教师已存在。"));
    }

    m_teachers.emplace_back(std::string{name});
    return {};
}

std::expected<void, AppError> CourseSelectionService::addCourse(
    std::string_view name,
    int id,
    std::string_view description) {
    if (isBlank(name)) {
        return std::unexpected(makeError(AppErrorCode::EmptyName, "课程名称不能为空。"));
    }
    if (id < 0) {
        return std::unexpected(makeError(AppErrorCode::InvalidId, "课程ID不能为负数。"));
    }
    if (findCourseIt(id) != m_courses.end()) {
        return std::unexpected(makeError(AppErrorCode::CourseAlreadyExists, "课程ID已存在。"));
    }

    m_courses.emplace_back(std::string{name}, id, std::string{description});
    return {};
}

std::expected<void, AppError> CourseSelectionService::selectCourse(int studentId, int courseId) {
    auto studentIt = findStudentIt(studentId);
    if (studentIt == m_students.end()) {
        return std::unexpected(makeError(AppErrorCode::StudentNotFound, "未找到学生。"));
    }

    auto courseIt = findCourseIt(courseId);
    if (courseIt == m_courses.end()) {
        return std::unexpected(makeError(AppErrorCode::CourseNotFound, "未找到课程。"));
    }

    if (studentIt->hasSelectedCourse(courseId)) {
        return std::unexpected(makeError(AppErrorCode::CourseAlreadySelected, "该学生已经选择了这门课程。"));
    }

    studentIt->enroll(courseId);
    courseIt->enrollStudent(studentId);
    return {};
}

std::expected<void, AppError> CourseSelectionService::importScore(
    std::string_view teacherName,
    int courseId,
    int studentId,
    int score) {
    if (findTeacherIt(teacherName) == m_teachers.end()) {
        return std::unexpected(makeError(AppErrorCode::TeacherNotFound, "未找到教师。"));
    }
    return setScore(courseId, studentId, score);
}

std::expected<void, AppError> CourseSelectionService::setScore(int courseId, int studentId, int score) {
    if (score < 0 || score > 100) {
        return std::unexpected(makeError(AppErrorCode::InvalidScore, "成绩必须在0到100之间。"));
    }

    if (findStudentIt(studentId) == m_students.end()) {
        return std::unexpected(makeError(AppErrorCode::StudentNotFound, "未找到学生。"));
    }

    auto courseIt = findCourseIt(courseId);
    if (courseIt == m_courses.end()) {
        return std::unexpected(makeError(AppErrorCode::CourseNotFound, "未找到课程。"));
    }
    if (!courseIt->hasStudent(studentId)) {
        return std::unexpected(makeError(AppErrorCode::CourseNotSelected, "该学生没有选择这门课程。"));
    }

    courseIt->setScore(studentId, score);
    return {};
}

std::expected<std::reference_wrapper<const Student>, AppError> CourseSelectionService::findStudent(int id) const {
    auto it = findStudentIt(id);
    if (it == m_students.end()) {
        return std::unexpected(makeError(AppErrorCode::StudentNotFound, "未找到学生。"));
    }
    return std::cref(*it);
}

std::expected<std::reference_wrapper<const Teacher>, AppError> CourseSelectionService::findTeacher(std::string_view name) const {
    auto it = findTeacherIt(name);
    if (it == m_teachers.end()) {
        return std::unexpected(makeError(AppErrorCode::TeacherNotFound, "未找到教师。"));
    }
    return std::cref(*it);
}

std::expected<std::reference_wrapper<const Course>, AppError> CourseSelectionService::findCourse(int id) const {
    auto it = findCourseIt(id);
    if (it == m_courses.end()) {
        return std::unexpected(makeError(AppErrorCode::CourseNotFound, "未找到课程。"));
    }
    return std::cref(*it);
}

const std::vector<Student>& CourseSelectionService::students() const noexcept {
    return m_students;
}

const std::vector<Teacher>& CourseSelectionService::teachers() const noexcept {
    return m_teachers;
}

const std::vector<Course>& CourseSelectionService::courses() const noexcept {
    return m_courses;
}

std::vector<CourseSummary> CourseSelectionService::listCourses() const {
    std::vector<CourseSummary> summaries;
    summaries.reserve(m_courses.size());
    for (const auto& course : m_courses) {
        summaries.push_back(CourseSummary{
            .id = course.id(),
            .name = std::string{course.name()},
            .description = std::string{course.description()},
            .studentCount = course.studentCount()});
    }
    return summaries;
}

std::expected<StudentCoursesView, AppError> CourseSelectionService::studentCourses(int studentId) const {
    auto studentResult = findStudent(studentId);
    if (!studentResult) {
        return std::unexpected(studentResult.error());
    }

    const auto& student = studentResult->get();
    StudentCoursesView view{
        .studentId = student.id(),
        .studentName = std::string{student.name()},
        .courses = {}};

    for (int courseId : student.enrolledCourseIds()) {
        auto courseResult = findCourse(courseId);
        if (courseResult) {
            const auto& course = courseResult->get();
            view.courses.push_back(StudentCourseInfo{
                .courseId = course.id(),
                .courseName = std::string{course.name()},
                .description = std::string{course.description()}});
        }
    }
    return view;
}

std::expected<StudentScoresView, AppError> CourseSelectionService::studentScores(int studentId) const {
    auto studentResult = findStudent(studentId);
    if (!studentResult) {
        return std::unexpected(studentResult.error());
    }

    const auto& student = studentResult->get();
    StudentScoresView view{
        .studentId = student.id(),
        .studentName = std::string{student.name()},
        .scores = {}};

    for (int courseId : student.enrolledCourseIds()) {
        auto courseResult = findCourse(courseId);
        if (courseResult) {
            const auto& course = courseResult->get();
            view.scores.push_back(StudentScoreInfo{
                .courseId = course.id(),
                .courseName = std::string{course.name()},
                .score = course.scoreFor(student.id())});
        }
    }
    return view;
}

std::expected<CourseStatistics, AppError> CourseSelectionService::courseStatistics(int courseId) const {
    auto courseResult = findCourse(courseId);
    if (!courseResult) {
        return std::unexpected(courseResult.error());
    }

    const auto& course = courseResult->get();
    return CourseStatistics{
        .courseId = course.id(),
        .courseName = std::string{course.name()},
        .enrolledCount = course.studentCount(),
        .gradedCount = course.gradedCount(),
        .averageScore = course.averageScore(),
        .highestScore = course.highestScore(),
        .lowestScore = course.lowestScore()};
}

void CourseSelectionService::clear() noexcept {
    m_students.clear();
    m_teachers.clear();
    m_courses.clear();
}

std::vector<Student>::iterator CourseSelectionService::findStudentIt(int id) {
    return std::ranges::find_if(m_students, [id](const Student& student) {
        return student.id() == id;
    });
}

std::vector<Student>::const_iterator CourseSelectionService::findStudentIt(int id) const {
    return std::ranges::find_if(m_students, [id](const Student& student) {
        return student.id() == id;
    });
}

std::vector<Teacher>::const_iterator CourseSelectionService::findTeacherIt(std::string_view name) const {
    return std::ranges::find_if(m_teachers, [name](const Teacher& teacher) {
        return teacher.name() == name;
    });
}

std::vector<Course>::iterator CourseSelectionService::findCourseIt(int id) {
    return std::ranges::find_if(m_courses, [id](const Course& course) {
        return course.id() == id;
    });
}

std::vector<Course>::const_iterator CourseSelectionService::findCourseIt(int id) const {
    return std::ranges::find_if(m_courses, [id](const Course& course) {
        return course.id() == id;
    });
}

bool CourseSelectionService::isBlank(std::string_view value) {
    return std::ranges::all_of(value, [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
}

AppError CourseSelectionService::makeError(AppErrorCode code, std::string message) {
    return AppError{.code = code, .message = std::move(message)};
}

} // namespace scs
