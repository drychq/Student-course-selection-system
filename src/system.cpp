
#include "system.h"

using std::string;
using std::make_shared;
using std::cerr;
using std::cout;
using std::endl;
using std::shared_ptr;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::filesystem::path;
using std::filesystem::create_directories;
using std::filesystem::exists;

//数据文件名常量，便于统一管理
inline const std::string studentsFileName = "/students.txt";
inline const std::string teachersFileName = "/teachers.txt";
inline const std::string coursesFileName = "/courses.txt";
inline const std::string studentCoursesFileName = "/student_courses.txt";
inline const std::string scoresFileName = "/scores.txt";



//创建主菜单并进入事件循环
void System::userInterface() {
    MainMenu mainMenu;
    while (m_running) {
        mainMenu.display();
        mainMenu.execute(*this);
    }
}

//添加学生时检查ID唯一性，确保数据一致性
void System::addStudent(const string& name, int id) {
    for (const auto& student : m_students) {
        if (student->m_studentID == id) {
            cerr << "错误：学生ID " << id << " 已存在，不能重复添加。" << endl;
            return;
        }
    }

    auto newStudent = make_shared<Student>(name, id);
    m_students.push_back(newStudent);

    cout << "成功添加学生：" << name << "（ID: " << id << "）" << endl;
}

//添加课程时检查ID唯一性，确保数据一致性
void System::addCourse(const string& name, int id, const string& desc) {
    for (const auto& course : m_courses) {
        if (course->m_courseID == id) {
            cerr << "错误：课程ID " << id << " 已存在，无法添加重复ID的课程。" << endl;
            return;
        }
    }

    auto course = make_shared<Course>(name, id, desc);
    m_courses.push_back(course);

    cout << "成功添加课程：" << name << "（ID: " << id << "）" << endl;
}

//添加教师时检查姓名唯一性，避免混淆
void System::addTeacher(const string& name) {
    for (const auto& teacher : m_teachers) {
        if (teacher->m_name == name) {
            cerr << "错误：教师 \"" << name << "\" 已存在，无法添加同名教师。" << endl;
            return;
        }
    }

    auto newTeacher = make_shared<Teacher>(name);
    m_teachers.push_back(newTeacher);
    cout << "成功添加教师：" << name << endl;
}

//O(n)复杂度的查找接口，返回智能指针避免悬垂引用
shared_ptr<Student> System::findStudent(int id) {
    for (const auto& student : m_students) {
        if (student->m_studentID == id) {
            return student;
        }
    }
    return nullptr;
}

//O(n)复杂度的查找接口，返回智能指针避免悬垂引用
shared_ptr<Course> System::findCourse(int id) {
    for (const auto& course : m_courses) {
        if (course->m_courseID == id) {
            return course;
        }
    }
    return nullptr;
}

//O(n)复杂度的查找接口，使用姓名作为唯一标识
shared_ptr<Teacher> System::findTeacher(const string& name) {
    for (const auto& teacher : m_teachers) {
        if (teacher->m_name == name) {
            return teacher;
        }
    }
    return nullptr;
}

//委托给Teacher类处理课程展示
void System::showCoursesForTeacher(const Teacher& teacher) {
    teacher.viewCourses(m_courses);
}

//确保数据目录存在，创建不存在的目录
void System::ensureDirectoryExists(const string& dirname) {
    path dir(dirname);
    if (!exists(dir)) {
        create_directories(dir);
        cout << "创建目录：" << dirname << endl;
    }
}

//使用逗号分隔字符串，支持数据的序列化和反序列化
vector<string> System::parseLine(const string& line) {
    vector<string> result;
    stringstream ss(line);
    string item;

    while (std::getline(ss, item, ',')) {
        result.push_back(item);
    }

    return result;
}

//字符串转整数，处理转换异常
int System::stringToInt(const string& str) {
    int result = 0;
    stringstream ss(str);

    if (!(ss >> result)) {
        cerr << "错误：无法将字符串 \"" << str << "\" 转换为整数！" << endl;
        return 0;
    }

    char remaining;
    if (ss >> remaining) {
        cerr << "警告：字符串 \"" << str << "\" 包含非数字字符，只转换了开头的数字部分。" << endl;
    }

    return result;
}

//文件打开失败时返回false，允许调用者处理错误
bool System::openFileForWrite(ofstream& ofs, const string& filename) {
    ofs.open(filename);
    if (!ofs) {
        cerr << "无法打开文件进行写入: " << filename << endl;
        return false;
    }
    return true;
}

//文件打开失败时返回false，但不中断程序执行
bool System::openFileForRead(ifstream& ifs, const string& filename) {
    ifs.open(filename);
    if (!ifs) {
        cerr << "警告：无法打开文件进行读取: " << filename << endl;
        return false;
    }
    return true;
}

//保存学生基本信息，格式：姓名,ID
void System::saveStudentData(const string& filename) {
    ofstream ofs;
    if (!openFileForWrite(ofs, filename)) return;

    for (const auto& student : m_students) {
        ofs << student->m_name << "," << student->m_studentID << endl;
    }
    ofs.close();
    cout << "学生数据已保存到 " << filename << endl;
}

//保存教师信息，每行一个姓名
void System::saveTeacherData(const string& filename) {
    ofstream ofs;
    if (!openFileForWrite(ofs, filename)) return;

    for (const auto& teacher : m_teachers) {
        ofs << teacher->m_name << endl;
    }
    ofs.close();
    cout << "教师数据已保存到 " << filename << endl;
}

