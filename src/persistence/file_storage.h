#pragma once

#include "service/app_error.h"
#include "service/course_selection_service.h"

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace scs {

class FileStorage {
public:
    std::expected<void, AppError> save(const CourseSelectionService& service, const std::filesystem::path& directory) const;
    std::expected<void, AppError> load(CourseSelectionService& service, const std::filesystem::path& directory) const;

private:
    static std::vector<std::string> splitCsvLine(std::string_view line);
    static std::string_view trim(std::string_view value);
    static std::expected<int, AppError> parseInt(std::string_view value);
    static AppError makeFileError(const std::filesystem::path& file, std::string_view operation);
};

} // namespace scs
