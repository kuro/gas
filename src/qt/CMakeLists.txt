project(GasQt)

set(sources
    Chunk.h
    Chunk.inl
    Chunk.cpp
    GasModel.h
    GasModel.cpp
    Scanner.h
    Scanner.cpp
    )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
qt4_automoc(${sources})

add_library(GasQt ${headers} ${sources})
