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
    src/MazeGrid.h
    src/MazeGrid.cpp
    src/MazeGenerator.h
    src/MazeModel.h
    src/MazeModel.cpp
    src/MazeModelGeneration.cpp
    src/MazeModelSolving.cpp
    src/generators/GeneratorCatalog.h
    src/generators/GeneratorCatalog.cpp
    src/generators/BinaryTreeGenerator.h
    src/generators/BinaryTreeGenerator.cpp
    src/generators/DfsGenerator.h
    src/generators/DfsGenerator.cpp
    src/generators/GrowingTreeGenerator.h
    src/generators/GrowingTreeGenerator.cpp
    src/generators/HuntAndKillGenerator.h
    src/generators/HuntAndKillGenerator.cpp
    src/generators/DisjointSet.h
    src/generators/BoruvkaGenerator.h
    src/generators/BoruvkaGenerator.cpp
    src/generators/KruskalGenerator.h
    src/generators/KruskalGenerator.cpp
    src/generators/PrimGenerator.h
    src/generators/PrimGenerator.cpp
    src/generators/DivisionGenerator.h
    src/generators/DivisionGenerator.cpp
    src/generators/SidewinderGenerator.h
    src/generators/SidewinderGenerator.cpp
)

# So sources can include "MazeGrid.h" and "generators/DfsGenerator.h".
target_include_directories(QtMazeGenerator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(WIN32)
    target_sources(QtMazeGenerator PRIVATE resources/app.rc)
endif()

# Embed the stylesheet so the app never depends on a file next to the binary.
qt_add_resources(QtMazeGenerator "styles"
    PREFIX "/"
    BASE "resources"
    FILES resources/styles.qss
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
