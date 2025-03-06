#include "student.h"
#include <iostream>

using std::string;
using std::shared_ptr;
using std::cout;
using std::endl;

Student::Student(const string& name, int id) : m_name(name), m_studentID(id) {
}

//选课实现双向关联，确保数据一致性
void Student::selectCourse(shared_ptr<Course> course) {
    //检查重复选课，避免数据不一致
    for (const auto& enrolled : m_enrolledCourses) {
        if (enrolled->m_courseID == course->m_courseID) {
            cout << "警告：你已经选过这门课了，不能重复选择！" << endl;
            return;
        }
    }

    //先添加到学生的选课列表
    m_enrolledCourses.push_back(course);

    //再添加到课程的学生列表，保持双向关联
    course->addStudent(m_studentID);

    cout << "选课成功！你已经成功选择了课程：" << course->m_courseName << endl;
}

//O(n)复杂度的查询接口，n为已选课程数
bool Student::hasSelectedCourse(int courseID) const {
    for (const auto& course : m_enrolledCourses) {
        if (course->m_courseID == courseID) {
            return true;
        }
    }
    return false;
}

//格式化显示课程信息，包括课程ID、名称和描述
void Student::viewCourses() const {
    if (m_enrolledCourses.empty()) {
        cout << "学生 " << m_name << " 还没有选择任何课程。" << endl;
        return;
    }

    cout << "学生 " << m_name << " (ID: " << m_studentID << ") 已选课程：" << endl;
    cout << "----------------------------------------" << endl;
    cout << "课程ID\t课程名称\t\t课程描述" << endl;
    cout << "----------------------------------------" << endl;

    for (const auto& course : m_enrolledCourses) {
        cout << course->m_courseID << "\t" << course->m_courseName;

        //根据课程名称长度调整显示格式，保持对齐
        if (course->m_courseName.length() < 8) {
            cout << "\t\t";
        } else {
            cout << "\t";
        }

        cout << course->m_description << endl;
    }
    cout << "----------------------------------------" << endl;
}

//格式化显示成绩信息，处理未评分的特殊情况
void Student::viewScores() const {
    if (m_enrolledCourses.empty()) {
        cout << "学生 " << m_name << " 还没有选择任何课程。" << endl;
        return;
    }

    cout << "学生 " << m_name << " (ID: " << m_studentID << ") 的成绩：" << endl;
    cout << "----------------------------------------" << endl;
    cout << "课程名称\t\t成绩" << endl;
    cout << "----------------------------------------" << endl;

    for (const auto& course : m_enrolledCourses) {
        int score = course->getScore(m_studentID);
        cout << course->m_courseName;

        //根据课程名称长度调整显示格式，保持对齐
        if (course->m_courseName.length() < 8) {
            cout << "\t\t";
        } else {
            cout << "\t";
        }

        cout << (score == -1 ? "未评分" : std::to_string(score)) << endl;
    }
    cout << "----------------------------------------" << endl;
}
