
if (NOT MSVC)
    message(STATUS "Hardcore checks enabled")
    add_definitions(-pedantic -ansi)
endif ()

set(sources
    gas.h gas.c
    fdio.h fdio.c
    bufio.h bufio.c
    ntstring.h ntstring.c
    context.h context.c
    parser.h parser.c
    )

add_library(gas SHARED ${sources})

#install(TARGETS gas LIBRARY DESTINATION lib)
#install(FILES gas.h DESTINATION include)
