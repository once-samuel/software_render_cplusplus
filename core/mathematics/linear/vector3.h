#ifndef _VECTOR_3_H_
#define _VECTOR_3_H_

#include "CommonMacro.h"
#include <cmath>

namespace render {

	class vector3
	{
	public:
		float x, y, z;

		vector3(float X = 0.0f, float Y = 0.0f, float Z = 0.0f)
			:
			x(X), y(Y), z(Z)
		{}

		bool operator == (const vector3& that)
		{
			return 
				_FLT_EQUAL_FLT(x, that.x) && 
				_FLT_EQUAL_FLT(y, that.y) &&
				_FLT_EQUAL_FLT(z, that.z);
		}

		void Set(float X = 0.0f, float Y = 0.0f, float Z = 0.0f)
		{
			x = X;
			y = Y;
			z = Z;
		}

		bool IsZero()
		{
			return
				_FLT_EQUAL_ZERO(x) &&
				_FLT_EQUAL_ZERO(y) &&
				_FLT_EQUAL_ZERO(z);
		}

		float Length() const
		{
			return sqrt(x * x + y * y + z * z);
		}

		vector3 Normalize() const
		{
			float length = sqrt(x * x + y * y + z * z);
			return _FLT_EQUAL_ZERO(length) ? vector3() : vector3(x / length, y / length, z / length);
		}

		vector3 operator + () const
		{
			return *this;
		}
		vector3 operator + (const vector3& that) const
		{
			return vector3(x + that.x, y + that.y, z + that.z);
		}
		vector3& operator += (const vector3& that)
		{
			x += that.x;
			y += that.y;
			z += that.z;
			return *this;
		}

		vector3 operator - () const
		{
			return vector3(-x, -y, -z);
		}
		vector3 operator - (const vector3& that) const
		{
			return vector3(x - that.x, y - that.y, z - that.z);
		}
		vector3& operator -= (const vector3& that)
		{
			x -= that.x;
			y -= that.y;
			z -= that.z;
			return *this;
		}

		vector3 operator * (float num) const
		{
			return vector3(x * num, y * num, z * num);
		}
		vector3& operator *= (float num)
		{
			x *= num;
			y *= num;
			z *= num;
			return *this;
		}

		vector3 Mul(const vector3& that) const
		{
			return vector3(x * that.x, y * that.y, z * that.z);
		}

		float Dot(const vector3& that) const
		{
			return x * that.x + y * that.y + z * that.z;
		}

		vector3 Cross(const vector3& that) const
		{
			vector3 r;
			r.x = y * that.z - z * that.y;
			r.y = z * that.x - x * that.z;
			r.z = x * that.y - y * that.x;
			return r;
		}

		vector3 Projection(const vector3& that) const
		{
			float that_length_power2 =
				that.x * that.x + that.y * that.y + that.z * that.z;

			if (_FLT_EQUAL_ZERO(that_length_power2))
				return vector3();

			float temp = (x * that.x + y * that.y + z * that.z) / that_length_power2;

			return vector3(that.x * temp, that.y * temp, that.z * temp);
		}
	};

	//标量 * 向量
	inline vector3 operator * (float num, const vector3& vec)
	{
		return vector3(num * vec.x, num * vec.y, num * vec.z);
	}

}

#endif