cmake_minimum_required(VERSION 3.0.0)
project(nodepp-torify VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(NODEPP_TORIFY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Search for Nodepp Core
if (NOT TARGET nodepp)
    find_package(nodepp REQUIRED)
    if(NOT nodepp_FOUND)
        message(FATAL_ERROR "Nodepp not found. Please install nodepp.")
    endif()
endif()

# interface-target
add_library(nodepp-torify INTERFACE)
target_include_directories(nodepp-torify INTERFACE ${NODEPP_TORIFY_INCLUDE_DIR})
target_link_libraries(nodepp-torify INTERFACE nodepp ) #-----------------------#