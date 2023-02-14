#ifndef _MATRIX_4_H_
#define _MATRIX_4_H_

#include "vector3.h"
#include <cmath>

namespace render {

#define _M4_11 0x0
#define _M4_12 0x1
#define _M4_13 0x2
#define _M4_14 0x3
#define _M4_21 0x4
#define _M4_22 0x5
#define _M4_23 0x6
#define _M4_24 0x7
#define _M4_31 0x8
#define _M4_32 0x9
#define _M4_33 0xa
#define _M4_34 0xb
#define _M4_41 0xc
#define _M4_42 0xd
#define _M4_43 0xe
#define _M4_44 0xf

	class matrix4
	{
	public:

		//11,12,13,14
		//21,22,23,24
		//31,32,33,34
		//41,42,43,44

		float e[0x10];

		//构造
		matrix4()
		{
			Indentity();
		}

		//单位化
		matrix4& Indentity()
		{
			e[_M4_11] = 1.0f; e[_M4_12] = 0.0f; e[_M4_13] = 0.0f; e[_M4_14] = 0.0f;
			e[_M4_21] = 0.0f; e[_M4_22] = 1.0f; e[_M4_23] = 0.0f; e[_M4_24] = 0.0f;
			e[_M4_31] = 0.0f; e[_M4_32] = 0.0f; e[_M4_33] = 1.0f; e[_M4_34] = 0.0f;
			e[_M4_41] = 0.0f; e[_M4_42] = 0.0f; e[_M4_43] = 0.0f; e[_M4_44] = 1.0f;
			return *this;
		}

		//平移
		matrix4& Translate(float x, float y, float z)
		{
			e[_M4_11] = 1.0f; e[_M4_12] = 0.0f; e[_M4_13] = 0.0f; e[_M4_14] = 0.0f;
			e[_M4_21] = 0.0f; e[_M4_22] = 1.0f; e[_M4_23] = 0.0f; e[_M4_24] = 0.0f;
			e[_M4_31] = 0.0f; e[_M4_32] = 0.0f; e[_M4_33] = 1.0f; e[_M4_34] = 0.0f;
			e[_M4_41] = x;    e[_M4_42] = y;    e[_M4_43] = z;    e[_M4_44] = 1.0f;
			return *this;
		}
		matrix4& Translate(const vector3& v)
		{
			e[_M4_11] = 1.0f; e[_M4_12] = 0.0f; e[_M4_13] = 0.0f; e[_M4_14] = 0.0f;
			e[_M4_21] = 0.0f; e[_M4_22] = 1.0f; e[_M4_23] = 0.0f; e[_M4_24] = 0.0f;
			e[_M4_31] = 0.0f; e[_M4_32] = 0.0f; e[_M4_33] = 1.0f; e[_M4_34] = 0.0f;
			e[_M4_41] = v.x;  e[_M4_42] = v.y;  e[_M4_43] = v.z;  e[_M4_44] = 1.0f;
			return *this;
		}

		//缩放
		matrix4& Scale(float x, float y, float z)
		{
			e[_M4_11] = x;    e[_M4_12] = 0.0f; e[_M4_13] = 0.0f; e[_M4_14] = 0.0f;
			e[_M4_21] = 0.0f; e[_M4_22] = y;    e[_M4_23] = 0.0f; e[_M4_24] = 0.0f;
			e[_M4_31] = 0.0f; e[_M4_32] = 0.0f; e[_M4_33] = z;    e[_M4_34] = 0.0f;
			e[_M4_41] = 0.0f; e[_M4_42] = 0.0f; e[_M4_43] = 0.0f; e[_M4_44] = 1.0f;
			return *this;
		}
		matrix4& Scale(const vector3& v)
		{
			e[_M4_11] = v.x;  e[_M4_12] = 0.0f; e[_M4_13] = 0.0f; e[_M4_14] = 0.0f;
			e[_M4_21] = 0.0f; e[_M4_22] = v.y;  e[_M4_23] = 0.0f; e[_M4_24] = 0.0f;
			e[_M4_31] = 0.0f; e[_M4_32] = 0.0f; e[_M4_33] = v.z;  e[_M4_34] = 0.0f;
			e[_M4_41] = 0.0f; e[_M4_42] = 0.0f; e[_M4_43] = 0.0f; e[_M4_44] = 1.0f;
			return *this;
		}

