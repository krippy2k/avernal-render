cmake_minimum_required(VERSION 3.28)

project(
    avernal-render
    VERSION 0.1.0
    DESCRIPTION "High-level rendering library for the Avernal game engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

include(FetchContent)

# Find avernal-core
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-core/CMakeLists.txt")
    set(AVERNAL_CORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-core")
else()
    set(AVERNAL_CORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../avernal-core")
endif()

# Find avernal-rhi
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-rhi/CMakeLists.txt")
    set(AVERNAL_RHI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../avernal-rhi")
else()
    set(AVERNAL_RHI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../avernal-rhi")
endif()

if(NOT EXISTS "${AVERNAL_CORE_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "avernal-core not found at:\n"
        "  ${AVERNAL_CORE_SOURCE_DIR}\n"
        "Place avernal-core as a sibling of this project.")
endif()

if(NOT EXISTS "${AVERNAL_RHI_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "avernal-rhi not found at:\n"
        "  ${AVERNAL_RHI_SOURCE_DIR}\n"
        "Place avernal-rhi as a sibling of this project.")
endif()

FetchContent_Declare(
    avernal_core
    SOURCE_DIR "${AVERNAL_CORE_SOURCE_DIR}"
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(
    avernal_rhi
    SOURCE_DIR "${AVERNAL_RHI_SOURCE_DIR}"
    EXCLUDE_FROM_ALL
)

set(AVERNAL_CORE_BUILD_DEMO OFF)
set(AVERNAL_RHI_BUILD_DEMO OFF)

FetchContent_MakeAvailable(avernal_core avernal_rhi)

add_library(avernal-render)
add_library(avernal::render ALIAS avernal-render)

target_sources(
    avernal-render
    PRIVATE
        src/renderer.cpp
        src/mesh.cpp
        src/material.cpp
        src/texture.cpp
        src/camera.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/avernal/render/render.hpp
            include/avernal/render/renderer.hpp
            include/avernal/render/mesh.hpp
            include/avernal/render/material.hpp
            include/avernal/render/texture.hpp
            include/avernal/render/camera.hpp
)

target_link_libraries(avernal-render PUBLIC avernal::core avernal::rhi)
target_compile_features(avernal-render PUBLIC cxx_std_23)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(avernal-render PRIVATE -Wall -Wextra -Wpedantic)
elseif(MSVC)
    target_compile_options(avernal-render PRIVATE /W4)
endif()
