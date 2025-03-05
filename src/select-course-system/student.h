#pragma once

#include "course.h"

#include <string>
#include <vector>
#include <memory>

class Student
{
public:
    Student(const std::string& name, int id);

    //使用shared_ptr管理课程对象，确保课程在被选时不会被销毁
    void selectCourse(std::shared_ptr<Course> course);

    //提供只读接口以保护数据一致性
    void viewScores() const;

    // 显示学生已选课程列表
    void viewCourses() const;

    //O(n)复杂度的查询接口，n为已选课程数
    bool hasSelectedCourse(int courseID) const;

    // 返回已选课程数量，用于限制选课上限
    size_t getEnrolledCourseCount() const;

    // 直接访问，减少getter,但过多friend破坏了封装性，后续考虑重构
    friend class System;
    friend class Course;
    friend class Teacher;
    friend class StudentMenu;
    friend class TeacherMenu;


private:
    std::string m_name;
    int m_studentID;
    //使用shared_ptr确保课程对象在被引用时不会被销毁
    std::vector<std::shared_ptr<Course>> m_enrolledCourses;
};
