set_project("Student-course-selection-system")
set_version("2.0.0")
set_languages("c++latest")
set_warnings("allextra")

add_rules("mode.debug", "mode.release")
add_requires("sqlite3 3.53.0+0")
add_requires("sqlitecpp 3.3.3", {configs = {sqlite3_external = true}})

set_policy("build.c++.modules", true)
set_policy("build.c++.modules.std", true)

if is_plat("windows") then
    add_cxxflags("/utf-8")
end

target("student_course_core")
    set_kind("static")
    set_default(false)
    add_files("src/domain/domain.cppm", {public = true})
    add_files("src/domain/course.cppm", "src/domain/student.cppm", "src/domain/teacher.cppm")
    add_files("src/persistence/sqlite_storage.cppm", {public = true})
    add_files("src/persistence/sqlite_storage_impl.cppm")
    add_files("src/service/service.cppm", {public = true})
    add_files("src/service/course_selection_service.cppm")
    add_packages("sqlitecpp", "sqlite3")

target("Student-course-selection-system")
    set_kind("binary")
    add_deps("student_course_core")
    add_files("src/frontend/*.cppm")
    add_files("src/main.cppm")
    add_installfiles("assets/**", {prefixdir = "share/Student-course-selection-system/assets"})

target("student_course_selection_tests")
    set_kind("binary")
    set_default(false)
    add_deps("student_course_core")
    add_files("tests/*.cppm")
