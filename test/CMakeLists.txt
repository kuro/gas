include_directories(../src)

foreach(test tree encoding io)
    add_executable(${test} ${test}.c)
    target_link_libraries(${test} gas)

    add_test(${test} ${EXECUTABLE_OUTPUT_PATH}/${test})
endforeach()

