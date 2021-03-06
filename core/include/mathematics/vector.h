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
#include <fmt/format.h>

#include "mathematics/trigo.h"

namespace neko
{
//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
template<typename T>
struct Vec2;

template<typename T>
class Vec3;

template<typename T>
class alignas(4 * sizeof(T)) Vec4;

//-----------------------------------------------------------------------------
// Vec2
//-----------------------------------------------------------------------------
template<typename T>
struct Vec2
{
	union
	{
		struct
		{
			T x, y;
		};
		struct
		{
			T u, v;
		};
		T coord[2] {};
	};

	const static Vec2 zero;
	const static Vec2 one;
	const static Vec2 up;
	const static Vec2 down;
	const static Vec2 left;
	const static Vec2 right;

	//-----------------------------------------------------------------------------
	// Constructors
	//-----------------------------------------------------------------------------
	Vec2() noexcept = default;
	explicit Vec2(T same) noexcept : x(same), y(same) {}
	Vec2(T x, T y) noexcept : x(x), y(y) {}
	explicit Vec2(const T* ptr) noexcept : x(ptr[0]), y(ptr[1]) {}

	template<class U>
	explicit Vec2(U u) noexcept : x(T(u.x)), y(T(u.y))
	{}

	template<class U>
	Vec2(U x, U y) noexcept : x(T(x)), y(T(y))
	{}

	explicit Vec2(std::array<T, 2> v) noexcept
	{
		for (int i = 0; i < 2; i++) coord[i] = v[i];
	}

	const T& operator[](size_t pAxis) const { return coord[pAxis]; }
	T& operator[](size_t pAxis) { return coord[pAxis]; }

	//-----------------------------------------------------------------------------
	// Operators
	//-----------------------------------------------------------------------------
	Vec2<T> operator+(const Vec2<T>& rhs) const { return Vec2<T>(x + rhs.x, y + rhs.y); }

	Vec2<T>& operator+=(const Vec2<T>& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		return *this;
	}

	Vec2<T> operator-(const Vec2<T>& rhs) const { return Vec2<T>(x - rhs.x, y - rhs.y); }

	Vec2<T> operator-() const { return Vec2<T>(-x, -y); }

	Vec2<T>& operator-=(const Vec2<T>& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		return *this;
	}

	Vec2<T> operator*(T rhs) const { return Vec2<T>(x * rhs, y * rhs); }

	Vec2<T> operator*(const Vec2<T>& rhs) const { return Vec2<T>(x * rhs.x, y * rhs.y); }

	friend Vec2<T> operator*(T lhs, const Vec2<T>& rhs)
	{
		return Vec2<T>(rhs.x * lhs, rhs.y * lhs);
	}

	Vec2<T>& operator*=(T rhs)
	{
		this->x *= rhs;
		this->y *= rhs;
		return *this;
	}

	Vec2<T>& operator*=(Vec2<T> rhs)
	{
		this->x *= rhs.x;
		this->y *= rhs.y;
		return *this;
	}

	Vec2<T> operator/(const Vec2<T>& rhs) const
	{
		const T xx = x / rhs.x;
		return Vec2<T>(x / rhs.x, y / rhs.y);
	}

	Vec2<T> operator/(T rhs) const { return (*this) * (1.0f / rhs); }

	Vec2<T>& operator/=(T rhs)
	{
		this->x /= rhs;
		this->y /= rhs;
		return *this;
	}

	bool operator==(const Vec2<T>& other) const { return x == other.x && y == other.y; }

	bool operator!=(const Vec2<T>& other) const { return !(*this == other); }

	friend std::ostream& operator<<(std::ostream& os, const Vec2<T>& dt)
	{
		os << "Vec2(" << dt.x << "," << dt.y << ")";
		return os;
	}

	[[nodiscard]] std::string ToString() const { return fmt::format("Vec2({},{})", x, y); }

	//-----------------------------------------------------------------------------
	// Formulas
	//-----------------------------------------------------------------------------
	/// \brief Calculates the dot product from two vectors.
	static T Dot(const Vec2<T>& v1, const Vec2<T>& v2) { return v1.x * v2.x + v1.y * v2.y; }

	/// \brief Calculates the square magnitude.
	T SquareMagnitude() const { return Dot(*this, *this); }

	/// \brief Calculates the magnitude.
	T Magnitude() const { return std::sqrt(SquareMagnitude()); }

	/// \brief Calculates the normalized vector.
	Vec2<T> Normalized() const { return (*this) / (*this).Magnitude(); }

