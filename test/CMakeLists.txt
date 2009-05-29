
find_program(VALGRIND valgrind)
if (VALGRIND)
    set(VALGRIND_COMMAND ${VALGRIND} --tool=memcheck --leak-check=yes)
endif()

set(std_tests
    encoding
    io
    numbers
    parser
    tree
    )

set(qt_tests
    bufio
    cplusplus
    mapped
    writer
    )

string(REGEX REPLACE "([-_a-z0-9]+)" "\\1.cpp" std_files "${std_tests}")
string(REGEX REPLACE "([-_a-z0-9]+)" "\\1.cpp" qt_files  "${qt_tests}" )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_BINARY_DIR}/../src
    )

set(driver_files ${std_files})

if (QT4_FOUND)
    qt4_automoc(${qt_files})
    set(driver_files ${driver_files} ${qt_files})
    link_libraries(${QT_LIBRARIES})
endif ()

create_test_sourcelist(driver_list driver.cpp ${driver_files})

add_executable(driver ${driver_list})

# link libraries
target_link_libraries(driver gas)

# register tests

foreach(test ${std_tests})
    add_test(${test} ${VALGRIND_COMMAND} ./driver ${test})
endforeach()

if (QT4_FOUND)
    foreach(test ${qt_tests})
        add_test(${test} ${VALGRIND_COMMAND} ./driver ${test})
    endforeach()
endif ()

# ruby

add_test(ruby_tree ruby -I${CMAKE_CURRENT_SOURCE_DIR}/../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/ruby_tree.rb)

# vim: fdm=marker
