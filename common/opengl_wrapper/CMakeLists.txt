list(APPEND OPENGL_WRAPPER_LIBS Neko_Core freetype assimp gli glm)
if(Neko_SDL2)
    list(APPEND OPENGL_WRAPPER_LIBS sdl_engine)
endif()

find_package(OpenGL REQUIRED)
message(STATUS "OpenGL Libraries: ${OPENGL_LIBRARIES}")
add_compile_definitions(IMGUI_IMPL_OPENGL_LOADER_GLEW=1)
list(APPEND OPENGL_WRAPPER_LIBS "${OPENGL_LIBRARIES}" glew)

file(GLOB_RECURSE OPENGL_WRAPPER_SRC include/*.h src/*.cpp)
add_library(opengl_wrapper STATIC ${OPENGL_WRAPPER_SRC})
target_link_libraries(opengl_wrapper PUBLIC ${OPENGL_WRAPPER_LIBS})
target_include_directories(opengl_wrapper PUBLIC ${OPENGL_INCLUDE_DIR})

target_include_directories(opengl_wrapper PUBLIC "include/")
target_include_directories(opengl_wrapper PUBLIC "${EXTERNAL_DIR}/gli/")
target_include_directories(opengl_wrapper PUBLIC "${EXTERNAL_DIR}/glm/")

neko_lib_config(opengl_wrapper)
set_target_properties (opengl_wrapper PROPERTIES FOLDER Neko/Common)