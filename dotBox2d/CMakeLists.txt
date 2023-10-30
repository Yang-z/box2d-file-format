find_package(Boost 1.75 REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} DOTBOX2D_SOURCE_FILES)

add_library(dotBox2d ${DOTBOX2D_SOURCE_FILES})

target_include_directories(dotBox2d
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(dotBox2d box2d)

# set(CMAKE_CXX_FLAGS "-std=c++11")
# set(CMAKE_CXX_STANDARD 14)
set_target_properties(dotBox2d 
PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    # VERSION ${PROJECT_VERSION}
    # SOVERSION ${PROJECT_VERSION_MAJOR}
)
