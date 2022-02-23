cmake_minimum_required(VERSION 3.19)
project(BmCustomPlot)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(QT_VERSION 5)
set(REQUIRED_LIBS Core Gui Widgets PrintSupport)
set(REQUIRED_LIBS_QUALIFIED Qt5::Core Qt5::Gui Qt5::Widgets Qt5::PrintSupport)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB HEAD_FILES ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB UI_FILES ${PROJECT_SOURCE_DIR}/include/*.ui)
file(GLOB QRC_FILES *.qrc)

add_executable(${PROJECT_NAME} main.cpp ${HEAD_FILES} ${SRC_FILES} ${UI_FILES} ${QRC_FILES})


find_package(Qt${QT_VERSION} COMPONENTS ${REQUIRED_LIBS} REQUIRED)
target_link_libraries(${PROJECT_NAME} ${REQUIRED_LIBS_QUALIFIED})
