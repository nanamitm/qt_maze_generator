cmake_minimum_required(VERSION 3.21)
project(QtMazeGenerator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

qt_standard_project_setup()

qt_add_executable(QtMazeGenerator
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/MazeCanvas.h
    src/MazeCanvas.cpp
    src/MazeModel.h
    src/MazeModel.cpp
    src/MazeModelGeneration.cpp
    src/MazeModelSolving.cpp
)

if(WIN32)
    target_sources(QtMazeGenerator PRIVATE resources/app.rc)
endif()

add_custom_command(TARGET QtMazeGenerator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/styles.qss
        $<TARGET_FILE_DIR:QtMazeGenerator>/styles.qss
)

target_link_libraries(QtMazeGenerator PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

if(WIN32)
    set_target_properties(QtMazeGenerator PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
