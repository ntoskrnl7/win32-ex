cmake_minimum_required(VERSION 3.10)

include(../cmake/CPM.cmake)

if (CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
  message("CMAKE_BUILD_TYPE (default) : " ${CMAKE_BUILD_TYPE})
else()
  message("CMAKE_BUILD_TYPE : " ${CMAKE_BUILD_TYPE})
endif()

project(unittest LANGUAGES C CXX)

CPMAddPackage(NAME win32ex SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

if (MSVC_VERSION LESS_EQUAL 1700)
  if (MSVC_VERSION GREATER_EQUAL 1600)
    add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
  endif()
  CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG release-1.7.0
    VERSION 1.4.0
    OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
  )
  include_directories("${gtest_SOURCE_DIR}/include")
else()
  CPMAddPackage(
    NAME googletest
    GITHUB_REPOSITORY google/googletest
    GIT_TAG release-1.10.0
    VERSION 1.10.0
    OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
  )
endif()

file(GLOB SOURCE_FILES ./*/*.cpp ./*.cpp)
file(GLOB C_SOURCE_FILES ./*/*.c ./*.c)

add_executable(unittest ${SOURCE_FILES} ${C_SOURCE_FILES})
target_link_libraries(unittest win32ex gtest gtest_main)

enable_testing()

add_test(NAME unittest COMMAND unittest)