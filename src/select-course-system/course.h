#pragma once

#include <string>
#include <vector>

class Course
{
public:
    Course();

private:
    std::string m_courseName;
    int m_courseID;
    std::string m_description;
    //使用vector存储ID而不是对象指针，避免复杂的生命周期管理
    std::vector<int> m_studentIDs;
    //-1表示未评分，避免使用特殊对象表示空值
    std::vector<int> m_scores;
};
