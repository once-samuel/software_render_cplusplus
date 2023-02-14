#ifndef _MESH_SEGMENT_H_
#define _MESH_SEGMENT_H_

#include "CommonMacro.h"
#include "vector3.h"
#include "matrix4.h"
#include "MeshTriangle.h"
#include <vector>

namespace render {

	struct MESH_SEGMENT
	{
		//顶点
		std::vector<vector3> vertex;

		//线段索引
		std::vector<int> segment;

		//包围球半径
		float radius;
	};

	MESH_SEGMENT* MeshSegmentLoad(
		const char* file_name,
		const matrix4* init_transform = NULL);

	MESH_SEGMENT* MeshSegmentFormMeshTriangle(
		const MESH_TRIANGLE* mesh_triangle);

	void MeshSegmentUnload(MESH_SEGMENT* mesh_segment);
}

#endif
