#include "domain/course.h"

#include <algorithm>
#include <utility>

namespace scs {

Course::Course(std::string name, int id, std::string description)
    : m_name(std::move(name)),
      m_id(id),
      m_description(std::move(description)) {
}

int Course::id() const noexcept {
    return m_id;
}

std::string_view Course::name() const noexcept {
    return m_name;
}

std::string_view Course::description() const noexcept {
    return m_description;
}

const std::map<int, std::optional<int>>& Course::scoreBook() const noexcept {
    return m_scoresByStudentId;
}

bool Course::hasStudent(int studentId) const {
    return m_scoresByStudentId.contains(studentId);
}

std::size_t Course::studentCount() const noexcept {
    return m_scoresByStudentId.size();
}

std::size_t Course::gradedCount() const noexcept {
    return static_cast<std::size_t>(std::ranges::count_if(
        m_scoresByStudentId,
        [](const auto& entry) { return entry.second.has_value(); }));
}

double Course::averageScore() const noexcept {
    auto count = gradedCount();
    if (count == 0) {
        return 0.0;
    }

    double sum = 0.0;
    for (const auto& [_, score] : m_scoresByStudentId) {
        if (score.has_value()) {
            sum += *score;
        }
    }
    return sum / static_cast<double>(count);
}

std::optional<int> Course::highestScore() const noexcept {
    std::optional<int> highest;
    for (const auto& [_, score] : m_scoresByStudentId) {
        if (score.has_value() && (!highest.has_value() || *score > *highest)) {
            highest = score;
        }
    }
    return highest;
}

std::optional<int> Course::lowestScore() const noexcept {
    std::optional<int> lowest;
    for (const auto& [_, score] : m_scoresByStudentId) {
        if (score.has_value() && (!lowest.has_value() || *score < *lowest)) {
            lowest = score;
        }
    }
    return lowest;
}

std::optional<int> Course::scoreFor(int studentId) const {
    auto it = m_scoresByStudentId.find(studentId);
    if (it == m_scoresByStudentId.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool Course::enrollStudent(int studentId) {
    return m_scoresByStudentId.emplace(studentId, std::nullopt).second;
}

bool Course::setScore(int studentId, int score) {
    auto it = m_scoresByStudentId.find(studentId);
    if (it == m_scoresByStudentId.end()) {
        return false;
    }
    it->second = score;
    return true;
}

} // namespace scs
