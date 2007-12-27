include_directories(../src)

foreach(test test_tree test_encoding test_io test_numbers test_parser)
    add_executable(${test} ${test}.c)
    target_link_libraries(${test} gas)

    add_test(${test} ${EXECUTABLE_OUTPUT_PATH}/${test})
endforeach()

find_package(EXPAT)
if (EXPAT_FOUND)
    include_directories(${EXPAT_INCLUDE_PATH})
    add_executable(test_xml2gas test_xml2gas.c)
    target_link_libraries(test_xml2gas gas ${EXPAT_LIBRARIES})
    add_test(test_xml2gas ${EXECUTABLE_OUTPUT_PATH}/test_xml2gas)
endif()

include(UsePkgConfig)
PKGCONFIG(IlmBase ILM_INCLUDE_DIR ILM_LINK_DIR ILM_LINK_FLAGS ILM_CFLAGS)
add_definitions(${ILM_CFLAGS})
add_executable(test_half test_half.cpp)
target_link_libraries(test_half gas ${ILM_LINK_FLAGS})
add_test(test_half ${EXECUTABLE_OUTPUT_PATH}/test_half)

add_test(ruby_dl ruby -I${CMAKE_CURRENT_SOURCE_DIR}/../lib -I${CMAKE_CURRENT_BINARY_DIR}/../src ${CMAKE_CURRENT_SOURCE_DIR}/dl.rb)