	/// \brief Interpolate between two vectors.
	/// \param t The interpolation amount.
	static Vec2<T> Lerp(const Vec2<T>& v1, const Vec2<T>& v2, T t) { return v1 + (v2 - v1) * t; }

	/// \brief Reflect the inVec using the normal given (doesn't need to be normalized).
	/// \param inVec The vector to reflect.
	/// \param normal The normal vector of the line to reflect off.
	static Vec2<T> Reflect(const Vec2<T>& inVec, const Vec2<T>& normal)
	{
		return inVec - normal * 2 * Dot(inVec, normal);
	}

	/// \brief Project v1 on v2 (doesn't need to be normalized).
	/// \param v1 The vector to project.
	/// \param v2 The vector to project on.
	static Vec2<T> Project(const Vec2<T>& v1, const Vec2<T>& v2)
	{
		const auto dot = Dot(v1, v2);
		const auto mag = v2.SquareMagnitude();
		return {(dot / mag) * v2.x, (dot / mag) * v2.y};
	}

	/// \brief Calculates the angle between two vectors.
	static neko::radian_t AngleBetween(const Vec2& v1, const Vec2& v2)
	{
		const float mag1 = v1.Magnitude();
		const float mag2 = v2.Magnitude();
		if (mag1 == 0.0f || mag2 == 0.0f) return neko::radian_t(0.0f);

		const float dot      = Vec2<T>::Dot(v1, v2) / mag1 / mag2;
		const float det      = v1.x * v2.y - v1.y * v2.x;
		const radian_t angle = Atan2(det, dot);
		return angle;
	}

	/// \brief Rotates the Vec2 from the given angle (in degrees).
	Vec2<T> Rotate(neko::radian_t angle) const
	{
		return {x * Cos(angle) - y * Sin(angle), x * Sin(angle) + y * Cos(angle)};
	}
};
//-----------------------------------------------------------------------------
// Vec2 Aliases
//-----------------------------------------------------------------------------
using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;
using Vec2u = Vec2<unsigned>;

template<typename T>
inline const Vec2<T> Vec2<T>::zero = Vec2<T>(T(0));
template<typename T>
inline const Vec2<T> Vec2<T>::one = Vec2<T>(T(1));
template<typename T>
inline const Vec2<T> Vec2<T>::right = Vec2<T>(T(1), T(0));
template<typename T>
inline const Vec2<T> Vec2<T>::left = Vec2<T>(T(-1), T(0));
template<typename T>
inline const Vec2<T> Vec2<T>::up = Vec2<T>(T(0), T(1));
template<typename T>
inline const Vec2<T> Vec2<T>::down = Vec2<T>(T(0), T(-1));

//-----------------------------------------------------------------------------
// Vec3
//-----------------------------------------------------------------------------
template<typename T>
class Vec3
{
public:
	union
	{
		struct
		{
			T x;
			T y;
			T z;
		};
		struct
		{
			//For color
			T r;
			T g;
			T b;
		};
		T coord[3] {};
	};

	const static Vec3 zero;
	const static Vec3 one;
	const static Vec3 up;
	const static Vec3 down;
	const static Vec3 left;
	const static Vec3 right;
	const static Vec3 forward;
	const static Vec3 back;

	//-----------------------------------------------------------------------------
	// Constructors
	//-----------------------------------------------------------------------------
	Vec3() noexcept = default;
	explicit Vec3(const T same) noexcept : x(same), y(same), z(same) {}
	Vec3(const T x, const T y, const T z) noexcept : x(x), y(y), z(z) {}
	explicit Vec3(const T* ptr) noexcept : x(ptr[0]), y(ptr[1]), z(ptr[2]) {}

	template<class U>
	explicit Vec3(const Vec2<U> v, const U z = 0) noexcept : x(T(v.x)), y(T(v.y)), z(T(z))
	{}

	template<class U>
	explicit Vec3(const U& u) noexcept : x(T(u.x)), y(T(u.y)), z(T(u.z))
	{}

	template<class U>
	Vec3(const U x, const U y, const U z) noexcept : x(T(x)), y(T(y)), z(T(z))
	{}

	explicit Vec3(const std::array<T, 3>& v) noexcept
	{
		for (int i = 0; i < 3; i++) coord[i] = v[i];
	}

	const T& operator[](const std::size_t pAxis) const { return coord[pAxis]; }
	T& operator[](const std::size_t pAxis) { return coord[pAxis]; }

	//-----------------------------------------------------------------------------
	// Operators
	//-----------------------------------------------------------------------------
	Vec3<T> operator+(const Vec3<T>& rhs) const { return Vec3<T>(x + rhs.x, y + rhs.y, z + rhs.z); }

