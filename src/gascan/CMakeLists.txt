
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(EXPAT)
include_directories(${EXPAT_INCLUDE_PATH})

set(sources main.cpp gas2c.cpp)

if (EXPAT_FOUND)
    add_definitions(-DHAVE_EXPAT=1)
    set(sources ${sources} xml2gas.cpp)
else ()
    add_definitions(-DHAVE_EXPAT=0)
endif ()

add_executable(gascan ${sources})

message(${sources})

target_link_libraries(gascan
    gas
    ${EXPAT_LIBRARIES}
    )

install(PROGRAMS gascan DESTINATION bin)