		//绕x轴旋转
		matrix4& RotateX(float a)
		{
			float s = sin(a);
			float c = cos(a);
			e[_M4_11] = 1.0f; e[_M4_12] = 0.0f; e[_M4_13] = 0.0f; e[_M4_14] = 0.0f;
			e[_M4_21] = 0.0f; e[_M4_22] = c;    e[_M4_23] = s;    e[_M4_24] = 0.0f;
			e[_M4_31] = 0.0f; e[_M4_32] = -s;   e[_M4_33] = c;    e[_M4_34] = 0.0f;
			e[_M4_41] = 0.0f; e[_M4_42] = 0.0f; e[_M4_43] = 0.0f; e[_M4_44] = 1.0f;
			return *this;
		}
		//绕y轴旋转
		matrix4& RotateY(float a)
		{
			float s = sin(a);
			float c = cos(a);
			e[_M4_11] = c;    e[_M4_12] = 0.0f; e[_M4_13] = -s;   e[_M4_14] = 0.0f;
			e[_M4_21] = 0.0f; e[_M4_22] = 1.0f; e[_M4_23] = 0.0f; e[_M4_24] = 0.0f;
			e[_M4_31] = s;    e[_M4_32] = 0.0f; e[_M4_33] = c;    e[_M4_34] = 0.0f;
			e[_M4_41] = 0.0f; e[_M4_42] = 0.0f; e[_M4_43] = 0.0f; e[_M4_44] = 1.0f;
			return *this;
		}
		//绕z轴旋转
		matrix4& RotateZ(float a)
		{
			float s = sin(a);
			float c = cos(a);
			e[_M4_11] = c;    e[_M4_12] = s;    e[_M4_13] = 0.0f; e[_M4_14] = 0.0f;
			e[_M4_21] = -s;   e[_M4_22] = c;    e[_M4_23] = 0.0f; e[_M4_24] = 0.0f;
			e[_M4_31] = 0.0f; e[_M4_32] = 0.0f; e[_M4_33] = 1.0f; e[_M4_34] = 0.0f;
			e[_M4_41] = 0.0f; e[_M4_42] = 0.0f; e[_M4_43] = 0.0f; e[_M4_44] = 1.0f;
			return *this;
		}
		//绕指定轴旋转
		matrix4& Rotate(const vector3& n, float a)
		{
			//得到任意轴的单位向量
			vector3 axis = n.Normalize();

			//得到数据
			float xx = axis.x * axis.x;
			float yy = axis.y * axis.y;
			float zz = axis.z * axis.z;
			float xy = axis.x * axis.y;
			float yz = axis.y * axis.z;
			float zx = axis.z * axis.x;
			float c = cos(a);
			float one_sub_c = 1.0f - c;
			float s = sin(a);

			e[_M4_11] = xx * one_sub_c + c;
			e[_M4_12] = xy * one_sub_c + axis.z * s;
			e[_M4_13] = zx * one_sub_c - axis.y * s;
			e[_M4_14] = 0.0f;

			e[_M4_21] = xy * one_sub_c - axis.z * s;
			e[_M4_22] = yy * one_sub_c + c;
			e[_M4_23] = yz * one_sub_c + axis.x * s;
			e[_M4_24] = 0.0f;

			e[_M4_31] = zx * one_sub_c + axis.y * s;
			e[_M4_32] = yz * one_sub_c - axis.x * s;
			e[_M4_33] = zz * one_sub_c + c;
			e[_M4_34] = 0.0f;

			e[_M4_41] = 0.0f;
			e[_M4_42] = 0.0f;
			e[_M4_43] = 0.0f;
			e[_M4_44] = 1.0f;

			return *this;
		}
	};

	inline vector3* Vec3MulMat4(
		const vector3* v,
		const matrix4* m,
		vector3* r)
	{
		float temp[]
			=
		{
			v->x * m->e[_M4_11] + v->y * m->e[_M4_21] + v->z * m->e[_M4_31] + m->e[_M4_41],
			v->x * m->e[_M4_12] + v->y * m->e[_M4_22] + v->z * m->e[_M4_32] + m->e[_M4_42],
			v->x * m->e[_M4_13] + v->y * m->e[_M4_23] + v->z * m->e[_M4_33] + m->e[_M4_43],
			v->x * m->e[_M4_14] + v->y * m->e[_M4_24] + v->z * m->e[_M4_34] + m->e[_M4_44],
		};
		r->x = temp[0] / temp[3];
		r->y = temp[1] / temp[3];
		r->z = temp[2] / temp[3];
		return r;
	}

