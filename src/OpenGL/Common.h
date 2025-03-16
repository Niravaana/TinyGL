#pragma once
#include <cassert>
#include <cmath>
#define TGL_INLINE inline 
#define TGL_ASSERT(n) assert(n)

namespace TinyGl
{
	
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

		DCHECK(!res.HasNaNs());
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

	template<typename T>
	struct Triangle
	{
		T m_v0;
		T m_v1;
		T m_v2;
	};

}