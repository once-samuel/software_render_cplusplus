#ifndef _VECTOR_2_H_
#define _VECTOR_2_H_

#include "CommonMacro.h"
#include <cmath>

namespace render {

	class vector2
	{
	public:
		float x, y;

		vector2(float X = 0.0f, float Y = 0.0f)
			:
			x(X), y(Y)
		{}

		bool operator == (const vector2& that)
		{
			return
				_FLT_EQUAL_FLT(x, that.x) &&
				_FLT_EQUAL_FLT(y, that.y);
		}

		void Set(float X = 0.0f, float Y = 0.0f)
		{
			x = X;
			y = Y;
		}

		bool IsZero()
		{
			return
				_FLT_EQUAL_ZERO(x) &&
				_FLT_EQUAL_ZERO(y);
		}

		float Length() const
		{
			return sqrt(x * x + y * y);
		}

		vector2 Normalize() const
		{
			float length = sqrt(x * x + y * y);
			return _FLT_EQUAL_ZERO(length) ? vector2() : vector2(x / length, y / length);
		}

		vector2 operator + () const
		{
			return *this;
		}
		vector2 operator + (const vector2& that) const
		{
			return vector2(x + that.x, y + that.y);
		}
		vector2& operator += (const vector2& that)
		{
			x += that.x;
			y += that.y;
			return *this;
		}

		vector2 operator - () const
		{
			return vector2(-x, -y);
		}
		vector2 operator - (const vector2& that) const
		{
			return vector2(x - that.x, y - that.y);
		}
		vector2& operator -= (const vector2& that)
		{
			x -= that.x;
			y -= that.y;
			return *this;
		}

		vector2 operator * (float num) const
		{
			return vector2(x * num, y * num);
		}
		vector2& operator *= (float num)
		{
			x *= num;
			y *= num;
			return *this;
		}

		vector2 Mul(const vector2& that) const
		{
			return vector2(x * that.x, y * that.y);
		}

		float Dot(const vector2& that) const
		{
			return x * that.x + y * that.y;
		}

		vector2 Projection(const vector2& that) const
		{
			float that_length_power2 =
				that.x * that.x + that.y * that.y;

			if (_FLT_EQUAL_ZERO(that_length_power2))
				return vector2();

			float temp = (x * that.x + y * that.y) / that_length_power2;

			return vector2(that.x * temp, that.y * temp);
		}

		//顺时针
		vector2 Clockwise() const
		{
			return vector2(y, -x);
		}

		//逆时针
		vector2 Anticlockwise() const
		{
			return -Clockwise();
		}
	};

	//标量 * 向量
	inline vector2 operator * (float num, const vector2& vec)
	{
		return vector2(num * vec.x, num * vec.y);
	}

}

#endif