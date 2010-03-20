project(GasQt)

set(sources
    chunk.h
    chunk.inl
    chunk.cpp
    gasmodel.h
    gasmodel.cpp
    scanner.h
    scanner.cpp
    )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
qt4_automoc(${sources})

add_library(GasQt ${headers} ${sources})
