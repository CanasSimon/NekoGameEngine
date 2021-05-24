# Neko Game Engine 
develop branch: [![Build Status](https://travis-ci.com/EliasFarhan/NekoEngine.svg?branch=develop)](https://travis-ci.com/EliasFarhan/NekoEngine)

NekoGame is a 3D game engine that works on Windows and Linux, forked from the Neko repo that is used at [SAE Institute Geneva](https://sae.swiss).
This is my own personal fork that I use as a testing ground for experimentation, however I do intend for it to be a fully functional game engine in the future.

## Installation
### Requirements
- [CMake](https://cmake.org/download/) to generate the project files.
- [Python3](https://www.python.org/downloads/) for running the different scripts available.
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) (Optional) if you're plaaning on working with Vulkan.

Clang is recommended to compile the engine.

### Steps
- Clone the repository.
- Setup CMake as you usually would with your favorite IDE.
- Build the 'GenerateDataTool' in release to generate the files needed for assets compilation.

## Libraries used
- SDL2 [https://www.libsdl.org/index.php]
- GLAD [https://glad.dav1d.de/]
- Box2D [https://github.com/erincatto/box2d]
- googletest [https://github.com/google/googletest]
- google benchmark [https://github.com/google/benchmark]
- imgui [https://github.com/ocornut/imgui]
- sole uuid [https://github.com/r-lyeh-archived/sole]
- xxhash [https://github.com/Cyan4973/xxHash]
- stb_image [https://github.com/nothings/stb]
- assimp [https://github.com/assimp/assimp]
- Khronos' KTX-Software [https://github.com/KhronosGroup/KTX-Software]
- libzstd (used in KTX-Software, included for android port) [https://github.com/facebook/zstd]
- SFML net [https://www.sfml-dev.org/]
- freetype [https://www.freetype.org/]
- easy_profiler [https://github.com/yse/easy_profiler]
- fmt [https://github.com/fmtlib/fmt]
- nlohmann's json [https://github.com/nlohmann/json]
- units [https://github.com/nholthaus/units]

## Credits
- Elias Farhan
- Fred Dubouchet
- Simon Canas
- Luca Floreau
- Guillaume Jeannin
- Stephen Grosjean
- Sébastien Feser
