
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gas)

find_package(EXPAT)
include_directories(${EXPAT_INCLUDE_PATH})

if (EXPAT_FOUND)
    add_definitions(-DHAVE_EXPAT=1)
    add_executable(gascan
        main.cpp
        xml2gas.cpp
        )
else ()
    add_definitions(-DHAVE_EXPAT=0)
    add_executable(gascan
        main.cpp
        )
endif ()

target_link_libraries(gascan
    gas
    ${EXPAT_LIBRARIES}
    )

install(PROGRAMS gascan DESTINATION bin)
