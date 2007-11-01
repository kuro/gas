
if (NOT MSVC)
    message(STATUS "Hardcore checks enabled")
    add_definitions(-Werror)
    add_definitions(-pedantic -ansi)
endif ()

add_definitions(-DUNIX=${UNIX})

set(sources gas.h gas.c)
add_library(gas SHARED
    gas.h gas.c
    fdio.c
    bufio.c
    )

install(TARGETS gas LIBRARY DESTINATION lib)
install(FILES gas.h DESTINATION include)
