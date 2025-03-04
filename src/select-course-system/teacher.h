#pragma once

#include <string>

class Teacher
{
public:
    Teacher();

private:
    std::string m_name;  //使用姓名作为唯一标识，避免引入额外的ID系统
};
