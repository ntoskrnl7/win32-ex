cmake_minimum_required(VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(pkg-utils)

pkg_get_version(WIN32EX ${CMAKE_CURRENT_SOURCE_DIR}/include/Win32Ex/internal/version.h VERSION)

project(win32ex VERSION "${VERSION}" LANGUAGES C CXX)

option(WIN32EX_BUILD_TESTS "Set to ON to build tests" OFF)

add_library(win32ex INTERFACE)

include(cmake/CPM.cmake)
CPMAddPackage(
    GITHUB_REPOSITORY ntoskrnl7/ext
    VERSION 0.5.15
    OPTIONS "EXT_NO_WIN32_EX ON"
)

target_link_libraries(win32ex INTERFACE ext wtsapi32 userenv advapi32 psapi shell32)

target_include_directories(win32ex INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

if (WIN32EX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
