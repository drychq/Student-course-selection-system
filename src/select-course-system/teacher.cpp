#include "teacher.h"

using std::string;
using std::shared_ptr;


Teacher::Teacher(const string& name) : m_name(name) {
}

//通过Course对象直接管理成绩，避免引入额外的数据结构
void Teacher::importScore(shared_ptr<Course> course, int studentID, int score) {
    course->importScore(studentID, score);
}
