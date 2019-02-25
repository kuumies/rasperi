cmake_minimum_required(VERSION 3.0.0)
project(rasperi)

#---------------------------------------------------------------------
# Setup compiler

if(MSVC)
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DNOMINMAX)
    add_definitions(-D_DISABLE_EXTENDED_ALIGNED_STORAGE)

else(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -ggdb -g3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif(MSVC)

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)
add_definitions(-DGLM_FORCE_CTOR_INIT)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#---------------------------------------------------------------------
# Set variables

set (CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "" FORCE)

string (TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
if (BUILD_TYPE STREQUAL "debug")
    set (IS_DEBUG "ON")
endif(BUILD_TYPE STREQUAL "debug")

#---------------------------------------------------------------------
# External libraries that are prebuild

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/external/assimp
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/external/qt
)

include(add_assimp)
include(add_glm)
include(add_qt)

find_package(openmp REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")

#---------------------------------------------------------------------
# System install

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)
include(InstallRequiredSystemLibraries)

#---------------------------------------------------------------------
# Automate Qt

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#---------------------------------------------------------------------
# Application sources

file(GLOB_RECURSE H_SOURCES       src/*.h)
file(GLOB_RECURSE CPP_SOURCES     src/*.cpp)
file(GLOB_RECURSE UI_SOURCES      src/*.ui)
file(GLOB_RECURSE QRC_SOURCES     resource/*.qrc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(
    ${PROJECT_NAME}
    ${H_SOURCES}
    ${CPP_SOURCES}
    ${UI_SOURCES}
    ${QRC_SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME}
    ${QT_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
INSTALL(DIRECTORY resource/models   DESTINATION bin)
