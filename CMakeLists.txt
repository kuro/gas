set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS on)

project(Gas)

add_definitions(-DSIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P})

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(byteswap.h HAVE_BYTESWAP_H)
if (HAVE_BYTESWAP_H)
    add_definitions(-DHAVE_BYTESWAP_H=1)
else ()
    add_definitions(-DHAVE_BYTESWAP_H=0)
endif ()

enable_testing()
include(CPack)

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
    add_custom_target(tags ALL ${CTAGS} --recurse --extra=fq ${CMAKE_CURRENT_SOURCE_DIR})
endif ()
#}}}

subdirs(src test doc)

# vim: fdm=marker
