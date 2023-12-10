cmake_minimum_required(VERSION 3.8)

project(dot-box2d VERSION 0.0.1)

option(BUILD_SHARED_LIBS "Build as shared libraries" ON)


# "import" box2d project
# override options and reset output directories here
# or configure_file() could be used to do the job?
option(BOX2D_BUILD_UNIT_TESTS "Build the Box2D unit tests" OFF)
option(BOX2D_BUILD_TESTBED "Build the Box2D testbed" OFF)
option(BOX2D_BUILD_DOCS "Build the Box2D documentation" OFF)
option(BOX2D_USER_SETTINGS "Override Box2D settings with b2UserSettings.h" OFF)
set(BOX2D_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../box2d) # where box2d is located
add_subdirectory(${BOX2D_SOURCE_DIR} ${BOX2D_SOURCE_DIR}/build)
set_target_properties(box2d PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin
)


# dot-box2d
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
include(GNUInstallDirs)

option(DOT_BOX2D_BUILD_TEST "Build the dot-box2d test" ON)

add_subdirectory(src)

if (DOT_BOX2D_BUILD_TEST)
    add_subdirectory(test)
endif()
