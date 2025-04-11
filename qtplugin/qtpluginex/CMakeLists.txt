#cmake_minimum_required(VERSION 3.5)
#project(qtpluginex LANGUAGES CXX)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#add_executable(qtpluginex main.cpp)

#get from https://stackoverflow.com/questions/30628593/using-cmake-with-qt5-and-qt-plugins

project(test-plugin)
cmake_minimum_required(VERSION 3.5.0)
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )


set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)

#set(COMPILE_OPTIONS -std=c++11)

#add_library(test-plugin SHARED test-plugin-interface.hpp test-plugin.hpp  test-plugin.cpp )
add_library(test-plugin SHARED test-plugin.hpp  test-plugin.cpp )
target_link_libraries(test-plugin PRIVATE Qt5::Widgets)
target_compile_options(test-plugin PRIVATE ${COMPILE_OPTIONS})

#add_executable(test-plugin-exec test-plugin-interface.hpp test-plugin.hpp main.cpp )
#add_executable(test-plugin-exec  main.cpp )
#target_link_libraries(test-plugin-exec PRIVATE Qt5::Widgets)
#target_compile_options(test-plugin-exec PRIVATE ${COMPILE_OPTIONS})
