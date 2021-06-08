list(APPEND VK_WRAPPER_LIBS Neko_Core sdl_engine)

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan Libraries: ${Vulkan_LIBRARIES} Include: ${Vulkan_INCLUDE_DIRS}")
list(APPEND VK_WRAPPER_LIBS "${Vulkan_LIBRARIES}")

file(GLOB_RECURSE VK_WRAPPER_SRC include/*.h src/*.cpp)
add_library(vulkan_wrapper STATIC ${VK_WRAPPER_SRC})
target_link_libraries(vulkan_wrapper PUBLIC ${VK_WRAPPER_LIBS})

target_include_directories(vulkan_wrapper PUBLIC "include/")
target_include_directories(vulkan_wrapper PUBLIC "${EXTERNAL_DIR}/gli/")
target_include_directories(vulkan_wrapper PUBLIC "${EXTERNAL_DIR}/glm/")

target_include_directories(vulkan_wrapper PUBLIC "${Vulkan_INCLUDE_DIRS}")
neko_lib_config(vulkan_wrapper)
set_target_properties (vulkan_wrapper PROPERTIES FOLDER Neko/Common)