	inline matrix4* Mat4MulMat4(
		const matrix4* m1,
		const matrix4* m2,
		matrix4* r)
	{
		r->e[_M4_11] =
			m1->e[_M4_11] * m2->e[_M4_11] +
			m1->e[_M4_12] * m2->e[_M4_21] +
			m1->e[_M4_13] * m2->e[_M4_31] +
			m1->e[_M4_14] * m2->e[_M4_41];
		r->e[_M4_12] =
			m1->e[_M4_11] * m2->e[_M4_12] +
			m1->e[_M4_12] * m2->e[_M4_22] +
			m1->e[_M4_13] * m2->e[_M4_32] +
			m1->e[_M4_14] * m2->e[_M4_42];
		r->e[_M4_13] =
			m1->e[_M4_11] * m2->e[_M4_13] +
			m1->e[_M4_12] * m2->e[_M4_23] +
			m1->e[_M4_13] * m2->e[_M4_33] +
			m1->e[_M4_14] * m2->e[_M4_43];
		r->e[_M4_14] =
			m1->e[_M4_11] * m2->e[_M4_14] +
			m1->e[_M4_12] * m2->e[_M4_24] +
			m1->e[_M4_13] * m2->e[_M4_34] +
			m1->e[_M4_14] * m2->e[_M4_44];

		r->e[_M4_21] =
			m1->e[_M4_21] * m2->e[_M4_11] +
			m1->e[_M4_22] * m2->e[_M4_21] +
			m1->e[_M4_23] * m2->e[_M4_31] +
			m1->e[_M4_24] * m2->e[_M4_41];
		r->e[_M4_22] =
			m1->e[_M4_21] * m2->e[_M4_12] +
			m1->e[_M4_22] * m2->e[_M4_22] +
			m1->e[_M4_23] * m2->e[_M4_32] +
			m1->e[_M4_24] * m2->e[_M4_42];
		r->e[_M4_23] =
			m1->e[_M4_21] * m2->e[_M4_13] +
			m1->e[_M4_22] * m2->e[_M4_23] +
			m1->e[_M4_23] * m2->e[_M4_33] +
			m1->e[_M4_24] * m2->e[_M4_43];
		r->e[_M4_24] =
			m1->e[_M4_21] * m2->e[_M4_14] +
			m1->e[_M4_22] * m2->e[_M4_24] +
			m1->e[_M4_23] * m2->e[_M4_34] +
			m1->e[_M4_24] * m2->e[_M4_44];

		r->e[_M4_31] =
			m1->e[_M4_31] * m2->e[_M4_11] +
			m1->e[_M4_32] * m2->e[_M4_21] +
			m1->e[_M4_33] * m2->e[_M4_31] +
			m1->e[_M4_34] * m2->e[_M4_41];
		r->e[_M4_32] =
			m1->e[_M4_31] * m2->e[_M4_12] +
			m1->e[_M4_32] * m2->e[_M4_22] +
			m1->e[_M4_33] * m2->e[_M4_32] +
			m1->e[_M4_34] * m2->e[_M4_42];
		r->e[_M4_33] =
			m1->e[_M4_31] * m2->e[_M4_13] +
			m1->e[_M4_32] * m2->e[_M4_23] +
			m1->e[_M4_33] * m2->e[_M4_33] +
			m1->e[_M4_34] * m2->e[_M4_43];
		r->e[_M4_34] =
			m1->e[_M4_31] * m2->e[_M4_14] +
			m1->e[_M4_32] * m2->e[_M4_24] +
			m1->e[_M4_33] * m2->e[_M4_34] +
			m1->e[_M4_34] * m2->e[_M4_44];

		r->e[_M4_41] =
			m1->e[_M4_41] * m2->e[_M4_11] +
			m1->e[_M4_42] * m2->e[_M4_21] +
			m1->e[_M4_43] * m2->e[_M4_31] +
			m1->e[_M4_44] * m2->e[_M4_41];
		r->e[_M4_42] =
			m1->e[_M4_41] * m2->e[_M4_12] +
			m1->e[_M4_42] * m2->e[_M4_22] +
			m1->e[_M4_43] * m2->e[_M4_32] +
			m1->e[_M4_44] * m2->e[_M4_42];
		r->e[_M4_43] =
			m1->e[_M4_41] * m2->e[_M4_13] +
			m1->e[_M4_42] * m2->e[_M4_23] +
			m1->e[_M4_43] * m2->e[_M4_33] +
			m1->e[_M4_44] * m2->e[_M4_43];
		r->e[_M4_44] =
			m1->e[_M4_41] * m2->e[_M4_14] +
			m1->e[_M4_42] * m2->e[_M4_24] +
			m1->e[_M4_43] * m2->e[_M4_34] +
			m1->e[_M4_44] * m2->e[_M4_44];

		return r;
	}

}

#endif