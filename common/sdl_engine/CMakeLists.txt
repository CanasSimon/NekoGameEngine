if (MSVC)
    list(APPEND sdl_engine_libs SDL2main SDL2-static)
else ()
    list(APPEND sdl_engine_libs SDL2main SDL2-static Neko_Core)
endif ()

if(Neko_OpenGL AND NOT Neko_Vulkan)
    list(APPEND sdl_engine_libs opengl_wrapper)
endif()
if(Neko_Vulkan)
    list(APPEND sdl_engine_libs vulkan_wrapper)
endif()

file(GLOB_RECURSE sdl_engine_src include/*.h src/*.cpp)

add_library(sdl_engine STATIC ${sdl_engine_src})
target_link_libraries(sdl_engine PUBLIC ${sdl_engine_libs} imgui)

target_include_directories(sdl_engine PUBLIC "include/")
neko_lib_config(sdl_engine)
set_target_properties(sdl_engine PROPERTIES FOLDER Neko/Common)

