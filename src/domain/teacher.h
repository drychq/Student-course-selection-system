#pragma once

#include <string>
#include <string_view>

namespace scs {

class Teacher {
public:
    explicit Teacher(std::string name);

    [[nodiscard]] std::string_view name() const noexcept;

private:
    std::string m_name;
};

} // namespace scs
