project(Gas)

enable_testing()
include(CPack)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS on)

add_definitions(-Wall)

if (UNIX OR MINGW)
    add_definitions(-DUNIX=1)
else ()
    add_definitions(-DUNIX=0)
endif ()

# ctags {{{
find_program(CTAGS NAMES exuberant-ctags ctags)
if (CTAGS)
    add_custom_target(tags ALL ${CTAGS} --recurse --extra=fq ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif ()
#}}}

subdirs(src test doc)

# vim: fdm=marker
