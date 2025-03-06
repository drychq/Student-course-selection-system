#pragma once

#include "course.h"
#include <memory>
#include <string>

class Teacher
{
public:
    Teacher(const std::string& name);

    //通过Course对象直接管理成绩，避免引入额外的数据结构
    void importScore(std::shared_ptr<Course> course, int studentID, int score);

    //只读接口，用于查看课程信息
    void viewCourses(const std::vector<std::shared_ptr<Course>>& courses) const;

    //直接访问，减少getter
    friend class System;
    friend class Student;

private:
    std::string m_name;  //使用姓名作为唯一标识，避免引入额外的ID系统
};
