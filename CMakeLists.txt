cmake_minimum_required(VERSION 2.6)

project(Gas)

find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtTest)
include(${QT_USE_FILE})

include(FindPkgConfig)
pkg_check_modules(APR apr-1)

option(GAS_ENABLE_INSTALLER "Enable installer commands" OFF)
option(GAS_ENABLE_TESTS "Enable test building (Requires Qt)" OFF)

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    add_custom_target(doc)

    find_program(MARUKU maruku)
    if (MARUKU)
        add_custom_target(
            README.html
            COMMAND maruku README -o README.html
            DEPENDS README
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "rendering README.html"
            )
    endif ()

endif ()

subdirs(src doc)

if (GAS_ENABLE_INSTALLER)
    include(CPack)
endif ()

if (GAS_ENABLE_TESTS)
    enable_testing()
    if (QT4_FOUND)
        subdirs(test)
    else ()
        message("Unit tests require Qt")
    endif ()
endif ()

# ctags {{{
find_program(CTAGS
    NAMES exuberant-ctags ctags
    DOC "Path to exuberant ctags."
    )
if (CTAGS AND "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    add_custom_target(tags ALL
        COMMAND ${CTAGS} -R --extra=fq --langmap=c++:.inl.h.cpp ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/tags ${CMAKE_SOURCE_DIR}/tags
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
endif ()
#}}}

configure_file(gas.pc.in ${CMAKE_CURRENT_BINARY_DIR}/gas.pc @ONLY)
if(GAS_ENABLE_INSTALLER)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gas.pc DESTINATION lib/pkgconfig)
endif()



# vim: fdm=marker
