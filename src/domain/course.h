#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace scs {

class Course {
public:
    Course(std::string name, int id, std::string description);

    [[nodiscard]] int id() const noexcept;
    [[nodiscard]] std::string_view name() const noexcept;
    [[nodiscard]] std::string_view description() const noexcept;
    [[nodiscard]] const std::map<int, std::optional<int>>& scoreBook() const noexcept;

    [[nodiscard]] bool hasStudent(int studentId) const;
    [[nodiscard]] std::size_t studentCount() const noexcept;
    [[nodiscard]] std::size_t gradedCount() const noexcept;
    [[nodiscard]] double averageScore() const noexcept;
    [[nodiscard]] std::optional<int> highestScore() const noexcept;
    [[nodiscard]] std::optional<int> lowestScore() const noexcept;
    [[nodiscard]] std::optional<int> scoreFor(int studentId) const;

    bool enrollStudent(int studentId);
    bool setScore(int studentId, int score);

private:
    std::string m_name;
    int m_id;
    std::string m_description;
    std::map<int, std::optional<int>> m_scoresByStudentId;
};

} // namespace scs
