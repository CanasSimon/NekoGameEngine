include_directories("include/")

# Create the executables
file(GLOB_RECURSE GAME_EDITOR_DRC include/*.h src/*.cpp)
add_executable(game_editor ${GAME_EDITOR_DRC})

# Link the needed libraries
target_link_libraries(game_editor PUBLIC Neko_Core aer_lib sdl_engine)

# Link the correct graphics wrapper
if (Neko_OpenGL AND NOT Neko_Vulkan)
    target_link_libraries(game_editor PUBLIC opengl_wrapper)
elseif (Neko_Vulkan)
    target_link_libraries(game_editor PUBLIC vulkan_wrapper)
endif ()

neko_bin_config(game_editor) # Enables global settings for executables
data_generate(game_editor) # Generate data folder
set_target_properties(game_editor PROPERTIES FOLDER Neko/Main)
