#pragma once
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
#include "engine/log.h"
#include "mathematics/const.h"
#include "mathematics/matrix.h"
#include "mathematics/trigo.h"
#include "mathematics/vector.h"

namespace neko
{
struct Quaternion
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};

		float coords[4] {};
	};

	Quaternion() = default;

	explicit Quaternion(float same) : x(same), y(same), z(same), w(same) {}

	Quaternion(float X, float Y, float Z, float W) noexcept : x(X), y(Y), z(Z), w(W) {}

	/**
     * \brief Adding explicit constructor for quaternion-like type
     */
	template<class U>
	explicit Quaternion(U u) noexcept : x(u.x), y(u.y), z(u.z), w(u.w)
	{}

	const float& operator[](size_t p_axis) const { return coords[p_axis]; }
	float& operator[](size_t p_axis) { return coords[p_axis]; }

	//The dot product between two rotations.
	static float Dot(Quaternion a, Quaternion b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	}

	//Converts this quaternion to one with the same orientation but with a magnitude of 1.
	static Quaternion Normalized(Quaternion quaternion)
	{
		return quaternion / Magnitude(quaternion);
	}

	static float Magnitude(Quaternion quaternion)
	{
		return Sqrt(quaternion.x * quaternion.x + quaternion.y * quaternion.y +
					quaternion.z * quaternion.z + quaternion.w * quaternion.w);
	}

	/// Returns a quaternion rotated by 'rad' radians on the select 'axis'
	static Quaternion AngleAxis(radian_t rad, Vec3f axis)
	{
		if (axis.SquareMagnitude() == 0.0f) return Quaternion::Identity();

		Quaternion result = Quaternion::Identity();
		axis              = axis.Normalized();
		axis *= Sin(rad / 2.0f);
		result.x = axis.x;
		result.y = axis.y;
		result.z = axis.z;
		result.w = Cos(rad / 2.0f);

		return Normalized(result);
	}

	/// Calculates the angle in degrees between two quaternions a and b
	static degree_t Angle(Quaternion a, Quaternion b) { return 2.0f * Acos(std::abs(Dot(a, b))); }

	[[nodiscard]] Quaternion Conjugate() const { return Quaternion(-x, -y, -z, w); }

	/// Calculates the inverse of the quaternion
	[[nodiscard]] Quaternion Inverse() const
	{
		const Quaternion conj = Conjugate();
		const float mag       = Magnitude(*this);

		return conj / (mag * mag);
	}

	/// Returns a rotation that rotates z degrees around the z axis,
	/// x degrees around the x axis, and y degrees around the y axis;
	/// applied in that order
	static Quaternion FromEuler(const EulerAngles& angle)
	{
		const auto cy = Cos(angle.z * 0.5f);
		const auto sy = Sin(angle.z * 0.5f);
		const auto cp = Cos(angle.y * 0.5f);
		const auto sp = Sin(angle.y * 0.5f);
		const auto cr = Cos(angle.x * 0.5f);
		const auto sr = Sin(angle.x * 0.5f);

		Quaternion q;
		q.w = cr * cp * cy + sr * sp * sy;
		q.x = sr * cp * cy - cr * sp * sy;
		q.y = cr * sp * cy + sr * cp * sy;
		q.z = cr * cp * sy - sr * sp * cy;
		return q;
	}

	static EulerAngles ToEulerAngles(const Quaternion& q)
	{
		EulerAngles angles;

		// Roll (x-axis rotation)
		float sinRCosP = 2.0f * (q.w * q.x + q.y * q.z);
		float cosRCosP = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
		angles.x       = Atan2(sinRCosP, cosRCosP);

		// Pitch (y-axis rotation)
		float sinP = 2.0f * (q.w * q.y - q.z * q.x);

		// Use 90 degrees if out of range
		if (std::abs(sinP) >= 1) angles.y = radian_t(std::copysign(PI * 0.5f, sinP));
		else
			angles.y = Asin(sinP);

		// Yaw (z-axis rotation)
		float sinYCosP = 2.0f * (q.w * q.z + q.x * q.y);
		float cosYCosP = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
		angles.z       = Atan2(sinYCosP, cosYCosP);

		return angles;
	}

	static Quaternion FromRotationMatrix(const Mat4f& mat)
	{
		Quaternion q;
		float trace =
			mat[0][0] + mat[1][1] + mat[2][2];    // I removed + 1.0f; see discussion with Ethan
		if (trace > 0)
		{    // I changed M_EPSILON to 0
			float s = 0.5f / sqrtf(trace + 1.0f);
			q.w     = 0.25f / s;
			q.x     = (mat[2][1] - mat[1][2]) * s;
			q.y     = (mat[0][2] - mat[2][0]) * s;
			q.z     = (mat[1][0] - mat[0][1]) * s;
		}
		else
		{
			if (mat[0][0] > mat[1][1] && mat[0][0] > mat[2][2])
			{
				float s = 2.0f * sqrtf(1.0f + mat[0][0] - mat[1][1] - mat[2][2]);
				q.w     = (mat[2][1] - mat[1][2]) / s;
				q.x     = 0.25f * s;
				q.y     = (mat[0][1] + mat[1][0]) / s;
				q.z     = (mat[0][2] + mat[2][0]) / s;
			}
			else if (mat[1][1] > mat[2][2])
			{
				float s = 2.0f * sqrtf(1.0f + mat[1][1] - mat[0][0] - mat[2][2]);
				q.w     = (mat[0][2] - mat[2][0]) / s;
				q.x     = (mat[0][1] + mat[1][0]) / s;
				q.y     = 0.25f * s;
				q.z     = (mat[1][2] + mat[2][1]) / s;
			}
			else
			{
				float s = 2.0f * sqrtf(1.0f + mat[2][2] - mat[0][0] - mat[1][1]);
				q.w     = (mat[1][0] - mat[0][1]) / s;
				q.x     = (mat[0][2] + mat[2][0]) / s;
				q.y     = (mat[1][2] + mat[2][1]) / s;
				q.z     = 0.25f * s;
			}
		}
		q.w *= -1.0f;
		return q;
	}

	static Quaternion Identity() { return Quaternion(0, 0, 0, 1); }

	Quaternion operator/(Quaternion rhs) const { return *this * rhs.Inverse(); }

	Quaternion operator/(const float rhs) const
	{
		return Quaternion(x / rhs, y / rhs, z / rhs, w / rhs);
	}

	Quaternion& operator+=(const float rhs)
	{
		x /= rhs;
		y /= rhs;
		z /= rhs;
		w /= rhs;
		return *this;
	}

	Quaternion operator-(const Quaternion& rhs) const
	{
		return Quaternion(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}
	Quaternion& operator-=(const Quaternion& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		w -= rhs.w;
		return *this;
	}

	Quaternion operator+(const Quaternion& rhs) const
	{
		return Quaternion(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	Quaternion& operator+=(const Quaternion& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		w += rhs.w;
		return *this;
	}

	Quaternion operator*(const Quaternion& rhs) const
	{
		return Quaternion(w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
			w * rhs.y + y * rhs.w + z * rhs.x - x * rhs.z,
			w * rhs.z + z * rhs.w + x * rhs.y - y * rhs.x,
			w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
	}

	Vec3f operator*(const Vec3f& rhs) const
	{
		const Vec3f u(x, y, z);
		return 2.0f * Vec3f::Dot(u, rhs) * u + (w * w - Vec3f::Dot(u, u)) * rhs +
		       2.0f * w * Vec3f::Cross(u, rhs);
	}

	Quaternion operator*(const float rhs) const
	{
		return Quaternion(x * rhs, y * rhs, z * rhs, w * rhs);
	}

	Quaternion& operator*=(const Quaternion& rhs)
	{
		x *= rhs.x;
		y *= rhs.y;
		z *= rhs.z;
		w *= rhs.w;
		return *this;
	}

	bool operator==(const Quaternion& right) const
	{
		return x == right.x && y == right.y && z == right.z && w == right.w;
	}

	bool operator!=(const Quaternion& right) const { return !(*this == right); }

	friend std::ostream& operator<<(std::ostream& os, const Quaternion& quat)
	{
		os << "Quaternion(" << quat.x << "," << quat.y << "," << quat.z << "," << quat.w << ")";
		return os;
	}

	/// \brief Interpolates between q1 and q2 by t and normalizes the result afterwards <br>
	/// From http://www.technologicalutopia.com/sourcecode/xnageometry/quaternion.cs.htm
	/// \param t is clamped to the range [0, 1].
	static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t)
	{
		Quaternion quaternion = Quaternion();
		float num             = t;
		float num2            = 1.0f - num;
		float num5            = (((q1.x * q2.x) + (q1.y * q2.y)) + (q1.z * q2.z)) + (q1.w * q2.w);
		if (num5 >= 0.0f)
		{
			quaternion.x = (num2 * q1.x) + (num * q2.x);
			quaternion.y = (num2 * q1.y) + (num * q2.y);
			quaternion.z = (num2 * q1.z) + (num * q2.z);
			quaternion.w = (num2 * q1.w) + (num * q2.w);
		}
		else
		{
			quaternion.x = (num2 * q1.x) - (num * q2.x);
			quaternion.y = (num2 * q1.y) - (num * q2.y);
			quaternion.z = (num2 * q1.z) - (num * q2.z);
			quaternion.w = (num2 * q1.w) - (num * q2.w);
		}

		float num4 = (((quaternion.x * quaternion.x) + (quaternion.y * quaternion.y)) +
						 (quaternion.z * quaternion.z)) +
		             (quaternion.w * quaternion.w);
		float num3 = 1.0f / Sqrt(num4);
		quaternion.x *= num3;
		quaternion.y *= num3;
		quaternion.z *= num3;
		quaternion.w *= num3;
		return quaternion;
	}

	/// \brief Creates a rotation with the specified forward and upwards directions.
	/// \param lookAt The direction to look in.
	/// \param upDirection The vector that defines in which direction up is.
	static Quaternion LookRotation(const Vec3f& lookAt, const Vec3f& upDirection)
	{
		if (lookAt.Magnitude() == 0 || upDirection.Magnitude() == 0)
		{
			logDebug("Look rotation viewing vector is zero");
			return Quaternion(0, 0, 0, 1);
		}
		if (Vec3f::Cross(lookAt, upDirection) == Vec3f::zero)
		{
			logDebug("LookRotation vectors are colinear");
			return Quaternion(0, 0, 0, 1);
		}

		// From https://answers.unity.com/questions/467614/what-is-the-source-code-of-quaternionlookrotation.html
		Vec3f forward = lookAt;
		Vec3f up      = upDirection;
		Vec3f::OrthoNormalize(forward, up);
		forward = forward.Normalized();
		up      = up.Normalized();

		Vec3f right = Vec3f::Cross(up, forward).Normalized();
		Mat3f mat({right, up, forward});

		float num8 = (mat[0][0] + mat[1][1]) + mat[2][2];
		Quaternion quaternion;
		if (num8 > -1.0f)
		{
			float num    = Sqrt(num8 + 1.0f);
			quaternion.w = num * 0.5f;
			num          = 0.5f / num;
			quaternion.x = (mat[1][2] - mat[2][1]) * num;
			quaternion.y = (mat[2][0] - mat[0][2]) * num;
			quaternion.z = (mat[0][1] - mat[1][0]) * num;
			return Normalized(quaternion);
		}

		if ((mat[0][0] >= mat[1][1]) && (mat[0][0] >= mat[2][2]))
		{
			float num7   = Sqrt(((1.0f + mat[0][0]) - mat[1][1]) - mat[2][2]);
			float num4   = 0.5f / num7;
			quaternion.x = 0.5f * num7;
			quaternion.y = (mat[0][1] - mat[1][0]) * num4;
			quaternion.z = (mat[0][2] + mat[2][0]) * num4;
			quaternion.w = (mat[1][2] - mat[2][1]) * num4;
			return Normalized(quaternion);
		}

		if (mat[1][1] > mat[2][2])
		{
			float num6   = Sqrt(((1.0f + mat[1][1]) - mat[0][0]) - mat[2][2]);
			float num3   = 0.5f / num6;
			quaternion.x = (mat[1][0] + mat[0][1]) * num3;
			quaternion.y = 0.5f * num6;
			quaternion.z = (mat[2][1] + mat[1][2]) * num3;
			quaternion.w = (mat[2][0] - mat[0][2]) * num3;
			return Normalized(quaternion);
		}

		float num5   = Sqrt(((1.0f + mat[2][2]) - mat[0][0]) - mat[1][1]);
		float num2   = 0.5f / num5;
		quaternion.x = (mat[2][0] + mat[0][2]) * num2;
		quaternion.y = (mat[2][1] + mat[1][2]) * num2;
		quaternion.z = 0.5f * num5;
		quaternion.w = (mat[0][1] - mat[1][0]) * num2;
		return Normalized(quaternion);
	}
};
}    // namespace neko