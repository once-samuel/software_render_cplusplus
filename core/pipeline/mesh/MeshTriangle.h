#ifndef _MESH_TRIANGLE_H_
#define _MESH_TRIANGLE_H_

#include "CommonMacro.h"
#include "vector2.h"
#include "vector3.h"
#include "matrix4.h"
#include <vector>

namespace render {

	struct MESH_TRIANGLE
	{
		//顶点
		std::vector<vector3> vertex;

		//纹理
		std::vector<vector2> texture;

		//法线
		std::vector<vector3> normal;

		//三角索引
		std::vector<int> triangle;

		//包围球半径
		float radius;
	};

	MESH_TRIANGLE* MeshTriangleLoad(
		const char* file_name,
		const matrix4* init_transform = NULL);

	MESH_TRIANGLE* MeshTriangleCreateCube(
		float width, float height, float depth);
	MESH_TRIANGLE* MeshTriangleCreateSphere(
		float radius, int slices_in_xz_plane, int slices_in_y_axis);
	MESH_TRIANGLE* MeshTriangleCreateCone(
		float radius, float height, int slices_in_xz_plane);
	MESH_TRIANGLE* MeshTriangleCreateCylinder(
		float radius_top, float radius_bottom, float height, int slices_in_xz_plane);
	MESH_TRIANGLE* MeshTriangleCreatePipe(
		float radius_top_in, float radius_top_out, float radius_bottom_in, float radius_bottom_out, float height, int slices_in_xz_plane);
	MESH_TRIANGLE* MeshTriangleCreateTorus(
		float radius_in, float radius_out, int slices_in_xy_plane, int slices_in_xz_plane);

	void MeshTriangleUnload(MESH_TRIANGLE* mesh_triangle);
}

#endif
