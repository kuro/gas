
if (UNIX AND NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_BINARY_DIR}/gas.h
        ${CMAKE_CURRENT_SOURCE_DIR}/gas.h
        )
endif ()

#if (CMAKE_COMPILER_IS_GNUCC)
#    message(STATUS "Hardcore checks enabled")
#    #add_definitions(-pedantic -ansi)
#    add_definitions(-pedantic -std=c99)
#endif ()

set(headers
    ${CMAKE_CURRENT_BINARY_DIR}/gas.h
    bufio.h
    context.h
    etc.c
    fdio.h
    io.h
    ntstring.h
    memory.h
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
    io.c
    memory.c
    ntstring.c
    parser.c
    swap.c
    tree.c
    writer.c
    )

if (HAVE_FPRINTF)
    set(headers ${headers} fsio.h)
    set(sources ${sources} fsio.c)
endif ()

if (QT4_FOUND)
    set(headers
        ${headers}
        qt/context.h
        qt/chunk.h
        qt/chunk.inl
        qt/scanner.h
        )
    set(sources
        ${sources}
        qt/qiodevice-context.cpp
        qt/qtcpsocket-context.cpp
        qt/chunk.cpp
        qt/scanner.cpp
        )
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    qt4_automoc(${sources})
endif ()

if (APR_FOUND)
    include_directories(${APR_INCLUDE_DIRS})
    set(headers ${headers} apr/context.h)
    set(sources ${sources} apr/apr-file-context.c apr/apr-socket-context.c)
endif ()

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