	Vec3<T>& operator+=(const Vec3<T>& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;
		return *this;
	}

	Vec3<T> operator-(const Vec3<T>& rhs) const { return Vec3<T>(x - rhs.x, y - rhs.y, z - rhs.z); }

	Vec3<T> operator-() const { return Vec3<T>(-x, -y, -z); }

	Vec3<T>& operator-=(const Vec3<T>& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
		return *this;
	}

	Vec3<T> operator*(const T rhs) const { return Vec3<T>(x * rhs, y * rhs, z * rhs); }

	Vec3<T> operator*(const Vec3<T>& rhs) const { return Vec3<T>(x * rhs.x, y * rhs.y, z * rhs.z); }

	friend Vec3<T> operator*(const T lhs, const Vec3<T>& rhs)
	{
		return Vec3<T>(rhs.x * lhs, rhs.y * lhs, rhs.z * lhs);
	}

	Vec3<T>& operator*=(const T rhs)
	{
		this->x *= rhs;
		this->y *= rhs;
		this->z *= rhs;
		return *this;
	}

	Vec3<T> operator/(const T rhs) const { return (*this) * (1.0f / rhs); }

	Vec3<T>& operator/=(const T rhs)
	{
		*this = *this / rhs;
		return *this;
	}

	bool operator==(const Vec3<T>& rh) const { return x == rh.x && y == rh.y && z == rh.z; }

	bool operator!=(const Vec3<T>& rh) const { return !(*this == rh); }

	friend std::ostream& operator<<(std::ostream& os, const Vec3<T>& dt)
	{
		os << "Vec3(" << dt.x << ", " << dt.y << ", " << dt.z << ")";
		return os;
	}

	[[nodiscard]] std::string ToString() const { return fmt::format("Vec3({}, {}, {})", x, y, z); }

