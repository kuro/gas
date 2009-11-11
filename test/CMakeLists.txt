
find_program(VALGRIND valgrind)
if (VALGRIND)
    set(VALGRIND_COMMAND ${VALGRIND} --tool=memcheck --leak-check=yes)
endif()


set(tests
    bufio
    cplusplus
    encoding
    fsio
    io
    mapped
    numbers
    parser
    qt
    tree
    writer
    )

string(REGEX REPLACE "([-_a-z0-9]+)" "\\1.cpp" files "${tests}")

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )

set(driver_files ${files})

qt4_automoc(${files})

create_test_sourcelist(driver_list driver.cpp ${driver_files})

add_executable(driver ${driver_list})

target_link_libraries(driver gas ${QT_LIBRARIES})

foreach(test ${tests})
    add_test(${test}
        ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR}/..
        ${VALGRIND_COMMAND}
        ./test/driver ${test}
        )
endforeach()

# ruby

add_test(ruby_tree ruby -I${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/ruby_tree.rb)

# vim: fdm=marker
