
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

qt4_wrap_ui(ui_files EditWindow.ui)

set(sources
    test.cpp main.cpp
    xml2gas.cpp
    bin2c.cpp
    qtedit.h qtedit.cpp
    EditWindow.h EditWindow.cpp
    GasTreeModel.h GasTreeModel.cpp
    GasTableModel.h GasTableModel.cpp
    )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

qt4_automoc(${sources})

add_executable(gascan ${sources} ${ui_files})

target_link_libraries(gascan gas ${QT_LIBRARIES})

#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/gascan${CMAKE_EXECUTABLE_SUFFIX} DESTINATION bin)
install(TARGETS gascan RUNTIME DESTINATION bin)
