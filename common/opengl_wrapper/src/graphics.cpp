/*
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
 */
#include "gl/graphics.h"

#include <fmt/format.h>

#include "engine/log.h"

#include "gl/gl_include.h"
#include "graphics/texture.h"

#ifdef NEKO_PROFILE
#include "easy/profiler.h"
#endif

void CheckGlError(const char* file, int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::string log;
        // Process/log the error.
        switch (err)
        {
        case GL_INVALID_ENUM:
            log += "GL Invalid Enum";
            break;
        case GL_INVALID_VALUE:
            log += "GL Invalid Value";
            break;
        case GL_INVALID_OPERATION:
            log += "GL Invalid Operation";
            break;
        case GL_OUT_OF_MEMORY:
            log += "GL Out Of Memory";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            log += "GL Invalid Framebuffer Operation";
            break;
        default:
        	continue;
        }

        neko::LogError(fmt::format("{} in file: {} at line: {}",log, file, line));
    }
}


void CheckFramebuffer(const char* file, int line)
{
    const auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        std::string log;
        switch (status)
        {
            case GL_FRAMEBUFFER_UNDEFINED:
                log+="Framebuffer is undefined!";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                log+="Framebuffer is unsupported!";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                log+="Framebuffer has incomplete attachment!";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                log+="Framebuffer has incomplete missing attachment!";
                break;
            default:
                return;
        }

        neko::LogError(fmt::format("{} in file: {} at line: {}", log, file, line));
    }
}

namespace neko::gl
{
GlRenderer::GlRenderer() : Renderer()
{
}

void GlRenderer::ClearScreen()
{
#ifdef NEKO_PROFILE
    EASY_BLOCK("Clear Screen");
#endif
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
}