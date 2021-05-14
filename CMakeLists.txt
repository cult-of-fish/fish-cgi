cmake_minimum_required(VERSION 3.10)
project(fish-cgi CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

FetchContent_Declare(
    fish-shell
    GIT_REPOSITORY https://github.com/fish-shell/fish-shell.git
    GIT_SHALLOW    true
    GIT_TAG        3.2.2
)

FetchContent_MakeAvailable(fish-shell)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} fishlib fcgi fcgi++)
target_include_directories(${PROJECT_NAME} PUBLIC
    fish-shell
    ${CMAKE_BINARY_DIR}/_deps
    ${CMAKE_BINARY_DIR}/_deps/fish-shell-build
)
