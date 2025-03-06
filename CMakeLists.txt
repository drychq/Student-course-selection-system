cmake_minimum_required(VERSION 3.16)

project(Student-course-selection-system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Student-course-selection-system
    src/main.cpp
    src/student.h src/student.cpp
    src/teacher.h src/teacher.cpp
    src/course.h src/course.cpp
    src/system.h src/system.cpp
    src/menu.h src/menu.cpp
)

include(GNUInstallDirs)
install(TARGETS Student-course-selection-system
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
