cmake_minimum_required(VERSION 3.7)

project(cygnus)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)

add_compile_options(-std=c++11)

set(SOURCES
  main.cpp
  MainWindow.cpp
  Puzzle.cpp
)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES})

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Widgets
  Qt5::Gui
)
