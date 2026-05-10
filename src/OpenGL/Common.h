#pragma once
#include <cassert>
#include <cmath>
#define TGL_INLINE inline 
#define TGL_ASSERT(n) assert(n)

namespace TinyGl
{
	constexpr float Pi = 3.14159265358979323846f;
	using RealType = float; //can be switched to double just changing this
	using u32 = unsigned int;
	using u64 = unsigned long long;
	using u8 = unsigned char;

	static_assert(sizeof(u32) == 4);
	static_assert(sizeof(u64) == 8);
	static_assert(sizeof(u8) == 1);

	//Base Vector Type
	template<typename T, u32 N>
	struct Vector;

	template <typename T>
	inline bool isNaN(const T x) {
		return std::isnan(x);
	}
	template <>
	inline bool isNaN(const int x) {
		return false;
	}

	//Specializations 
	template<typename T>
	struct Vector<T, 2>
	{
		T x, y;

		TGL_INLINE constexpr const T& operator[](const size_t index) const
		{
			TGL_ASSERT(index >= 0 && index < 2);
			return (&x)[index];
		}

		TGL_INLINE Vector<T,2>& operator=(const Vector<T, 2>& b)
		{
			x = b.x;
			y = b.y;

			return *this;
		}

		bool HasNaNs() const { return isNaN(x) || isNaN(y); }
	};

	template<typename T>
	struct Vector<T, 3>
	{
		T x, y, z;

		TGL_INLINE constexpr const T& operator[](const size_t index) const
		{
			TGL_ASSERT(index >= 0 && index < 3);
			return (&x)[index];
		}

		TGL_INLINE Vector<T,3>& operator=(const Vector<T, 3>& b)
		{
			x = b.x;
			y = b.y;
			z = b.z;

			return *this;
		}

		bool HasNaNs() const { return isNaN(x) || isNaN(y) || isNaN(z); }
	};

	template<typename T>
	struct Vector<T, 4>
	{
		T x, y, z, w;

		TGL_INLINE constexpr const T& operator[](const size_t index) const
		{
			TGL_ASSERT(index >= 0 && index < 4);
			return (&x)[index];
		}

		TGL_INLINE Vector<T,4>& operator=(const Vector<T, 4>& b)
		{
			x = b.x;
			y = b.y;
			z = b.z;
			w = b.w;
			return *this;
		}
		bool HasNaNs() const { return isNaN(x) || isNaN(y) || isNaN(z) || isNaN(z); }
	};

	//vector type aliases
	using Vector2 = Vector<RealType, 2>;
	using Vector3 = Vector<RealType, 3>;
	using Vector4 = Vector<RealType, 4>;
	using Point2 = Vector<RealType, 2>;
	using Point3 = Vector<RealType, 3>;

	template<typename T, u32 N>
	TGL_INLINE constexpr Vector<T, N> operator*(const Vector<T, N>& a, const Vector<T, N>& b)
	{
		static_assert(N >= 2 && N <= 4);
		Vector<T, N> res;
		res.x = a.x * b.x;
		res.y = a.y * b.y;

		if constexpr (N >= 3) res.z = a.z * b.z;
		if constexpr (N == 4) res.w = a.w * b.w;

		TGL_ASSERT(!res.HasNaNs());
		return res;
	}

	template<typename T, u32 N>
	TGL_INLINE constexpr Vector<T, N> operator*(float f, const Vector<T, N>& a)
	{
		static_assert(N >= 2 && N <= 4);
		Vector<T, N> res;
		res.x = a.x * f;
		res.y = a.y * f;

		if constexpr (N >= 3) res.z = a.z * f;
		if constexpr (N == 4) res.w = a.w * f;

		TGL_ASSERT(!res.HasNaNs());
		return res;
	}

	template<typename T, u32 N>
	TGL_INLINE constexpr Vector<T, N> operator/(const Vector<T, N>& a, const Vector<T, N>& b)
	{
		static_assert(N >= 2 && N <= 4);
		Vector<T, N> res;
		TGL_ASSERT(b.x != 0 && b.y != 0);
		res.x = a.x / b.x;
		res.y = a.y / b.y;

		if constexpr (N >= 3) {
			TGL_ASSERT(b.z != 0); res.z = a.z / b.z;
		}
		if constexpr (N == 4) {
			TGL_ASSERT(b.w != 0); res.w = a.w / b.w;
		}

		TGL_ASSERT(!res.HasNaNs());
		return res;
	}

	template<typename T, u32 N>
	TGL_INLINE constexpr Vector<T, N> operator/(const Vector<T, N>& a, float v)
	{
		static_assert(N >= 2 && N <= 4);
		Vector<T, N> res;
		TGL_ASSERT(v != 0);
		res.x = a.x / v;
		res.y = a.y / v;

		if constexpr (N >= 3) {
			res.z = a.z / v;
		}
		if constexpr (N == 4) {
			res.w = a.w / v;
		}

		TGL_ASSERT(!res.HasNaNs());
		return res;
	}

	TGL_INLINE constexpr RealType sum(const Vector2& a)
	{
		return a.x + a.y;
	}

	TGL_INLINE constexpr RealType sum(const Vector3& a)
	{
		return a.x + a.y + a.z;
	}

	TGL_INLINE constexpr RealType sum(const Vector4& a)
	{
		return a.x + a.y + a.z + a.w;
	}

	TGL_INLINE Vector3 cross(const Vector3& a, const Vector3& b)
	{
		assert(!a.HasNaNs() && !b.HasNaNs());
		return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
	}

