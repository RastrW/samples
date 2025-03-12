cmake_minimum_required(VERSION 3.23) # 3.20 min version for "astra" build
project( qrastr VERSION 0.1 LANGUAGES CXX )

add_compile_definitions( QICSGRID_NO )
if(MSVC)
    add_compile_definitions( COMPILE_WIN )
    add_compile_definitions( _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS )
    add_compile_definitions( NOMINMAX )
    if (CMAKE_BUILD_TYPE MATCHES Debug)
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # Debug was fall on this with "__acrt_first_block == header", beacuse Qt have MtD heap but this have Mt heap
    endif()
endif()
add_compile_definitions(ASTRA_NIX)
add_compile_definitions(
        $<$<CONFIG:Debug>:q4_INLINE=0>
        $<$<CONFIG:Debug>:YYDEBUG>
)
add_compile_definitions( QTN_NAMESPACE ) ## !! IT'S CRYTICAL!!

#set(CMAKE_GENERATOR "Ninja")
message("generator is set to ${CMAKE_GENERATOR}")
set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
#set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}//..//${CMAKE_BUILD_TYPE}" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//${CMAKE_BUILD_TYPE}" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//${CMAKE_BUILD_TYPE}" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}//..//${CMAKE_BUILD_TYPE}" )
option( QRASTR_BUILD_WITH_MCR "QRASTR_BUILD_WITH_MCR" ON )
#set( COMPILE_DIR "C:/Projects/tfs/rastr/RastrWin/thirdparty/_deps/compile_cmake" CACHE PATH "Installation directory for third-party projects" )# for example C:\projects\rastr\RastrWin\thirdparty\_deps\compile
#set( COMPILE_DIR "C:/Projects/compile_cmake" )# for example C:\projects\rastr\RastrWin\thirdparty\_deps\compile
if( NOT IS_DIRECTORY ${COMPILE_DIR} )
    message(FATAL_ERROR "Not defined COMPILE_DIR")
else()
    message(NOTICE "COMPILE_DIR= ${COMPILE_DIR}")
