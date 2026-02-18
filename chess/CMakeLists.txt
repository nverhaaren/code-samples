cmake_minimum_required(VERSION 3.20)
project(chess VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# chess_lib: the engine logic, usable without the CLI
add_library(chess_lib STATIC chess.cpp)
target_include_directories(chess_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(chess_lib PRIVATE -Wall -Wextra -Wpedantic)

# chess: the command-line UI
add_executable(chess Main.cpp)
target_link_libraries(chess PRIVATE chess_lib)
target_compile_options(chess PRIVATE -Wall -Wextra)
