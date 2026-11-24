cmake_minimum_required(VERSION 3.16)
project(path_finder VERSION 0.1.0 LANGUAGES CXX)

# c++17, no compiler-specific extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# default to a release build if none was picked (-DCMAKE_BUILD_TYPE=Debug for debugging)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# strict warnings, and treat every warning as an error
if(MSVC)
    set(WARNINGS /W4 /WX)
else()
    set(WARNINGS -Wall -Wextra -Wpedantic -Werror)
endif()

# the path finding code, shared by the program and the tests
add_library(pathfinder_core src/grid.cpp src/search.cpp)
target_include_directories(pathfinder_core PUBLIC include)
target_compile_options(pathfinder_core PRIVATE ${WARNINGS})

# the command-line program
add_executable(pathfinder src/main.cpp)
target_link_libraries(pathfinder PRIVATE pathfinder_core)
target_compile_options(pathfinder PRIVATE ${WARNINGS})

# tests: googletest is downloaded at configure time
option(PATHFINDER_BUILD_TESTS "build the unit tests" ON)
if(PATHFINDER_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # needed on windows
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(pathfinder_tests tests/grid_test.cpp tests/search_test.cpp)
    target_link_libraries(pathfinder_tests PRIVATE pathfinder_core GTest::gtest_main)
    target_compile_options(pathfinder_tests PRIVATE ${WARNINGS})
    # tests find the example maps through this path
    target_compile_definitions(pathfinder_tests PRIVATE MAPS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/maps")

    include(GoogleTest)
    gtest_discover_tests(pathfinder_tests)
endif()