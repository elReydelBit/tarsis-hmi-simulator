#===========================================
# Block 1: Project identity and C++ standard
#===========================================

cmake_minimum_required(VERSION 3.16)
project(tarsis-hmi-simulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#===========================================
# Block 2: Find Qt6 and required modules
#===========================================

find_package(Qt6 REQUIRED COMPONENTS Widgets)

#=========================================================
# Block 3: Enable Qt's meta-object compiler (moc, uic,rcc)
#=========================================================

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#=========================================================
# Block 4: Define the executable and link Qt
#=========================================================

add_executable(tarsis-hmi-simulator src/main.cpp)
target_link_libraries(tarsis-hmi-simulator PRIVATE Qt6::Widgets)






