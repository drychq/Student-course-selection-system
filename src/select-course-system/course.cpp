#include "course.h"
#include <iostream>

using std::string;
using std::cout;
using std::endl;

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

//O(n)复杂度的查询接口，n为选课人数
bool Course::hasStudent(int studentID) const {
    return findStudentIndex(studentID) != -1;
}

//线性查找学生ID，因为选课人数通常较少，复杂度可接受
int Course::findStudentIndex(int studentID) const {
    for (int i = 0; i < m_studentIDs.size(); ++i) {
        if (m_studentIDs[i] == studentID) {
            return i;
        }
    }
    return -1;
}

//成绩导入时进行合法性验证，确保数据一致性
void Course::importScore(int studentID, int score) {
    //成绩范围检查
    if (score < 0 || score > 100) {
        cout << "错误：成绩必须在0到100之间！" << endl;
        return;
    }

    int index = findStudentIndex(studentID);

    if (index != -1) {
        //提示成绩覆盖，避免误操作
        if (m_scores[index] != -1) {
            cout << "警告：学生ID " << studentID << " 在课程 \"" << m_courseName
                 << "\" 中的成绩将被覆盖。" << endl;
        }

        m_scores[index] = score;
        cout << "成功导入学生ID " << studentID << " 在课程 \"" << m_courseName
             << "\" 中的成绩：" << score << endl;
    } else {
        cout << "错误：学生ID " << studentID << " 未选择课程 \"" << m_courseName
             << "\"，无法导入成绩。" << endl;
    }
}

//成绩查询接口，未找到返回-1表示未评分
int Course::getScore(int studentID) const {
    int index = findStudentIndex(studentID);
    return index != -1 ? m_scores[index] : -1;
}
