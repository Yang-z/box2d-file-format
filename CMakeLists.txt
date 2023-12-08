cmake_minimum_required(VERSION 3.8)

project(dot-box2d)

add_subdirectory(${CMAKE_SOURCE_DIR}/../box2d ${CMAKE_BINARY_DIR}/box2d)

option(DOT_BOX2D_BUILD_TEST "Build the dot-box2d test" ON)

add_subdirectory(src)

if (DOT_BOX2D_BUILD_TEST)
    add_subdirectory(test)
endif()
