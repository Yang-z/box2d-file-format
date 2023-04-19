cmake_minimum_required(VERSION 3.8)

project(dotBox2d)

add_subdirectory(${CMAKE_SOURCE_DIR}/../box2d ${CMAKE_BINARY_DIR}/box2d)

option(DOTBOX2D_BUILD_TEST "Build the dotBox2d test" ON)

add_subdirectory(dotBox2d)

if (DOTBOX2D_BUILD_TEST)
    add_subdirectory(test)
endif()
