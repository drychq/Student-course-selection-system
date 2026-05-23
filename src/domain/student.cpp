#include "domain/student.h"

#include <utility>

namespace scs {

Student::Student(std::string name, int id)
    : m_name(std::move(name)), m_id(id) {
}

int Student::id() const noexcept {
    return m_id;
}

std::string_view Student::name() const noexcept {
    return m_name;
}

const std::set<int>& Student::enrolledCourseIds() const noexcept {
    return m_enrolledCourseIds;
}

bool Student::hasSelectedCourse(int courseId) const {
    return m_enrolledCourseIds.contains(courseId);
}

bool Student::enroll(int courseId) {
    return m_enrolledCourseIds.insert(courseId).second;
}

} // namespace scs
