/* ----------------------------------------------------
 MIT License

 Copyright (c) 2020 SAE Institute Switzerland AG

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

 Author : Floreau Luca
 Co-Author :
 Date : 29.09.2020
---------------------------------------------------------- */
#ifdef NEKO_OPENGL
#include "gl/graphics.h"
#include "gl/gl_window.h"
#elif NEKO_VULKAN
#include "vk/graphics.h"
#include "vk/renderers/renderer_editor.h"
#endif

#include "aer/aer_engine.h"

int main(int, char**)
{
	using namespace neko;
	Configuration config;
	config.windowName = "Neko Game Editor";
	config.flags      = Configuration::NONE;
	config.windowSize = Vec2u(1280, 720);

	Filesystem filesystem;
	aer::AerEngine engine(filesystem, &config, aer::ModeEnum::EDITOR);
#ifdef NEKO_OPENGL
	sdl::GlWindow window;
	gl::GlRenderer renderer;
#elif NEKO_VULKAN
	sdl::VulkanWindow window;
	vk::VkRenderer renderer(&window);
	renderer.SetRenderer(std::make_unique<vk::RendererEditor>());
#endif

	engine.SetWindowAndRenderer(&window, &renderer);

	engine.Init();
	engine.EngineLoop();
	return 0;
}
