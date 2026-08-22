include(FetchContent)

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.5
)

# This project only needs the benchmark library, not Google Benchmark's own tests.
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googlebenchmark)

file(GLOB_RECURSE benchmark_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/performance_tests/*.cpp"
)

if(NOT benchmark_sources)
    message(WARNING
        "No benchmarks were found under ${CMAKE_CURRENT_SOURCE_DIR}/performance_tests"
    )
else()
    add_executable(cpp_lab_benchmarks
        ${benchmark_sources}
    )

    target_link_libraries(cpp_lab_benchmarks
        PRIVATE
            cpp_lab
            benchmark::benchmark_main
    )

    add_custom_target(run_cpp_lab_benchmarks
        COMMAND $<TARGET_FILE:cpp_lab_benchmarks>
            --benchmark_repetitions=5
            --benchmark_report_aggregates_only=true
        DEPENDS cpp_lab_benchmarks
        USES_TERMINAL
        VERBATIM
    )
endif()
