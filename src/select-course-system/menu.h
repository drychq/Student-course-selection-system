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

