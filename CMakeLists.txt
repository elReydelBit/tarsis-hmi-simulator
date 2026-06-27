#===========================================
# Block 1: Project identity and C++ standard
#===========================================

# This line says: "I need CMake version 3.16 or newer".
# If your CMake is older, the build will stop with an error.
cmake_minimum_required(VERSION 3.16)

# This gives our project a name. CMake uses this name
# internally, for example in build messages.
project(tarsis-hmi-simulator)

# This tells the compiler: "use C++17 rules".
# We need C++17 because of features like make_shared,
# weak_ptr::lock(), and structured code we use later.
set(CMAKE_CXX_STANDARD 17)

# This means: C++17 is NOT optional. If the compiler
# cannot use C++17, stop the build instead of using
# an older standard silently.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This creates a file called "compile_commands.json".
# Tools like VSCode use this file to understand your
# project (autocomplete, error checking, etc).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#===========================================
# Block 2: Find Qt6 and required modules
#===========================================

# This searches for Qt6 on the system.
# "REQUIRED" means: if Qt6 is missing, stop with an error.
# "COMPONENTS Widgets Network" means: we need two specific
# parts of Qt6 -> Widgets (buttons, windows, UI) and
# Network (TCP/UDP sockets).
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

#=========================================================
# Block 3: Enable Qt's meta-object compiler (moc, uic,rcc)
#=========================================================

# Qt uses special tools to process its own code
# (signals, slots, .ui files). These three lines turn
# those tools ON automatically, so we don't run them
# by hand every time.

# AUTOMOC: handles signals and slots (Q_OBJECT macro).
set(CMAKE_AUTOMOC ON)

# AUTOUIC: handles .ui files (visual forms made in Qt Designer).
set(CMAKE_AUTOUIC ON)

# AUTORCC: handles .qrc files (resources, like icons or images).
set(CMAKE_AUTORCC ON)



#=========================================================
# Block 4: Define the executable and link Qt
#=========================================================

# This creates our actual program (the .exe / binary file).
# "tarsis-hmi-simulator" is the program's name.
# The files after it are the source files we compile together.
add_executable(tarsis-hmi-simulator src/main.cpp src/mainwindow.cpp src/mainwindow.h  src/mosquittomqttpublisher.cpp  src/mosquittomqttpublisher.cpp)

# This connects (links) our program to the Qt libraries we
# found in Block 2. Without this line, the code would
# compile but FAIL at the linking step (undefined references).
# "PRIVATE" means: only THIS target uses these libraries,
# not anything else that might depend on it later.
target_link_libraries(tarsis-hmi-simulator PRIVATE Qt6::Widgets  Qt6::Network)

#=========================================================
# Block 5: Find and link libmosquitto (for MQTT)
#=========================================================

# PkgConfig is a tool. It helps CMake find libraries
# that are already installed on the Linux system.
find_package(PkgConfig REQUIRED)

# This line searches for "libmosquitto" using PkgConfig.
# If found, it gives us two things automatically:
#   MOSQUITTO_INCLUDE_DIRS -> where mosquitto.h is
#   MOSQUITTO_LIBRARIES    -> the actual library to link
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

# Now we tell our executable: "use these mosquitto files".
# This is the SAME target we already defined in Block 4.
target_include_directories(tarsis-hmi-simulator PRIVATE ${MOSQUITTO_INCLUDE_DIRS})
target_link_libraries(tarsis-hmi-simulator PRIVATE ${MOSQUITTO_LIBRARIES})