cmake_minimum_required(VERSION 3.16)

project(qastra LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD 17) #20
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core LinguistTools)

 add_compile_definitions(PLAIN_RASTR_EMBED)

#C:\projects\rastr\RastrWin\astra
#C:\projects\rastr\flytfs\IPlainRastrTest
include_directories(
    "C:/projects/rastr/flytfs/IPlainRastrTest/"
    "C:/projects/rastr/RastrWin/astra/"
)

set(TS_FILES qastra_ru_RU.ts)

add_executable(qastra
  main.cpp
  ${TS_FILES}
  qastra.h qastra.cpp
)
target_link_libraries(qastra Qt${QT_VERSION_MAJOR}::Core)

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

include(GNUInstallDirs)
install(TARGETS qastra
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
