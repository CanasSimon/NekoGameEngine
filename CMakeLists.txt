cmake_minimum_required(VERSION 3.16)
project(Neko)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Includes different utility functions
include(cmake/data_generation.cmake)
include(cmake/neko_executable.cmake)
include(cmake/test.cmake)

# Selects the correct toolchain to use
message(STATUS "CMake System Name: ${CMAKE_SYSTEM_NAME}")
if (MSVC)
    include(cmake/platforms/msvc.cmake)
elseif (UNIX)
	string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
    if (BUILD_TYPE STREQUAL "DEBUG")
        message(VERBOSE "Enabled '-fstandalone-debug' for LLDB")
        add_compile_options(-fstandalone-debug)
    endif ()

    execute_process(COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE)
    message(STATUS "UNIX Architecture: ${ARCHITECTURE}")
    if ("${ARCHITECTURE}" MATCHES "arm")
        include(cmake/platforms/arm.cmake)
    elseif ("${ARCHITECTURE}" MATCHES "aarch")
        include(cmake/platforms/arm.cmake)
    else ()
        include(cmake/platforms/linux.cmake)
    endif ()
endif ()

# Sets paths
set_property(GLOBAL PROPERTY USE_FOLDERS On)
set(NEKO_CORE_DIR       "${CMAKE_SOURCE_DIR}/core")
set(EXTERNAL_DIR        "${CMAKE_SOURCE_DIR}/externals")
set(MAIN_PROJECTS_DIR   "${CMAKE_SOURCE_DIR}/main")
set(UTILITIES_DIR       "${EXTERNAL_DIR}/utilities")

# Different CMake variables
set(Aer_Lib         ON  CACHE BOOL "Activate Aer_Lib")
set(Neko_Assert     ON  CACHE BOOL "Activate Neko Assertion")
set(Neko_Benchmark  OFF CACHE BOOL "Activate Benchmark")
set(Neko_FMOD       ON  CACHE BOOL "Activate FMOD")
set(Neko_Freetype   ON  CACHE BOOL "Activate Freetype")
set(Neko_OpenGL     ON  CACHE BOOL "Activate OpenGL")
set(Neko_ImGui      ON  CACHE BOOL "Activate ImGui")
set(Neko_PhysFS     ON  CACHE BOOL "Activate PhysFS")
set(Neko_PhysX      ON  CACHE BOOL "Activate PhysX")
set(Neko_Profile    OFF CACHE BOOL "Activate Easy Profile")
set(Neko_SDL2       ON  CACHE BOOL "Activate SDL2")
set(Neko_Test       ON  CACHE BOOL "Activate Neko Test")
set(Neko_Vulkan     OFF CACHE BOOL "Activate Vulkan")

# Enables google test
if(Neko_Test)
    find_package(GTest CONFIG REQUIRED)

    add_library(gtest ALIAS GTest::gtest)
    add_library(gtest_main ALIAS GTest::gtest_main)
endif()

# Enables neko_assert
if(Neko_Assert)
    add_compile_definitions("NEKO_ASSERT=1")
endif()

# Enables Google Benchmark
if (Neko_Benchmark)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
    set(HAVE_STD_REGEX ON CACHE BOOL "")

    add_subdirectory("${EXTERNAL_DIR}/benchmark")
    add_subdirectory("benchmark/")
    neko_lib_config(benchmark)

    set_target_properties(benchmark PROPERTIES FOLDER Externals/GTest)
    set_target_properties(benchmark_main PROPERTIES FOLDER Externals/GTest)
endif ()

# Enables FMOD
if (Neko_FMOD)
    add_compile_definitions("NEKO_FMOD=1")
    set(FMOD_ROOT "${EXTERNAL_DIR}/fmod")
    if (WIN32)
        include_directories("${FMOD_ROOT}/win_x64/inc")
    elseif (UNIX)
        include_directories("${FMOD_ROOT}/linux_x86_64/inc")
    endif ()
    add_subdirectory(${FMOD_ROOT})
    neko_lib_config(fmod)

    add_subdirectory(common/fmod_wrapper)
endif ()

# Enables Freetype
if (Neko_Freetype)
    find_package(freetype CONFIG REQUIRED)
