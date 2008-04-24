
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    )

if (CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Werror)
endif ()

subdirs(gas)

if (QT4_FOUND)
    subdirs(gascan)
endif ()

