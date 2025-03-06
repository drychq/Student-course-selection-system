#include "menu.h"

#include "menu.h"
#include "system.h"
#include "student.h"
#include "teacher.h"
#include "course.h"
#include <limits>
#include <cctype>

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::numeric_limits;
using std::streamsize;
using std::shared_ptr;
using std::vector;

// 清除输入缓冲区
// 用于处理输入错误后的恢复
void Menu::clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

// 获取整数输入
// 包含输入验证，确保输入为非负整数
int Menu::getIntInput(const string& input) {
    if (!input.empty()) {
        cout << input;
    }

    int value;
    cin >> value;
    if (cin.fail() || value < 0) {
        cout << "无效输入，请输入一个非负整数。\n";
        clearInput();
        return -1;
    }
    return value;
}

// 获取字符串输入
// 处理空输入的情况
string Menu::getStringInput(const string& input) {
    if (!input.empty()) {
        cout << input;
    }

    string value;
    cin >> value;

    // 检查输入是否为空
    if (value.empty()) {
        cout << "输入不能为空，请重新输入。\n";
        return "";
    }

    return value;
}

// 获取用户确认
bool Menu::getConfirmation(const string& input) {
    cout << input << " (y/n): ";
    char response;
    cin >> response;
    clearInput();

    // 转换为小写进行比较
    response = std::tolower(response);

    return (response == 'y');
}

// MainMenu方法实现
void MainMenu::display() {
    cout << "\n--- 课程选择系统 ---\n";
    cout << "1. 学生管理\n";
    cout << "2. 教师管理\n";
    cout << "3. 课程管理\n";
    cout << "4. 数据管理\n";
    cout << "5. 退出\n";
    cout << "请输入您的选择: ";
}

void MainMenu::execute(System& system) {
    int choice = getIntInput("");
    switch (choice) {
    case 1: {
        StudentMenu menu;
        menu.display();
        menu.execute(system);
        break;
    }
    case 2: {
        TeacherMenu menu;
        menu.display();
        menu.execute(system);
        break;
    }
    case 3: {
        CourseMenu menu;
        menu.display();
        menu.execute(system);
        break;
    }
    case 4: {
        DataMenu menu;
        menu.display();
        menu.execute(system);
        break;
    }
    case 5:
        cout << "感谢使用，再见！" << endl;
        system.stop();
        break;
    default:
        cout << "无效选择，请重试。\n";
    }
}

// StudentMenu方法实现
void StudentMenu::display() {
    cout << "\n--- 学生管理 ---\n";
    cout << "1. 添加学生\n";
    cout << "2. 学生选课\n";
    cout << "3. 查看学生成绩\n";
    cout << "4. 查看学生已选课程\n";
    cout << "5. 返回主菜单\n";
    cout << "请输入您的选择: ";
}

void StudentMenu::execute(System& system) {
    int choice = getIntInput("");
    switch (choice) {
    case 1:
        addStudent(system);
        break;
    case 2:
        selectCourse(system);
        break;
    case 3:
        viewScores(system);
        break;
    case 4:
        viewCourses(system);
        break;
    case 5:
        return;
    default:
        cout << "无效选择，请重试。\n";
    }
}

void StudentMenu::addStudent(System& system) {
    string name = getStringInput("输入学生姓名: ");
    if (name.empty()) return;

    int id = getIntInput("输入学生ID: ");
    if (id == -1) return;

    // 验证学生ID唯一性，防止重复添加
    if (system.findStudent(id)) {
        cout << "错误：学生ID " << id << " 已存在，请使用其他ID。\n";
        return;
    }

    // 确认添加学生
    if (getConfirmation("确定要添加学生 \"" + name + "\" (ID: " + std::to_string(id) + ") 吗？")) {
        system.addStudent(name, id);
        cout << "学生添加成功！\n";
    } else {
        cout << "已取消添加学生操作。\n";
    }
}

