cmake_minimum_required(VERSION 3.10)
project(fish-cgi CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# https://discourse.nixos.org/t/get-clangd-to-find-standard-headers-in-nix-shell/11268/10
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
  ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FETCH_FISH "Fetch fish-shell from the internet" ON)

if($<BOOL:$FETCH_FISH>)
  FetchContent_Declare(
    fish-shell
    GIT_REPOSITORY https://github.com/fish-shell/fish-shell.git
    GIT_SHALLOW    true
    GIT_TAG        3.2.2)
else()
  FetchContent_Declare(fish-shell URL ${FISH_PATH})
endif()

# FetchContent_MakeAvailable(fish-shell)
# https://gitlab.kitware.com/cmake/cmake/-/merge_requests/3930
# work around by manually populating content,
# https://cmake.org/cmake/help/latest/module/FetchContent.html#id5
FetchContent_GetProperties(fish-shell)
set(lcName "fish-shell")
if(NOT ${lcName}_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(fish-shell)

  # Bring the populated content into the build
  add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} fishlib fcgi fcgi++)
target_include_directories(${PROJECT_NAME} PUBLIC
  fish-shell
  ${CMAKE_BINARY_DIR}/_deps
  ${CMAKE_BINARY_DIR}/_deps/fish-shell-build)

install(TARGETS ${PROJECT_NAME})
