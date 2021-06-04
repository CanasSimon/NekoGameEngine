if (NOT Emscripten)
    if (Neko_Android)
        list(APPEND sdl_engine_libs SDL2-static Neko_Core SDL2main)
    elseif (MSVC)
        list(APPEND sdl_engine_libs SDL2main)
        set_target_properties(SDL2main PROPERTIES FOLDER Externals)
    else ()
        list(APPEND sdl_engine_libs SDL2-static Neko_Core)
        set_target_properties(SDL2-static PROPERTIES FOLDER Externals)
    endif ()
endif ()

if(Neko_GLES3 AND NOT Neko_VULKAN)
    list(APPEND sdl_engine_libs gles3_wrapper)
endif()
if(Neko_VULKAN)
    list(APPEND sdl_engine_libs vulkan_wrapper)
endif()

file(GLOB_RECURSE sdl_engine_src include/*.h src/*.cpp)

add_library(sdl_engine STATIC ${sdl_engine_src})
target_link_libraries(sdl_engine PUBLIC ${sdl_engine_libs} imgui)

target_include_directories(sdl_engine PUBLIC "include/")
neko_lib_config(sdl_engine)
set_target_properties(sdl_engine PROPERTIES FOLDER Neko/Common)

