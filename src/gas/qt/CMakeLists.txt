
include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    )

set(headers
    context.h
    )

set(sources
    qiodevice-context.cpp
    qtcpsocket-context.cpp
    )

# use BUILD_SHARED_LIBS as necessary
add_library(gas-qt ${headers} ${sources})

if (GAS_ENABLE_INSTALLER)
    install(FILES ${headers} DESTINATION include/gas/qt)

    if (WIN32)
        install(TARGETS gas-qt
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            )
    else ()
        install(TARGETS gas-qt
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            )
    endif ()
endif ()
