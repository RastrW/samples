cmake_minimum_required(VERSION 3.16)

project(qtitan2_cmake VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

#QTITANGRID[0]
# a) for Demo builded only Release config!
# b) near the output *.exe file must be lockated  dir "platforms" from  QTitan Demo bin folder, else
# you will get get "This application failed to start because no Qt platform plugin ..."
# c) also neaded QtitanBase2.dll and QtitanGrid8.dll

#QTITANGRID[1]
add_compile_definitions( QTN_NAMESPACE ) # !!!
set(DIR_QTITAN "C:/Qt/Developer_Machines/QtitanComponents2024.2.0_Demo/")
set(INCLUDES
    "${DIR_QTITAN}/include"
)
link_directories("${DIR_QTITAN}/bin/")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qtitan2_cmake
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qtitan2_cmake APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(qtitan2_cmake SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qtitan2_cmake
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(qtitan2_cmake PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

#QTITANGRID[2]
target_link_libraries( ${PROJECT_NAME} PRIVATE QtitanBase2 )
target_link_libraries( ${PROJECT_NAME} PRIVATE QtitanGrid8 )
target_include_directories( ${PROJECT_NAME} PRIVATE
  ${INCLUDES}
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.qtitan2_cmake)
endif()
set_target_properties(qtitan2_cmake PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS qtitan2_cmake
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qtitan2_cmake)
endif()
