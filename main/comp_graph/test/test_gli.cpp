#include <gli/gli.hpp>
#include <gl/gl_include.h>
#include <gtest/gtest.h>

#include "math/vector.h"

GLuint CreateTexture(char const* Filename)
{
	gli::texture texture = gli::load(Filename);
	if (texture.empty())
		return 0;

	gli::gl glProfile(gli::gl::PROFILE_ES30);
	const gli::gl::format format = glProfile.translate(texture.format(), texture.swizzles());
	GLenum target = glProfile.translate(texture.target());
	assert(gli::is_compressed(Texture.format()) && Target == gli::TARGET_2D);

	GLuint textureName = 0;
	glGenTextures(1, &textureName);
	glBindTexture(target, textureName);
	glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(texture.levels() - 1));

    neko::Vec3GLi extent{};
	glTexStorage2D(target, static_cast<GLint>(texture.levels()), format.Internal, extent.x, extent.y);
	for (std::size_t level = 0; level < texture.levels(); ++level)
	{
        neko::Vec3GLi levelExtent(texture.extent(level));
		glCompressedTexSubImage2D(
			target, static_cast<GLint>(level), 0, 0, levelExtent.x, levelExtent.y,
			format.Internal, static_cast<GLsizei>(texture.size(level)), texture.data(0, 0, level));
	}

	return textureName;
}