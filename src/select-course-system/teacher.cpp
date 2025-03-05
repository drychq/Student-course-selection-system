#include "teacher.h"
#include <iostream>

using std::string;
using std::shared_ptr;
using std::cout;
using std::endl;


Teacher::Teacher(const string& name) : m_name(name) {
}

//通过Course对象直接管理成绩，避免引入额外的数据结构
void Teacher::importScore(shared_ptr<Course> course, int studentID, int score) {
    course->importScore(studentID, score);
}

void Teacher::viewCourses(const std::vector<std::shared_ptr<Course> > &courses) const{
    if (courses.empty()) {
        cout << "当前没有任何课程。" << endl;
        return;
    }

    cout << "所有课程信息：" << endl;
    cout << "----------------------------------------" << endl;
    cout << "课程ID\t课程名称\t\t课程描述\t\t选课人数" << endl;
    cout << "----------------------------------------" << endl;

    for (const auto& course : courses) {
        cout << course->m_courseID << "\t" << course->m_courseName;

        //根据课程名称长度调整显示格式，保持对齐
        if (course->m_courseName.length() < 8) {
            cout << "\t\t";
        } else {
            cout << "\t";
        }

        cout << course->m_description;

        //根据课程描述长度调整显示格式，保持对齐
        if (course->m_description.length() < 8) {
            cout << "\t\t";
        } else if (course->m_description.length() < 16) {
            cout << "\t";
        } else {
            cout << "\t";
        }

        cout << course->getStudentCount() << endl;
    }
    cout << "----------------------------------------" << endl;
}
