#pragma once

#include "course.h"

#include <string>
#include <vector>
#include <memory>

class Student
{
public:
    Student();

private:
    std::string m_name;
    int m_studentID;
    //使用shared_ptr确保课程对象在被引用时不会被销毁
    std::vector<std::shared_ptr<Course>> m_enrolledCourses;
};
