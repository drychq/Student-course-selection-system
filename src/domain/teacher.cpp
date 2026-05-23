#include "domain/teacher.h"

#include <utility>

namespace scs {

Teacher::Teacher(std::string name)
    : m_name(std::move(name)) {
}

std::string_view Teacher::name() const noexcept {
    return m_name;
}

} // namespace scs
