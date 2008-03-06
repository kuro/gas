set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS on)

project(Gas)

if (CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Wall)
    set(GAS_ENABLE_CPP 1)

    set(BUILD_SHARED_LIBS ON)
endif ()

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(byteswap.h HAVE_BYTESWAP_H)
CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES(assert.h HAVE_ASSERT_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

if (NOT ${BIG_ENDIAN})
    include(TestBigEndian)
    TEST_BIG_ENDIAN(BIG_ENDIAN)
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" tmp)
if ("${tmp}" MATCHES "DEBUG")
    set(GAS_DEBUG 1)
endif ()

if (CMAKE_COMPILER_IS_GNUCC)
    set(GAS_INLINE "inline")
else ()
    set(GAS_INLINE "")
endif ()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gas/gas.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/gas/gas.h
    )

subdirs(src doc)

#include(CPack)

if (CMAKE_COMPILER_IS_GNUCC)
    enable_testing()
    subdirs(test)
endif ()

# ctags {{{
find_program(CTAGS NAMES exuberant-ctags ctags)
if (CTAGS)
    add_custom_target(tags ALL ${CTAGS} --recurse --extra=fq --langmap=c++:.inl.h.cpp ${CMAKE_CURRENT_SOURCE_DIR})
endif ()
#}}}

# vim: fdm=marker
