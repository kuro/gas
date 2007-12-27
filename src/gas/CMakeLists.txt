
if (NOT MSVC)
    message(STATUS "Hardcore checks enabled")
    add_definitions(-pedantic -ansi)
endif ()

set(headers
    bufio.h
    context.h
    fdio.h
    fsio.h
    gas.h
    gas.inl
    ntstring.h
    parser.h
    swap.h
    writer.h
    )

set(sources
    bufio.c
    context.c
    fdio.c
    fsio.c
    gas.c
    ntstring.c
    parser.c
    swap.c
    writer.c
    )

add_library(gas SHARED ${headers} ${sources})

install(TARGETS gas LIBRARY DESTINATION lib)
install(FILES ${headers} DESTINATION include/gas)
