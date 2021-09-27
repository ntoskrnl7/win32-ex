if (CMAKE_VERSION VERSION_GREATER 3.10 OR CMAKE_VERSION VERSION_EQUAL 3.10)
    include_guard()
endif()

function(pkg_get_version header_file version_arg)
file(STRINGS ${header_file} version_defines
    REGEX "#define WIN32EX_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${version_defines})
    if(ver MATCHES "#define WIN32EX_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
        set(WIN32EX_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(VERSION ${WIN32EX_VERSION_MAJOR}.${WIN32EX_VERSION_MINOR}.${WIN32EX_VERSION_PATCH})

if (CMAKE_VERSION VERSION_GREATER 3.15 OR CMAKE_VERSION VERSION_EQUAL 3.15)
    message(DEBUG "win32-ex version ${VERSION}")
else()
    message(STATUS "win32-ex version ${VERSION}")
endif()

set(${version_arg} ${VERSION} PARENT_SCOPE)
endfunction()
