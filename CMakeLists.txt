cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME yuntai_v2)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

enable_language(C CXX ASM)

add_executable(${CMAKE_PROJECT_NAME})

add_subdirectory(cmake/stm32cubemx)

target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

function(include_sub_directories_recursively root_dir)
    if(IS_DIRECTORY ${root_dir})
        include_directories(${root_dir})
    endif()

    file(GLOB children LIST_DIRECTORIES true ${root_dir}/*)
    foreach(child ${children})
        if(IS_DIRECTORY ${child})
            include_sub_directories_recursively(${child})
        endif()
    endforeach()
endfunction()

file(GLOB_RECURSE SOURCES
    "APP/*.c"
    "APP/*.cpp"
    "BSP/*.c"
    "BSP/*.cpp"
    "Module/*.c"
    "Module/*.cpp"
    "Algorithm/*.c"
    "Algorithm/*.cpp"
)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${SOURCES}
)

include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/APP)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/BSP)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Module)
include_sub_directories_recursively(${CMAKE_SOURCE_DIR}/Algorithm)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    LINKER:-u,_printf_float
    LINKER:-u,_scanf_float
)
