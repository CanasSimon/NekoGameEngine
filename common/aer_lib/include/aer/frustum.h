#pragma once
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

 Author : Canas Simon
 Co-Author : Floreau Luca
 Date : 21.07.2020
---------------------------------------------------------- */

#include <graphics/camera.h>
#include "mathematics/aabb.h"
#include "mathematics/vector.h"

namespace neko
{
struct Plane
{
	Plane() = default;
	explicit Plane(const Vec3f& newPoint, const Vec3f& newNormal)
	   : point(newPoint), normal(newNormal)
	{}

	explicit Plane(const Vec3f& pointA, const Vec3f& pointB, const Vec3f& pointC)
	{
		point  = pointA;
		normal = CalculateNormalFrom(pointA, pointB, pointC);
	}

	[[nodiscard]] static Vec3f CalculateNormalFrom(
		const Vec3f& pointA, const Vec3f& pointB, const Vec3f& pointC)
	{
		Vec3f vecA = pointA - pointB;
		Vec3f vecB = pointC - pointB;
		return Vec3f::Cross(vecA, vecB).Normalized();
	}

	[[nodiscard]] float Distance(const Vec3f& targetPoint) const
	{
		return Vec3f::Dot(targetPoint - point, normal);
	}

	Vec3f normal;    //Towards inside
	Vec3f point;
};

class Frustum
{
public:
	Frustum() = default;
	explicit Frustum(const Camera3D& camera);

	bool Contains(const Vec3f& point);
	bool Contains(const Aabb3d& aabb);

	float cameraRecoil = 0.0f;

private:
	enum Planes
	{
		NEAR_  = 0,
		FAR_   = 1,
		RIGHT  = 2,
		LEFT   = 3,
		TOP    = 4,
		BOTTOM = 5
	};

	std::array<Plane, 6> planes_;
};
}    // namespace neko
