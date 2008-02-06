set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS on)

project(Gas)

add_definitions(-Wall)

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(byteswap.h HAVE_BYTESWAP_H)
CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES(assert.h HAVE_ASSERT_H)

include(TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)

string(TOUPPER "${CMAKE_BUILD_TYPE}" tmp)
if ("${tmp}" MATCHES "DEBUG")
    set(GAS_DEBUG 1)
endif ()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gas/gas.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/gas/gas.h
    )

enable_testing()
include(CPack)

subdirs(src test doc)

# ctags {{{
find_program(CTAGS NAMES exuberant-ctags ctags)
if (CTAGS)
    add_custom_target(tags ALL ${CTAGS} --recurse --extra=fq --langmap=c++:.inl.h.cpp ${CMAKE_CURRENT_SOURCE_DIR})
endif ()
#}}}

# vim: fdm=marker
