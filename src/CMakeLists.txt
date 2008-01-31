
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

if (NOT MSVC)
    add_definitions(-Werror)
endif ()

subdirs(gas)
if (NOT MSVC)
    subdirs(gascan)
endif ()

