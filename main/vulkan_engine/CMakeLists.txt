if (Neko_Vulkan)
    include_directories("include/")

    file(GLOB_RECURSE vk_engine_src include/*.h src/*.cpp)

    add_executable(vk_engine ${vk_engine_src})
    target_link_libraries(vk_engine PUBLIC Neko_Core sdl_engine vulkan_wrapper aer_lib)
    neko_bin_config(vk_engine)
    data_generate(vk_engine)
    set_target_properties(vk_engine_Generate_Data PROPERTIES FOLDER Vk/Main)

    add_dependencies(vk_engine_Generate_Data GenerateDataTool)
    add_dependencies(vk_engine vk_engine_Generate_Data)

    set_target_properties(vk_engine PROPERTIES FOLDER Vk/Main)
endif ()