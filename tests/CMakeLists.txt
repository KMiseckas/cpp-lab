include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)

# Prevent GoogleTest from overriding project-wide compiler settings on Windows.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE unit_test_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/unit_tests/*.cpp"
)

if(NOT unit_test_sources)
    message(WARNING
        "No unit tests were found under ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests"
    )
endif()

add_executable(cpp_lab_tests
    ${unit_test_sources}
)

target_link_libraries(cpp_lab_tests
    PRIVATE
        cpp_lab
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(cpp_lab_tests)
