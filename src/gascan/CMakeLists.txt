
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../gas)

find_package(EXPAT)
include_directories(${EXPAT_INCLUDE_PATH})

add_executable(gascan
    main.cpp
    xml2gas.cpp
    )

target_link_libraries(gascan
    gas
    ${EXPAT_LIBRARIES}
    )

install(PROGRAMS gascan DESTINATION bin)
