#include "persistence/file_storage.h"
#include "service/app_error.h"
#include "service/course_selection_service.h"

#include <cassert>
#include <filesystem>

using scs::AppErrorCode;
using scs::CourseSelectionService;
using scs::FileStorage;

namespace {

void assertErrorCode(const auto& result, AppErrorCode code) {
    assert(!result);
    assert(result.error().code == code);
}

void testEntityManagement() {
    CourseSelectionService service;

    assert(service.addStudent("Alice", 1));
    assert(service.addTeacher("ProfLi"));
    assert(service.addCourse("Math", 101, "Algebra"));

    assertErrorCode(service.addStudent("Bob", 1), AppErrorCode::StudentAlreadyExists);
    assertErrorCode(service.addTeacher("ProfLi"), AppErrorCode::TeacherAlreadyExists);
    assertErrorCode(service.addCourse("Physics", 101, "Mechanics"), AppErrorCode::CourseAlreadyExists);
}

void testEnrollmentAndScores() {
    CourseSelectionService service;
    assert(service.addStudent("Alice", 1));
    assert(service.addTeacher("ProfLi"));
    assert(service.addCourse("Math", 101, "Algebra"));
    assert(service.addCourse("Physics", 102, "Mechanics"));

    assert(service.selectCourse(1, 101));
    assertErrorCode(service.selectCourse(1, 101), AppErrorCode::CourseAlreadySelected);
    assertErrorCode(service.selectCourse(2, 101), AppErrorCode::StudentNotFound);
    assertErrorCode(service.selectCourse(1, 999), AppErrorCode::CourseNotFound);

    assert(service.importScore("ProfLi", 101, 1, 95));
    assertErrorCode(service.importScore("ProfLi", 102, 1, 88), AppErrorCode::CourseNotSelected);
    assertErrorCode(service.importScore("ProfLi", 101, 1, 101), AppErrorCode::InvalidScore);
    assertErrorCode(service.importScore("Missing", 101, 1, 90), AppErrorCode::TeacherNotFound);

    auto scores = service.studentScores(1);
    assert(scores);
    assert(scores->scores.size() == 1);
    assert(scores->scores[0].score == 95);
}

void testCourseStatistics() {
    CourseSelectionService service;
    assert(service.addStudent("Alice", 1));
    assert(service.addStudent("Bob", 2));
    assert(service.addTeacher("ProfLi"));
    assert(service.addCourse("Math", 101, "Algebra"));
    assert(service.selectCourse(1, 101));
    assert(service.selectCourse(2, 101));
    assert(service.importScore("ProfLi", 101, 1, 80));
    assert(service.importScore("ProfLi", 101, 2, 100));

    auto stats = service.courseStatistics(101);
    assert(stats);
    assert(stats->enrolledCount == 2);
    assert(stats->gradedCount == 2);
    assert(stats->averageScore == 90.0);
    assert(stats->highestScore == 100);
    assert(stats->lowestScore == 80);
}

void testPersistenceRoundTrip() {
    CourseSelectionService service;
    FileStorage storage;
    assert(service.addStudent("Alice", 1));
    assert(service.addTeacher("ProfLi"));
    assert(service.addCourse("Math", 101, "Algebra"));
    assert(service.selectCourse(1, 101));
    assert(service.importScore("ProfLi", 101, 1, 93));

    auto directory = std::filesystem::current_path() / "build" / "service-test-data";
    std::filesystem::remove_all(directory);
    assert(storage.save(service, directory));

    CourseSelectionService loaded;
    assert(storage.load(loaded, directory));
    auto student = loaded.findStudent(1);
    auto teacher = loaded.findTeacher("ProfLi");
    auto course = loaded.findCourse(101);
    assert(student);
    assert(teacher);
    assert(course);

    auto courses = loaded.studentCourses(1);
    assert(courses);
    assert(courses->courses.size() == 1);

    auto scores = loaded.studentScores(1);
    assert(scores);
    assert(scores->scores.size() == 1);
    assert(scores->scores[0].score == 93);

    std::filesystem::remove_all(directory);
}

} // namespace

int main() {
    testEntityManagement();
    testEnrollmentAndScores();
    testCourseStatistics();
    testPersistenceRoundTrip();
    return 0;
}
