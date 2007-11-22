
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (NOT MSVC)
    add_definitions(-Werror)
endif ()

subdirs(gas gascan)
