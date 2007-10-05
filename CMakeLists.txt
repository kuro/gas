project(Gas C)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS on)
enable_testing()

add_definitions(-Wall -Werror)

subdirs(src test)

find_program(CTAGS NAMES exuberant-ctags ctags)
add_custom_target(tags ALL ${CTAGS} --recurse --extra=fq ${CMAKE_CURRENT_SOURCE_DIR}/src)
