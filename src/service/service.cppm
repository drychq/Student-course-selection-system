export module scs.service;

import std;
import scs.domain;
import scs.persistence;

export namespace scs {

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
    [[nodiscard]] static std::expected<CourseSelectionService, AppError> create(SqliteStorage& storage);

    [[nodiscard]] std::expected<void, AppError> addStudent(std::string_view name, int id);
    [[nodiscard]] std::expected<void, AppError> addTeacher(std::string_view name);
    [[nodiscard]] std::expected<void, AppError> addCourse(std::string_view name, int id, std::string_view description);
    [[nodiscard]] std::expected<void, AppError> selectCourse(int studentId, int courseId);
    [[nodiscard]] std::expected<void, AppError> importScore(
        std::string_view teacherName,
        int courseId,
        int studentId,
        int score);
    [[nodiscard]] std::expected<void, AppError> setScore(int courseId, int studentId, int score);

    [[nodiscard]] std::expected<std::reference_wrapper<const Student>, AppError> findStudent(int id) const;
    [[nodiscard]] std::expected<std::reference_wrapper<const Teacher>, AppError> findTeacher(std::string_view name) const;
    [[nodiscard]] std::expected<std::reference_wrapper<const Course>, AppError> findCourse(int id) const;

    [[nodiscard]] const std::vector<Student>& students() const noexcept;
    [[nodiscard]] const std::vector<Teacher>& teachers() const noexcept;
    [[nodiscard]] const std::vector<Course>& courses() const noexcept;
    [[nodiscard]] std::vector<CourseSummary> listCourses() const;

    [[nodiscard]] std::expected<StudentCoursesView, AppError> studentCourses(int studentId) const;
    [[nodiscard]] std::expected<StudentScoresView, AppError> studentScores(int studentId) const;
    [[nodiscard]] std::expected<CourseStatistics, AppError> courseStatistics(int courseId) const;

private:
    SqliteStorage& m_storage;
    std::vector<Student> m_students;
    std::vector<Teacher> m_teachers;
    std::vector<Course> m_courses;

    CourseSelectionService(SqliteStorage& storage, StorageSnapshot snapshot);

    [[nodiscard]] std::vector<Student>::iterator findStudentIt(int id);
    [[nodiscard]] std::vector<Student>::const_iterator findStudentIt(int id) const;
    [[nodiscard]] std::vector<Teacher>::const_iterator findTeacherIt(std::string_view name) const;
    [[nodiscard]] std::vector<Course>::iterator findCourseIt(int id);
    [[nodiscard]] std::vector<Course>::const_iterator findCourseIt(int id) const;

    [[nodiscard]] static bool isBlank(std::string_view value);
    [[nodiscard]] static AppError makeError(AppErrorCode code, std::string message);
};

} // namespace scs
