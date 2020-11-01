cmake_minimum_required(VERSION 3.10)
project(fish-cgi CXX)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(fish-shell)

# Include fish-shell sources as well as the generated config.h.
#
add_executable(${PROJECT_NAME} fish_cgi.cpp)
target_link_libraries(${PROJECT_NAME} fishlib fcgi fcgi++)
target_include_directories(${PROJECT_NAME} PUBLIC fish-shell ${CMAKE_BINARY_DIR}/fish-shell)
