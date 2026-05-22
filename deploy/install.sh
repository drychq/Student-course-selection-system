#!/bin/bash


INSTALL_PATH="/usr/local/StudentCourseSystem"
ICON_PATH="/usr/local/share/icons"
DESKTOP_FILE="/usr/share/applications/student_course_system.desktop"


echo "Creating installation directories..."
sudo mkdir -p $INSTALL_PATH
sudo mkdir -p $ICON_PATH


echo "Building the project..."
mkdir -p build
cd build

cmake .. || { echo "CMake failed"; exit 1; }

make || { echo "Build failed"; exit 1; }

cd ..


echo "Copying executable and resources..."
sudo cp build/Student-course-selection-system $INSTALL_PATH
sudo cp assets/选课.png $ICON_PATH/student_course_system.png


echo "Deploying .desktop file..."
sudo cp deploy/student_course_system.desktop $DESKTOP_FILE

sudo sed -i "s|{INSTALL_PATH}|$INSTALL_PATH|g" $DESKTOP_FILE

echo "Installation completed! You can now find 'Student Course System' in the system menu."

