module scs.frontend;

import std;
import scs.persistence;
import scs.service;

namespace scs {

namespace {

std::string scoreText(const std::optional<int>& score) {
    return score.has_value() ? std::to_string(*score) : "未评分";
}

} // namespace

CliApp::CliApp(CourseSelectionService& service)
    : m_service(service) {
}

void CliApp::run() {
    while (m_running) {
        showMainMenu();
        handleMainMenu();
    }
}

void CliApp::showMainMenu() const {
    std::println("\n--- 课程选择系统 ---");
    std::println("1. 学生管理");
    std::println("2. 教师管理");
    std::println("3. 课程管理");
    std::println("4. 退出");
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
        std::println("感谢使用，再见！");
        m_running = false;
        break;
    default:
        std::println("无效选择，请重试。");
        break;
    }
}

void CliApp::handleStudentMenu() {
    std::println("\n--- 学生管理 ---");
    std::println("1. 添加学生");
    std::println("2. 学生选课");
    std::println("3. 查看学生成绩");
    std::println("4. 查看学生已选课程");
    std::println("5. 返回主菜单");

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
        std::println("无效选择，请重试。");
        break;
    }
}

void CliApp::handleTeacherMenu() {
    std::println("\n--- 教师管理 ---");
    std::println("1. 添加教师");
    std::println("2. 导入学生成绩");
    std::println("3. 查看课程信息");
    std::println("4. 返回主菜单");

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
        std::println("无效选择，请重试。");
        break;
    }
}

void CliApp::handleCourseMenu() {
    std::println("\n--- 课程管理 ---");
    std::println("1. 添加课程");
    std::println("2. 课程统计");
    std::println("3. 返回主菜单");

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
        std::println("无效选择，请重试。");
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
        std::println("已取消添加学生操作。");
        return;
    }

    auto result = m_service.addStudent(name, *id);
    if (!result) {
        printError(result.error());
        return;
    }
    std::println("学生添加成功！");
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
        std::println("已取消选课操作。");
        return;
    }

    auto result = m_service.selectCourse(*studentId, *courseId);
    if (!result) {
        printError(result.error());
        return;
    }
    std::println("选课成功！");
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
        std::println("学生 {} 还没有选择任何课程。", view.studentName);
        return;
    }

    std::println("学生 {} (ID: {}) 的成绩：", view.studentName, view.studentId);
    std::println("{:<12}{:<24}{}", "课程ID", "课程名称", "成绩");
    for (const auto& row : view.scores) {
        std::println("{:<12}{:<24}{}", row.courseId, row.courseName, scoreText(row.score));
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
        std::println("学生 {} 还没有选择任何课程。", view.studentName);
        return;
    }

    std::println("学生 {} (ID: {}) 已选课程：", view.studentName, view.studentId);
    std::println("{:<12}{:<24}{}", "课程ID", "课程名称", "课程描述");
    for (const auto& course : view.courses) {
        std::println("{:<12}{:<24}{}", course.courseId, course.courseName, course.description);
    }
}

void CliApp::addTeacher() {
    auto name = readToken("输入教师姓名: ");
    if (name.empty()) {
        return;
    }

    if (!confirm("确定要添加教师 \"" + name + "\" 吗？")) {
        std::println("已取消添加教师操作。");
        return;
    }

    auto result = m_service.addTeacher(name);
    if (!result) {
        printError(result.error());
        return;
    }
    std::println("教师添加成功！");
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
        std::println("已取消成绩导入操作。");
        return;
    }

    auto result = m_service.importScore(teacherName, *courseId, *studentId, *score);
    if (!result) {
        printError(result.error());
        return;
    }
    std::println("成绩导入成功！");
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
        std::println("已取消添加课程操作。");
        return;
    }

    if (!confirm("确定要添加课程 \"" + name + "\" (ID: " + std::to_string(*id) + ") 吗？")) {
        std::println("已取消添加课程操作。");
        return;
    }

    auto result = m_service.addCourse(name, *id, description);
    if (!result) {
        printError(result.error());
        return;
    }
    std::println("课程添加成功！");
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
    std::println("课程 \"{}\" (ID: {}) 统计信息：", stats.courseName, stats.courseId);
    std::println("选课人数：{}", stats.enrolledCount);
    std::println("已评分人数：{}", stats.gradedCount);
    if (stats.gradedCount == 0) {
        std::println("暂无成绩数据");
        return;
    }
    std::println("平均分：{:.2f}", stats.averageScore);
    std::println("最高分：{}", *stats.highestScore);
    std::println("最低分：{}", *stats.lowestScore);
}

void CliApp::printCourses() const {
    auto courses = m_service.listCourses();
    if (courses.empty()) {
        std::println("当前没有任何课程。");
        return;
    }

    std::println("所有课程信息：");
    std::println("{:<12}{:<24}{:<32}{}", "课程ID", "课程名称", "课程描述", "选课人数");
    for (const auto& course : courses) {
        std::println("{:<12}{:<24}{:<32}{}", course.id, course.name, course.description, course.studentCount);
    }
}

void CliApp::printError(const AppError& error) const {
    std::println("错误：{}", error.message);
}

std::optional<int> CliApp::readNonNegativeInt(std::string_view prompt) {
    std::print(std::cout, "{}", prompt);
    int value = 0;
    std::cin >> value;
    if (std::cin.fail() || value < 0) {
        std::println("无效输入，请输入一个非负整数。");
        clearInput();
        return std::nullopt;
    }
    clearInput();
    return value;
}

std::string CliApp::readToken(std::string_view prompt) {
    std::print(std::cout, "{}", prompt);
    std::string value;
    std::cin >> value;
    if (std::cin.fail()) {
        clearInput();
        return {};
    }
    clearInput();
    if (value.empty()) {
        std::println("输入不能为空，请重新输入。");
    }
    return value;
}

std::string CliApp::readLine(std::string_view prompt) {
    std::print(std::cout, "{}", prompt);
    std::string value;
    std::getline(std::cin, value);
    if (std::cin.fail()) {
        clearInput();
        return {};
    }
    return value;
}

bool CliApp::confirm(std::string_view prompt) {
    std::print(std::cout, "{} (y/n): ", prompt);
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
