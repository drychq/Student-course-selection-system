set_project("Student-course-selection-system")
set_version("1.0.0")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

if is_plat("windows") then
    add_cxxflags("/utf-8")
end

target("student_course_core")
    set_kind("static")
    set_default(false)
    add_files("src/domain/*.cpp")
    add_files("src/service/*.cpp")
    add_files("src/persistence/*.cpp")
    add_includedirs("src", {public = true})

target("Student-course-selection-system")
    set_kind("binary")
    add_deps("student_course_core")
    add_files("src/main.cpp")
    add_files("src/frontend/*.cpp")
    add_includedirs("src")
    add_installfiles("assets/**", {prefixdir = "share/Student-course-selection-system/assets"})

target("student_course_selection_tests")
    set_kind("binary")
    set_default(false)
    add_deps("student_course_core")
    add_files("tests/*.cpp")
    add_includedirs("src")
