#include "aer/frustum.h"

namespace neko
{
Frustum::Frustum(const Camera3D& camera)
{
	Vec3f direction         = -camera.reverseDirection;
	Vec3f position          = camera.position - direction * cameraRecoil;
	Vec3f right             = camera.GetRight();
	Vec3f up                = camera.GetUp();
	float nearPlaneDistance = camera.nearPlane;
	float farPlaneDistance  = camera.farPlane;
	degree_t fovY           = camera.fovY;
	degree_t fovX           = camera.GetFovX();

	float heightNear = Sin(fovY / 2) * nearPlaneDistance * 2;
	float widthNear  = Sin(fovY / 2) * nearPlaneDistance * 2;
	float heightFar  = Sin(fovX / 2) * farPlaneDistance * 2;
	float widthFar   = Sin(fovY / 2) * farPlaneDistance * 2;

	planes_[NEAR_] =
		Plane(position + direction.Normalized() * nearPlaneDistance, direction.Normalized());
	planes_[FAR_] =
		Plane(position + direction.Normalized() * farPlaneDistance, -direction.Normalized());

	//Near plane
	Vec3f ntr = planes_[0].point + up * heightNear + right * widthNear;
	Vec3f nbr = planes_[0].point - up * heightNear + right * widthNear;
	Vec3f nbl = planes_[0].point - up * heightNear - right * widthNear;

	//Far plane
	Vec3f ftr = planes_[1].point + up * heightFar + right * widthFar;
	Vec3f ftl = planes_[1].point + up * heightFar - right * widthFar;
	Vec3f fbl = planes_[1].point - up * heightFar - right * widthFar;

	planes_[RIGHT]  = Plane(ntr, nbr, ftr);
	planes_[LEFT]   = Plane(ftl, fbl, nbl);
	planes_[TOP]    = Plane(ntr, ftr, ftl);
	planes_[BOTTOM] = Plane(nbr, nbl, fbl);

	Vec3f ntl = planes_[NEAR_].point + up * heightNear - right * widthNear;
	Vec3f fbr = planes_[FAR_].point - up * heightFar + right * widthFar;
}

bool Frustum::Contains(const Vec3f& point)
{
	for (int i = 0; i < 6; i++)
		if (planes_[i].Distance(point) < 0.0f) return false;

	return true;
}

bool Frustum::Contains(const Aabb3d& aabb)
{
	std::array<Vec3f, 8> aabbBounds;
	aabbBounds[0] = aabb.lowerLeftBound;
	aabbBounds[1] = Vec3f(aabb.lowerLeftBound.x, aabb.lowerLeftBound.y, aabb.upperRightBound.z);
	aabbBounds[2] = Vec3f(aabb.lowerLeftBound.x, aabb.upperRightBound.y, aabb.lowerLeftBound.z);
	aabbBounds[3] = Vec3f(aabb.lowerLeftBound.x, aabb.upperRightBound.y, aabb.upperRightBound.z);
	aabbBounds[4] = Vec3f(aabb.upperRightBound.x, aabb.lowerLeftBound.y, aabb.lowerLeftBound.z);
	aabbBounds[5] = Vec3f(aabb.upperRightBound.x, aabb.lowerLeftBound.y, aabb.upperRightBound.z);
	aabbBounds[6] = Vec3f(aabb.upperRightBound.x, aabb.upperRightBound.y, aabb.lowerLeftBound.z);
	aabbBounds[7] = aabb.upperRightBound;

	for (int i = 0; i < 6; i++)
	{
		float min = planes_[i].Distance(aabbBounds[0]);
		float max = min;
		for (int j = 1; j < 8; j++)
		{
			if (planes_[i].Distance(aabbBounds[j]) < min) min = planes_[i].Distance(aabbBounds[j]);
			if (planes_[i].Distance(aabbBounds[j]) > max) max = planes_[i].Distance(aabbBounds[j]);
		}

		if (max < 0.0f) return false;
	}

	return true;
}
}    // namespace neko
