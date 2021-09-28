cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(pkg-utils)

pkg_get_version(${CMAKE_CURRENT_SOURCE_DIR}/include/internal/version.h VERSION)

project(win32ex VERSION "${VERSION}" LANGUAGES C CXX)

option(WIN32EX_BUILD_TESTS "Set to ON to build tests" OFF)

add_library(win32ex INTERFACE)

target_include_directories(win32ex INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if (WIN32EX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
