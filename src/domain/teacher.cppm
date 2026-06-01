module scs.domain;

import std;

namespace scs {

Teacher::Teacher(std::string name)
    : m_name(std::move(name)) {
}

std::string_view Teacher::name() const noexcept {
    return m_name;
}

} // namespace scs
