#pragma once

#include <string>
#include <string_view>

namespace scs {

enum class AppErrorCode {
    EmptyName,
    InvalidId,
    InvalidScore,
    StudentAlreadyExists,
    TeacherAlreadyExists,
    CourseAlreadyExists,
    StudentNotFound,
    TeacherNotFound,
    CourseNotFound,
    CourseAlreadySelected,
    CourseNotSelected,
    FileOpenFailed,
    InvalidData
};

struct AppError {
    AppErrorCode code;
    std::string message;
};

inline std::string_view toString(AppErrorCode code) noexcept {
    switch (code) {
    case AppErrorCode::EmptyName:
        return "EmptyName";
    case AppErrorCode::InvalidId:
        return "InvalidId";
    case AppErrorCode::InvalidScore:
        return "InvalidScore";
    case AppErrorCode::StudentAlreadyExists:
        return "StudentAlreadyExists";
    case AppErrorCode::TeacherAlreadyExists:
        return "TeacherAlreadyExists";
    case AppErrorCode::CourseAlreadyExists:
        return "CourseAlreadyExists";
    case AppErrorCode::StudentNotFound:
        return "StudentNotFound";
    case AppErrorCode::TeacherNotFound:
        return "TeacherNotFound";
    case AppErrorCode::CourseNotFound:
        return "CourseNotFound";
    case AppErrorCode::CourseAlreadySelected:
        return "CourseAlreadySelected";
    case AppErrorCode::CourseNotSelected:
        return "CourseNotSelected";
    case AppErrorCode::FileOpenFailed:
        return "FileOpenFailed";
    case AppErrorCode::InvalidData:
        return "InvalidData";
    }
    return "Unknown";
}

} // namespace scs
