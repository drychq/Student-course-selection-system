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
        //addStudent(system);TODO
        break;
    case 2:
        //selectCourse(system);TODO
        break;
    case 3:
        //viewScores(system);TODO
        break;
    case 4:
        //viewCourses(system);TODO
        break;
    case 5:
        return;
    default:
        cout << "无效选择，请重试。\n";
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
        //addTeacher(system);
        break;
    case 2:
        //importScore(system);
        break;
    case 3:
        //viewCourses(system);
        break;
    case 4:
        return;
    default:
        cout << "无效选择，请重试。\n";
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
        //addCourse(system);TODO
        break;
    case 2:
        //courseStatistics(system);TODO
        break;
    case 3:
        return;
    default:
        cout << "无效选择，请重试。\n";
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
        //saveData(system);TODO
        break;
    case 2:
        //loadData(system);TODO
        break;
    case 3:
        return;
    default:
        cout << "无效选择，请重试。\n";
    }
}


