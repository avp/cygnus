cmake_minimum_required(VERSION 3.0)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE
  STRING "Minimum OS X deployment version")

project(cygnus
  VERSION 0.0.1
)

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
  set_source_files_properties(
    resources/icon.icns
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
  )

  add_executable(${PROJECT_NAME} MACOSX_BUNDLE
    ${BUNDLE_TYPE} ${SOURCES}
    resources.rc
    resources/icon.icns
  )

  set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/Info.plist.in"
  )
else()
  add_executable(${PROJECT_NAME} WIN32
    ${BUNDLE_TYPE} ${SOURCES}
    resources.rc
  )
endif()

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
  add_custom_target(run
    COMMAND ${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
  )
endif()
