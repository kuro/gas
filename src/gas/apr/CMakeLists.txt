
include_directories(
    ${APR_INCLUDE_DIRS}
    )

set(headers
    context.h
    )

set(sources
    apr-file-context.c
    apr-socket-context.c
    )

# use BUILD_SHARED_LIBS as necessary
add_library(gas-apr ${headers} ${sources})

if (GAS_ENABLE_INSTALLER)
    install(FILES ${headers} DESTINATION include/gas/apr)

    if (WIN32)
        install(TARGETS gas-apr
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            )
    else ()
        install(TARGETS gas-apr
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            )
    endif ()
endif ()
