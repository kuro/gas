
find_package(Doxygen)

if (DOT)
    set(HAVE_DOT "YES")
else ()
    set(HAVE_DOT "NO")
endif ()

if (NOT DOXYGEN_FOUND)
    message("Could not find Doxygen")
else ()

    #add_custom_target(all_doc COMMAND echo blah)
    add_custom_target(doc)
    add_custom_target(pdf)

    #add_custom_command(TARGET doc DEPENDS all_doc)
    #add_custom_target(doc DEPENDS all_doc)
    #add_custom_target(doc DEPENDS all_doc)

    foreach(src gas)
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/${src}.dox.in
            ${CMAKE_CURRENT_BINARY_DIR}/${src}.dox
            )
        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${src})
        add_custom_target(${src}_doc
            COMMAND ${DOXYGEN} ${CMAKE_CURRENT_BINARY_DIR}/${src}.dox
            )
        add_dependencies(doc ${src}_doc)
        if (CMAKE_MAKE_PROGRAM)
            add_custom_target(${src}_pdf
                COMMAND ${CMAKE_MAKE_PROGRAM} -s -C ${CMAKE_CURRENT_BINARY_DIR}/${src}/latex
                #DEPENDS ${src}_doc
                )
            add_dependencies(pdf ${src}_pdf)
        endif()
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${src}
            DESTINATION share/doc)
    endforeach()

endif ()

find_program(RDOC NAMES rdoc)
if (RDOC)
    add_custom_target(rdoc
        COMMAND ${RDOC} -o ${CMAKE_CURRENT_BINARY_DIR}/rdoc ${CMAKE_CURRENT_SOURCE_DIR}/../lib
        )
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rdoc DESTINATION
        share/doc/gas/rdoc
        )
    add_dependencies(doc rdoc)
endif ()