	template<typename T, u32 N>
	TGL_INLINE constexpr T dot(const Vector<T, N>& a, const Vector<T, N>& b)
	{
		return sum(a * b);
	}

	template<typename T, u32 N>
	TGL_INLINE constexpr T sqLength(const Vector<T, N>& a)
	{
		return dot(a , a);
	}

	template<typename T, u32 N>
	TGL_INLINE constexpr T length(const Vector<T, N>& a)
	{
		return std::sqrtf(sqLength(a));
	}

	template<typename T, u32 N>
	TGL_INLINE Vector<T, N> normalize(const Vector<T, N>& a)
	{
		return a / length(a);
	}

	struct Matrix4x4
	{
		Vector4 row[4];
		
		/*TGL_INLINE Matrix4x4& operator=(const Matrix4x4& b)
		{
			row[0] = b.row[0];
			row[1] = b.row[1];
			row[2] = b.row[2];
			row[3] = b.row[3];

			return *this;
		}*/
	};

	template<typename T>
	struct Triangle
	{
		T v0;
		T v1;
		T v2;
	};

	TGL_INLINE Matrix4x4 transpose(const Matrix4x4& m)
	{
		return {
			Vector4{ m.row[0].x, m.row[1].x, m.row[2].x, m.row[3].x },
			Vector4{ m.row[0].y, m.row[1].y, m.row[2].y, m.row[3].y },
			Vector4{ m.row[0].z, m.row[1].z, m.row[2].z, m.row[3].z },
			Vector4{ m.row[0].w, m.row[1].w, m.row[2].w, m.row[3].w },
		};
	}

	TGL_INLINE Matrix4x4 matMultiply(const Matrix4x4& a, const Matrix4x4& b)
	{
		const Matrix4x4 bTranspose = transpose(b);

		return { Vector4{ dot(a.row[0], bTranspose.row[0]), dot(a.row[0], bTranspose.row[1]), dot(a.row[0], bTranspose.row[2]), dot(a.row[0], bTranspose.row[3]) },
				 Vector4{ dot(a.row[1], bTranspose.row[0]), dot(a.row[1], bTranspose.row[1]), dot(a.row[1], bTranspose.row[2]), dot(a.row[1], bTranspose.row[3]) },
				 Vector4{ dot(a.row[2], bTranspose.row[0]), dot(a.row[2], bTranspose.row[1]), dot(a.row[2], bTranspose.row[2]), dot(a.row[2], bTranspose.row[3]) },
				 Vector4{ dot(a.row[3], bTranspose.row[0]), dot(a.row[3], bTranspose.row[1]), dot(a.row[3], bTranspose.row[2]), dot(a.row[3], bTranspose.row[3]) } };
	}

	TGL_INLINE Vector4 mulMatVec(const Matrix4x4& a, const Vector4& b)
	{
		return { dot(a.row[0], b), dot(a.row[1], b), dot(a.row[2], b), dot(a.row[3], b) };
	}

	TGL_INLINE Vector4 mulVecMat(const Vector4& a, const Matrix4x4& b)
	{
		const Matrix4x4 bTranspose = transpose(b);
		return { dot(a, bTranspose.row[0]), dot(a, bTranspose.row[1]), dot(a, bTranspose.row[2]), dot(a, bTranspose.row[3]) };
	}

	TGL_INLINE Matrix4x4 mt4x4Identity()
	{
		Matrix4x4 m;
		m.row[0] = { 1.f, 0.f, 0.f, 0.f };
		m.row[1] = { 0.f, 1.f, 0.f, 0.f };
		m.row[2] = { 0.f, 0.f, 1.f, 0.f };
		m.row[3] = { 0.f, 0.f, 0.f, 1.f };

		return m;
	}

	TGL_INLINE Matrix4x4 mt4x4Scale(const Vector3& s)
	{
		Matrix4x4 m;
		m.row[0] = { s.x, 0.f, 0.f, 0.f };
		m.row[1] = { 0.f, s.y, 0.f, 0.f };
		m.row[2] = { 0.f, 0.f, s.z, 0.f };
		m.row[3] = { 0.f, 0.f, 0.f, 1.f };

		return m;
	}

	TGL_INLINE Matrix4x4 mt4x4Translate(const Vector3& t)
	{
		Matrix4x4 m;
		m.row[0] = { 1.f, 0.f, 0.f, t.x };
		m.row[1] = { 0.f, 1.f, 0.f, t.y };
		m.row[2] = { 0.f, 0.f, 1.f, t.z };
		m.row[3] = { 0.f, 0.f, 0.f, 1.f };

		return m;
	}

	TGL_INLINE Matrix4x4 mt4x4Rotation(Vector4 r)
	{
		float angle = r.x;
		Vector3 axis = { r.y, r.z, r.w };
		float angleRad = angle * (Pi / 180.0f);
		float c = std::cosf(angleRad);
		float s = std::sinf(angleRad);
		axis = normalize(axis);
		float t = 1.0f - c;
		float x = axis.x, y = axis.y, z = axis.z;

		Matrix4x4 rotation;
		rotation.row[0] = { c + t*x*x,     t*x*y - s*z,  t*x*z + s*y,  0.0f };
		rotation.row[1] = { t*x*y + s*z,   c + t*y*y,    t*y*z - s*x,  0.0f };
		rotation.row[2] = { t*x*z - s*y,   t*y*z + s*x,  c + t*z*z,    0.0f };
		rotation.row[3] = { 0.0f,          0.0f,          0.0f,          1.0f };
		return rotation;
	}
}