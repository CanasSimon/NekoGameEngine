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
#include "mathematics/transform.h"
#include "engine/assert.h"

namespace neko::Transform3d
{
Mat4f Translate(const Mat4f& transform, const Vec3f& translation)
{
	return TranslationMatrixFrom(translation) * transform;
}

Mat4f TranslationMatrixFrom(const Vec3f& translation)
{
	Mat4f mat;
	mat[3] = Vec4f(translation, 1.0f);
	return mat;
}

Mat4f Scale(const Mat4f& transform, const Vec3f& scale)
{
	return ScalingMatrixFrom(scale) * transform;
}

Mat4f ScalingMatrixFrom(const Vec3f& scale)
{
	Mat4f mat;
	mat[0][0] = scale.x;
	mat[1][1] = scale.y;
	mat[2][2] = scale.z;
	return mat;
}

Mat4f Rotate(const Mat4f& transform, const degree_t angle, const Vec3f& axis)
{
	return RotationMatrixFrom(angle, axis) * transform;
}

Mat4f Rotate(const Mat4f& transform, const radian_t angle, const Vec3f& axis)
{
	return RotationMatrixFrom(angle, axis) * transform;
}

Mat4f Rotate(const Mat4f& transform, const Quaternion& quaternion)
{
	return RotationMatrixFrom(quaternion) * transform;
}

Mat4f Rotate(const Mat4f& transform, const EulerAngles& eulerAngles)
{
	const Quaternion quaternion = Quaternion::FromEuler(eulerAngles);
	return RotationMatrixFrom(quaternion) * transform;
}

Mat4f RotationMatrixFrom(const degree_t angle, const Vec3f& axis)
{
	const Vec3f normalizedAxis = axis.Normalized();

	const float x   = normalizedAxis[0];
	const float y   = normalizedAxis[1];
	const float z   = normalizedAxis[2];
	const float c   = Cos(angle);
	const float s   = Sin(angle);
	const float t   = 1.0f - c;
	const float txx = t * x * x;
	const float tyy = t * y * y;
	const float tzz = t * z * z;
	const float txy = t * x * y;
	const float txz = t * x * z;
	const float tyz = t * y * z;
	const float sx  = s * x;
	const float sy  = s * y;
	const float sz  = s * z;

	Mat4f mat;
	mat[0] = Vec4f(txx + c, txy - sz, txz + sy, 0.0f);
	mat[1] = Vec4f(txy + sz, tyy + c, tyz - sx, 0.0f);
	mat[2] = Vec4f(txz - sy, tyz + sx, tzz + c, 0.0f);
	return mat;
}

Mat4f RotationMatrixFrom(const radian_t angle, const Vec3f& axis)
{
	const Vec3f normalizedAxis = axis.Normalized();

	const float x   = normalizedAxis[0];
	const float y   = normalizedAxis[1];
	const float z   = normalizedAxis[2];
	const float c   = Cos(angle);
	const float s   = Sin(angle);
	const float t   = 1.0f - c;
	const float txx = t * x * x;
	const float tyy = t * y * y;
	const float tzz = t * z * z;
	const float txy = t * x * y;
	const float txz = t * x * z;
	const float tyz = t * y * z;
	const float sx  = s * x;
	const float sy  = s * y;
	const float sz  = s * z;

	Mat4f mat;
	mat[0] = Vec4f(txx + c, txy - sz, txz + sy, 0.0f);
	mat[1] = Vec4f(txy + sz, tyy + c, tyz - sx, 0.0f);
	mat[2] = Vec4f(txz - sy, tyz + sx, tzz + c, 0.0f);
	return mat;
}

Mat4f RotationMatrixFrom(const EulerAngles& eulerAngles)
{
	const radian_t rotX = eulerAngles[0];
	const radian_t rotY = eulerAngles[1];
	const radian_t rotZ = eulerAngles[2];
	const float cosX    = Cos(rotX);
	const float sinX    = Sin(rotX);
	const float cosY    = Cos(rotY);
	const float sinY    = Sin(rotY);
	const float cosZ    = Cos(rotZ);
	const float sinZ    = Sin(rotZ);

	Mat3f matX, matY, matZ;
	matX[1] = Vec3f(0.0f, cosX, sinX);
	matX[2] = Vec3f(0.0f, -sinX, cosX);
	matY[0] = Vec3f(cosY, 0.0f, -sinY);
	matY[2] = Vec3f(sinY, 0.0f, cosY);
	matZ[0] = Vec3f(cosZ, sinZ, 0.0f);
	matZ[1] = Vec3f(-sinZ, cosZ, 0.0f);

	return Mat4f(matZ * matY * matX);
}

Mat4f RotationMatrixFrom(const Quaternion& q)
{
	const Mat4f left  = Mat4f(std::array<Vec4f, 4> {Vec4f(q.w, -q.z, q.y, -q.x),
        Vec4f(q.z, q.w, -q.x, -q.y),
        Vec4f(-q.y, q.x, q.w, -q.z),
        Vec4f(q.x, q.y, q.z, q.w)});
	const Mat4f right = Mat4f(std::array<Vec4f, 4> {Vec4f(q.w, -q.z, q.y, q.x),
		Vec4f(q.z, q.w, -q.x, q.y),
		Vec4f(-q.y, q.x, q.w, q.z),
		Vec4f(-q.x, -q.y, -q.z, q.w)});

	return left.Transpose() * right.Transpose();
}

Mat4f LookAt(const Mat4f& transform, const Vec3f& targetPosition)
{
	const Vec3f direction = targetPosition - GetPosition(transform);
	auto quat             = Quaternion::FromEuler(GetRotation(transform));

	// From https://www.gamedev.net/forums/topic/56471-extracting-direction-vectors-from-quaternion/
	Vec3f up = Vec3f(2 * (quat.x * quat.y - quat.w * quat.z),
		1 - 2 * (quat.x * quat.x + quat.z * quat.z),
		2 * (quat.y * quat.z + quat.w * quat.x));
	return Transform3d::Transform(GetPosition(transform),
		Quaternion::ToEulerAngles(Quaternion::LookRotation(direction, up)),
		GetScale(transform));
}

Mat4f Transform(const Vec3f& pos, const Quaternion& rot, const Vec3f& scale)
{
	Mat4f mat = ScalingMatrixFrom(scale);
	mat       = RotationMatrixFrom(rot) * mat;
	mat       = Translate(mat, pos);

	return mat;
}

Mat4f Transform(const Vec3f& pos, const EulerAngles& rot, const Vec3f& scale)
{
	Mat4f mat = ScalingMatrixFrom(scale);
	mat       = RotationMatrixFrom(rot) * mat;
	mat       = Translate(mat, pos);

	return mat;
}

Mat4f Perspective(radian_t fovY, float aspect, float nearPlane, float farPlane)
{
	neko_assert(
		fabsf(aspect - std::numeric_limits<float>::epsilon()) > 0.0f, "Aspect should not be zero");

	const float tanHalfFovY = Tan(fovY / 2.0f);

	Mat4f perspective {Mat4f::Zero};
	perspective[0][0] = 1.0f / (aspect * tanHalfFovY);
	perspective[1][1] = 1.0f / (tanHalfFovY);
	perspective[2][2] = -(farPlane + nearPlane) / (farPlane - nearPlane);
	perspective[2][3] = -1.0f;
	perspective[3][2] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
	return perspective;
}

Mat4f Orthographic(
	float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
	Mat4f result;
	result[0] = Vec4f(2.0f / (right - left), 0.0f, 0.0f, 0.0f);
	result[1] = Vec4f(0.0f, 2.0f / (top - bottom), 0.0f, 0.0f);
	result[2] = Vec4f(0.0f, 0.0f, -2.0f / (farPlane - nearPlane), 0.0f);
	result[3] = Vec4f(-(right + left) / (right - left),
		-(top + bottom) / (top - bottom),
		-(farPlane + nearPlane) / (farPlane - nearPlane),
		1.0f);

	return result;
}

Vec3f GetPosition(const Mat4f& transform)
{
	return Vec3f(transform[3][0], transform[3][1], transform[3][2]);
}

EulerAngles GetRotation(const Mat4f& transform)
{
	// Remove scaling to prepare for extraction of rotation
	Mat4f newTransform = transform;
	Vec3f scale        = GetScale(transform);
	if (scale.x != 0.0f)
	{
		newTransform[0][0] /= scale.x;
		newTransform[0][1] /= scale.x;
		newTransform[0][2] /= scale.x;
	}
	if (scale.y != 0.0f)
	{
		newTransform[1][0] /= scale.y;
		newTransform[1][1] /= scale.y;
		newTransform[1][2] /= scale.y;
	}
	if (scale.z != 0.0f)
	{
		newTransform[2][0] /= scale.z;
		newTransform[2][1] /= scale.z;
		newTransform[2][2] /= scale.z;
	}

	newTransform[3][3] = 1.0f;
	return Quaternion::ToEulerAngles(Quaternion::FromRotationMatrix(newTransform));
}

Vec3f GetScale(const Mat4f& transform)
{
	return Vec3f(
		transform[3][3] * Vec3f(transform[0][0], transform[0][1], transform[0][2]).Magnitude(),
		transform[3][3] * Vec3f(transform[1][0], transform[1][1], transform[1][2]).Magnitude(),
		transform[3][3] * Vec3f(transform[2][0], transform[2][1], transform[2][2]).Magnitude());
}
}    // namespace neko::Transform3d
