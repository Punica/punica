cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 11)

project (plugin_without_create)
set(CMAKE_BUILD_TYPE Release)

set(PLUGIN_WITHOUT_CREATE_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PUNICA_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../)

include_directories(${PUNICA_ROOT_DIR}/include)

add_library(plugin_without_create SHARED ${PLUGIN_WITHOUT_CREATE_SOURCES_DIR}/plugin_without_create.cpp)
