cmake_minimum_required(VERSION 3.5)

project(qrastr VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_definitions(COMPILE_WIN)
    add_compile_definitions(QICSGRID_NO)
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools   REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools PrintSupport Xml  REQUIRED)

set(TS_FILES qrastr_ru_RU.ts)

if(MSVC)
    set(DEPENDENCIES_LOCATION "c:/projects/compile/")
    set(ASTRA_LOCATION        "c:/projects/rastr/RastrWin/astra/")
else()
    set(DEPENDENCIES_LOCATION "/home/ustas/projects/compile/")
    set(ASTRA_LOCATION "/home/ustas/projects/git_r/rastr/RastrWin/astra/")
endif()

set(INCLUDES
        "${DEPENDENCIES_LOCATION}/fmt/include"
        "${DEPENDENCIES_LOCATION}/qicstable/include"
        "${ASTRA_LOCATION}/"
)

if(MSVC)
    link_directories("${ASTRA_LOCATION}/build/Debug/")
else()
    link_directories("${DEPENDENCIES_LOCATION}/qicstable/lib/")
    link_directories("${ASTRA_LOCATION}/build/")
endif()


set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    mdiChildTable.h
    mdiChildTable.cpp
    mdiChildGrid.h
    mdiChildGrid.cpp
    mdiChildHeaderGrid.h
    mdiChildHeaderGrid.cpp
    rastrdatamodel.h
    rastrdatamodel.cpp
    astra_shared.h
    ${TS_FILES}
    qrastr.qrc
    ${DEPENDENCIES_LOCATION}/fmt/src/format.cc
    ${DEPENDENCIES_LOCATION}/fmt/src/os.cc
    mymodel.cpp
    mymodel.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qrastr
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qrastr APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(qrastr SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qrastr
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries( qrastr PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::PrintSupport Qt${QT_VERSION_MAJOR}::Xml ) #Qt5::PrintSupport

if(MSVC)
    target_link_libraries( qrastr PRIVATE astra_shrd.lib )
else()
    target_link_libraries( qrastr PRIVATE libqicstable.so )
    target_link_libraries( qrastr PRIVATE libastra_shrd.so )
endif()

target_include_directories(qrastr PRIVATE
  ${INCLUDES}
)

set_target_properties(qrastr PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qrastr)
endif()
