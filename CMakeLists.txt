cmake_minimum_required(VERSION 3.16)
project(JajaBoard VERSION 2.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport)
find_package(Qt6 QUIET COMPONENTS WebSockets)
qt_standard_project_setup()

set(SOURCES
    src/main.cpp src/mainwindow.cpp src/canvas.cpp src/pptimporter.cpp
    src/screencap.cpp src/projectfile.cpp src/collab.cpp src/ocrengine.cpp
)

# 注意这里加上了 resources.qrc
qt_add_executable(JajaBoard ${SOURCES} resources.qrc)

target_link_libraries(JajaBoard PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)

if (Qt6WebSockets_FOUND)
    target_link_libraries(JajaBoard PRIVATE Qt6::WebSockets)
    target_compile_definitions(JajaBoard PRIVATE JAJA_HAVE_WEBSOCKETS)
endif()