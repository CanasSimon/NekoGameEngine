# Copied from https://github.com/microsoft/vcpkg/pull/13849#issuecomment-724370147
set(PX_GENERATE_STATIC_LIBRARIES ON CACHE BOOL "Generate static libraries")

file(GLOB_RECURSE PHYSX_WRAPPER_SRC include/*.h src/*.cpp)

list(APPEND PHYSX_WRAPPER_LIBS aer_lib Neko_Core)
if (Neko_OpenGL AND NOT Neko_Vulkan)
    list(APPEND PHYSX_WRAPPER_LIBS opengl_wrapper)
elseif (Neko_Vulkan)
    list(APPEND PHYSX_WRAPPER_LIBS vulkan_wrapper)
endif ()

include(FindPackageHandleStandardArgs)
include(SelectLibraryConfigurations)

if (CMAKE_SIZEOF_VOID_P EQUAL "8")
    set(PHYSX_LIBRARY_SUFFIX "64")
else ()
    set(PHYSX_LIBRARY_SUFFIX "32")
endif ()

set(PHYSX_COMMON_LIBRARY_NAMES
        PhysXCommon_${PHYSX_LIBRARY_SUFFIX}
        PhysXCommon_static_${PHYSX_LIBRARY_SUFFIX}
        libPhysXCommon_${PHYSX_LIBRARY_SUFFIX}
        libPhysXCommon_static_${PHYSX_LIBRARY_SUFFIX})
set(PHYSX_PHYSICS_LIBRARY_NAMES
        PhysX_${PHYSX_LIBRARY_SUFFIX}
        PhysX_static_${PHYSX_LIBRARY_SUFFIX}
        libPhysX_${PHYSX_LIBRARY_SUFFIX}
        libPhysX_static_${PHYSX_LIBRARY_SUFFIX})
set(PHYSX_FOUNDATION_LIBRARY_NAMES
        PhysXFoundation_${PHYSX_LIBRARY_SUFFIX}
        PhysXFoundation_static_${PHYSX_LIBRARY_SUFFIX}
        libPhysXFoundation_${PHYSX_LIBRARY_SUFFIX}
        libPhysXFoundation_static_${PHYSX_LIBRARY_SUFFIX})
set(PHYSX_COOKING_LIBRARY_NAMES
        PhysXCooking_${PHYSX_LIBRARY_SUFFIX}
        PhysXCooking_static_${PHYSX_LIBRARY_SUFFIX}
        libPhysXCooking_${PHYSX_LIBRARY_SUFFIX}
        libPhysXCooking_static_${PHYSX_LIBRARY_SUFFIX})
set(PHYSX_EXTENSIONS_LIBRARY_NAMES
        PhysXExtensions_${PHYSX_LIBRARY_SUFFIX}
        PhysXExtensions_static_${PHYSX_LIBRARY_SUFFIX}
        libPhysXExtensions_${PHYSX_LIBRARY_SUFFIX}
        libPhysXExtensions_static_${PHYSX_LIBRARY_SUFFIX})
set(PHYSX_CHARACTER_LIBRARY_NAMES
        PhysXCharacterKinematic_${PHYSX_LIBRARY_SUFFIX}
        PhysXCharacterKinematic_static_${PHYSX_LIBRARY_SUFFIX}
        libPhysXCharacterKinematic_${PHYSX_LIBRARY_SUFFIX}
        libPhysXCharacterKinematic_static_${PHYSX_LIBRARY_SUFFIX})
set(PHYSX_PVD_LIBRARY_NAMES
        PhysXPvdSDK_${PHYSX_LIBRARY_SUFFIX}
        PhysXPvdSDK_static_${PHYSX_LIBRARY_SUFFIX}
        libPhysXPvdSDK_${PHYSX_LIBRARY_SUFFIX}
        libPhysXPvdSDK_static_${PHYSX_LIBRARY_SUFFIX})

# Stoopid vcpkg build debug and optimized libraries with the same name but in different directories.
foreach (PREFIX_PATH ${CMAKE_PREFIX_PATH})
    if (${PREFIX_PATH} MATCHES "[Dd][Ee][Bb][Uu][Gg]\/?$")
        list(APPEND PHYSX_DEBUG_PATHS ${PREFIX_PATH})
    else ()
        list(APPEND PHYSX_RELEASE_PATHS ${PREFIX_PATH})
    endif ()
    list(APPEND PHYSX_PREFIX ${PREFIX_PATH})
endforeach ()

# For non-vcpkg builds... *Gulp*
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if (MSVC)
        set(PHYSX_BIN_DIR "win.")
        if (CMAKE_SIZEOF_VOID_P EQUAL "8")
            string(APPEND PHYSX_BIN_DIR "x86_64.")
        else ()
            string(APPEND PHYSX_BIN_DIR "x86.")
        endif ()
        string(APPEND PHYSX_BIN_DIR "vc${MSVC_TOOLSET_VERSION}.mt")
    endif ()

    if (MSVC_TOOLSET_VERSION EQUAL 120)
        set(PHYSX_DIR "vc12win${PHYSX_LIBRARY_SUFFIX}")
        # There is no toolset version 130...
    elseif (MSVC_TOOLSET_VERSION EQUAL 140)
        set(PHYSX_DIR "vc14win${PHYSX_LIBRARY_SUFFIX}")
    elseif (MSVC_TOOLSET_VERSION EQUAL 141)
        set(PHYSX_DIR "vc15win${PHYSX_LIBRARY_SUFFIX}")
    elseif (MSVC_TOOLSET_VERSION EQUAL 142)
        set(PHYSX_DIR "vc16win${PHYSX_LIBRARY_SUFFIX}")
    elseif (NOT VCPKG_TOOLCHAIN)
        message(WARNING "PhysX: Unhandled MSVC Toolset ${MSVC_TOOLSET_VERSION}. You may need to manually specify artifacts.")
    endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PHYSX_DIR "linux")
    set(PHYSX_BIN_DIR "linux.clang")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PHYSX_DIR "mac64")
    set(PHYSX_BIN_DIR "mac64.xcode")
elseif (NOT VCPKG_TOOLCHAIN)
    message(WARNING "PhysX: Unhandled system: ${CMAKE_SYSTEM_NAME}. You may need to manually specify artifacts.")
endif ()

if (PHYSX_DIR)
    foreach (PREFIX_PATH ${CMAKE_PREFIX_PATH})
        if (PHYSX_BIN_DIR)
            list(APPEND PHYSX_DEBUG_PATHS "${PREFIX_PATH}/${PHYSX_DIR}/PhysX/bin/${PHYSX_BIN_DIR}/debug")
            list(APPEND PHYSX_RELEASE_PATHS "${PREFIX_PATH}/${PHYSX_DIR}/PhysX/bin/${PHYSX_BIN_DIR}/release")
        elseif (NOT VCPKG_TOOLCHAIN)
            list(APPEND PHYSX_DEBUG_PATHS ${PREFIX_PATH})
            list(APPEND PHYSX_RELEASE_PATHS ${PREFIX_PATH})
        endif ()
        list(APPEND PHYSX_PREFIX "${PREFIX_PATH}/${PHYSX_DIR}/PhysX")
        list(APPEND PHYSX_PREFIX "${PREFIX_PATH}/${PHYSX_DIR}/PxShared")
    endforeach ()
elseif (NOT VCPKG_TOOLCHAIN)
    set(PHYSX_DEBUG_PATHS ${CMAKE_PREFIX_PATH})
    set(PHYSX_RELEASE_PATHS ${CMAKE_PREFIX_PATH})
endif ()

macro(find_physx_library VAR_NAME)
    find_library(${VAR_NAME}_LIBRARY_RELEASE
            NAMES ${${VAR_NAME}_LIBRARY_NAMES}
            PATHS ${PHYSX_RELEASE_PATHS}
            PATH_SUFFIXES lib
            NO_CMAKE_PATH)
    find_library(${VAR_NAME}_LIBRARY_DEBUG
            NAMES ${${VAR_NAME}_LIBRARY_NAMES}
            PATHS ${PHYSX_DEBUG_PATHS}
            PATH_SUFFIXES lib
            NO_CMAKE_PATH)
    select_library_configurations(${VAR_NAME})
endmacro()

find_physx_library(PHYSX_COMMON)
find_physx_library(PHYSX_PHYSICS)
find_physx_library(PHYSX_FOUNDATION)
find_physx_library(PHYSX_COOKING)
find_physx_library(PHYSX_EXTENSIONS)
find_physx_library(PHYSX_CHARACTER)
find_physx_library(PHYSX_PVD)

find_path(PHYSX_INCLUDE_DIR NAMES PxPhysicsAPI.h
        PATHS ${PHYSX_PREFIX}
        PATH_SUFFIXES include/physx include
        NO_CMAKE_PATH)
mark_as_advanced(PHYSX_INCLUDE_DIR)

# PhysX has two include directories, but vcpkg merges them together. In case of a manual build,
# check and compensate for the lack of merging here.
if(PHYSX_INCLUDE_DIR)
    if(NOT EXISTS ${PHYSX_INCLUDE_DIR}/foundation/Px.h)
        find_path(PHYSX_FOUNDATION_INCLUDE_DIR NAMES foundation/Px.h
                PATHS ${PHYSX_PREFIX}
                PATH_SUFFIXES include/physx include
                NO_CMAKE_PATHS
                )
        mark_as_advanced(PHYSX_FOUNDATION_INCLUDE_DIR)
    else()
        set(PHYSX_FOUNDATION_INCLUDE_DIR ${PHYSX_INCLUDE_DIR} CACHE PATH "Path to a file.")
    endif()
endif()

find_package_handle_standard_args(PhysX
        REQUIRED_VARS PHYSX_COMMON_LIBRARY
        PHYSX_PHYSICS_LIBRARY
        PHYSX_FOUNDATION_LIBRARY
        PHYSX_COOKING_LIBRARY
        PHYSX_EXTENSIONS_LIBRARY
        PHYSX_CHARACTER_LIBRARY
        PHYSX_PVD_LIBRARY
        PHYSX_INCLUDE_DIR
        PHYSX_FOUNDATION_INCLUDE_DIR
        REASON_FAILURE_MESSAGE "Be sure that PhysX 4.1 is available.")

set(PHYSX_LIBRARIES
        ${PHYSX_CHARACTER_LIBRARY}
        ${PHYSX_EXTENSIONS_LIBRARY}
        ${PHYSX_PHYSICS_LIBRARY}
        ${PHYSX_PVD_LIBRARY}
        ${PHYSX_COOKING_LIBRARY}
        ${PHYSX_COMMON_LIBRARY}
        ${PHYSX_FOUNDATION_LIBRARY})
set(PHYSX_INCLUDE_DIRS
        ${PHYSX_INCLUDE_DIR}
        ${PHYSX_FOUNDATION_INCLUDE_DIR})

add_library(physx_wrapper STATIC ${PHYSX_WRAPPER_SRC})
target_link_libraries(physx_wrapper PUBLIC ${PHYSX_WRAPPER_LIBS} ${PHYSX_LIBRARIES})
target_include_directories(physx_wrapper PUBLIC "include/" ${PHYSX_INCLUDE_DIRS})
set_target_properties(physx_wrapper PROPERTIES FOLDER Neko/Common)