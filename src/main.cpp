#include "frontend/cli_app.h"
#include "persistence/file_storage.h"
#include "service/course_selection_service.h"

int main() {
    scs::CourseSelectionService service;
    scs::FileStorage storage;
    scs::CliApp app(service, storage);

    app.run();
    return 0;
}
