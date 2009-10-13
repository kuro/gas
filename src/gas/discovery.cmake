
if (CMAKE_COMPILER_IS_GNUCC)
    add_definitions(-Wall)
endif ()

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(byteswap.h   HAVE_BYTESWAP_H  )
CHECK_INCLUDE_FILES(stdint.h     HAVE_STDINT_H    )
CHECK_INCLUDE_FILES(unistd.h     HAVE_UNISTD_H    )
CHECK_INCLUDE_FILES(libgen.h     HAVE_LIBGEN_H    )
CHECK_INCLUDE_FILES(stdio.h      HAVE_STDIO_H     )
CHECK_INCLUDE_FILES(netinet/in.h HAVE_NETINET_IN_H)

include(CheckFunctionExists)
check_function_exists("fprintf" HAVE_FPRINTF)
check_function_exists("htonl"   HAVE_HTONL)

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

if (UNIX AND CMAKE_COMPILER_IS_GNUCC)
    set(GAS_INLINE "inline")
elseif (MSVC)
    set(GAS_INLINE "__inline")
endif ()