endif ()

# Assimp
find_package(assimp CONFIG REQUIRED)
add_library(assimp ALIAS assimp::assimp)

# Tiny obj loader
add_compile_definitions("TINYOBJLOADER_IMPLEMENTATION=1")

# Enables the Render Engine
if (Neko_OpenGL AND NOT Neko_Vulkan)
    add_compile_definitions("NEKO_OPENGL=1")

    find_package(GLEW CONFIG REQUIRED)
    add_library(glew ALIAS GLEW::GLEW)

    find_package(gli CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)
    add_library(glm ALIAS glm::glm)

    set(OPENGL_WRAPPER_DIR "common/opengl_wrapper" CACHE INTERNAL "")
    add_subdirectory(${OPENGL_WRAPPER_DIR})
elseif (Neko_Vulkan)
    add_compile_definitions("NEKO_VULKAN=1")

    set(VULKAN_WRAPPER_DIR "common/vulkan_wrapper" CACHE INTERNAL "")
    add_subdirectory(${VULKAN_WRAPPER_DIR})
endif ()

# Enables ImGui
if (Neko_ImGui)
    add_compile_definitions("NEKO_IMGUI=1")
    add_compile_definitions("IMGUI_USER_CONFIG=<utils/imconfig.h>")

    find_package(imgui CONFIG REQUIRED)
    add_library(imgui ALIAS imgui::imgui)
endif ()

# Enables PhysFS
if(Neko_PhysFS)
    add_subdirectory(common/physfs_wrapper)
endif()

# Enables PhysX
if(Neko_PhysX)
    add_compile_definitions("NDEBUG=1")
    add_subdirectory("common/physx_wrapper")
endif()

# Enables Easy Profile
if (Neko_Profile)
    add_compile_definitions("EASY_PROFILE_USE=1")
    add_subdirectory("${EXTERNAL_DIR}/easy_profiler")

    set_target_properties(easy_profiler PROPERTIES FOLDER Externals)
    set_target_properties(profiler_converter PROPERTIES FOLDER Externals)
    set_target_properties(profiler_sample PROPERTIES FOLDER Externals)
    set_target_properties(profiler_reader PROPERTIES FOLDER Externals)
    set_target_properties(profiler_sample_disabled_profiler PROPERTIES FOLDER Externals)
endif ()

# Enables SDL2
if (Neko_SDL2)
    find_package(SDL2 CONFIG REQUIRED)
    add_library(SDL2main ALIAS SDL2::SDL2main)
    add_library(SDL2-static ALIAS SDL2::SDL2-static)

    add_subdirectory("common/sdl_engine")
    target_precompile_headers(sdl_engine PRIVATE "common/sdl_engine/include/sdl_wrapper_pch.h")
endif ()

# Enables aer_lib
if (Aer_Lib)
    add_subdirectory("common/aer_lib")
endif ()

# Includes Neko_Core
add_subdirectory(${NEKO_CORE_DIR})
target_precompile_headers(Neko_Core PRIVATE "${NEKO_CORE_DIR}/include/core_pch.h")

# Link extra libraries
find_package(fmt CONFIG REQUIRED)
find_package(units CONFIG REQUIRED)
target_link_libraries(Neko_Core PUBLIC fmt::fmt units::units)

find_path(STB_INCLUDE_DIRS "stb.h")
target_include_directories(Neko_Core PUBLIC ${STB_INCLUDE_DIRS})

# Includes executables located in ${MAIN_PROJECTS_DIR}
message(STATUS "Looking for executables...")
file(GLOB MAIN_PROJECTS "${MAIN_PROJECTS_DIR}/*")
foreach (MAIN_PROJECT_PATH ${MAIN_PROJECTS})
    get_filename_component(MAIN_PROJECT_NAME ${MAIN_PROJECT_PATH} NAME)

    message(VERBOSE "${MAIN_PROJECT_PATH} found at: '${MAIN_PROJECT_NAME}'")
    set(${MAIN_PROJECT_NAME} ON CACHE BOOL "")
    if (${MAIN_PROJECT_NAME})
        add_subdirectory(${MAIN_PROJECT_PATH})
    endif ()
endforeach ()