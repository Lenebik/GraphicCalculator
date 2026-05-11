cmake_minimum_required(VERSION 3.16)
project(GraphicCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH
        "/opt/homebrew/opt/qt"
        "/opt/homebrew/opt/qt6"
        "/usr/local/opt/qt"
        "/usr/local/opt/qt6")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

set(SOURCES
    main.cpp
    src/MainWindow.cpp
    src/widgets/InputField.cpp
    src/widgets/KeyboardWidget.cpp
    src/widgets/GraphWidget.cpp
    src/widgets/SettingsPanel.cpp
    src/parser/Tokenizer.cpp
    src/parser/Parser.cpp
    src/parser/Evaluator.cpp
)

set(HEADERS
    src/MainWindow.h
    src/widgets/InputField.h
    src/widgets/KeyboardWidget.h
    src/widgets/GraphWidget.h
    src/widgets/SettingsPanel.h
    src/parser/Token.h
    src/parser/AstNode.h
    src/parser/Tokenizer.h
    src/parser/Parser.h
    src/parser/Evaluator.h
)

qt_add_executable(GraphicCalculator
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(GraphicCalculator PRIVATE Qt6::Widgets)
target_include_directories(GraphicCalculator PRIVATE ${CMAKE_SOURCE_DIR}/src)

set_target_properties(GraphicCalculator PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
