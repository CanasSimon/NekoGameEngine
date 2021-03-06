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

 Author : Simon Canas
 Co-Author :
 Date : 16.02.2021
---------------------------------------------------------- */
#include "gl/graphics.h"
#include "gl/gl_window.h"

#include "dev/dev_engine.h"

int main(int, char**)
{
	neko::Configuration config;
	config.windowName   = "Feature Development Lab";
	config.windowSize   = neko::Vec2u(1280, 720);

	neko::sdl::GlWindow window;
	neko::gl::GlRenderer renderer;
	neko::Filesystem filesystem;
	neko::dev::DevEngine engine(filesystem, &config);

	engine.SetWindowAndRenderer(&window, &renderer);

    engine.Init();
	engine.EngineLoop();
	return 0;
}
