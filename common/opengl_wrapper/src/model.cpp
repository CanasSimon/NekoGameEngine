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
#include "gl/model.h"

namespace neko::gl
{
void Model::Draw(const Shader& shader) const
{
	for (auto& mesh : meshes_) mesh.Draw(shader);
}

void Model::DrawInstanced(const Shader& shader, const Mat4f& modelMatrices, int count) const
{
	glBindBuffer(GL_ARRAY_BUFFER, instanceVbo_);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Mat4f) * count, &modelMatrices);

	for (auto& mesh : meshes_) mesh.DrawInstanced(shader, count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Model::Destroy()
{
	for (auto& mesh : meshes_) mesh.Destroy();
}
}    // namespace neko::gl