	//-----------------------------------------------------------------------------
	// Formulas
	//-----------------------------------------------------------------------------
	/// \brief Calculates the dot product from two vectors.
	static T Dot(const Vec3<T>& v1, const Vec3<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	/// \brief Calculates the cross product from two Vec3.
	static Vec3<T> Cross(const Vec3<T>& v1, const Vec3<T>& v2)
	{
		return Vec3<T>(
			v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
	}

	/// \brief Calculates the square magnitude.
	[[nodiscard]] T SquareMagnitude() const { return Dot(*this, *this); }

	/// \brief Calculates the magnitude.
	template<typename ReturnT = float>
	ReturnT Magnitude() const
	{
		return std::sqrt(SquareMagnitude());
	}

	/// \brief Calculates the normalized vector.
	[[nodiscard]] Vec3<T> Normalized() const
	{
		if ((*this).Magnitude() == 0.0f) return Vec3<T>::zero;
		return (*this) / (*this).Magnitude();
	}

	/// \brief Interpolate between two vectors.
	/// \param t the interpolation amount.
	static Vec3<T> Lerp(const Vec3<T>& v1, const Vec3<T>& v2, T t) { return v1 + (v2 - v1) * t; }

	/// \brief Reflect the inVec using the normal given (doesn't need to be normalized).
	/// \param inVec the vector to reflect.
	/// \param normal the normal vector of the line to reflect off.
	static Vec3<T> Reflect(const Vec3<T>& inVec, const Vec3<T>& normal)
	{
		Vec3<T> normalized = normal.Normalized();
		return inVec - normalized * 2 * Dot(inVec, normalized);
	}

	static Vec3<T> Refract(const Vec3<T>& inVec, const Vec3<T>& normal, const T eta)
	{
		Vec3<T> n = normal.Normalized();
		const T k = 1 - eta * eta * (1.0 - Dot(n, inVec) * Dot(n, inVec));
		return k < 0 ? Vec3<T>::zero : eta * inVec - (eta * Dot(n, inVec) + std::sqrt(k)) * n;
	}

	/// \brief Project v1 on v2 (doesn't need to be normalized).
	/// \param v1 the vector to project.
	/// \param v2 the vector to project on.
	static Vec3<T> Project(const Vec3<T>& v1, const Vec3<T>& v2)
	{
		const auto dot = Dot(v1, v2);
		const auto mag = v2.SquareMagnitude();
		return {(dot / mag) * v2.x, (dot / mag) * v2.y, (dot / mag) * v2.z};
	}

	static neko::radian_t AngleBetween(const Vec3<T>& v1, const Vec3<T>& v2)
	{
		const float mag1 = v1.Magnitude();
		const float mag2 = v2.Magnitude();
		if (mag1 == 0.0f || mag2 == 0.0f) return neko::radian_t(0.0f);

		const float dot      = Vec3<T>::Dot(v1, v2) / mag1 / mag2;
		const float det      = v1.x * v2.y - v1.y * v2.x;
		const radian_t angle = Atan2(det, dot);
		return angle;
	}

	/**
     * \brief Makes vectors normalized and orthogonal to each other.
     * \param normal ref from the normal
     * \param tangent ref from the tangent
     */
	//from https://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process
	static Vec3<T> OrthoNormalize(const Vec3<T>& normal, const Vec3<T>& tangent)
	{
		return tangent - Project(tangent, normal);
	}

	/**
     * \brief Projects a vector onto a plane defined by a normal orthogonal to the plane.
     * \param vector The direction from the vector towards the plane.
     * \param planeNormal The location of the vector above the plane.
     * \return Vec3 The location of the vector on the plane.
     */
	//from https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector3.cs
	static Vec3<T> ProjectOnPlane(const Vec3<T>& vector, const Vec3<T>& planeNormal)
	{
		float sqrMag = Dot(planeNormal, planeNormal);
		if (sqrMag < 0.0f) return vector;
		else
		{
			const auto dot = Dot(vector, planeNormal);
			return {vector.x - planeNormal.x * dot / sqrMag,
				vector.y - planeNormal.y * dot / sqrMag,
				vector.z - planeNormal.z * dot / sqrMag};
		}
	}

	/**
     * \brief Returns the angle between from and to.
     * \param from The vector from which the angular difference is measured.
     * \param to The vector to which the angular difference is measured.
     * \return The angle between the two vectors.
     */
	//https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Vector3.cs
	static radian_t Angle(const Vec3<T>& from, const Vec3<T>& to)
	{
		// sqrt(a) * sqrt(b) = sqrt(a * b) -- valid for real numbers
		float denominator = sqrt(from.SquareMagnitude() * to.SquareMagnitude());
		if (denominator <= 0.0f) return radian_t(0.0f);

		float dot = Dot(from, to) / denominator;
		if (dot < -1.0f) dot = -1.0f;
		if (dot > 1.0f) dot = 1.0f;
		return Acos(dot);
	}
};
//-----------------------------------------------------------------------------
// Vec3 Aliases
//-----------------------------------------------------------------------------
using Vec3f        = Vec3<float>;
using Vec3i        = Vec3<int>;
using Vec3u        = Vec3<unsigned int>;
using EulerAngles  = Vec3<degree_t>;
using RadianAngles = Vec3<radian_t>;

template<typename T>
inline const Vec3<T> Vec3<T>::zero = Vec3<T>(T(0));
template<typename T>
inline const Vec3<T> Vec3<T>::one = Vec3<T>(T(1));
template<typename T>
inline const Vec3<T> Vec3<T>::right = Vec3<T>(T(1), T(0), T(0));
template<typename T>
inline const Vec3<T> Vec3<T>::left = Vec3<T>(T(-1), T(0), T(0));
template<typename T>
inline const Vec3<T> Vec3<T>::up = Vec3<T>(T(0), T(1), T(0));
template<typename T>
inline const Vec3<T> Vec3<T>::down = Vec3<T>(T(0), T(-1), T(0));
template<typename T>
inline const Vec3<T> Vec3<T>::forward = Vec3<T>(T(0), T(0), T(1));
template<typename T>
inline const Vec3<T> Vec3<T>::back = Vec3<T>(T(0), T(0), T(-1));

//-----------------------------------------------------------------------------
// Vec4
//-----------------------------------------------------------------------------
template<typename T>
class alignas(4 * sizeof(T)) Vec4
{
public:
	union
	{
		struct
		{
			T x, y, z, w;
		};
		struct
		{
			T r, g, b, a;
		};
		T coord[4] {};
	};

	const static Vec4 zero;
	const static Vec4 one;

	//-----------------------------------------------------------------------------
	// Constructors
	//-----------------------------------------------------------------------------
	Vec4() noexcept = default;
	explicit Vec4(const T same) noexcept : x(same), y(same), z(same), w(same) {}
	Vec4(const T x, const T y, const T z, const T w) noexcept : x(x), y(y), z(z), w(w) {}
	explicit Vec4(Vec2<T> v, const T z = 0, const T w = 0) noexcept : x(v.x), y(v.y), z(z), w(w) {}
	explicit Vec4(const Vec3<T>& v, const T w = 0) noexcept : x(v.x), y(v.y), z(v.z), w(w) {}
	explicit Vec4(const T* ptr) noexcept : x(ptr[0]), y(ptr[1]), z(ptr[2]), w(ptr[3]) {}

	template<class U>
	explicit Vec4(const U& u) noexcept : x(T(u.x)), y(T(u.y)), z(T(u.z)), w(T(u.w))
	{}

	template<class U>
	Vec4(const U x, const U y, const U z, const U w) noexcept : x(T(x)), y(T(y)), z(T(z)), w(T(w))
	{}

	explicit Vec4(const std::array<T, 4>& v) noexcept
	{
		for (int i = 0; i < 4; i++) coord[i] = v[i];
	}

	const T& operator[](const std::size_t pAxis) const { return coord[pAxis]; }
	T& operator[](const std::size_t pAxis) { return coord[pAxis]; }

	//-----------------------------------------------------------------------------
	// Operators
	//-----------------------------------------------------------------------------
	Vec4<T> operator+(const Vec4<T>& rhs) const
	{
		return Vec4<T>(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	Vec4<T>& operator+=(const Vec4<T>& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;
		this->w += rhs.w;
		return *this;
	}

	Vec4<T> operator-(const Vec4<T>& rhs) const
	{
		return Vec4<T>(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}

	Vec4<T> operator-() const { return Vec4<T>(-x, -y, -z, -w); }

	Vec4<T>& operator-=(const Vec4<T>& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
		this->w -= rhs.w;
		return *this;
	}

	Vec4<T> operator*(const T rhs) const { return Vec4<T>(x * rhs, y * rhs, z * rhs, w * rhs); }

	Vec4<T> operator*(const Vec4<T>& rhs) const
	{
		return Vec4<T>(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
	}

	friend Vec4<T> operator*(const T lhs, const Vec4<T>& rhs)
	{
		return Vec4<T>(rhs.x * lhs, rhs.y * lhs, rhs.z * lhs, rhs.w * lhs);
	}

	Vec4<T>& operator*=(const T rhs)
	{
		this->x *= rhs;
		this->y *= rhs;
		this->z *= rhs;
		this->w *= rhs;
		return *this;
	}

	Vec4<T> operator/(const T rhs) const { return (*this) * (1.0f / rhs); }

	Vec4<T>& operator/=(const T rhs)
	{
		*this = *this / rhs;
		return *this;
	}

	bool operator==(const Vec4<T>& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	bool operator!=(const Vec4<T>& other) const { return !(*this == other); }

	friend std::ostream& operator<<(std::ostream& os, const Vec4<T>& dt)
	{
		os << "Vec4(" << dt.x << "," << dt.y << "," << dt.z << "," << dt.w << ")";
		return os;
	}

	[[nodiscard]] std::string ToString() const
	{
		return fmt::format("Vec4({},{},{},{})", x, y, z, w);
	}

	//-----------------------------------------------------------------------------
	// Formulas
	//-----------------------------------------------------------------------------
	/// \brief Calculates the dot product from two vectors.
	static inline T Dot(const Vec4<T>& v1, const Vec4<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
	}

	/// \brief Calculates the 3D dot product from two vectors.
	static T Dot3(const Vec4<T>& v1, const Vec4<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	/// \brief Calculates the square magnitude.
	T SquareMagnitude() const { return Dot(*this, *this); }

	/// \brief Calculates the magnitude.
	template<typename ReturnT = float>
	ReturnT Magnitude() const
	{
		return std::sqrt(SquareMagnitude());
	}

	/// \brief Calculates the normalized vector.
	Vec4<T> Normalized() const { return (*this) / (*this).Magnitude(); }

	/// \brief Interpolate between two vectors.
	/// \param t the interpolation amount.
	static Vec4<T> Lerp(const Vec4<T>& v1, const Vec4<T>& v2, const T t)
	{
		return v1 + (v2 - v1) * t;
	}

	/// \brief Project v1 on v2 (doesn't need to be normalized).
	/// \param v1 the vector to project.
	/// \param v2 the vector to project on.
	static Vec4<T> Project(const Vec4<T>& v1, const Vec4<T>& v2)
	{
		const auto dot = Dot(v1, v2);
		const auto mag = v2.SquareMagnitude();
		return {
			(dot / mag) * v2.x,
			(dot / mag) * v2.y,
			(dot / mag) * v2.z,
			(dot / mag) * v2.w,
		};
	}
};

//-----------------------------------------------------------------------------
// Vec4 Aliases
//-----------------------------------------------------------------------------
using Vec4f = Vec4<float>;
using Vec4i = Vec4<int>;
using Vec4u = Vec4<unsigned>;

template<typename T>
inline const Vec4<T> Vec4<T>::zero = Vec4<T>(T(0));
template<typename T>
inline const Vec4<T> Vec4<T>::one = Vec4<T>(T(1));
}    // namespace neko