// 学生选课
void StudentMenu::selectCourse(System& system) {
    int studentID = getIntInput("请输入学生ID: ");
    if (studentID == -1) {
        cout << "输入无效！ID不能是负数\n";
        return;
    }

    auto student = system.findStudent(studentID);
    if (!student) {
        cout << "找不到该学生！检查ID是否正确\n";
        return;
    }

    int courseID = getIntInput("请输入课程ID: ");
    if (courseID == -1) {
        cout << "输入无效！ID必须是正整数\n";
        return;
    }

    auto course = system.findCourse(courseID);
    if (!course) {
        cout << "找不到该课程！可能是ID输错了\n";
        return;
    }

    // 防止重复选课
    if (student->hasSelectedCourse(courseID)) {
        cout << "错误：该学生已经选择了这门课程，不能重复选课。\n";
        return;
    }

    if (getConfirmation("确定要为学生 " + student->m_name + " 选择课程 " + course->m_courseName + " 吗？")) {
        student->selectCourse(course);
        cout << "选课成功！学生 " << student->m_name << " 已选择课程 " << course->m_courseName << "。\n";
    } else {
        cout << "已取消选课操作。\n";
    }
}

void StudentMenu::viewScores(System& system) {
    int studentID = getIntInput("输入学生ID: ");
    if (studentID == -1) return;

    auto student = system.findStudent(studentID);
    if (student) {
        student->viewScores();
    } else {
        cout << "未找到学生。\n";
    }
}

void StudentMenu::viewCourses(System& system) {
    int studentID = getIntInput("输入学生ID: ");
    if (studentID == -1) return;

    auto student = system.findStudent(studentID);
    if (student) {
        student->viewCourses();
    } else {
        cout << "未找到学生。\n";
    }
}

// TeacherMenu方法实现
void TeacherMenu::display() {
    cout << "\n--- 教师管理 ---\n";
    cout << "1. 添加教师\n";
    cout << "2. 导入学生成绩\n";
    cout << "3. 查看课程信息\n";
    cout << "4. 返回主菜单\n";
    cout << "请输入您的选择: ";
}

void TeacherMenu::execute(System& system) {
    int choice = getIntInput("");
    switch (choice) {
    case 1:
        addTeacher(system);
        break;
    case 2:
        importScore(system);
        break;
    case 3:
        viewCourses(system);
        break;
    case 4:
        return;
    default:
        cout << "无效选择，请重试。\n";
    }
}

void TeacherMenu::addTeacher(System& system) {
    string name = getStringInput("输入教师姓名: ");
    if (name.empty()) return;

    // 验证教师名称唯一性
    if (system.findTeacher(name)) {
        cout << "错误：教师 \"" << name << "\" 已存在，请使用其他名称。\n";
        return;
    }

    if (getConfirmation("确定要添加教师 \"" + name + "\" 吗？")) {
        system.addTeacher(name);
        cout << "教师添加成功！\n";
    } else {
        cout << "已取消添加教师操作。\n";
    }
}

// 导入学生成绩
void TeacherMenu::importScore(System& system) {
    string teacherName = getStringInput("请输入教师姓名: ");
    auto teacher = system.findTeacher(teacherName);
    if (!teacher) {
        cout << "找不到该教师！请检查姓名是否正确\n";
        return;
    }

    int courseID = getIntInput("请输入课程ID: ");
    if (courseID == -1) {
        cout << "输入无效！课程ID必须是正整数\n";
        return;
    }

    auto course = system.findCourse(courseID);
    if (!course) {
        cout << "找不到该课程！ID可能输错了\n";
        return;
    }

    int studentID = getIntInput("请输入学生ID: ");
    if (studentID == -1) {
        cout << "输入无效！学生ID不能是负数\n";
        return;
    }

    auto student = system.findStudent(studentID);
    if (!student) {
        cout << "找不到该学生！ID可能不存在\n";
        return;
    }

    // 验证学生是否选修了该课程
    if (!course->hasStudent(studentID)) {
        cout << "错误：该学生没有选择这门课程，无法录入成绩。\n";
        return;
    }

    int score = getIntInput("请输入成绩(0-100): ");
    if (score < 0 || score > 100) {
        cout << "错误：成绩必须在0到100之间，请重新输入。\n";
        return;
    }

    if (getConfirmation("确定要为学生 " + student->m_name + " 的课程 " + course->m_courseName + " 导入成绩 " + std::to_string(score) + " 吗？")) {
        teacher->importScore(course, studentID, score);
        cout << "成绩导入成功！\n";
    } else {
        cout << "已取消成绩导入操作。\n";
    }
}

