project(GasQt)

set(headers
    Chunk.h
    Chunk.inl
    GasModel.h
    Scanner.h
    )

set(sources
    Chunk.cpp
    GasModel.cpp
    Scanner.cpp
    )

qt4_automoc(${sources})

add_library(GasQt ${headers} ${sources})

if (GAS_ENABLE_INSTALLER)
    install(FILES ${headers} DESTINATION include/gas/qt)

    if (WIN32)
        install(TARGETS GasQt
            RUNTIME DESTINATION bin
            ARCHIVE DESTINATION lib
            )
    else ()
        install(TARGETS GasQt
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            )
    endif ()
endif ()
