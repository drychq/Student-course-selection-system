#pragma once

#include <iostream>
#include <string>
#include <limits>
#include <memory>
#include "system.h"

//前向声明以避免循环依赖
class System;
class Student;
class Course;
class Teacher;

//菜单基类采用模板方法模式，定义了菜单操作的基本框架
class Menu {
public:
    virtual ~Menu() = default;

    //纯虚函数定义接口，子类必须实现
    virtual void display() = 0;
    virtual void execute(System& system) = 0;

protected:
    //输入处理工具函数，处理各种输入异常情况
    void clearInput();
    int getIntInput(const std::string& prompt);
    std::string getStringInput(const std::string& prompt);
    bool getConfirmation(const std::string& prompt);
};

//主菜单作为顶层控制器，管理各个子菜单
class MainMenu : public Menu {
public:
    void display() override;
    void execute(System& system) override;
};

//学生管理菜单，处理所有学生相关操作
class StudentMenu : public Menu {
public:
    void display() override;
    void execute(System& system) override;

private:
    //内部实现函数，每个函数处理一个具体的学生管理任务
    void addStudent(System& system);
    void selectCourse(System& system);
    void viewScores(System& system);
    void viewCourses(System& system);
};

//教师管理菜单，处理所有教师相关操作
class TeacherMenu : public Menu {
public:
    void display() override;
    void execute(System& system) override;

private:
    //内部实现函数，每个函数处理一个具体的教师管理任务
    void addTeacher(System& system);
    void importScore(System& system);
    void viewCourses(System& system);
};

//课程管理菜单，处理所有课程相关操作
class CourseMenu : public Menu {
public:
    void display() override;
    void execute(System& system) override;

private:
    //内部实现函数，每个函数处理一个具体的课程管理任务
    void addCourse(System& system);
    void courseStatistics(System& system);
};

//数据管理菜单，处理系统数据的持久化
class DataMenu : public Menu {
public:
    void display() override;
    void execute(System& system) override;

private:
    //内部实现函数，处理数据的保存和加载
    void saveData(System& system);
    void loadData(System& system);
};
