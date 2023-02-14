#include "MeshTriangle.h"
#include "Render.h"
#include <cstdio>
#include <algorithm>

namespace render {

	static void ComputeNormal(
		const std::vector<vector3>* vertex,
		const std::vector<int>* triangle,
		std::vector<vector3>* normal)
	{
		//得到顶点数量
		int vertex_count = (int)vertex->size();

		//存储所有顶点的法线累积
		std::vector<std::vector<vector3>> normals_reserve;
		normals_reserve.resize(vertex_count);
		for (int i = 0; i < vertex_count; ++i)
			normals_reserve[i].clear();

		//得到三角索引数量
		int triangle_count = (int)triangle->size();

		//循环三角形表
		for (int i = 0; i < triangle_count; i += 3)
		{
			//得到三角对应的顶点下标
			int vertex_index[3] =
			{
				triangle->at(i),
				triangle->at(i + 1),
				triangle->at(i + 2),
			};

			//计算面法线
			vector3 u1 = vertex->at(vertex_index[0]) - vertex->at(vertex_index[1]);
			vector3 u2 = vertex->at(vertex_index[1]) - vertex->at(vertex_index[2]);
			vector3 normal = u1.Cross(u2);
			normal = normal.Normalize();

			for (int j = 0; j < 3; ++j)
			{
				//得到当前顶点下标
				int vi = vertex_index[j];

				//在对应顶点的法线积累里面进行查找
				std::vector<vector3>::iterator it = std::find(
					normals_reserve[vi].begin(),
					normals_reserve[vi].end(),
					normal);

				//找不到就添加该面法线法线到顶点的法线积累
				if (normals_reserve[vi].end() == it)
					normals_reserve[vi].push_back(normal);
			}
		}

		//设置所有顶点法线都为零向量
		normal->resize(vertex->size());
		for (int i = 0; i < vertex_count; ++i)
			normal->at(i).Set(0.0f, 0.0f, 0.0f);

		//得到最终法线
		for (int i = 0; i < vertex_count; ++i)
		{
			int normals_size = (int)normals_reserve[i].size();

			for (int j = 0; j < normals_size; ++j)
				normal->at(i) += normals_reserve[i][j];

			normal->at(i) = normal->at(i).Normalize();
		}

		//法线=顶点+法线，在其后的法线变换运算之中，该法线点也进行世界变换，
		//然后再减去对应世界变换之后的顶点，就是法线在世界坐标系下面的朝向了
		for (int i = 0; i < vertex_count; ++i)
			normal->at(i) += vertex->at(i);
	}

	MESH_TRIANGLE* MeshTriangleLoad(
		const char* file_name,
		const matrix4* init_transform)
	{
		//以文本形式打开文件
		FILE* file = fopen(file_name, "r");
		if (NULL == file)
			return NULL;

		//创建模型
		MESH_TRIANGLE* mesh_triangle = new MESH_TRIANGLE;

		//得到类型
		//0：顶点空间坐标
		//非0：顶点空间坐标和纹理空间坐标
		int type = -1;
		fscanf(file, "(%d)\n", &type);

		//得到顶点数量
		int vertex_count = -1;
		fscanf(file, "(%d)\n", &vertex_count);

		//得到所有顶点坐标
		if (0 == type)
		{
			for (int i = 0; i < vertex_count; ++i)
			{
				vector3 vertex;
				fscanf(file, "(%f,%f,%f)\n", &vertex.x, &vertex.y, &vertex.z);
				mesh_triangle->vertex.push_back(vertex);
			}
		}
		else
		{
			for (int i = 0; i < vertex_count; ++i)
			{
				vector2 texture;
				vector3 vertex;
				fscanf(file, "(%f,%f,%f,%f,%f)\n", &vertex.x, &vertex.y, &vertex.z, &texture.x, &texture.y);
				mesh_triangle->vertex.push_back(vertex);
				mesh_triangle->texture.push_back(texture);
			}
		}

		//得到三角索引数量
		int triangle_count;
		fscanf(file, "(%d)\n", &triangle_count);

		//得到所有线段索引
		for (int i = 0; i < triangle_count; ++i)
		{
			int triangle_index[3];
			fscanf(file, "(%d,%d,%d)\n", &triangle_index[0], &triangle_index[1], &triangle_index[2]);
			mesh_triangle->triangle.push_back(triangle_index[0]);
			mesh_triangle->triangle.push_back(triangle_index[1]);
			mesh_triangle->triangle.push_back(triangle_index[2]);
		}

		//关闭文件
		fclose(file);

		//初始变换
		if (NULL != init_transform)
		{
			for (int i = 0; i < vertex_count; ++i)
				Vec3MulMat4(&mesh_triangle->vertex[i], init_transform, &mesh_triangle->vertex[i]);
		}

		//得到法线
		ComputeNormal(
			&mesh_triangle->vertex,
			&mesh_triangle->triangle,
			&mesh_triangle->normal);

		//得到包围球半径
		mesh_triangle->radius = 
			ComputeLocalShpereRadius(&mesh_triangle->vertex);

		return mesh_triangle;
	}

