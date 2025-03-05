#include "course.h"

using std::string;

Course::Course(const string& name, int id, const string& desc)
    : m_courseName(name), m_courseID(id), m_description(desc) {
}

//通过ID管理学生选课关系，避免对象间的循环引用
void Course::addStudent(int studentID) {
    //使用线性查找，因为选课人数通常较少，不需要额外的数据结构
    for (int i = 0; i < m_studentIDs.size(); ++i) {
        if (m_studentIDs[i] == studentID) {
            return;
        }
    }

    //成绩初始化为-1表示未评分，避免使用特殊对象或额外的标志位
    m_studentIDs.push_back(studentID);
    m_scores.push_back(-1);
}
