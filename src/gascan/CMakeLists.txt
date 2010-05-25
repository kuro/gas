
project(gascan)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/../gas
    )

set(sources
    test.cpp main.cpp
    xml2gas.cpp
    gas2xml.cpp
    bin2c.cpp
    )

if (QT_QTGUI_FOUND)
    set(sources
        ${sources}
        qtedit.h qtedit.cpp
        EditWindow.h EditWindow.cpp
        GasTreeModel.h GasTreeModel.cpp
        GasTableModel.h GasTableModel.cpp
        )

    qt4_wrap_ui(ui_files EditWindow.ui)
endif()

qt4_automoc(${sources})

add_executable(gascan ${sources} ${ui_files})

target_link_libraries(gascan gas GasQt ${QT_LIBRARIES})

#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/gascan${CMAKE_EXECUTABLE_SUFFIX} DESTINATION bin)
install(TARGETS gascan RUNTIME DESTINATION bin)
