cmake_minimum_required(VERSION 3.0)

project(cygnus)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(SOURCES
  main.cpp
  MainWindow.cpp

  ClueWidget.cpp
  PuzzleWidget.cpp
  TimerWidget.cpp

  FilledLabel.cpp

  Puzzle.cpp
)

if (APPLE)
  set(BUNDLE_TYPE MACOSX_BUNDLE)
endif()

add_executable(${PROJECT_NAME} WIN32
${BUNDLE_TYPE} ${SOURCES}
resources.rc)

target_link_libraries(${PROJECT_NAME}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 14)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  target_compile_definitions(${PROJECT_NAME} PUBLIC
    QT_NO_DEBUG_OUTPUT
  )
  target_compile_options(${PROJECT_NAME} PUBLIC
    "-O3"
  )
else()
  target_compile_options(${PROJECT_NAME} PUBLIC
    "-Og"
  )
endif()

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
