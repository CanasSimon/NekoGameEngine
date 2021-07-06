if (Neko_Vulkan)
    include_directories("include/")

    # Create the executables
    file(GLOB_RECURSE RAYTRACING_ENGINE_DRC include/*.h src/*.cpp)
    add_executable(raytracing_engine ${RAYTRACING_ENGINE_DRC})

    # Link the needed libraries
    target_link_libraries(raytracing_engine PUBLIC Neko_Core sdl_engine vulkan_wrapper)

    neko_bin_config(raytracing_engine) # Enables global settings for executables
    data_generate(raytracing_engine) # Generate data folder
    set_target_properties(raytracing_engine PROPERTIES FOLDER Neko/Main)
endif ()
