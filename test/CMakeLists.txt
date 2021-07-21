cmake_minimum_required(VERSION 3.10)

project(unittest)

if (MSVC_VERSION LESS_EQUAL 1700)
if (MSVC_VERSION GREATER_EQUAL 1600)
add_definitions(-DGTEST_HAS_TR1_TUPLE=0)
endif()
configure_file(CMakeLists.1.7.0.txt.in googletest-download/CMakeLists.txt)
else()
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
endif()
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
                 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
                 EXCLUDE_FROM_ALL)
include_directories("${gtest_SOURCE_DIR}/include")


include_directories("../include")

file(GLOB SOURCE_FILES ./*/*.cpp ./*.cpp)
file(GLOB C_SOURCE_FILES ./*/*.c ./*.c)

add_executable(unittest ${SOURCE_FILES} ${C_SOURCE_FILES})
target_link_libraries(unittest gtest gtest_main)
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