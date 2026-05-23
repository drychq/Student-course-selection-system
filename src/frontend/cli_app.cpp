#include "frontend/cli_app.h"

#include <cctype>
#include <iomanip>
#include <iostream>
#include <limits>

namespace scs {

namespace {

std::string scoreText(const std::optional<int>& score) {
    return score.has_value() ? std::to_string(*score) : "未评分";
}

} // namespace

CliApp::CliApp(CourseSelectionService& service, FileStorage& storage)
    : m_service(service), m_storage(storage) {
}

void CliApp::run() {
    while (m_running) {
        showMainMenu();
        handleMainMenu();
    }
}

void CliApp::showMainMenu() const {
    std::cout << "\n--- 课程选择系统 ---\n";
    std::cout << "1. 学生管理\n";
    std::cout << "2. 教师管理\n";
    std::cout << "3. 课程管理\n";
    std::cout << "4. 数据管理\n";
    std::cout << "5. 退出\n";
}

void CliApp::handleMainMenu() {
    auto choice = readNonNegativeInt("请输入您的选择: ");
    if (!choice) {
        return;
    }

    switch (*choice) {
    case 1:
        handleStudentMenu();
        break;
    case 2:
        handleTeacherMenu();
        break;
    case 3:
        handleCourseMenu();
        break;
    case 4:
        handleDataMenu();
        break;
    case 5:
        std::cout << "感谢使用，再见！\n";
        m_running = false;
        break;
    default:
        std::cout << "无效选择，请重试。\n";
        break;
    }
}

void CliApp::handleStudentMenu() {
    std::cout << "\n--- 学生管理 ---\n";
    std::cout << "1. 添加学生\n";
    std::cout << "2. 学生选课\n";
    std::cout << "3. 查看学生成绩\n";
    std::cout << "4. 查看学生已选课程\n";
    std::cout << "5. 返回主菜单\n";

    auto choice = readNonNegativeInt("请输入您的选择: ");
    if (!choice) {
        return;
    }

    switch (*choice) {
    case 1:
        addStudent();
        break;
    case 2:
        selectCourse();
        break;
    case 3:
        viewStudentScores();
        break;
    case 4:
        viewStudentCourses();
        break;
    case 5:
        break;
    default:
        std::cout << "无效选择，请重试。\n";
        break;
    }
}

void CliApp::handleTeacherMenu() {
    std::cout << "\n--- 教师管理 ---\n";
    std::cout << "1. 添加教师\n";
    std::cout << "2. 导入学生成绩\n";
    std::cout << "3. 查看课程信息\n";
    std::cout << "4. 返回主菜单\n";

    auto choice = readNonNegativeInt("请输入您的选择: ");
    if (!choice) {
        return;
    }

    switch (*choice) {
    case 1:
        addTeacher();
        break;
    case 2:
        importScore();
        break;
    case 3:
        viewCoursesForTeacher();
        break;
    case 4:
        break;
    default:
        std::cout << "无效选择，请重试。\n";
        break;
    }
}

void CliApp::handleCourseMenu() {
    std::cout << "\n--- 课程管理 ---\n";
    std::cout << "1. 添加课程\n";
    std::cout << "2. 课程统计\n";
    std::cout << "3. 返回主菜单\n";

    auto choice = readNonNegativeInt("请输入您的选择: ");
    if (!choice) {
        return;
    }

    switch (*choice) {
    case 1:
        addCourse();
        break;
    case 2:
        showCourseStatistics();
        break;
    case 3:
        break;
    default:
        std::cout << "无效选择，请重试。\n";
        break;
    }
}

void CliApp::handleDataMenu() {
    std::cout << "\n--- 数据管理 ---\n";
    std::cout << "1. 保存数据\n";
    std::cout << "2. 加载数据\n";
    std::cout << "3. 返回主菜单\n";

    auto choice = readNonNegativeInt("请输入您的选择: ");
    if (!choice) {
        return;
    }

    switch (*choice) {
    case 1:
        saveData();
        break;
    case 2:
        loadData();
        break;
    case 3:
        break;
    default:
        std::cout << "无效选择，请重试。\n";
        break;
    }
}

void CliApp::addStudent() {
    auto name = readToken("输入学生姓名: ");
    if (name.empty()) {
        return;
    }
    auto id = readNonNegativeInt("输入学生ID: ");
    if (!id) {
        return;
    }

    if (!confirm("确定要添加学生 \"" + name + "\" (ID: " + std::to_string(*id) + ") 吗？")) {
        std::cout << "已取消添加学生操作。\n";
        return;
    }

    auto result = m_service.addStudent(name, *id);
    if (!result) {
        printError(result.error());
        return;
    }
    std::cout << "学生添加成功！\n";
}

void CliApp::selectCourse() {
    auto studentId = readNonNegativeInt("请输入学生ID: ");
    if (!studentId) {
        return;
    }
    auto courseId = readNonNegativeInt("请输入课程ID: ");
    if (!courseId) {
        return;
    }

    if (!confirm("确定要为该学生选择课程吗？")) {
        std::cout << "已取消选课操作。\n";
        return;
    }

    auto result = m_service.selectCourse(*studentId, *courseId);
    if (!result) {
        printError(result.error());
        return;
    }
    std::cout << "选课成功！\n";
}

void CliApp::viewStudentScores() {
    auto studentId = readNonNegativeInt("输入学生ID: ");
    if (!studentId) {
        return;
    }

    auto result = m_service.studentScores(*studentId);
    if (!result) {
        printError(result.error());
        return;
    }

    const auto& view = *result;
    if (view.scores.empty()) {
        std::cout << "学生 " << view.studentName << " 还没有选择任何课程。\n";
        return;
    }

    std::cout << "学生 " << view.studentName << " (ID: " << view.studentId << ") 的成绩：\n";
    std::cout << std::left << std::setw(12) << "课程ID" << std::setw(24) << "课程名称" << "成绩\n";
    for (const auto& row : view.scores) {
        std::cout << std::left << std::setw(12) << row.courseId
                  << std::setw(24) << row.courseName
                  << scoreText(row.score) << '\n';
    }
}

void CliApp::viewStudentCourses() {
    auto studentId = readNonNegativeInt("输入学生ID: ");
    if (!studentId) {
        return;
    }

    auto result = m_service.studentCourses(*studentId);
    if (!result) {
        printError(result.error());
        return;
    }

    const auto& view = *result;
    if (view.courses.empty()) {
        std::cout << "学生 " << view.studentName << " 还没有选择任何课程。\n";
        return;
    }

    std::cout << "学生 " << view.studentName << " (ID: " << view.studentId << ") 已选课程：\n";
    std::cout << std::left << std::setw(12) << "课程ID" << std::setw(24) << "课程名称" << "课程描述\n";
    for (const auto& course : view.courses) {
        std::cout << std::left << std::setw(12) << course.courseId
                  << std::setw(24) << course.courseName
                  << course.description << '\n';
    }
}

void CliApp::addTeacher() {
    auto name = readToken("输入教师姓名: ");
    if (name.empty()) {
        return;
    }

    if (!confirm("确定要添加教师 \"" + name + "\" 吗？")) {
        std::cout << "已取消添加教师操作。\n";
        return;
    }

    auto result = m_service.addTeacher(name);
    if (!result) {
        printError(result.error());
        return;
    }
    std::cout << "教师添加成功！\n";
}

void CliApp::importScore() {
    auto teacherName = readToken("请输入教师姓名: ");
    if (teacherName.empty()) {
        return;
    }
    auto courseId = readNonNegativeInt("请输入课程ID: ");
    if (!courseId) {
        return;
    }
    auto studentId = readNonNegativeInt("请输入学生ID: ");
    if (!studentId) {
        return;
    }
    auto score = readNonNegativeInt("请输入成绩(0-100): ");
    if (!score) {
        return;
    }

    if (!confirm("确定要导入该成绩吗？")) {
        std::cout << "已取消成绩导入操作。\n";
        return;
    }

    auto result = m_service.importScore(teacherName, *courseId, *studentId, *score);
    if (!result) {
        printError(result.error());
        return;
    }
    std::cout << "成绩导入成功！\n";
}

void CliApp::viewCoursesForTeacher() {
    auto teacherName = readToken("输入教师姓名: ");
    if (teacherName.empty()) {
        return;
    }
    auto teacher = m_service.findTeacher(teacherName);
    if (!teacher) {
        printError(teacher.error());
        return;
    }
    printCourses();
}

void CliApp::addCourse() {
    auto name = readToken("输入课程名称: ");
    if (name.empty()) {
        return;
    }
    auto id = readNonNegativeInt("输入课程ID: ");
    if (!id) {
        return;
    }
    auto description = readLine("输入课程描述: ");
    if (description.empty() && !confirm("课程描述为空，是否继续添加课程？")) {
        std::cout << "已取消添加课程操作。\n";
        return;
    }

    if (!confirm("确定要添加课程 \"" + name + "\" (ID: " + std::to_string(*id) + ") 吗？")) {
        std::cout << "已取消添加课程操作。\n";
        return;
    }

    auto result = m_service.addCourse(name, *id, description);
    if (!result) {
        printError(result.error());
        return;
    }
    std::cout << "课程添加成功！\n";
}

void CliApp::showCourseStatistics() {
    auto courseId = readNonNegativeInt("输入课程ID: ");
    if (!courseId) {
        return;
    }

    auto result = m_service.courseStatistics(*courseId);
    if (!result) {
        printError(result.error());
        return;
    }

    const auto& stats = *result;
    std::cout << "课程 \"" << stats.courseName << "\" (ID: " << stats.courseId << ") 统计信息：\n";
    std::cout << "选课人数：" << stats.enrolledCount << '\n';
    std::cout << "已评分人数：" << stats.gradedCount << '\n';
    if (stats.gradedCount == 0) {
        std::cout << "暂无成绩数据\n";
        return;
    }
    std::cout << "平均分：" << std::fixed << std::setprecision(2) << stats.averageScore << '\n';
    std::cout << "最高分：" << *stats.highestScore << '\n';
    std::cout << "最低分：" << *stats.lowestScore << '\n';
}

void CliApp::saveData() {
    auto directory = readLine("输入要保存的数据目录名称: ");
    if (directory.empty()) {
        return;
    }
    if (!confirm("确定要将数据保存到目录 \"" + directory + "\" 吗？")) {
        std::cout << "已取消保存数据操作。\n";
        return;
    }

    auto result = m_storage.save(m_service, directory);
    if (!result) {
        printError(result.error());
        return;
    }
    std::cout << "数据保存成功！\n";
}

void CliApp::loadData() {
    auto directory = readLine("输入要加载的数据目录名称: ");
    if (directory.empty()) {
        return;
    }
    if (!confirm("确定要从目录 \"" + directory + "\" 加载数据吗？这将覆盖当前所有数据！")) {
        std::cout << "已取消加载数据操作。\n";
        return;
    }

    auto result = m_storage.load(m_service, directory);
    if (!result) {
        printError(result.error());
        return;
    }
    std::cout << "数据加载成功！\n";
}

void CliApp::printCourses() const {
    auto courses = m_service.listCourses();
    if (courses.empty()) {
        std::cout << "当前没有任何课程。\n";
        return;
    }

    std::cout << "所有课程信息：\n";
    std::cout << std::left << std::setw(12) << "课程ID"
              << std::setw(24) << "课程名称"
              << std::setw(32) << "课程描述"
              << "选课人数\n";
    for (const auto& course : courses) {
        std::cout << std::left << std::setw(12) << course.id
                  << std::setw(24) << course.name
                  << std::setw(32) << course.description
                  << course.studentCount << '\n';
    }
}

void CliApp::printError(const AppError& error) const {
    std::cout << "错误：" << error.message << '\n';
}

std::optional<int> CliApp::readNonNegativeInt(std::string_view prompt) {
    std::cout << prompt;
    int value = 0;
    std::cin >> value;
    if (std::cin.fail() || value < 0) {
        std::cout << "无效输入，请输入一个非负整数。\n";
        clearInput();
        return std::nullopt;
    }
    clearInput();
    return value;
}

std::string CliApp::readToken(std::string_view prompt) {
    std::cout << prompt;
    std::string value;
    std::cin >> value;
    if (std::cin.fail()) {
        clearInput();
        return {};
    }
    clearInput();
    if (value.empty()) {
        std::cout << "输入不能为空，请重新输入。\n";
    }
    return value;
}

std::string CliApp::readLine(std::string_view prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    if (std::cin.fail()) {
        clearInput();
        return {};
    }
    return value;
}

bool CliApp::confirm(std::string_view prompt) {
    std::cout << prompt << " (y/n): ";
    char response = '\0';
    std::cin >> response;
    clearInput();
    return std::tolower(static_cast<unsigned char>(response)) == 'y';
}

void CliApp::clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace scs