//保存课程信息，格式：名称,ID,描述
void System::saveCourseData(const std::string& filename) {
    ofstream ofs;
    if (!openFileForWrite(ofs, filename)) return;

    for (const auto& course : m_courses) {
        ofs << course->m_courseName << ","
            << course->m_courseID << ","
            << course->m_description << endl;
    }
    ofs.close();
    cout << "课程数据已保存到 " << filename << endl;
}

//保存选课关系，格式：学生ID,课程ID
void System::saveStudentCourseData(const string& filename) {
    ofstream ofs;
    if (!openFileForWrite(ofs, filename)) return;

    for (const auto& student : m_students) {
        for (const auto& course : student->m_enrolledCourses) {
            ofs << student->m_studentID << "," << course->m_courseID << endl;
        }
    }
    ofs.close();
    cout << "学生选课数据已保存到 " << filename << endl;
}

//保存成绩信息，格式：课程ID,学生ID,成绩
void System::saveScoreData(const string& filename) {
    ofstream ofs;
    if (!openFileForWrite(ofs, filename)) return;

    for (const auto& course : m_courses) {
        for (const auto& student : m_students) {
            int score = course->getScore(student->m_studentID);
            if (score >= 0) {
                ofs << course->m_courseID << ","
                    << student->m_studentID << ","
                    << score << endl;
            }
        }
    }
    ofs.close();
    cout << "成绩数据已保存到 " << filename << endl;
}

//加载学生数据，忽略空行和格式错误的行
void System::loadStudentData(const string& filename) {
    ifstream ifs;
    if (!openFileForRead(ifs, filename)) return;

    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;

        vector<string> fields = parseLine(line);
        if (fields.size() >= 2) {
            string name = fields[0];
            int id = stringToInt(fields[1]);
            addStudent(name, id);
        }
    }
    ifs.close();
    cout << "已加载学生数据" << endl;
}

//加载教师数据，忽略空行
void System::loadTeacherData(const string& filename) {
    ifstream ifs;
    if (!openFileForRead(ifs, filename)) return;

    string line;
    while (getline(ifs, line)) {
        if (!line.empty()) {
            addTeacher(line);
        }
    }
    ifs.close();
    cout << "已加载教师数据" << endl;
}

//加载课程数据，忽略空行和格式错误的行
void System::loadCourseData(const string& filename) {
    ifstream ifs;
    if (!openFileForRead(ifs, filename)) return;

    string line;
    while (getline(ifs, line)) {
        if (line.empty()) continue;

        vector<string> fields = parseLine(line);
        if (fields.size() >= 3) {
            string name = fields[0];
            int id = stringToInt(fields[1]);
            string desc = fields[2];
            addCourse(name, id, desc);
        }
    }
    ifs.close();
    cout << "已加载课程数据" << endl;
}

//加载选课关系，重建学生和课程的双向关联
void System::loadStudentCourseData(const string& filename) {
    ifstream ifs;
    if (!openFileForRead(ifs, filename)) return;

    string line;
    int successCount = 0;

    while (getline(ifs, line)) {
        if (line.empty()) continue;

        vector<string> fields = parseLine(line);
        if (fields.size() >= 2) {
            int studentID = stringToInt(fields[0]);
            int courseID = stringToInt(fields[1]);

            auto student = findStudent(studentID);
            auto course = findCourse(courseID);

            if (student && course) {
                student->selectCourse(course);
                successCount++;
            }
        }
    }
    ifs.close();
    cout << "已加载 " << successCount << " 条选课数据" << endl;
}

//加载成绩数据，处理格式错误和异常情况
void System::loadScoreData(const string& filename) {
    ifstream ifs;
    if (!openFileForRead(ifs, filename)) return;

    string line;
    int successCount = 0;

    while (getline(ifs, line)) {
        if (line.empty()) continue;

        vector<string> fields = parseLine(line);
        if (fields.size() >= 3) {
            try {
                int courseID = stringToInt(fields[0]);
                int studentID = stringToInt(fields[1]);
                int score = stringToInt(fields[2]);

                auto course = findCourse(courseID);
                if (course) {
                    course->importScore(studentID, score);
                    successCount++;
                }
            } catch (const std::exception& e) {
                cerr << "错误：解析成绩数据时出错：" << e.what() << endl;
            }
        }
    }
    ifs.close();
    cout << "已加载 " << successCount << " 条成绩数据" << endl;
}

//保存所有数据到指定目录
void System::saveData(const string& dirname) {
    ensureDirectoryExists(dirname);

    saveStudentData(dirname + studentsFileName);
    saveTeacherData(dirname + teachersFileName);
    saveCourseData(dirname + coursesFileName);
    saveStudentCourseData(dirname + studentCoursesFileName);
    saveScoreData(dirname + scoresFileName);

    cout << "所有数据已成功保存到 " << dirname << " 目录" << endl;
}

//加载所有数据，先清空现有数据
void System::loadData(const string& dirname) {
    m_students.clear();
    m_teachers.clear();
    m_courses.clear();

    loadStudentData(dirname + studentsFileName);
    loadTeacherData(dirname + teachersFileName);
    loadCourseData(dirname + coursesFileName);
    loadStudentCourseData(dirname + studentCoursesFileName);
    loadScoreData(dirname + scoresFileName);

    cout << "数据加载完成" << endl;
}
