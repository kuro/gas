
if (CMAKE_COMPILER_IS_GNUCC)
    message(STATUS "Hardcore checks enabled")
    #add_definitions(-pedantic -ansi)
    add_definitions(-pedantic -std=c99)
endif ()

set(headers
    ${CMAKE_CURRENT_BINARY_DIR}/gas.h
    bufio.h
    context.h
    fdio.h
    fsio.h
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

# use BUILD_SHARED_LIBS as necessary
add_library(gas ${headers} ${sources})

if (GAS_ENABLE_INSTALLER)
    install(FILES ${headers} DESTINATION include/gas)

    if (WIN32)
        install(TARGETS gas
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            )
    else ()
        install(TARGETS gas
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            )
    endif ()
endif ()
