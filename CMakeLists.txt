cmake_minimum_required(VERSION 4.2)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(cpp-lab
    DESCRIPTION "C++ lab: A collection of C++ small utilities, exercises and experiments."
    LANGUAGES CXX)

include(CTest)

add_library(cpp_lab INTERFACE)

file(GLOB_RECURSE include_files "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

target_include_directories(cpp_lab
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_sources(cpp_lab
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        FILES
            ${include_files}
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
