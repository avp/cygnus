cmake_minimum_required(VERSION 3.0)

project(cygnus)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_compile_options(-std=c++14)

set(SOURCES
  main.cpp
  MainWindow.cpp

  ClueWidget.cpp
  PuzzleWidget.cpp
  TimerWidget.cpp

  FilledLabel.cpp

  Puzzle.cpp
)

if (CMAKE_BUILD_TYPE EQUAL "RELEASE")
  add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

if (APPLE)
  set(BUNDLE_TYPE MACOSX_BUNDLE)
endif()

add_executable(${PROJECT_NAME} WIN32
${BUNDLE_TYPE} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
)

if (APPLE)
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/Info.plist.in
  )

  add_custom_target(run
    COMMAND ${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
  )
endif()
