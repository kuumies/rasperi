#---------------------------------------------------------------------

project(gladlib)
cmake_minimum_required(VERSION 2.8.11)

#---------------------------------------------------------------------

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
set(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

#---------------------------------------------------------------------

set(SOURCES
    include/glad/glad.h
    include/KHR/khrplatform.h
    src/glad.c
)

#---------------------------------------------------------------------

add_library(${PROJECT_NAME} STATIC ${SOURCES} )

target_link_libraries(
    ${PROJECT_NAME}
)
