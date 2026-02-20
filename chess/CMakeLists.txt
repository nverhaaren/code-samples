cmake_minimum_required(VERSION 3.20)
project(chess VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# chess_lib: the engine logic, usable without the CLI
add_library(chess_lib STATIC chess.cpp)
target_include_directories(chess_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(chess_lib PRIVATE -Wall -Wextra -Wpedantic)

# chess: the command-line UI
add_executable(chess Main.cpp)
target_link_libraries(chess PRIVATE chess_lib)
target_compile_options(chess PRIVATE -Wall -Wextra)

# Tests
option(ENABLE_TESTS "Build tests" ON)
option(ENABLE_COVERAGE "Enable code coverage reporting" OFF)

if(ENABLE_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)

    enable_testing()

    add_executable(chess_tests tests/chess_tests.cpp)
    target_link_libraries(chess_tests PRIVATE chess_lib Catch2::Catch2WithMain)
    target_compile_options(chess_tests PRIVATE -Wall -Wextra)

    if(ENABLE_COVERAGE)
        target_compile_options(chess_lib PRIVATE --coverage -O0 -g)
        target_link_options(chess_lib PRIVATE --coverage)
        target_compile_options(chess_tests PRIVATE --coverage -O0 -g)
        target_link_options(chess_tests PRIVATE --coverage)
        # chess links against chess_lib which is compiled with --coverage, so
        # it also needs --coverage on the link step to pull in libgcov.
        target_link_options(chess PRIVATE --coverage)
    endif()

    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    include(CTest)
    include(Catch)
    catch_discover_tests(chess_tests)
endif()