	MESH_TRIANGLE* MeshTriangleCreateCube(float width, float height, float depth)
	{
		if (_FLT_LESS_EQUAL_FLT(width, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(height, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(depth, 0.0f))
			return NULL;

		MESH_TRIANGLE* mesh_triangle = new MESH_TRIANGLE;

		//顶点
		float wd2 = width / 2.0f;
		float hd2 = height / 2.0f;
		float dd2 = depth / 2.0f;
		float vertex[8][3] =
		{
			{+wd2, +hd2, +dd2},
			{-wd2, +hd2, +dd2},
			{-wd2, +hd2, -dd2},
			{+wd2, +hd2, -dd2},
			{+wd2, -hd2, +dd2},
			{-wd2, -hd2, +dd2},
			{-wd2, -hd2, -dd2},
			{+wd2, -hd2, -dd2},
		};
		for (int i = 0; i < 8; ++i)
			mesh_triangle->vertex.push_back(vector3(vertex[i][0], vertex[i][1], vertex[i][2]));

		//三角
		int triangle[12][3] =
		{
			{0, 3, 2}, {0, 2, 1},
			{4, 6, 7}, {4, 5, 6},
			{3, 7, 6}, {3, 6, 2},
			{0, 4, 7}, {0, 7, 3},
			{0, 5, 4}, {0, 1, 5},
			{1, 6, 5}, {1, 2, 6},
		};
		for (int i = 0; i < 12; ++i)
		{
			for (int j = 0; j < 3; ++j)
				mesh_triangle->triangle.push_back(triangle[i][j]);
		}
	
		//法线
		ComputeNormal(
			&mesh_triangle->vertex,
			&mesh_triangle->triangle,
			&mesh_triangle->normal);

		//包围球半径
		mesh_triangle->radius = 
			ComputeLocalShpereRadius(&mesh_triangle->vertex);

		return mesh_triangle;
	}
	MESH_TRIANGLE* MeshTriangleCreateSphere(float radius, int slices_in_xz_plane, int slices_in_y_axis)
	{
		//参数判断
		if (_FLT_LESS_EQUAL_FLT(radius, 0.0f) ||
			slices_in_xz_plane < 3 ||
			slices_in_y_axis < 2)
			return NULL;

		MESH_TRIANGLE* mesh_triangle = new MESH_TRIANGLE;

		//上下点
		float up_down_vertex[2][3] =
		{
			{0.0f, +radius, 0.0f},
			{0.0f, -radius, 0.0f},
		};
		for (int i = 0; i < 2; ++i)
			mesh_triangle->vertex.push_back(vector3(up_down_vertex[i][0], up_down_vertex[i][1], up_down_vertex[i][2]));

		//单一弧线上的点
		float angle = 0.0f;
		float angle_add = _PI / slices_in_y_axis;
		for (int i = 0; i < slices_in_y_axis - 1; ++i)
		{
			matrix4 mat4;
			mat4.RotateZ(angle += angle_add);
			vector3 vec3;
			Vec3MulMat4(&mesh_triangle->vertex[0], &mat4, &vec3);
			mesh_triangle->vertex.push_back(vec3);
		}

		//其它弧线上的点
		angle = 0.0f;
		angle_add = _PI_M2 / slices_in_xz_plane;
		for (int i = 0; i < slices_in_xz_plane - 1; ++i)
		{
			angle += angle_add;
			for (int j = 0; j < slices_in_y_axis - 1; ++j)
			{
				matrix4 mat4;
				mat4.RotateY(angle);
				vector3 vec3;
				Vec3MulMat4(&mesh_triangle->vertex[2 + j], &mat4, &vec3);
				mesh_triangle->vertex.push_back(vec3);
			}
		}

		//三角
		for (int i = 0; i < slices_in_xz_plane; ++i)
		{
			//左右侧点的起始下标
			int right_begin_i = 2 + i * (slices_in_y_axis - 1);
			int left_begin_i = 2 + ((i + 1) % slices_in_xz_plane) * (slices_in_y_axis - 1);

			//上下三角
			int up_down_triangle[2][3] =
			{
				{0, right_begin_i, left_begin_i},
				{1, left_begin_i + slices_in_y_axis - 2, right_begin_i + slices_in_y_axis - 2},
			};
			for (int j = 0; j < 2; ++j)
			{
				for (int k = 0; k < 3; ++k)
					mesh_triangle->triangle.push_back(up_down_triangle[j][k]);
			}

			//中部三角
			for (int j = 0; j < slices_in_y_axis - 2; ++j)
			{

				int right_top_i = right_begin_i + j;
				int right_bottom_i = right_top_i + 1;
				int left_top_i = left_begin_i + j;
				int left_bottom_i = left_top_i + 1;

				int central_triangle[2][3] =
				{
					{right_top_i, right_bottom_i, left_top_i},
					{left_top_i, right_bottom_i, left_bottom_i},
				};
				for (int k = 0; k < 2; ++k)
				{
					for (int l = 0; l < 3; ++l)
						mesh_triangle->triangle.push_back(central_triangle[k][l]);
				}
			}
		}

		//法线
		ComputeNormal(
			&mesh_triangle->vertex,
			&mesh_triangle->triangle,
			&mesh_triangle->normal);

		//包围球半径
		mesh_triangle->radius = radius;

		return mesh_triangle;
	}
	MESH_TRIANGLE* MeshTriangleCreateCone(float radius, float height, int slices_in_xz_plane)
	{
		//参数判断
		if (_FLT_LESS_EQUAL_FLT(radius, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(height, 0.0f) ||
			slices_in_xz_plane < 3)
			return NULL;

		MESH_TRIANGLE* mesh_triangle = new MESH_TRIANGLE;

		//上下点
		float hd2 = height / 2.0f;
		float up_down_vertex[2][3] =
		{
			{0.0f, +hd2, 0.0f},
			{0.0f, -hd2, 0.0f},
		};
		for (int i = 0; i < 2; ++i)
			mesh_triangle->vertex.push_back(vector3(up_down_vertex[i][0], up_down_vertex[i][1], up_down_vertex[i][2]));

		//顶点
		float angle = 0.0f;
		float angle_add = _PI_M2 / slices_in_xz_plane;
		for (int i = 0; i < slices_in_xz_plane; ++i)
		{
			matrix4 mat4;
			mat4.RotateY(angle);
			vector3 vec3(radius, -hd2, 0.0f);
			Vec3MulMat4(&vec3, &mat4, &vec3);
			mesh_triangle->vertex.push_back(vec3);
			angle += angle_add;
		}

		//三角
		for (int i = 0; i < slices_in_xz_plane; ++i)
		{
			int cur_i = 2 + i;
			int nxt_i = 2 + ((cur_i + 1) - 2) % slices_in_xz_plane;

			int triangle[2][3] =
			{
				{0, cur_i, nxt_i},
				{1, nxt_i, cur_i},
			};

			for (int j = 0; j < 2; ++j)
			{
				for (int k = 0; k < 3; ++k)
					mesh_triangle->triangle.push_back(triangle[j][k]);
			}
		}

		//法线
		ComputeNormal(
			&mesh_triangle->vertex,
			&mesh_triangle->triangle,
			&mesh_triangle->normal);

		//包围球半径
		mesh_triangle->radius =
			ComputeLocalShpereRadius(&mesh_triangle->vertex);

		return mesh_triangle;
	}
	MESH_TRIANGLE* MeshTriangleCreateCylinder(float radius_top, float radius_bottom, float height, int slices_in_xz_plane)
	{
		//参数判断
		if (_FLT_LESS_EQUAL_FLT(radius_top, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(radius_bottom, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(height, 0.0f) ||
			slices_in_xz_plane < 3)
			return NULL;

		MESH_TRIANGLE* mesh_triangle = new MESH_TRIANGLE;

		//上下点
		float hd2 = height / 2.0f;
		float up_down_vertex[2][3] =
		{
			{0.0f, +hd2, 0.0f},
			{0.0f, -hd2, 0.0f},
		};
		for (int i = 0; i < 2; ++i)
			mesh_triangle->vertex.push_back(vector3(up_down_vertex[i][0], up_down_vertex[i][1], up_down_vertex[i][2]));

		//顶点
		float angle = 0.0f;
		float angle_add = _PI_M2 / slices_in_xz_plane;
		for (int i = 0; i < slices_in_xz_plane; ++i)
		{
			matrix4 mat4;
			mat4.RotateY(angle);
			vector3 vec3_top(radius_top, +hd2, 0.0f);
			vector3 vec3_bottom(radius_bottom, -hd2, 0.0f);
			Vec3MulMat4(&vec3_top, &mat4, &vec3_top);
			Vec3MulMat4(&vec3_bottom, &mat4, &vec3_bottom);
			mesh_triangle->vertex.push_back(vec3_top);
			mesh_triangle->vertex.push_back(vec3_bottom);
			angle += angle_add;
		}

		//三角
		for (int i = 0; i < slices_in_xz_plane; ++i)
		{
			int right_top_i = 2 + i * 2;
			int right_bottom_i = right_top_i + 1;
			int left_top_i = 2 + ((right_bottom_i + 1) - 2) % (slices_in_xz_plane * 2);
			int left_bottom_i = left_top_i + 1;

			int triangle[4][3] =
			{
				{0, right_top_i, left_top_i},
				{left_top_i, right_top_i, left_bottom_i},
				{right_bottom_i, left_bottom_i, right_top_i},
				{1, left_bottom_i, right_bottom_i},
			};

			for (int j = 0; j < 4; ++j)
			{
				for (int k = 0; k < 3; ++k)
					mesh_triangle->triangle.push_back(triangle[j][k]);
			}
		}

		//法线
		ComputeNormal(
			&mesh_triangle->vertex,
			&mesh_triangle->triangle,
			&mesh_triangle->normal);

		//包围球半径
		mesh_triangle->radius =
			ComputeLocalShpereRadius(&mesh_triangle->vertex);

		return mesh_triangle;
	}
	MESH_TRIANGLE* MeshTriangleCreatePipe(float radius_top_in, float radius_top_out, float radius_bottom_in, float radius_bottom_out, float height, int slices_in_xz_plane)
	{
		//参数判断
		if (_FLT_LESS_EQUAL_FLT(radius_top_in, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(radius_top_out, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(radius_top_out, radius_top_in) ||
			_FLT_LESS_EQUAL_FLT(radius_bottom_in, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(radius_bottom_out, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(radius_bottom_out, radius_bottom_in) ||
			_FLT_LESS_EQUAL_FLT(height, 0.0f) ||
			slices_in_xz_plane < 3)
			return NULL;

		MESH_TRIANGLE* mesh_triangle = new MESH_TRIANGLE;

		float hd2 = height / 2.0f;

		//顶点
		float angle = 0.0f;
		float angle_add = _PI_M2 / slices_in_xz_plane;
		for (int i = 0; i < slices_in_xz_plane; ++i)
		{
			matrix4 mat4;
			mat4.RotateY(angle);
			vector3 vec3_top_in(radius_top_in, +hd2, 0.0f);
			vector3 vec3_top_out(radius_top_out, +hd2, 0.0f);
			vector3 vec3_bottom_in(radius_bottom_in, -hd2, 0.0f);
			vector3 vec3_bottom_out(radius_bottom_out, -hd2, 0.0f);
			Vec3MulMat4(&vec3_top_in, &mat4, &vec3_top_in);
			Vec3MulMat4(&vec3_top_out, &mat4, &vec3_top_out);
			Vec3MulMat4(&vec3_bottom_in, &mat4, &vec3_bottom_in);
			Vec3MulMat4(&vec3_bottom_out, &mat4, &vec3_bottom_out);
			mesh_triangle->vertex.push_back(vec3_top_in);
			mesh_triangle->vertex.push_back(vec3_top_out);
			mesh_triangle->vertex.push_back(vec3_bottom_in);
			mesh_triangle->vertex.push_back(vec3_bottom_out);
			angle += angle_add;
		}

		//三角
		for (int i = 0; i < slices_in_xz_plane; ++i)
		{
			int right_top_in_i = i * 4;
			int right_top_out_i = right_top_in_i + 1;
			int right_bottom_in_i = right_top_out_i + 1;
			int right_bottom_out_i = right_bottom_in_i + 1;
			int left_top_in_i = (right_bottom_out_i + 1) % (slices_in_xz_plane * 4);
			int left_top_out_i = left_top_in_i + 1;
			int left_bottom_in_i = left_top_out_i + 1;
			int left_bottom_out_i = left_bottom_in_i + 1;

			int triangle[8][3] =
			{
				{right_top_in_i, left_top_in_i, right_bottom_in_i},
				{left_bottom_in_i, right_bottom_in_i, left_top_in_i},

				{left_top_in_i, right_top_in_i, left_top_out_i},
				{right_top_out_i, left_top_out_i, right_top_in_i},

				{left_top_out_i, right_top_out_i, left_bottom_out_i},
				{right_bottom_out_i, left_bottom_out_i, right_top_out_i},

				{left_bottom_out_i, right_bottom_out_i, left_bottom_in_i},
				{right_bottom_in_i, left_bottom_in_i, right_bottom_out_i},
			};

			for (int j = 0; j < 8; ++j)
			{
				for (int k = 0; k < 3; ++k)
					mesh_triangle->triangle.push_back(triangle[j][k]);
			}
		}

		//法线
		ComputeNormal(
			&mesh_triangle->vertex,
			&mesh_triangle->triangle,
			&mesh_triangle->normal);

		//包围球半径
		mesh_triangle->radius =
			ComputeLocalShpereRadius(&mesh_triangle->vertex);

		return mesh_triangle;
	}
	MESH_TRIANGLE* MeshTriangleCreateTorus(float radius_in, float radius_out, int slices_in_xy_plane, int slices_in_xz_plane)
	{
		//参数判断
		if (_FLT_LESS_EQUAL_FLT(radius_in, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(radius_out, 0.0f) ||
			_FLT_LESS_EQUAL_FLT(radius_out, radius_in) ||
			slices_in_xy_plane < 3 ||
			slices_in_xz_plane < 3)
			return NULL;

		MESH_TRIANGLE* mesh_triangle = new MESH_TRIANGLE;

		//剖面圆半径
		float radius_bisect_circle = (radius_out - radius_in) / 2.0f;

		//剖面圆顶点
		float angle = 0.0f;
		float angle_add = _PI_M2 / slices_in_xy_plane;
		for (int i = 0; i < slices_in_xy_plane; ++i)
		{
			matrix4 mat4_r;
			matrix4 mat4_t;
			matrix4 mat4;
			mat4_r.RotateZ(angle);
			mat4_t.Translate(radius_in + radius_bisect_circle, 0.0f, 0.0f);
			Mat4MulMat4(&mat4_r, &mat4_t, &mat4);
			vector3 vec3(radius_bisect_circle, 0.0f, 0.0f);
			Vec3MulMat4(&vec3, &mat4, &vec3);
			mesh_triangle->vertex.push_back(vec3);
			angle += angle_add;
		}

		//剖面圆旋转得到的其它顶点
		angle = 0.0f;
		angle_add = _PI_M2 / slices_in_xz_plane;
		for (int i = 0; i < slices_in_xz_plane - 1; ++i)
		{
			angle += angle_add;
			matrix4 mat4;
			mat4.RotateY(angle);
			for (int j = 0; j < slices_in_xy_plane; ++j)
			{
				vector3 vec3;
				Vec3MulMat4(&mesh_triangle->vertex[j], &mat4, &vec3);
				mesh_triangle->vertex.push_back(vec3);
			}
		}

		//三角
		for (int i = 0; i < slices_in_xz_plane; ++i)
		{
			int right_begin_i = i * slices_in_xy_plane;
			int left_begin_i = ((i + 1) % slices_in_xz_plane) * slices_in_xy_plane;

			for (int j = 0; j < slices_in_xy_plane; ++j)
			{
				int right_bottom_i = right_begin_i + j;
				int right_top_i = right_begin_i + (right_bottom_i + 1) % slices_in_xy_plane;
				int left_bottom_i = left_begin_i + j;
				int left_top_i = left_begin_i + (left_bottom_i + 1) % slices_in_xy_plane;

				int triangle[2][3] =
				{
					{left_top_i, right_top_i, left_bottom_i},
					{right_bottom_i, left_bottom_i, right_top_i},
				};

				for (int k = 0; k < 2; ++k)
				{
					for (int l = 0; l < 3; ++l)
						mesh_triangle->triangle.push_back(triangle[k][l]);
				}
			}
		}

		//法线
		ComputeNormal(
			&mesh_triangle->vertex,
			&mesh_triangle->triangle,
			&mesh_triangle->normal);

		//包围球半径
		mesh_triangle->radius =
			ComputeLocalShpereRadius(&mesh_triangle->vertex);

		return mesh_triangle;
	}

	void MeshTriangleUnload(MESH_TRIANGLE* mesh_triangle)
	{
		if (NULL != mesh_triangle)
			delete mesh_triangle;
	}

}