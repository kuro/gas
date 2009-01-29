cmake_minimum_required(VERSION 2.4)

project(Gas)

find_package(Qt4)

include(FindPkgConfig)
pkg_check_modules(APR apr-1)

option(GAS_ENABLE_CPP "Enable C++ wrapper" ON)
option(GAS_ENABLE_INSTALLER "Enable installer commands" OFF)
option(GAS_ENABLE_TESTS "Enable test building" OFF)
option(GAS_USE_LONG_TYPES "Enable use of long types" ON)

if (CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Wall)
endif ()

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(byteswap.h HAVE_BYTESWAP_H)
CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES(libgen.h HAVE_LIBGEN_H)

include(CheckFunctionExists)
check_function_exists("basename" HAVE_BASENAME)

include(CheckTypeSize)
check_type_size("short int" GAS_SIZEOF_SHORT_INT)
check_type_size("int"       GAS_SIZEOF_INT)
check_type_size("long int"  GAS_SIZEOF_LONG_INT)

if (NOT BIG_ENDIAN)
    include(TestBigEndian)
    TEST_BIG_ENDIAN(BIG_ENDIAN)
endif ()

string(TOUPPER "${CMAKE_BUILD_TYPE}" tmp)
if ("${tmp}" MATCHES "DEBUG")
    set(GAS_DEBUG 1)
endif ()

if (CMAKE_COMPILER_IS_GNUCC)
    set(GAS_INLINE "inline")
elseif (MSVC)
    set(GAS_INLINE "__inline")
else ()
    set(GAS_INLINE "static")
endif ()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gas/gas.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/gas/gas.h
    )

execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink 
    ${CMAKE_CURRENT_BINARY_DIR}/src/gas/gas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gas/gas.h
    )

subdirs(src doc)

if (GAS_ENABLE_INSTALLER)
    include(CPack)
endif ()

if (GAS_ENABLE_TESTS)
    enable_testing()
    subdirs(test)
endif ()

# ctags {{{
find_program(CTAGS NAMES exuberant-ctags)
if (CTAGS AND "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    add_custom_target( tags ALL
        COMMAND ${CTAGS} --recurse --extra=fq --langmap=c++:.inl.h.cpp ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/tags ${CMAKE_CURRENT_SOURCE_DIR}/tags
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
endif ()
#}}}

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/README.html
    DEPENDS README
    COMMAND maruku ${CMAKE_CURRENT_SOURCE_DIR}/README -o ${CMAKE_CURRENT_SOURCE_DIR}/README.html
    )
add_custom_target(README.html)

configure_file(gas.pc.in ${CMAKE_CURRENT_BINARY_DIR}/gas.pc @ONLY)
if(GAS_ENABLE_INSTALLER)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gas.pc DESTINATION lib/pkgconfig)
endif()



# vim: fdm=marker
