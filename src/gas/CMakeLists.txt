
if (NOT MSVC)
    message(STATUS "Hardcore checks enabled")
    #add_definitions(-pedantic -ansi)
    add_definitions(-pedantic -std=c99)
endif ()

set(headers
    bufio.h
    context.h
    fdio.h
    fsio.h
    gas.h
    ntstring.h
    parser.h
    swap.h
    tree.h
    tree.inl
    types.h
    writer.h
    )

set(sources
    bufio.c
    context.c
    fdio.c
    fsio.c
    ntstring.c
    parser.c
    swap.c
    tree.c
    writer.c
    )

add_library(gas SHARED ${headers} ${sources})

install(TARGETS gas LIBRARY DESTINATION lib)
install(FILES ${headers} DESTINATION include/gas)
