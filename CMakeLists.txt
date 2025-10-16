cmake_minimum_required(VERSION 3.19)
project(pbl4 LANGUAGES CXX)

# Sửa lại thành dòng này
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
qt_standard_project_setup()
# --- Bật tự động Qt ---
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(src)
add_subdirectory(third_party)
add_subdirectory(resources)

