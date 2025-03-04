cmake_minimum_required(VERSION 3.16)

project(select-course-system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(select-course-system main.cpp
    student.h student.cpp
    teacher.h teacher.cpp
    course.h course.cpp
    system.h system.cpp
    menu.h menu.cpp)

include(GNUInstallDirs)
install(TARGETS select-course-system
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
