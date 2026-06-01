export module scs.frontend;

import std;
import scs.persistence;
import scs.service;

export namespace scs {

class CliApp {
public:
    explicit CliApp(CourseSelectionService& service);

    void run();

private:
    CourseSelectionService& m_service;
    bool m_running = true;

    void showMainMenu() const;
    void handleMainMenu();
    void handleStudentMenu();
    void handleTeacherMenu();
    void handleCourseMenu();

    void addStudent();
    void selectCourse();
    void viewStudentScores();
    void viewStudentCourses();

    void addTeacher();
    void importScore();
    void viewCoursesForTeacher();

    void addCourse();
    void showCourseStatistics();

    void printCourses() const;
    void printError(const AppError& error) const;

    [[nodiscard]] std::optional<int> readNonNegativeInt(std::string_view prompt);
    [[nodiscard]] std::string readToken(std::string_view prompt);
    [[nodiscard]] std::string readLine(std::string_view prompt);
    [[nodiscard]] bool confirm(std::string_view prompt);
    void clearInput();
};

} // namespace scs
