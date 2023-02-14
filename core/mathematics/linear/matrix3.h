#ifndef _MATRIX_3_H_
#define _MATRIX_3_H_

#include "vector2.h"
#include <cmath>

namespace render {

#define _M3_11 0
#define _M3_12 1
#define _M3_13 2
#define _M3_21 3
#define _M3_22 4
#define _M3_23 5
#define _M3_31 6
#define _M3_32 7
#define _M3_33 8

	class matrix3
	{
	public:

		//e[0],e[1],e[2]
		//e[3],e[4],e[5]
		//e[6],e[7],e[8]

		float e[9];

		//构造
		matrix3()
		{
			Indentity();
		}

		//单位化
		matrix3& Indentity()
		{
			e[_M3_11] = 1.0f; e[_M3_12] = 0.0f; e[_M3_13] = 0.0f;
			e[_M3_21] = 0.0f; e[_M3_22] = 1.0f; e[_M3_23] = 0.0f;
			e[_M3_31] = 0.0f; e[_M3_32] = 0.0f; e[_M3_33] = 1.0f;
			return *this;
		}

		//平移
		matrix3& Translate(float x, float y)
		{
			e[_M3_11] = 1.0f; e[_M3_12] = 0.0f; e[_M3_13] = 0.0f;
			e[_M3_21] = 0.0f; e[_M3_22] = 1.0f; e[_M3_23] = 0.0f;
			e[_M3_31] = x;	  e[_M3_32] = y;    e[_M3_33] = 1.0f;
			return *this;
		}
		matrix3& Translate(const vector2& v)
		{
			e[_M3_11] = 1.0f; e[_M3_12] = 0.0f; e[_M3_13] = 0.0f;
			e[_M3_21] = 0.0f; e[_M3_22] = 1.0f; e[_M3_23] = 0.0f;
			e[_M3_31] = v.x;  e[_M3_32] = v.y;  e[_M3_33] = 1.0f;
			return *this;
		}

		//缩放
		matrix3& Scale(float x, float y)
		{
			e[_M3_11] = x;    e[_M3_12] = 0.0f; e[_M3_13] = 0.0f;
			e[_M3_21] = 0.0f; e[_M3_22] = y;    e[_M3_23] = 0.0f;
			e[_M3_31] = 0.0f; e[_M3_32] = 0.0f; e[_M3_33] = 1.0f;
			return *this;
		}
		matrix3& Scale(const vector2& v)
		{
			e[_M3_11] = v.x;  e[_M3_12] = 0.0f; e[_M3_13] = 0.0f;
			e[_M3_21] = 0.0f; e[_M3_22] = v.y;  e[_M3_23] = 0.0f;
			e[_M3_31] = 0.0f; e[_M3_32] = 0.0f; e[_M3_33] = 1.0f;
			return *this;
		}

		//旋转
		matrix3& Rotate(float a)
		{
			e[_M3_11] = cos(a);     e[_M3_12] = sin(a);    e[_M3_13] = 0.0f;
			e[_M3_21] = -e[_M3_12]; e[_M3_22] = e[_M3_11]; e[_M3_23] = 0.0f;
			e[_M3_31] = 0.0f;       e[_M3_32] = 0.0f;      e[_M3_33] = 1.0f;
			return *this;
		}
	};

	inline vector2* Vec2MulMat3(
		const vector2* v,
		const matrix3* m,
		vector2* r)
	{
		float temp[]
			=
		{
			v->x * m->e[_M3_11] + v->y * m->e[_M3_21] + m->e[_M3_31],
			v->x * m->e[_M3_12] + v->y * m->e[_M3_22] + m->e[_M3_32],
			v->x * m->e[_M3_13] + v->y * m->e[_M3_23] + m->e[_M3_33],
		};
		r->x = temp[0] / temp[2];
		r->y = temp[1] / temp[2];
		return r;
	}

	inline matrix3* Mat3MulMat3(
		const matrix3* m1,
		const matrix3* m2,
		matrix3* r)
	{
		r->e[_M3_11] =
			m1->e[_M3_11] * m2->e[_M3_11] +
			m1->e[_M3_12] * m2->e[_M3_21] +
			m1->e[_M3_13] * m2->e[_M3_31];
		r->e[_M3_12] =
			m1->e[_M3_11] * m2->e[_M3_12] +
			m1->e[_M3_12] * m2->e[_M3_22] +
			m1->e[_M3_13] * m2->e[_M3_32];
		r->e[_M3_13] =
			m1->e[_M3_11] * m2->e[_M3_13] +
			m1->e[_M3_12] * m2->e[_M3_23] +
			m1->e[_M3_13] * m2->e[_M3_33];

		r->e[_M3_21] =
			m1->e[_M3_21] * m2->e[_M3_11] +
			m1->e[_M3_22] * m2->e[_M3_21] +
			m1->e[_M3_23] * m2->e[_M3_31];
		r->e[_M3_22] =
			m1->e[_M3_21] * m2->e[_M3_12] +
			m1->e[_M3_22] * m2->e[_M3_22] +
			m1->e[_M3_23] * m2->e[_M3_32];
		r->e[_M3_23] =
			m1->e[_M3_21] * m2->e[_M3_13] +
			m1->e[_M3_22] * m2->e[_M3_23] +
			m1->e[_M3_23] * m2->e[_M3_33];

		r->e[_M3_31] =
			m1->e[_M3_31] * m2->e[_M3_11] +
			m1->e[_M3_32] * m2->e[_M3_21] +
			m1->e[_M3_33] * m2->e[_M3_31];
		r->e[_M3_32] =
			m1->e[_M3_31] * m2->e[_M3_12] +
			m1->e[_M3_32] * m2->e[_M3_22] +
			m1->e[_M3_33] * m2->e[_M3_32];
		r->e[_M3_33] =
			m1->e[_M3_31] * m2->e[_M3_13] +
			m1->e[_M3_32] * m2->e[_M3_23] +
			m1->e[_M3_33] * m2->e[_M3_33];

		return r;
	}

}

#endif