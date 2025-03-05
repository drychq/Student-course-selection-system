#pragma once


#include "student.h"
#include "teacher.h"
#include "course.h"
#include "menu.h"

#include <memory>
#include <string>


//前向声明
class Menu;

//系统类作为整个应用的核心控制器
class System
{
public:
    //默认构造函数，初始化系统运行状态
    System() : m_running(true) {}

    //实体管理接口，负责对象的创建和生命周期管理
    void addStudent(const std::string& name, int id);
    void addTeacher(const std::string& name);
    void addCourse(const std::string& name, int id, const std::string& desc);

    //查找接口，O(n)复杂度，返回智能指针避免悬垂引用
    std::shared_ptr<Student> findStudent(int id);
    std::shared_ptr<Teacher> findTeacher(const std::string& name);
    std::shared_ptr<Course> findCourse(int id);

    //用户界面和系统控制
    void userInterface();
    void setRunning(bool running) { m_running = running; }
    void stop() { m_running = false; }
    bool isRunning() const { return m_running; }


    //教师课程管理接口
    void showCoursesForTeacher(const Teacher& teacher);

    //数据访问接口，返回引用以支持修改
    std::vector<std::shared_ptr<Student>>& getStudents() { return m_students; }
    std::vector<std::shared_ptr<Course>>& getCourses() { return m_courses; }

    //工具函数
    int stringToInt(const std::string& str);

private:
    //使用智能指针管理所有实体对象，确保资源自动释放
    std::vector<std::shared_ptr<Student>> m_students;
    std::vector<std::shared_ptr<Teacher>> m_teachers;
    std::vector<std::shared_ptr<Course>> m_courses;
    bool m_running;  //系统运行状态标志

};