endif()
list( PREPEND CMAKE_PREFIX_PATH ${COMPILE_DIR} )
list( PREPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/qmcr/" )

set( TS_FILES qrastr_ru_RU.ts )
if(MSVC)
    set( DEPENDENCIES_LOCATION "c:/projects/compile/" )
    #set( ASTRA_LOCATION        "c:/projects/tfs/rastr/RastrWin/astra/" )
    #set( ASTRA_LOCATION        "c:/projects/rastr/RastrWin/astra" )
else()
    set( DEPENDENCIES_LOCATION "/home/ustas/projects/compile/" )
    #set( ASTRA_LOCATION        "/home/ustas/projects/git_r/rastr/RastrWin/astra/" )
    #set( DEPENDENCIES_GIT      "/home/ustas/projects/git_r/" )
    set( DEPENDENCIES_GIT      "/home/ustas/projects/git_main/" )
endif()
if( NOT IS_DIRECTORY ${ASTRA_LOCATION} )
    message(FATAL_ERROR "Not defined ASTRA_LOCATION !") # example "c:/projects/rastr/RastrWin/astra"  "c:/projects/tfs/rastr/RastrWin/astra/"
else()
    message(NOTICE "ASTRA_LOCATION= ${ASTRA_LOCATION}")
endif()

find_package( Metakit REQUIRED )
find_package( fmt     REQUIRED )
#find_package(cryptopp REQUIRED)
find_package( QT NAMES Qt6 Qt5 Test COMPONENTS REQUIRED )
find_package( Qt${QT_VERSION_MAJOR} COMPONENTS
    Core
    Widgets
    LinguistTools
    PrintSupport
    Xml
    #widgets-private
    Test
    REQUIRED
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_standard_project_setup()
endif()
add_subdirectory("plugins/rastr")
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/qmcr/"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/qmcr"
)

set(INCLUDES
    "${ASTRA_LOCATION}/"
    #"${DEPENDENCIES_LOCATION}/fmt/include"
    "${ASTRA_LOCATION}/fmt/include/"
    "${DEPENDENCIES_LOCATION}/qicstable/include"
    "${DEPENDENCIES_LOCATION}/metakit/include" #-win
    "${DEPENDENCIES_GIT}/metakit/include"      #-lin
    "${DEPENDENCIES_LOCATION}/eigen-eigen"
    "${DEPENDENCIES_LOCATION}/boost/boost_1_76_0" # in_nix
    "${DEPENDENCIES_LOCATION}/boost/boost_1_80_0" # in_win
    "${DEPENDENCIES_LOCATION}/sci/scintilla/include"
    "${DEPENDENCIES_LOCATION}/sci/scintilla/src"
    "${DEPENDENCIES_LOCATION}/sci/scintilla/qt/ScintillaEditBase"
    "${DEPENDENCIES_LOCATION}/sci/scintilla/qt/ScintillaEdit"
    "${DEPENDENCIES_LOCATION}/Qt-Advanced-Docking-System/src"
    "${DEPENDENCIES_LOCATION}/spdlog/include/"
    "${ASTRA_LOCATION}/../../flytfs/IPlainRastrTest/"
    "${DEPENDENCIES_LOCATION}/QtitanDataGrid8.2.0_win_qt_6_7/include" #win
    "${DEPENDENCIES_LOCATION}/QtitanDataGrid8.2.0/include" #lin Qtitan
)

if(MSVC)
    link_directories("${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    link_directories("${ASTRA_LOCATION}/build/${CMAKE_BUILD_TYPE}/")
    link_directories("${DEPENDENCIES_LOCATION}/metakit/builds/")
    link_directories("${ASTRA_LOCATION}/../build-astra-vs-Release/")
    link_directories("${DEPENDENCIES_LOCATION}/Qt-Advanced-Docking-System/build/vs-Debug/lib/")
    link_directories("${DEPENDENCIES_LOCATION}/Qt-Advanced-Docking-System/build/vs-Release/lib/")
    link_directories("${DEPENDENCIES_LOCATION}/QtitanDataGrid8.2.0_win_qt_6_7/bin/")
else()
    link_directories("${DEPENDENCIES_LOCATION}/qicstable/lib/")
    link_directories("${ASTRA_LOCATION}/build/")
endif()

set(PROJECT_SOURCES
    app.h
    app.cpp
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mdiChildTable.h
    mdiChildTable.cpp
    mdiChildGrid.h
    mdiChildGrid.cpp
    mdiChildHeaderGrid.h
    mdiChildHeaderGrid.cpp
    rastrdatamodel.h
    rastrdatamodel.cpp
    astra_shared.h
    ColPropForm.h
    ColPropForm.cpp
    ColPropForm.ui
    ${TS_FILES}
    qrastr.qrc
    rmodel.cpp
    rmodel.h
    singleton_dclp.hpp
    params.h
    params.cpp
    common_qrastr.h
    common_qrastr.cpp
    ${ASTRA_LOCATION}/ResourceManager.cpp
    appsettings.json
    params.h
    params.cpp
    common_qrastr.h
    common_qrastr.cpp
    rastrhlp.h
    rastrhlp.cpp
    rdata.h
    rdata.cpp
    rtablesdatamanager.h
    rtablesdatamanager.cpp
    IDataBlocksWrappers.h
    IDataBlocksWrappers.cpp
    QDataBlocks.h
    QDataBlocks.cpp
    testmodel.h
    testmodel.cpp
    rtabwidget.h
    rtabwidget.cpp
    rtableview.h
    rtableview.cpp
    filtertableheader.h
    filtertableheader.cpp
    filterlineedit.h
    filterlineedit.cpp
    utils.h
    CondFormat.h
    CondFormat.cpp
    plugin_interfaces.h
    qastra.h
    qastra.cpp
    tsthints.h
    tsthints.cpp
    delegatecombobox.cpp
    delegatecombobox.h
    delegatedoubleitem.h
    delegatedoubleitem.cpp
    delegatecheckbox.h
    delegatecheckbox.cpp
    tsthintsmvc.h
    tsthintsmvc.cpp
    qastra_events_data.h
    formsettings.h
    formsettings.cpp
    formsettingsdatas.h
    formsettingsdatas.cpp
    formsettingsdatas.ui
    formsettingsforms.h
    formsettingsforms.cpp
    formsettingsforms.ui
    formsettingsonloadfiles.h
    formsettingsonloadfiles.cpp
    formsettingsonloadfiles.ui
    formsettingsstackeditem.h
    formsettingsstackeditem.cpp
    formsettingsonloadtemplates.h
    formsettingsonloadtemplates.cpp
    formsettingsonloadtemplates.ui
    formsettingsonloadforms.h
    formsettingsonloadforms.cpp
    formsettingsonloadforms.ui
    formfilenew.h
    formfilenew.cpp
    formfilenew.ui
    formselection.h
    formselection.cpp
    formselection.ui
    formprotocol.h
    formprotocol.cpp
    formprotocol.ui
    protocoltreeitem.h
    protocoltreeitem.cpp
    protocoltreemodel.h
    protocoltreemodel.cpp
    formselection.h
    formselection.cpp
    formselection.ui
    CondFormatManager.h
    CondFormatManager.cpp
    CondFormatManager.ui
    formselection.h
    formselection.cpp
    formselection.ui
    Palette.h
    Palette.cpp
    Settings.h
    Settings.cpp
    condformatjson.h
    condformatjson.cpp
    string_bool.h
    string_bool.cpp
    formexportcsv.h
    formexportcsv.cpp
    formexportcsv.ui
    qtitangrid.h
    qtitangrid.cpp
    formgroupcorrection.h
    formgroupcorrection.cpp
    formgroupcorrection.ui
    formimportcsv2.h
    formimportcsv2.cpp
    formimportcsv2.ui
    linkedform.h
    linkedform.cpp

)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)

qt_add_executable( ${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        formcalcidop.h formcalcidop.cpp formcalcidop.ui
    )
    qt_create_translation( QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES} )
else()
    add_executable( ${PROJECT_NAME}
        ${PROJECT_SOURCES}
    )
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()
target_link_libraries( ${PROJECT_NAME} PRIVATE Metakit::mk4         )
target_link_libraries( ${PROJECT_NAME} PRIVATE fmt::fmt-header-only )
#target_link_libraries( ${PROJECT_NAME} PRIVATE cryptopp::cryptopp )
if(MSVC)
    target_link_libraries( ${PROJECT_NAME} PRIVATE astra_py     ) # when build with pybind11
    target_link_libraries( ${PROJECT_NAME} PRIVATE qmcr         )
    target_link_libraries( ${PROJECT_NAME} PRIVATE Qt::WidgetsPrivate)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        target_link_libraries( ${PROJECT_NAME} PRIVATE qtadvanceddockingd )
        target_link_libraries( ${PROJECT_NAME} PRIVATE QtitanBased2 )
        target_link_libraries( ${PROJECT_NAME} PRIVATE QtitanGridd8 )
    else()
        target_link_libraries( ${PROJECT_NAME} PRIVATE qtadvanceddocking  )
        target_link_libraries( ${PROJECT_NAME} PRIVATE QtitanBase2 )
        target_link_libraries( ${PROJECT_NAME} PRIVATE QtitanGrid8 )
    endif()
else()
    target_link_libraries( ${PROJECT_NAME} PRIVATE libqicstable.so  )
    target_link_libraries( ${PROJECT_NAME} PRIVATE libastra_shrd.so )
    target_link_libraries( ${PROJECT_NAME} PRIVATE libqmcr.co       )
endif()
target_link_libraries( ${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Test
)
target_include_directories( ${PROJECT_NAME} PRIVATE
  ${INCLUDES}
)
set_target_properties( ${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable( ${PROJECT_NAME} )
endif()

# To Start qrastr.exe it is need to copy the QT directory "platforms" found in the QT plugins directory
# https://forum.qt.io/topic/119488/this-application-failed-to-start-because-no-qt-platform-plugin-could-be-initialsed/3