void TeacherMenu::viewCourses(System& system) {
    string teacherName = getStringInput("输入教师姓名: ");
    auto teacher = system.findTeacher(teacherName);
    if (teacher) {
        system.showCoursesForTeacher(*teacher);
    } else {
        cout << "未找到教师。\n";
    }
}

// CourseMenu方法实现
void CourseMenu::display() {
    cout << "\n--- 课程管理 ---\n";
    cout << "1. 添加课程\n";
    cout << "2. 课程统计\n";
    cout << "3. 返回主菜单\n";
    cout << "请输入您的选择: ";
}

void CourseMenu::execute(System& system) {
    int choice = getIntInput("");
    switch (choice) {
    case 1:
        addCourse(system);
        break;
    case 2:
        courseStatistics(system);
        break;
    case 3:
        return;
    default:
        cout << "无效选择，请重试。\n";
    }
}

void CourseMenu::addCourse(System& system) {
    string name = getStringInput("输入课程名称: ");
    if (name.empty()) return;

    int id = getIntInput("输入课程ID: ");
    if (id == -1) return;

    // 验证课程ID唯一性
    if (system.findCourse(id)) {
        cout << "错误：课程ID " << id << " 已存在，请使用其他ID。\n";
        return;
    }

    cin.ignore();
    cout << "输入课程描述: ";
    string desc;
    std::getline(cin, desc);

    // 处理空描述情况
    if (desc.empty()) {
        cout << "警告：课程描述为空。\n";
        if (!getConfirmation("是否继续添加课程？")) {
            cout << "已取消添加课程操作。\n";
            return;
        }
    }

    if (getConfirmation("确定要添加课程 \"" + name + "\" (ID: " + std::to_string(id) + ") 吗？")) {
        system.addCourse(name, id, desc);
        cout << "课程添加成功！\n";
    } else {
        cout << "已取消添加课程操作。\n";
    }
}

void CourseMenu::courseStatistics(System& system) {
    int courseID = getIntInput("输入课程ID: ");
    if (courseID == -1) return;

    auto course = system.findCourse(courseID);
    if (course) {
        course->showStatistics();
    } else {
        cout << "未找到课程。\n";
    }
}

// DataMenu方法实现
void DataMenu::display() {
    cout << "===== 数据管理 =====\n";
    cout << "1. 保存数据\n";
    cout << "2. 加载数据\n";
    cout << "3. 返回主菜单\n";
    cout << "请输入您的选择: ";
}

void DataMenu::execute(System& system) {
    int choice = getIntInput("");
    switch (choice) {
    case 1:
        saveData(system);
        break;
    case 2:
        loadData(system);
        break;
    case 3:
        return;
    default:
        cout << "无效选择，请重试。\n";
    }
}

void DataMenu::loadData(System &system) {
    string dirname = getStringInput("输入要加载的数据目录名称: ");
    if (dirname.empty()) return;

    // 提醒用户数据加载会覆盖当前数据
    if (getConfirmation("确定要从目录 \"" + dirname + "\" 加载数据吗？这将覆盖当前所有数据！")) {
        system.loadData(dirname);
        cout << "数据加载成功！\n";
    } else {
        cout << "已取消加载数据操作。\n";
    }
}

void DataMenu::saveData(System& system) {
    string dirname = getStringInput("输入要保存的数据目录名称: ");
    if (dirname.empty()) return;

    if (getConfirmation("确定要将数据保存到目录 \"" + dirname + "\" 吗？")) {
        system.saveData(dirname);
        cout << "数据保存成功！\n";
    } else {
        cout << "已取消保存数据操作。\n";
    }
}
