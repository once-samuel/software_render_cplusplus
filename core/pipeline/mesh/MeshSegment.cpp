#include "MeshSegment.h"
#include "Render.h"
#include <cstdio>

namespace render {

	MESH_SEGMENT* MeshSegmentLoad(
		const char* file_name,
		const matrix4* init_transform)
	{
		//以文本形式打开文件
		FILE* file = fopen(file_name, "r");
		if (NULL == file)
			return NULL;

		//创建模型
		MESH_SEGMENT* mesh_segment = new MESH_SEGMENT;

		//得到顶点数量
		int vertex_count = -1;
		fscanf(file, "(%d)\n", &vertex_count);

		//得到所有顶点
		for (int i = 0; i < vertex_count; ++i)
		{
			vector3 vertex;
			fscanf(file, "(%f,%f,%f)\n", &vertex.x, &vertex.y, &vertex.z);
			mesh_segment->vertex.push_back(vertex);
		}

		//得到线段索引数量
		int segment_count;
		fscanf(file, "(%d)\n", &segment_count);

		//得到所有线段索引
		for (int i = 0; i < segment_count; ++i)
		{
			int segment_index[2];
			fscanf(file, "(%d,%d)\n", &segment_index[0], &segment_index[1]);
			mesh_segment->segment.push_back(segment_index[0]);
			mesh_segment->segment.push_back(segment_index[1]);
		}

		//关闭文件
		fclose(file);

		//初始变换
		if (NULL != init_transform)
		{
			for (int i = 0; i < vertex_count; ++i)
				Vec3MulMat4(&mesh_segment->vertex[i], init_transform, &mesh_segment->vertex[i]);
		}

		//得到包围球半径
		mesh_segment->radius = ComputeLocalShpereRadius(&mesh_segment->vertex);

		return mesh_segment;
	}

	MESH_SEGMENT* MeshSegmentFormMeshTriangle(
		const MESH_TRIANGLE* mesh_triangle)
	{
		MESH_SEGMENT* mesh_segment = new MESH_SEGMENT;

		//顶点
		mesh_segment->vertex = mesh_triangle->vertex;

		//线段
		int triangle_count = (int)mesh_triangle->triangle.size();
		for (int i = 0; i < triangle_count; i += 3)
		{
			int ti0 = mesh_triangle->triangle[i];
			int ti1 = mesh_triangle->triangle[i + 1];
			int ti2 = mesh_triangle->triangle[i + 2];

			int segment[3][2] =
			{
				{ti0, ti1},
				{ti1, ti2},
				{ti2, ti0},
			};

			for (int j = 0; j < 3; ++j)
			{
				bool find = false;
				int segment_count = (int)mesh_segment->segment.size();
				for (int k = 0; k < segment_count; k += 2)
				{
					int si0 = mesh_segment->segment[k];
					int si1 = mesh_segment->segment[k + 1];
					if ((segment[j][0] == si0 && segment[j][1] == si1) ||
						(segment[j][0] == si1 && segment[j][1] == si0))
					{
						find = true;
						break;
					}
				}
				if (!find)
				{
					mesh_segment->segment.push_back(segment[j][0]);
					mesh_segment->segment.push_back(segment[j][1]);
				}
			}
		}

		//包围球半径
		mesh_segment->radius = mesh_triangle->radius;

		return mesh_segment;
	}

	void MeshSegmentUnload(MESH_SEGMENT* mesh_segment)
	{
		if (NULL != mesh_segment)
			delete mesh_segment;
	}

}