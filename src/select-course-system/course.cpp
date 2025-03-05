#include "course.h"

using std::string;

Course::Course(const string& name, int id, const string& desc)
    : m_courseName(name), m_courseID(id), m_description(desc) {
}

