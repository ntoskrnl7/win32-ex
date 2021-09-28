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
target_link_libraries(unittest win32ex gtest gtest_main Wtsapi32 Userenv)

if (MSYS OR MINGW)
target_link_libraries(unittest gtest gtest_main Wtsapi32 Userenv)
else()
target_link_libraries(unittest gtest gtest_main)
endif()

# SET_TARGET_PROPERTIES(unittest PROPERTIES LINK_FLAGS "/level='asInvoker' /uiAccess='false'")

if (MSVC_VERSION LESS_EQUAL 1600)
  add_definitions("/EHsc")
endif()

get_property(CXX_STANDARD_SPECIFIED TARGET unittest PROPERTY CXX_STANDARD SET)
if (CXX_STANDARD_SPECIFIED)
  get_property(CXX_STANDARD_VAR TARGET unittest PROPERTY CXX_STANDARD)
  message("CXX_STANDARD : " ${CXX_STANDARD_VAR})
else()
  set_property(TARGET unittest PROPERTY CXX_STANDARD 17)
  get_property(CXX_STANDARD_VAR TARGET unittest PROPERTY CXX_STANDARD)
  message("CXX_STANDARD (default): " ${CXX_STANDARD_VAR})
endif()
set_property(TARGET unittest PROPERTY CXX_STANDARD_REQUIRED ON)

enable_testing()

add_test(NAME unittest COMMAND unittest)