#pragma once

#include <string>
#include <vector>

class Course
{
public:
    Course(const std::string& name, int id, const std::string& desc);

    //通过ID而不是对象指针管理学生，避免shared_ptr循环引用导致内存泄漏
    void addStudent(int studentID);
    void importScore(int studentID, int score);
    int getScore(int studentID) const;

    //提供统计功能以支持教学质量分析
    void showStatistics() const;

    //快速查询接口，O(1)复杂度
    bool hasStudent(int studentID) const;
    size_t getStudentCount() const;

    //直接访问，减少getter,但过多friend破坏了封装性，后续考虑重构
    friend class Student;
    friend class System;
    friend class Teacher;
    friend class StudentMenu;
    friend class TeacherMenu;

private:
    std::string m_courseName;
    int m_courseID;
    std::string m_description;
    //使用vector存储ID而不是对象指针，避免复杂的生命周期管理
    std::vector<int> m_studentIDs;
    //-1表示未评分，避免使用特殊对象表示空值
    std::vector<int> m_scores;

    //内部工具函数
    int findStudentIndex(int studentID) const;

    //统计相关函数，支持成绩分析功能
    int calculateGradedCount() const;
    double calculateAverageScore() const;
    int calculateHighestScore() const;
    int calculateLowestScore() const;
};
