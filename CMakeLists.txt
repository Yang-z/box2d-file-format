cmake_minimum_required(VERSION 3.8)

project(dotBox2d)

option(DOTBOX2D_BUILD_TEST "Build the dotBox2d test" ON)

add_subdirectory(dotBox2d)

if (DOTBOX2D_BUILD_TEST)
    add_subdirectory(test)
endif()
