#pragma once

#include "student.h"
#include "teacher.h"

class system
{
public:
    //默认构造函数，初始化系统运行状态
    system() : m_running(true) {}

private:
    //使用智能指针管理所有实体对象，确保资源自动释放
    std::vector<std::shared_ptr<Student>> m_students;
    std::vector<std::shared_ptr<Teacher>> m_teachers;
    std::vector<std::shared_ptr<Course>> m_courses;
    bool m_running;  //系统运行状态标志
};
