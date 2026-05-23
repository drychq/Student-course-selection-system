#pragma once

#include <set>
#include <string>
#include <string_view>

namespace scs {

class Student {
public:
    Student(std::string name, int id);

    [[nodiscard]] int id() const noexcept;
    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] const std::set<int>& enrolledCourseIds() const noexcept;
    [[nodiscard]] bool hasSelectedCourse(int courseId) const;

    bool enroll(int courseId);

private:
    std::string m_name;
    int m_id;
    std::set<int> m_enrolledCourseIds;
};

} // namespace scs
