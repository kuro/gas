project(Gas C)

enable_testing()
include(CPack)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS on)

# warning management {{{
add_definitions(-Wall)

if (NOT MSVC)
    message(STATUS "Hardcore checks enabled")
    add_definitions(-Werror)
    add_definitions(-pedantic -ansi)
endif ()
#}}}

subdirs(src test)

# ctags {{{
find_program(CTAGS NAMES exuberant-ctags ctags)
add_custom_target(tags ALL ${CTAGS} --recurse --extra=fq ${CMAKE_CURRENT_SOURCE_DIR}/src)
#}}}

# vim: fdm=marker
