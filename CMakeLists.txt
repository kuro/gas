project(Gas)

add_definitions(-DSIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})

enable_testing()
include(CPack)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS on)

add_definitions(-Wall)

if (UNIX OR MINGW)
    add_definitions(-DUNIX=1)
else ()
    add_definitions(-DUNIX=0)
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" tmp)
if ("${tmp}" MATCHES "DEBUG")
    add_definitions(-DDEBUG)
endif ()

# ctags {{{
find_program(CTAGS NAMES exuberant-ctags ctags)
if (CTAGS)
    add_custom_target(tags ALL ${CTAGS} --recurse --extra=fq ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif ()
#}}}

subdirs(src test doc)

# vim: fdm=marker
