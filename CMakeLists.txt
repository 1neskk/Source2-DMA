cmake_minimum_required(VERSION 3.17)
project(CS2-DMA CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_WIN32_EXECUTABLE ON)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
add_executable(${PROJECT_NAME} ${SOURCES})
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/DMALibrary)

set(IMGUI_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_win32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/backends/imgui_impl_dx11.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE d3d11)

add_library(DMALibrary STATIC IMPORTED)
set_target_properties(DMALibrary PROPERTIES IMPORTED_LOCATION
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/DMALibrary.lib")

add_library(VMM STATIC IMPORTED)
set_target_properties(VMM PROPERTIES IMPORTED_LOCATION
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/vmm.lib")

add_library(LeechCore STATIC IMPORTED)
set_target_properties(LeechCore PROPERTIES IMPORTED_LOCATION
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/leechcore.lib")

target_link_libraries(${PROJECT_NAME} PRIVATE DMALibrary VMM LeechCore)

set(ARCHITECTURE "x64")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})