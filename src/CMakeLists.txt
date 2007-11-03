
if (NOT MSVC)
    add_definitions(-Werror)
endif ()

subdirs(gas gascan)
