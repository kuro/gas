
find_package(Doxygen)

if (NOT WIN32)
    set(DEST share/)
endif ()

if (DOT)
    set(HAVE_DOT "YES")
else ()
    set(HAVE_DOT "NO")
endif ()

execute_process(
    COMMAND ./versionfilter.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_NUMBER
    )

if (DOXYGEN_FOUND)

    configure_file(help.qhcp.in ${CMAKE_CURRENT_BINARY_DIR}/help.qhcp)

    foreach(src gas)
        configure_file(${src}.dox.in ${CMAKE_CURRENT_BINARY_DIR}/${src}.dox)
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${src})
        add_custom_target(${src}_doc
            COMMAND ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/${src}.dox
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            )
        add_dependencies(doc ${src}_doc)
        if (GAS_ENABLE_INSTALLER)
            install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${src}
                DESTINATION ${DEST}doc)
        endif()
    endforeach()

    add_dependencies(gas_doc README.html)

    add_custom_command(
        TARGET gas_doc
        POST_BUILD
        COMMAND qcollectiongenerator help.qhcp -o help.qhc
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "generating collection"
        )

endif ()

find_program(RDOC NAMES rdoc)
if (RDOC)
    add_custom_target(rdoc
        COMMAND ${RDOC} -o ${CMAKE_CURRENT_BINARY_DIR}/rdoc ${CMAKE_CURRENT_SOURCE_DIR}/../lib
        )
    #file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rdoc)
    if (GAS_ENABLE_INSTALLER)
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rdoc DESTINATION
            ${DEST}doc/gas/rdoc
            )
    endif()
    add_dependencies(doc rdoc)
endif ()
