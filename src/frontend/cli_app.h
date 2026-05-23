#pragma once

#include "persistence/file_storage.h"
#include "service/course_selection_service.h"

#include <optional>
#include <string>
#include <string_view>

namespace scs {

class CliApp {
public:
    CliApp(CourseSelectionService& service, FileStorage& storage);

    void run();

private:
    CourseSelectionService& m_service;
    FileStorage& m_storage;
    bool m_running = true;

    void showMainMenu() const;
    void handleMainMenu();
    void handleStudentMenu();
    void handleTeacherMenu();
    void handleCourseMenu();
    void handleDataMenu();

    void addStudent();
    void selectCourse();
    void viewStudentScores();
    void viewStudentCourses();

    void addTeacher();
    void importScore();
    void viewCoursesForTeacher();

    void addCourse();
    void showCourseStatistics();

    void saveData();
    void loadData();

    void printCourses() const;
    void printError(const AppError& error) const;

    std::optional<int> readNonNegativeInt(std::string_view prompt);
    std::string readToken(std::string_view prompt);
    std::string readLine(std::string_view prompt);
    bool confirm(std::string_view prompt);
    void clearInput();
};

} // namespace scs
