
if (CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Werror)
endif ()

subdirs(gas)

if (QT4_FOUND)
    subdirs(qt gascan)
endif ()
