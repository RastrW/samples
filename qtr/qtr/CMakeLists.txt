cmake_minimum_required(VERSION 3.23)

project(qtr VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    #add_compile_options( -fpermissive )
    add_compile_options( /permissive- )
    set( CMAKE_SYSTEM_VERSION 10.0.19041.0) # sets sdk version

    #O:\projects\qt\build\generators
    list(APPEND CMAKE_PREFIX_PATH "O:/projects/qt/build/generators/")

    #set( md4c_DIR               "O:/projects/qt/build/generators/")
    #set( PostgreSQL_DIR         "O:/projects/qt/build/generators/")
    #set( SQLite3_DIR            "O:/projects/qt/build/generators/")
    #set( Iconv_DIR              "O:/projects/qt/build/generators/")
    #set( Intl_DIR               "O:/projects/qt/build/generators/")
    #set( glib_DIR               "O:/projects/qt/build/generators/")
    #set( harfbuzz_DIR           "O:/projects/qt/build/generators/")
    #set( PCRE2_DIR              "O:/projects/qt/build/generators/")
    #set( libffi_DIR             "O:/projects/qt/build/generators/")
    #set( brotli_DIR             "O:/projects/qt/build/generators/")
    #set( PNG_DIR                "O:/projects/qt/build/generators/")
    #set( BZip2_DIR              "O:/projects/qt/build/generators/")
    #set( freetype_DIR           "O:/projects/qt/build/generators/")
    #set( double-conversion_DIR  "O:/projects/qt/build/generators/")
    #set( ZLIB_DIR               "O:/projects/qt/build/generators/")
    #set( ZLIB_LIBRARY           "O:/projects/qt/build/generators/")

    #set( OpenSSL_DIR            "O:/projects/qt/build/generators/")
    #set( OPENSSL_ROOT_DIR       "O:/projects/qt/build/generators/")
    #set( OPENSSL_CRYPTO_LIBRARY "O:/projects/qt/build/generators/")


    #include("O:/projects/qt/build/generators/md4c-config.cmake")
    #include("O:/projects/qt/build/generators/PostgreSQLConfig.cmake")
    #include("O:/projects/qt/build/generators/SQLite3Config.cmake")
    #include("O:/projects/qt/build/generators/IconvConfig.cmake")
    #include("O:/projects/qt/build/generators/IntlConfig.cmake")
    #include("O:/projects/qt/build/generators/PCRE2Config.cmake")
    #include("O:/projects/qt/build/generators/libffi-config.cmake")
    #include("O:/projects/qt/build/generators/glib-config.cmake")
    #include("O:/projects/qt/build/generators/brotli-config.cmake")
    #include("O:/projects/qt/build/generators/BZip2Config.cmake")
    #include("O:/projects/qt/build/generators/freetype-config.cmake")
    #include("O:/projects/qt/build/generators/harfbuzz-config.cmake")
    #include("O:/projects/qt/build/generators/double-conversion-config.cmake")
    #include("O:/projects/qt/build/generators/ZLIBConfig.cmake")
    #include("O:/projects/qt/build/generators/OpenSSLConfig.cmake")
    #include("O:/projects/qt/build/generators/Qt6Config.cmake")


    #find_package(ZLIB REQUIRED)
    #find_package(OpenSSL REQUIRED)
endif()


find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools REQUIRED)

set(TS_FILES tst_lng_ru_RU.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        mdichild.h
        mdichild.cpp
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qtr
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qtr APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(qtr SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qtr
            ${PROJECT_SOURCES}
        )
    endif()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

if(MSVC)
    target_compile_options(qtr PUBLIC "/Zc:__cplusplus")

    #include("C:/Users/ustas/.conan2/p/qt683f93dba072b/p/include/QtWidgets/")
    target_include_directories(qtr PRIVATE "C:/Users/ustas/.conan2/p/qt683f93dba072b/p/include/QtWidgets/")
    target_include_directories(qtr PRIVATE "C:/Users/ustas/.conan2/p/qt683f93dba072b/p/include/")
    target_include_directories(qtr PRIVATE "C:/Users/ustas/.conan2/p/qt683f93dba072b/p/include/QtCore")

    #link_directories( qtr "C:/Users/ustas/.conan2/p/qt683f93dba072b/p/lib" )
    #target_link_directories( qtr "C:/Users/ustas/.conan2/p/qt683f93dba072b/p/lib" )
    #target_link_libraries( astra PRIVATE mk4vc22x64    ) # libmk4.so
    #target_link_libraries( qtr PRIVATE "C:/Users/ustas/.conan2/p/qt683f93dba072b/p/lib/Qt6Core.lib"  ) 

    #set_property(TARGET qtr PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>") 
endif()    

target_link_libraries(qtr PRIVATE Qt${QT_VERSION_MAJOR}::Widgets )

set_target_properties(qtr PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qtr)
endif()
