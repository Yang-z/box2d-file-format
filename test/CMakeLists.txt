find_package(Boost 1.75 REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCE_FILES)
add_executable(test ${TEST_SOURCE_FILES})
target_link_libraries(test PRIVATE dot-box2d)

################################################################

# print the type of boost lib
if(Boost_FOUND)
    message(STATUS "Boost found!")
    message(STATUS "Boost include directories: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")

    # Check if the Boost libraries contain shared libraries
    if(Boost_USE_STATIC_LIBS)
        message(STATUS "Boost is statically linked.")
    else()
        message(STATUS "Boost is dynamically linked.")
    endif()
else()
    message(FATAL_ERROR "Boost not found.")
endif()

# print dirs
message("[${PROJECT_NAME}] CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("[${PROJECT_NAME}] CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message("[${PROJECT_NAME}] CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message("[${PROJECT_NAME}] CMAKE_INSTALL_DATAROOTDIR: ${CMAKE_INSTALL_DATAROOTDIR}")
message("[${PROJECT_NAME}] CMAKE_INSTALL_INCLUDEDIR: ${CMAKE_INSTALL_INCLUDEDIR}")
message("[${PROJECT_NAME}] CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")
message("[${PROJECT_NAME}] CMAKE_INSTALL_BINDIR: ${CMAKE_INSTALL_BINDIR}")
message("[${PROJECT_NAME}] CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
