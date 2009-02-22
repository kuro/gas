
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(sources test.cpp main.cpp gas2c.cpp)

if (QT4_FOUND)
    include_directories(
        ${QT_INCLUDE_DIR}
        ${QT_QTCORE_INCLUDE_DIR}
        ${QT_QTGUI_INCLUDE_DIR}
        ${QT_QTXML_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        )
    add_definitions(-DHAVE_QT4=1)
    qt4_wrap_ui(ui_files EditWindow.ui)
    set(sources ${sources} xml2gas.cpp)
    set(sources ${sources} qtedit.h qtedit.cpp)
    set(sources ${sources}
        ${ui_files}
        EditWindow.h EditWindow.cpp
        GasTreeModel.h GasTreeModel.cpp
        GasTableModel.h GasTableModel.cpp
        )
    qt4_automoc(${sources})
    link_libraries(
        gas-qt
        ${QT_QTCORE_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTXML_LIBRARY}
        )
else ()
    add_definitions(-DHAVE_QT4=0)
endif ()

add_executable(gascan ${sources})

target_link_libraries(gascan
    gas
    )

#install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/gascan${CMAKE_EXECUTABLE_SUFFIX} DESTINATION bin)
install(TARGETS gascan RUNTIME DESTINATION bin)
