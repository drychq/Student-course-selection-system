#include "system.h"

using std::string;
using std::make_shared;
using std::cerr;
using std::cout;
using std::endl;
using std::shared_ptr;

//创建主菜单并进入事件循环
void System::userInterface() {
    MainMenu mainMenu;
    while (m_running) {
        mainMenu.display();
        mainMenu.execute(*this);
    }
}

//添加学生时检查ID唯一性，确保数据一致性
void System::addStudent(const string& name, int id) {
    for (const auto& student : m_students) {
        if (student->m_studentID == id) {
            cerr << "错误：学生ID " << id << " 已存在，不能重复添加。" << endl;
            return;
        }
    }

    auto newStudent = make_shared<Student>(name, id);
    m_students.push_back(newStudent);

    cout << "成功添加学生：" << name << "（ID: " << id << "）" << endl;
}

//添加课程时检查ID唯一性，确保数据一致性
void System::addCourse(const string& name, int id, const string& desc) {
    for (const auto& course : m_courses) {
        if (course->m_courseID == id) {
            cerr << "错误：课程ID " << id << " 已存在，无法添加重复ID的课程。" << endl;
            return;
        }
    }

    auto course = make_shared<Course>(name, id, desc);
    m_courses.push_back(course);

    cout << "成功添加课程：" << name << "（ID: " << id << "）" << endl;
}

//O(n)复杂度的查找接口，返回智能指针避免悬垂引用
shared_ptr<Student> System::findStudent(int id) {
    for (const auto& student : m_students) {
        if (student->m_studentID == id) {
            return student;
        }
    }
    return nullptr;
}

//O(n)复杂度的查找接口，返回智能指针避免悬垂引用
shared_ptr<Course> System::findCourse(int id) {
    for (const auto& course : m_courses) {
        if (course->m_courseID == id) {
            return course;
        }
    }
    return nullptr;
}
