#include "Render.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace render {

	matrix4* ComputeTransformCamera(
		matrix4* mat4,
		const vector3* eye,
		const vector3* at,
		const vector3* up)
	{
		//得到摄像机z轴
		vector3 z = *at - *eye;
		z = z.Normalize();

		//得到摄像机x轴
		vector3 x = up->Cross(z);
		x = x.Normalize();

		//得到摄像机y轴
		vector3 y = z.Cross(x);

		mat4->e[_M4_11] = x.x; mat4->e[_M4_12] = y.x; mat4->e[_M4_13] = z.x; mat4->e[_M4_14] = 0.0f;
		mat4->e[_M4_21] = x.y; mat4->e[_M4_22] = y.y; mat4->e[_M4_23] = z.y; mat4->e[_M4_24] = 0.0f;
		mat4->e[_M4_31] = x.z; mat4->e[_M4_32] = y.z; mat4->e[_M4_33] = z.z; mat4->e[_M4_34] = 0.0f;
		mat4->e[_M4_41] = -eye->Dot(x);
		mat4->e[_M4_42] = -eye->Dot(y);
		mat4->e[_M4_43] = -eye->Dot(z);
		mat4->e[_M4_44] = 1.0f;

		return mat4;
	}

	matrix4* ComputeTransformView(
		matrix4* mat4,
		int x,
		int y,
		int w,
		int h)
	{
		float wd2 = w / 2.0f;
		float hd2 = h / 2.0f;

		mat4->e[_M4_11] = wd2;  mat4->e[_M4_12] = 0.0f; mat4->e[_M4_13] = 0.0f; mat4->e[_M4_14] = 0.0f;
		mat4->e[_M4_21] = 0.0f; mat4->e[_M4_22] = -hd2; mat4->e[_M4_23] = 0.0f; mat4->e[_M4_24] = 0.0f;
		mat4->e[_M4_31] = 0.0f; mat4->e[_M4_32] = 0.0f; mat4->e[_M4_33] = 1.0f; mat4->e[_M4_34] = 0.0f;
		mat4->e[_M4_41] = x + wd2;
		mat4->e[_M4_42] = y + hd2;
		mat4->e[_M4_43] = 0.0f;
		mat4->e[_M4_44] = 1.0f;

		return mat4;
	}

	float ComputeLocalShpereRadius(
		const std::vector<vector3>* vertex)
	{
		float max_radius =
			vertex->at(0).x * vertex->at(0).x +
			vertex->at(0).y * vertex->at(0).y +
			vertex->at(0).z * vertex->at(0).z;

		int vertex_count = (int)vertex->size();

		for (int i = 1; i < vertex_count; ++i)
		{
			float radius =
				vertex->at(i).x * vertex->at(i).x +
				vertex->at(i).y * vertex->at(i).y +
				vertex->at(i).z * vertex->at(i).z;

			if (_FLT_LESS_FLT(max_radius, radius))
				max_radius = radius;
		}

		return sqrt(max_radius);
	}

	vector3 Render::ComputerCenterInCamera()
	{
		//将本地坐标系原点（包围球球心）转换到摄像机坐标系
		vector3 center_in_local(0.0f, 0.0f, 0.0f);
		vector3 center_in_world;
		vector3 center_in_camera;
		Vec3MulMat4(&center_in_local, &m_TransformWorld, &center_in_world);
		Vec3MulMat4(&center_in_world, &m_TransformCamera, &center_in_camera);

		return center_in_camera;
	}

	bool Render::CoordinateCameraFrustumTest(float sphere_radius)
	{
		//得到摄像机坐标系下面包围球球心
		vector3 center_in_camera = ComputerCenterInCamera();

		//视锥体裁剪
		if (_FLT_LESS_FLT(m_FarPlaneZInCamera, center_in_camera.z - sphere_radius) ||
			_FLT_LESS_FLT(center_in_camera.z + sphere_radius, m_NearPlaneZInCamera) ||
			_FLT_LESS_FLT(+center_in_camera.z, center_in_camera.x - sphere_radius) ||
			_FLT_LESS_FLT(center_in_camera.x + sphere_radius, -center_in_camera.z) ||
			_FLT_LESS_FLT(+center_in_camera.z, center_in_camera.y - sphere_radius)  ||
			_FLT_LESS_FLT(center_in_camera.y + sphere_radius, -center_in_camera.z))
			return false;
		else
			return true;
	}

	void Render::Draw3DMeshSegmentNearPlaneClip(
		float sphere_radius,
		const std::vector<int>* segment_origin)
	{
		//得到摄像机坐标系下面包围球球心
		vector3 center_in_camera = ComputerCenterInCamera();

		//无需进行近截面裁剪
		if (_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, center_in_camera.z - sphere_radius))
		{
			m_pSegmentAfterNearPlaneClip = segment_origin;
			return;
		}

		//更新原始线段索引到更新线段索引
		m_SegmentByNearPlaneClip.clear();
		int segment_count = (int)segment_origin->size();
		for (int i = 0; i < segment_count; i += 2)
		{
			//得到线段两点
			int i0 = segment_origin->at(i);
			int i1 = segment_origin->at(i + 1);
			vector3* v0 = &m_VertexInCamera[i0];
			vector3* v1 = &m_VertexInCamera[i1];

			//0 1
			//< <：2点小
			//< =：1点等1点小
			//< >：1点小1点大
			//= <：1点等1点小
			//= =：2点等
			//= >：1点等1点大
			//> <：1点小1点大
			//> =：1点等1点大
			//> >：2点大

			//两点的z值同时小于等于近截面，3种情况：2点小、1点等1点小、2点等
			if (_FLT_LESS_EQUAL_FLT(v0->z, m_NearPlaneZInCamera) && _FLT_LESS_EQUAL_FLT(v1->z, m_NearPlaneZInCamera))
			{
				//不放入更新线段索引
			}
			//两点的z值同时大于等于近截面，2种情况：2点大、1点等1点大
			else if (_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, v0->z) && _FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, v1->z))
			{
				//放入更新线段索引
				m_SegmentByNearPlaneClip.push_back(i0);
				m_SegmentByNearPlaneClip.push_back(i1);
			}
			//1种情况：1点小1点大
			else
			{
				//计算截点xy坐标，因为摄像机坐标系下面任意顶点的xyz都是线性关系，故有以下等式
				//m_CoordinateCameraNearPlane - v0->z   x - v0->x   y - v0->y
				//----------------------------------- = --------- = ---------
				//m_CoordinateCameraNearPlane - v1->z   x - v1->x   y - v1->y
				vector3 cut_vertex(0.0f, 0.0f, m_NearPlaneZInCamera);
				float temp = (cut_vertex.z - v0->z) / (cut_vertex.z - v1->z);
				cut_vertex.x = _FLT_EQUAL_FLT(v0->x, v1->x) ? v0->x : (temp * v1->x - v0->x) / (temp - 1.0f);
				cut_vertex.y = _FLT_EQUAL_FLT(v0->y, v1->y) ? v0->y : (temp * v1->y - v0->y) / (temp - 1.0f);

				//截点入摄像机坐标系顶点表
				m_VertexInCamera.push_back(cut_vertex);

				//0点小于等于近截面更新点和保留1点
				if (_FLT_LESS_EQUAL_FLT(v0->z, m_NearPlaneZInCamera))
				{
					//截点下标入更新线段索引表
					m_SegmentByNearPlaneClip.push_back((int)m_VertexInCamera.size() - 1);

					//1点入更新线段索引表
					m_SegmentByNearPlaneClip.push_back(i1);
				}
				//1点小于等于近截面保留0点和更新点
				else
				{
					//0点入更新线段索引表
					m_SegmentByNearPlaneClip.push_back(i0);

					//截点下标入更新线段索引表
					m_SegmentByNearPlaneClip.push_back((int)m_VertexInCamera.size() - 1);
				}
			}
		}

		m_pSegmentAfterNearPlaneClip = &m_SegmentByNearPlaneClip;
	}

	void Render::Draw3DMeshSegmentRasterize_ab0_dt0(
		const vector3* v0,
		const vector3* v1,
		int color)
	{
		SEGMENT seg =
			{ (int)v0->x, (int)v0->y, (int)v1->x, (int)v1->y };

		if (!SegmentClip(&m_RectangleView, &seg))
			return;

		int* current_video_buffer
			= m_pVideoBuffer + seg.y1 * m_BufferWidth + seg.x1;

		int delta_x = seg.x2 - seg.x1;
		int delta_y = seg.y2 - seg.y1;

		int add_x, add_y;
		if (delta_x < 0)
		{
			delta_x = -delta_x;
			add_x = -1;
		}
		else
			add_x = 1;
		if (delta_y < 0)
		{
			delta_y = -delta_y;
			add_y = -m_BufferWidth;
		}
		else
			add_y = m_BufferWidth;

		int delta_2x = delta_x << 1;
		int delta_2y = delta_y << 1;

		if (delta_x > delta_y)
		{
			int p = delta_2y - delta_x;

			for (int i = delta_x; i >= 0; --i)
			{
				*current_video_buffer = color;

				if (p >= 0)
				{
					current_video_buffer += add_y;
					p -= delta_2x;
				}

				current_video_buffer += add_x;
				p += delta_2y;
			}
		}
		else
		{
			int p = delta_2x - delta_y;

			for (int i = delta_y; i >= 0; --i)
			{
				*current_video_buffer = color;

				if (p >= 0)
				{
					current_video_buffer += add_x;
					p -= delta_2y;
				}

				current_video_buffer += add_y;
				p += delta_2x;
			}
		}
	}

	void Render::Draw3DMeshSegmentRasterize_ab0_dt1(
		const vector3* v0,
		const vector3* v1,
		int color)
	{
		SEGMENT seg =
		{ (int)v0->x, (int)v0->y, (int)v1->x, (int)v1->y };

		if ((seg.x1 == seg.x2 && seg.y1 == seg.y2) || !SegmentClip(&m_RectangleView, &seg))
			return;

		int* current_video_buffer
			= m_pVideoBuffer + seg.y1 * m_BufferWidth + seg.x1;
		float* current_depth_buffer
			= m_pDepthBuffer + seg.y1 * m_BufferWidth + seg.x1;

		int delta_x = seg.x2 - seg.x1;
		int delta_y = seg.y2 - seg.y1;

		int add_x, add_y;
		if (delta_x < 0)
		{
			delta_x = -delta_x;
			add_x = -1;
		}
		else
			add_x = 1;
		if (delta_y < 0)
		{
			delta_y = -delta_y;
			add_y = -m_BufferWidth;
		}
		else
			add_y = m_BufferWidth;

		int delta_2x = delta_x << 1;
		int delta_2y = delta_y << 1;

		if (delta_x > delta_y)
		{
			int p = delta_2y - delta_x;

			//得到1/z初始量
			float z_reciprocal = v0->z;

			//得到1/z随x变化的变化量
			float z_reciprocal_rate_by_x =
				(v1->z - v0->z) / delta_x;

			for (int i = delta_x; i >= 0; --i)
			{
				//设置颜色和深度值
				if (_FLT_LESS_FLT(*current_depth_buffer, z_reciprocal))
				{
					*current_video_buffer = color;
					*current_depth_buffer = z_reciprocal;
				}

				if (p >= 0)
				{
					current_video_buffer += add_y;
					current_depth_buffer += add_y;
					p -= delta_2x;
				}

				current_video_buffer += add_x;
				current_depth_buffer += add_x;
				p += delta_2y;

				//1/z变化
				z_reciprocal += z_reciprocal_rate_by_x;
			}
		}
		else
		{
			int p = delta_2x - delta_y;

			//得到1/z初始量
			float z_reciprocal = v0->z;

			//得到1/z随y变化的变化量
			float z_reciprocal_rate_by_y =
				(v1->z - v0->z) / delta_y;

			for (int i = delta_y; i >= 0; --i)
			{
				//设置颜色和深度值
				if (_FLT_LESS_FLT(*current_depth_buffer, z_reciprocal))
				{
					*current_video_buffer = color;
					*current_depth_buffer = z_reciprocal;
				}

				if (p >= 0)
				{
					current_video_buffer += add_x;
					current_depth_buffer += add_x;
					p -= delta_2y;
				}

				current_video_buffer += add_y;
				current_depth_buffer += add_y;
				p += delta_2x;

				//1/z变化
				z_reciprocal += z_reciprocal_rate_by_y;
			}
		}
	}

	void Render::Draw3DMeshSegmentRasterize_ab1_dt0(
		const vector3* v0,
		const vector3* v1,
		int color)
	{
		SEGMENT seg =
			{ (int)v0->x, (int)v0->y, (int)v1->x, (int)v1->y };

		if ((seg.x1 == seg.x2 && seg.y1 == seg.y2) || !SegmentClip(&m_RectangleView, &seg))
			return;

		int* current_video_buffer
			= m_pVideoBuffer + seg.y1 * m_BufferWidth + seg.x1;

		int delta_x = seg.x2 - seg.x1;
		int delta_y = seg.y2 - seg.y1;

		int add_x, add_y;
		if (delta_x < 0)
		{
			delta_x = -delta_x;
			add_x = -1;
		}
		else
			add_x = 1;
		if (delta_y < 0)
		{
			delta_y = -delta_y;
			add_y = -m_BufferWidth;
		}
		else
			add_y = m_BufferWidth;

		int delta_2x = delta_x << 1;
		int delta_2y = delta_y << 1;

		if (delta_x > delta_y)
		{
			int p = delta_2y - delta_x;

			for (int i = delta_x; i >= 0; --i)
			{
				//设置混合颜色
				*current_video_buffer = _COLOR_SET(
					(int)(_COLOR_GET_R(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_R(color) * m_ForegroundAlphaBlendValue),
					(int)(_COLOR_GET_G(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_G(color) * m_ForegroundAlphaBlendValue),
					(int)(_COLOR_GET_B(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_B(color) * m_ForegroundAlphaBlendValue));

				if (p >= 0)
				{
					current_video_buffer += add_y;
					p -= delta_2x;
				}

				current_video_buffer += add_x;
				p += delta_2y;
			}
		}
		else
		{
			int p = delta_2x - delta_y;

			for (int i = delta_y; i >= 0; --i)
			{
				//设置混合颜色
				*current_video_buffer = _COLOR_SET(
					(int)(_COLOR_GET_R(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_R(color) * m_ForegroundAlphaBlendValue),
					(int)(_COLOR_GET_G(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_G(color) * m_ForegroundAlphaBlendValue),
					(int)(_COLOR_GET_B(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_B(color) * m_ForegroundAlphaBlendValue));

				if (p >= 0)
				{
					current_video_buffer += add_x;
					p -= delta_2y;
				}

				current_video_buffer += add_y;
				p += delta_2x;
			}
		}
	}

	void Render::Draw3DMeshSegmentRasterize_ab1_dt1(
		const vector3* v0,
		const vector3* v1,
		int color)
	{
		SEGMENT seg =
		{ (int)v0->x, (int)v0->y, (int)v1->x, (int)v1->y };

		if ((seg.x1 == seg.x2 && seg.y1 == seg.y2) || !SegmentClip(&m_RectangleView, &seg))
			return;

		int* current_video_buffer
			= m_pVideoBuffer + seg.y1 * m_BufferWidth + seg.x1;
		float* current_depth_buffer
			= m_pDepthBuffer + seg.y1 * m_BufferWidth + seg.x1;

		int delta_x = seg.x2 - seg.x1;
		int delta_y = seg.y2 - seg.y1;

		int add_x, add_y;
		if (delta_x < 0)
		{
			delta_x = -delta_x;
			add_x = -1;
		}
		else
			add_x = 1;
		if (delta_y < 0)
		{
			delta_y = -delta_y;
			add_y = -m_BufferWidth;
		}
		else
			add_y = m_BufferWidth;

		int delta_2x = delta_x << 1;
		int delta_2y = delta_y << 1;

		if (delta_x > delta_y)
		{
			int p = delta_2y - delta_x;

			//得到1/z初始量
			float z_reciprocal = v0->z;

			//得到1/z随x变化的变化量
			float z_reciprocal_rate_by_x =
				(v1->z - v0->z) / delta_x;

			for (int i = delta_x; i >= 0; --i)
			{
				//设置混合颜色和深度值
				if (_FLT_LESS_FLT(*current_depth_buffer, z_reciprocal))
				{
					*current_video_buffer = _COLOR_SET(
						(int)(_COLOR_GET_R(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_R(color) * m_ForegroundAlphaBlendValue),
						(int)(_COLOR_GET_G(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_G(color) * m_ForegroundAlphaBlendValue),
						(int)(_COLOR_GET_B(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_B(color) * m_ForegroundAlphaBlendValue));

					*current_depth_buffer = z_reciprocal;
				}

				if (p >= 0)
				{
					current_video_buffer += add_y;
					current_depth_buffer += add_y;
					p -= delta_2x;
				}

				current_video_buffer += add_x;
				current_depth_buffer += add_x;
				p += delta_2y;

				//1/z变化
				z_reciprocal += z_reciprocal_rate_by_x;
			}
		}
		else
		{
			int p = delta_2x - delta_y;

			//得到1/z初始量
			float z_reciprocal = v0->z;

			//得到1/z随y变化的变化量
			float z_reciprocal_rate_by_y =
				(v1->z - v0->z) / delta_y;

			for (int i = delta_y; i >= 0; --i)
			{
				//设置混合颜色和深度值
				if (_FLT_LESS_FLT(*current_depth_buffer, z_reciprocal))
				{
					*current_video_buffer = _COLOR_SET(
						(int)(_COLOR_GET_R(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_R(color) * m_ForegroundAlphaBlendValue),
						(int)(_COLOR_GET_G(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_G(color) * m_ForegroundAlphaBlendValue),
						(int)(_COLOR_GET_B(*current_video_buffer) * m_BackgroundAlphaBlendValue + _COLOR_GET_B(color) * m_ForegroundAlphaBlendValue));

					*current_depth_buffer = z_reciprocal;
				}

				if (p >= 0)
				{
					current_video_buffer += add_x;
					current_depth_buffer += add_x;
					p -= delta_2y;
				}

				current_video_buffer += add_y;
				current_depth_buffer += add_y;
				p += delta_2x;

				//1/z变化
				z_reciprocal += z_reciprocal_rate_by_y;
			}
		}
	}

	bool Render::IsLightWorldEnable()
	{
		int light_world_count = (int)m_LightWorld.size();
		for (int i = 0; i < light_world_count; ++i)
		{
			if (m_LightWorld[i].enable)
				return true;
		}
		return false;
	}

	void Render::IlluminationCompute(
		const std::vector<vector3>* normal,
		const vector3* eye)
	{
		//法线数量，也是顶点数量
		int normal_count = (int)normal->size();

		//颜色叠加环境光和自发光
		m_ColorAfterIlluminationCompute.resize(normal_count);
		for (int i = 0; i < normal_count; ++i)
		{
			m_ColorAfterIlluminationCompute[i] = 
				m_ColorLightAmbient.Mul(m_Material.ambient)  +
				m_Material.emissive;
		}

		//光源表中存在有效光源，叠加定向光和点光源
		if (IsLightWorldEnable())
		{
			//法线转换
			m_NormalInWorld.resize(normal_count);
			for (int i = 0; i < normal_count; ++i)
			{
				Vec3MulMat4(&normal->at(i), &m_TransformWorld, &m_NormalInWorld[i]);
				m_NormalInWorld[i] -= m_VertexInWorld[i];
				m_NormalInWorld[i] = m_NormalInWorld[i].Normalize();
			}

			//光源处理
			int light_world_count = (int)m_LightWorld.size();
			if (NULL != eye)
			{
				for (int i = 0; i < light_world_count; ++i)
				{
					if (!m_LightWorld[i].enable)
						continue;

					switch (m_LightWorld[i].light.type)
					{
						//定向光
					case _LIGHT_DIRECTION:
						{
							//得到光方向的反方向
							vector3 light_directory_negative = -m_LightWorld[i].light.directory;

							//循环每个顶点
							for (int j = 0; j < normal_count; ++j)
							{
								//余弦值
								float c = 0.0f;

								//漫反射效果
								{
									//光方向的反方向和顶点法线夹角余弦值
									c = light_directory_negative.Dot(m_NormalInWorld[j]);

									//夹角为锐角
									if (_FLT_LESS_FLT(0.0f, c))
									{
										//叠加定向光漫反射效果
										m_ColorAfterIlluminationCompute[j] += 
											m_LightWorld[i].light.color.Mul(m_Material.diffuse) * c;
									}
								}

								//镜面反射效果
								{
									//得到反射光
									vector3 reflect =
										m_NormalInWorld[j] * (m_NormalInWorld[j].Dot(light_directory_negative)) * 2.0f + 
										m_LightWorld[i].light.directory;
									reflect = reflect.Normalize();

									//得到视线反方向
									vector3 sight_negative = *eye - m_VertexInWorld[j];
									sight_negative = sight_negative.Normalize();

									//得到反射光和视线反方向夹角余弦值
									c = reflect.Dot(sight_negative);

									//若夹角为锐角
									if (_FLT_LESS_FLT(0.0f, c))
									{
										//叠加定向光镜面反射效果
										m_ColorAfterIlluminationCompute[j] += 
											m_LightWorld[i].light.color.Mul(m_Material.specular) * pow(c, m_Material.power);
									}
								}
							}

							break;
						}
						//点光源
					case _LIGHT_DOT:
						{
							//循环每个顶点
							for (int j = 0; j < normal_count; ++j)
							{
								//得到顶点到点光源的向量，也就是光方向的反向量
								vector3 light_directory_negative = m_LightWorld[i].light.position - m_VertexInWorld[j];

								//得到光方向的反方向单位向量
								vector3 light_directory_negative_normal = 
									light_directory_negative.Normalize();

								//得到顶点到点光源的向量长度
								float light_directory_negative_length = 
									light_directory_negative.Length();

								//当前顶点受到照射
								if (_FLT_LESS_EQUAL_FLT(light_directory_negative_length, m_LightWorld[i].light.radius))
								{
									//余弦值
									float c = 0.0f;

									//漫反射效果
									{
										//光方向的反方向和顶点法线夹角余弦值
										c = light_directory_negative_normal.Dot(m_NormalInWorld[j]);

										//夹角为锐角
										if (_FLT_LESS_FLT(0.0f, c))
										{
											//得到顶点到点光源的向量长度与点光源范围的比值
											float ratio = 1.0f - light_directory_negative_length / m_LightWorld[i].light.radius;

											//叠加定向光漫反射效果
											m_ColorAfterIlluminationCompute[j] += 
												m_LightWorld[i].light.color.Mul(m_Material.diffuse) * c * ratio;
										}
									}

									//镜面反射效果
									{
										//得到反射光
										vector3 reflect =
											m_NormalInWorld[j] * (m_NormalInWorld[j].Dot(light_directory_negative_normal)) * 2.0f + 
											-light_directory_negative;
										reflect = reflect.Normalize();

										//得到视线反方向
										vector3 sight_negative = *eye - m_VertexInWorld[j];
										sight_negative = sight_negative.Normalize();

										//得到反射光和视线反方向夹角余弦值
										c = reflect.Dot(sight_negative);

										//若夹角为锐角
										if (_FLT_LESS_FLT(0.0f, c))
										{
											//得到顶点到点光源的向量长度与点光源范围的比值
											float ratio = 1.0f - light_directory_negative_length / m_LightWorld[i].light.radius;

											//叠加定向光镜面反射效果
											m_ColorAfterIlluminationCompute[j] += 
												m_LightWorld[i].light.color.Mul(m_Material.specular) * pow(c, m_Material.power) * ratio;
										}
									}
								}
							}

							break;
						}
					}
				}
			}
			else
			{
				for (int i = 0; i < light_world_count; ++i)
				{
					if (!m_LightWorld[i].enable)
						continue;

					switch (m_LightWorld[i].light.type)
					{
						//定向光
					case _LIGHT_DIRECTION:
						{
							//得到光方向的反方向
							vector3 light_directory_negative = -m_LightWorld[i].light.directory;

							for (int j = 0; j < normal_count; ++j)
							{
								//余弦值
								float c = 0.0f;

								//漫反射效果
								{
									//光方向的反方向和顶点法线夹角余弦值
									c = light_directory_negative.Dot(m_NormalInWorld[j]);

									//夹角为锐角
									if (_FLT_LESS_FLT(0.0f, c))
									{
										//叠加定向光漫反射效果
										m_ColorAfterIlluminationCompute[j] += 
											m_LightWorld[i].light.color.Mul(m_Material.diffuse) * c;
									}
								}
							}

							break;
						}
						//点光源
					case _LIGHT_DOT:
						{
							for (int j = 0; j < normal_count; ++j)
							{
								//得到顶点到点光源的向量，也就是光方向的反向量
								vector3 light_directory_negative = m_LightWorld[i].light.position - m_VertexInWorld[j];

								//得到光方向的反方向单位向量
								vector3 light_directory_negative_normal = 
									light_directory_negative.Normalize();

								//得到顶点到点光源的向量长度
								float light_directory_negative_length = 
									light_directory_negative.Length();

								//当前顶点受到照射
								if (_FLT_LESS_EQUAL_FLT(light_directory_negative_length, m_LightWorld[i].light.radius))
								{
									//余弦值
									float c = 0.0f;

									//漫反射效果
									{
										//光方向的反方向和顶点法线夹角余弦值
										c = light_directory_negative_normal.Dot(m_NormalInWorld[j]);

										//夹角为锐角
										if (_FLT_LESS_FLT(0.0f, c))
										{
											//得到顶点到点光源的向量长度与点光源范围的比值
											float ratio = 1.0f - light_directory_negative_length / m_LightWorld[i].light.radius;

											//叠加定向光漫反射效果
											m_ColorAfterIlluminationCompute[j] += 
												m_LightWorld[i].light.color.Mul(m_Material.diffuse) * c * ratio;
										}
									}
								}
							}

							break;
						}
					}
				}
			}
		}

		//范围约束
		for (int i = 0; i < normal_count; ++i)
		{
			if (_FLT_LESS_FLT(255.0f, m_ColorAfterIlluminationCompute[i].x))
				m_ColorAfterIlluminationCompute[i].x = 255.0f;

			if (_FLT_LESS_FLT(255.0f, m_ColorAfterIlluminationCompute[i].y))
				m_ColorAfterIlluminationCompute[i].y = 255.0f;

			if (_FLT_LESS_FLT(255.0f, m_ColorAfterIlluminationCompute[i].z))
				m_ColorAfterIlluminationCompute[i].z = 255.0f;
		}
	}

	void Render::Draw3DMeshTriangleNearPlaneClip_ts0_ic0(
		float sphere_radius,
		const std::vector<int>* triangle_origin)
	{
		//得到摄像机坐标系下面包围球球心
		vector3 center_camera = ComputerCenterInCamera();

		//无需进行近截面裁剪
		if (_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, center_camera.z - sphere_radius))
		{
			m_pTriangleAfterNearPlaneClip = triangle_origin;
			return;
		}

		//更新原始线段索引到更新线段索引
		m_TriangleAfterNearPlaneClip.clear();
		int triangle_count = (int)triangle_origin->size();
		for (int i = 0; i < triangle_count; i += 3)
		{
			//得到三角索引
			int index[3] =
			{
				triangle_origin->at(i),
				triangle_origin->at(i + 1),
				triangle_origin->at(i + 2),
			};

			//得到顶点
			vector3* vertex[3] =
			{
				&m_VertexInCamera[index[0]],
				&m_VertexInCamera[index[1]],
				&m_VertexInCamera[index[2]],
			};

			//三点的z值同时小于等于近截面，4种情况：3点小、2点小1点等、1点小2点等、3点等
			if (_FLT_LESS_EQUAL_FLT(vertex[0]->z, m_NearPlaneZInCamera) &&
				_FLT_LESS_EQUAL_FLT(vertex[1]->z, m_NearPlaneZInCamera) &&
				_FLT_LESS_EQUAL_FLT(vertex[2]->z, m_NearPlaneZInCamera))
			{
				//不放入更新三角索引
			}
			//三点的z值同时大于等于近截面，3种情况：3点大、2点大1点等、1点大2点等
			else if (_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[0]->z) &&
					 _FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[1]->z) &&
					 _FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[2]->z))
			{
				//放入更新线段索引
				m_TriangleAfterNearPlaneClip.push_back(index[0]);
				m_TriangleAfterNearPlaneClip.push_back(index[1]);
				m_TriangleAfterNearPlaneClip.push_back(index[2]);
			}
			//2种情况：1点小2点大、2点小1点大
			else
			{
				//得到具体情况
				int less_near_count =
					(_FLT_LESS_FLT(vertex[0]->z, m_NearPlaneZInCamera) ? 1 : 0) +
					(_FLT_LESS_FLT(vertex[1]->z, m_NearPlaneZInCamera) ? 1 : 0) +
					(_FLT_LESS_FLT(vertex[2]->z, m_NearPlaneZInCamera) ? 1 : 0);

				//1点小2点大
				if (1 == less_near_count)
				{
					//得到2大点和1小点的下标
					int more_index[2] = {-1, -1};
					int less_index = -1;
					if (_FLT_LESS_FLT(vertex[0]->z, m_NearPlaneZInCamera))
					{
						less_index = 0;
						more_index[0] = 1;
						more_index[1] = 2;
					}
					else if (_FLT_LESS_FLT(vertex[1]->z, m_NearPlaneZInCamera))
					{
						less_index = 1;
						more_index[0] = 2;
						more_index[1] = 0;
					}
					else
					{
						less_index = 2;
						more_index[0] = 0;
						more_index[1] = 1;
					}

					//得到2大点和1小点
					vector3* less_vertex = vertex[less_index];
					vector3* more_vertex[2] =
					{
						vertex[more_index[0]],
						vertex[more_index[1]],
					};

					//计算截点xy坐标，因为摄像机坐标系下面任意顶点的xyz都是线性关系，故有以下等式
					//m_CoordinateCameraNearPlane - 大->z   截->x - 大->x   截->y - 大->y
					//----------------------------------- = ------------- = -------------
					//m_CoordinateCameraNearPlane - 小->z   截->x - 小->x   截->y - 小->y
					vector3 cut_vertex[2] =
					{
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
					};
					float temp[2] =
					{
						(cut_vertex[0].z - more_vertex[0]->z) / (cut_vertex[0].z - less_vertex->z),
						(cut_vertex[1].z - more_vertex[1]->z) / (cut_vertex[1].z - less_vertex->z),
					};
					float temp_minus_1[2] =
					{
						temp[0] - 1.0f,
						temp[1] - 1.0f,
					};
					cut_vertex[0].x =
						_FLT_EQUAL_FLT(more_vertex[0]->x, less_vertex->x) ? less_vertex->x : (temp[0] * less_vertex->x - more_vertex[0]->x) / temp_minus_1[0];
					cut_vertex[0].y =
						_FLT_EQUAL_FLT(more_vertex[0]->y, less_vertex->y) ? less_vertex->y : (temp[0] * less_vertex->y - more_vertex[0]->y) / temp_minus_1[0];
					cut_vertex[1].x =
						_FLT_EQUAL_FLT(more_vertex[1]->x, less_vertex->x) ? less_vertex->x : (temp[1] * less_vertex->x - more_vertex[1]->x) / temp_minus_1[1];
					cut_vertex[1].y =
						_FLT_EQUAL_FLT(more_vertex[1]->y, less_vertex->y) ? less_vertex->y : (temp[1] * less_vertex->y - more_vertex[1]->y) / temp_minus_1[1];

					//新点入表并得到下标
					int cut_vertex_index[2] = { -1, -1 };
					m_VertexInCamera.push_back(cut_vertex[0]);
					cut_vertex_index[0] = (int)m_VertexInCamera.size() - 1;
					m_VertexInCamera.push_back(cut_vertex[1]);
					cut_vertex_index[1] = (int)m_VertexInCamera.size() - 1;

					//得到2个新三角索引
					int cut_triangle_index[2][3] =
					{
						{
							index[0],
							index[1],
							index[2],
						},
						{
							index[0],
							index[1],
							index[2],
						},
					};
					//新三角索引0：保留2大点的索引，加入1截点索引
					cut_triangle_index[0][less_index] = cut_vertex_index[0];
					//新三角索引1：保留1大点的索引，加入2截点索引
					cut_triangle_index[1][more_index[0]] = cut_vertex_index[0];
					cut_triangle_index[1][less_index] = cut_vertex_index[1];

					//放入更新线段索引
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][2]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][2]);
				}
				//2点小1点大
				else
				{
					//得到1大点和2小点的下标
					int more_index = -1;
					int less_index[2] = {-1, -1};
					if (_FLT_LESS_FLT(m_NearPlaneZInCamera, vertex[0]->z))
					{
						more_index = 0;
						less_index[0] = 1;
						less_index[1] = 2;
					}
					else if (_FLT_LESS_FLT(m_NearPlaneZInCamera, vertex[1]->z))
					{
						more_index = 1;
						less_index[0] = 2;
						less_index[1] = 0;
					}
					else
					{
						more_index = 2;
						less_index[0] = 0;
						less_index[1] = 1;
					}

					//得到1大点和2小点
					vector3* more_vertex = vertex[more_index];
					vector3* less_vertex[2] =
					{
						vertex[less_index[0]],
						vertex[less_index[1]],
					};

					//计算截点xy坐标，因为摄像机坐标系下面任意顶点的xyz都是线性关系，故有以下等式
					//m_CoordinateCameraNearPlane - 大->z   截->x - 大->x   截->y - 大->y
					//----------------------------------- = ------------- = -------------
					//m_CoordinateCameraNearPlane - 小->z   截->x - 小->x   截->y - 小->y
					vector3 cut_vertex[2] =
					{
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
					};
					float temp[2] =
					{
						(cut_vertex[0].z - more_vertex->z) / (cut_vertex[0].z - less_vertex[0]->z),
						(cut_vertex[1].z - more_vertex->z) / (cut_vertex[1].z - less_vertex[1]->z),
					};
					float temp_minus_1[2] =
					{
						temp[0] - 1.0f,
						temp[1] - 1.0f,
					};
					cut_vertex[0].x = 
						_FLT_EQUAL_FLT(more_vertex->x, less_vertex[0]->x) ? more_vertex->x : (temp[0] * less_vertex[0]->x - more_vertex->x) / temp_minus_1[0];
					cut_vertex[0].y =
						_FLT_EQUAL_FLT(more_vertex->y, less_vertex[0]->y) ? more_vertex->y : (temp[0] * less_vertex[0]->y - more_vertex->y) / temp_minus_1[0];
					cut_vertex[1].x =
						_FLT_EQUAL_FLT(more_vertex->x, less_vertex[1]->x) ? more_vertex->x : (temp[1] * less_vertex[1]->x - more_vertex->x) / temp_minus_1[1];
					cut_vertex[1].y =
						_FLT_EQUAL_FLT(more_vertex->y, less_vertex[1]->y) ? more_vertex->y : (temp[1] * less_vertex[1]->y - more_vertex->y) / temp_minus_1[1];

					//新点入表并得到下标
					int cut_vertex_index[2] = { -1, -1 };
					m_VertexInCamera.push_back(cut_vertex[0]);
					cut_vertex_index[0] = (int)m_VertexInCamera.size() - 1;
					m_VertexInCamera.push_back(cut_vertex[1]);
					cut_vertex_index[1] = (int)m_VertexInCamera.size() - 1;

					//得到1个新三角索引
					int cut_triangle_index[3] =
					{
						index[0],
						index[1],
						index[2],
					};
					//新三角索引：保留1大点的索引，加入2截点索引
					cut_triangle_index[less_index[0]] = cut_vertex_index[0];
					cut_triangle_index[less_index[1]] = cut_vertex_index[1];

					//放入更新线段索引
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[2]);
				}
			}
		}

		m_pTriangleAfterNearPlaneClip = &m_TriangleAfterNearPlaneClip;
	}

	void Render::Draw3DMeshTriangleNearPlaneClip_ts0_ic1(
		float sphere_radius,
		const std::vector<int>* triangle_origin)
	{
		//得到摄像机坐标系下面包围球球心
		vector3 center_camera = ComputerCenterInCamera();

		//无需进行近截面裁剪
		if (_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, center_camera.z - sphere_radius))
		{
			m_pTriangleAfterNearPlaneClip = triangle_origin;
			return;
		}

		//更新原始线段索引到更新线段索引
		m_TriangleAfterNearPlaneClip.clear();
		int triangle_count = (int)triangle_origin->size();
		for (int i = 0; i < triangle_count; i += 3)
		{
			//得到三角索引
			int index[3] =
			{
				triangle_origin->at(i),
				triangle_origin->at(i + 1),
				triangle_origin->at(i + 2),
			};

			//得到顶点
			vector3* vertex[3] =
			{
				&m_VertexInCamera[index[0]],
				&m_VertexInCamera[index[1]],
				&m_VertexInCamera[index[2]],
			};

			//得到颜色
			vector3* color[3] =
			{
				&m_ColorAfterIlluminationCompute[index[0]],
				&m_ColorAfterIlluminationCompute[index[1]],
				&m_ColorAfterIlluminationCompute[index[2]],
			};

			//三点的z值同时小于等于近截面，4种情况：3点小、2点小1点等、1点小2点等、3点等
			if (_FLT_LESS_EQUAL_FLT(vertex[0]->z, m_NearPlaneZInCamera) &&
				_FLT_LESS_EQUAL_FLT(vertex[1]->z, m_NearPlaneZInCamera) &&
				_FLT_LESS_EQUAL_FLT(vertex[2]->z, m_NearPlaneZInCamera))
			{
				//不放入更新三角索引
			}
			//三点的z值同时大于等于近截面，3种情况：3点大、2点大1点等、1点大2点等
			else if (
				_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[0]->z) &&
				_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[1]->z) &&
				_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[2]->z))
			{
				//放入更新线段索引
				m_TriangleAfterNearPlaneClip.push_back(index[0]);
				m_TriangleAfterNearPlaneClip.push_back(index[1]);
				m_TriangleAfterNearPlaneClip.push_back(index[2]);
			}
			//2种情况：1点小2点大、2点小1点大
			else
			{
				//得到具体情况
				int less_near_count =
					(_FLT_LESS_FLT(vertex[0]->z, m_NearPlaneZInCamera) ? 1 : 0) +
					(_FLT_LESS_FLT(vertex[1]->z, m_NearPlaneZInCamera) ? 1 : 0) +
					(_FLT_LESS_FLT(vertex[2]->z, m_NearPlaneZInCamera) ? 1 : 0);

				//1点小2点大
				if (1 == less_near_count)
				{
					//得到2大点和1小点的下标
					int more_index[2] = {-1, -1};
					int less_index = -1;
					if (_FLT_LESS_FLT(vertex[0]->z, m_NearPlaneZInCamera))
					{
						less_index = 0;
						more_index[0] = 1;
						more_index[1] = 2;
					}
					else if (_FLT_LESS_FLT(vertex[1]->z, m_NearPlaneZInCamera))
					{
						less_index = 1;
						more_index[0] = 2;
						more_index[1] = 0;
					}
					else
					{
						less_index = 2;
						more_index[0] = 0;
						more_index[1] = 1;
					}

					//得到2大点和1小点，及其颜色
					vector3* less_vertex = vertex[less_index];
					vector3* more_vertex[2] =
					{
						vertex[more_index[0]],
						vertex[more_index[1]],
					};
					vector3* less_color = color[less_index];
					vector3* more_color[2] =
					{
						color[more_index[0]],
						color[more_index[1]],
					};

					//计算截点xy坐标，因为摄像机坐标系下面任意顶点的xyz都是线性关系，故有以下等式
					//m_CoordinateCameraNearPlane - 大->z   截->x - 大->x   截->y - 大->y
					//----------------------------------- = ------------- = -------------
					//m_CoordinateCameraNearPlane - 小->z   截->x - 小->x   截->y - 小->y
					vector3 cut_vertex[2] =
					{
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
					};
					vector3 cut_color[2] =
					{
						vector3(0.0f, 0.0f, 0.0f),
						vector3(0.0f, 0.0f, 0.0f),
					};
					float temp[2] =
					{
						(cut_vertex[0].z - more_vertex[0]->z) / (cut_vertex[0].z - less_vertex->z),
						(cut_vertex[1].z - more_vertex[1]->z) / (cut_vertex[1].z - less_vertex->z),
					};
					float temp_minus_1[2] =
					{
						temp[0] - 1.0f,
						temp[1] - 1.0f,
					};
					cut_vertex[0].x =
						_FLT_EQUAL_FLT(more_vertex[0]->x, less_vertex->x) ? less_vertex->x : (temp[0] * less_vertex->x - more_vertex[0]->x) / temp_minus_1[0];
					cut_vertex[0].y =
						_FLT_EQUAL_FLT(more_vertex[0]->y, less_vertex->y) ? less_vertex->y : (temp[0] * less_vertex->y - more_vertex[0]->y) / temp_minus_1[0];
					cut_color[0].x =
						_FLT_EQUAL_FLT(more_color[0]->x, less_color->x) ? less_color->x : (temp[0] * less_color->x - more_color[0]->x) / temp_minus_1[0];
					cut_color[0].y =
						_FLT_EQUAL_FLT(more_color[0]->y, less_color->y) ? less_color->y : (temp[0] * less_color->y - more_color[0]->y) / temp_minus_1[0];
					cut_color[0].z =
						_FLT_EQUAL_FLT(more_color[0]->z, less_color->z) ? less_color->z : (temp[0] * less_color->z - more_color[0]->z) / temp_minus_1[0];
					cut_vertex[1].x =
						_FLT_EQUAL_FLT(more_vertex[1]->x, less_vertex->x) ? less_vertex->x : (temp[1] * less_vertex->x - more_vertex[1]->x) / temp_minus_1[1];
					cut_vertex[1].y =
						_FLT_EQUAL_FLT(more_vertex[1]->y, less_vertex->y) ? less_vertex->y : (temp[1] * less_vertex->y - more_vertex[1]->y) / temp_minus_1[1];
					cut_color[1].x =
						_FLT_EQUAL_FLT(more_color[1]->x, less_color->x) ? less_color->x : (temp[1] * less_color->x - more_color[1]->x) / temp_minus_1[1];
					cut_color[1].y =
						_FLT_EQUAL_FLT(more_color[1]->y, less_color->y) ? less_color->y : (temp[1] * less_color->y - more_color[1]->y) / temp_minus_1[1];
					cut_color[1].z =
						_FLT_EQUAL_FLT(more_color[1]->z, less_color->z) ? less_color->z : (temp[1] * less_color->z - more_color[1]->z) / temp_minus_1[1];
					
					//新点入表并得到下标
					int cut_vertex_index[2] = { -1, -1 };
					m_VertexInCamera.push_back(cut_vertex[0]);
					m_ColorAfterIlluminationCompute.push_back(cut_color[0]);
					cut_vertex_index[0] = (int)m_VertexInCamera.size() - 1;
					m_VertexInCamera.push_back(cut_vertex[1]);
					m_ColorAfterIlluminationCompute.push_back(cut_color[1]);
					cut_vertex_index[1] = (int)m_VertexInCamera.size() - 1;

					//得到2个新三角索引
					int cut_triangle_index[2][3] =
					{
						{
							index[0],
							index[1],
							index[2],
						},
						{
							index[0],
							index[1],
							index[2],
						},
					};
					//新三角索引0：保留2大点的索引，加入1截点索引
					cut_triangle_index[0][less_index] = cut_vertex_index[0];
					//新三角索引1：保留1大点的索引，加入2截点索引
					cut_triangle_index[1][more_index[0]] = cut_vertex_index[0];
					cut_triangle_index[1][less_index] = cut_vertex_index[1];

					//放入更新线段索引
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][2]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][2]);
				}
				//2点小1点大
				else
				{
					//得到1大点和2小点的下标
					int more_index = -1;
					int less_index[2] = {-1, -1};
					if (_FLT_LESS_FLT(m_NearPlaneZInCamera, vertex[0]->z))
					{
						more_index = 0;
						less_index[0] = 1;
						less_index[1] = 2;
					}
					else if (_FLT_LESS_FLT(m_NearPlaneZInCamera, vertex[1]->z))
					{
						more_index = 1;
						less_index[0] = 2;
						less_index[1] = 0;
					}
					else
					{
						more_index = 2;
						less_index[0] = 0;
						less_index[1] = 1;
					}

					//得到1大点和2小点，及其颜色
					vector3* more_vertex = vertex[more_index];
					vector3* less_vertex[2] =
					{
						vertex[less_index[0]],
						vertex[less_index[1]],
					};
					vector3* more_color = color[more_index];
					vector3* less_color[2] =
					{
						color[less_index[0]],
						color[less_index[1]],
					};

					//计算截点xy坐标，因为摄像机坐标系下面任意顶点的xyz都是线性关系，故有以下等式
					//m_CoordinateCameraNearPlane - 大->z   截->x - 大->x   截->y - 大->y
					//----------------------------------- = ------------- = -------------
					//m_CoordinateCameraNearPlane - 小->z   截->x - 小->x   截->y - 小->y
					vector3 cut_vertex[2] =
					{
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
					};
					vector3 cut_color[2] =
					{
						vector3(0.0f, 0.0f, 0.0f),
						vector3(0.0f, 0.0f, 0.0f),
					};
					float temp[2] =
					{
						(cut_vertex[0].z - more_vertex->z) / (cut_vertex[0].z - less_vertex[0]->z),
						(cut_vertex[1].z - more_vertex->z) / (cut_vertex[1].z - less_vertex[1]->z),
					};
					float temp_minus_1[2] =
					{
						temp[0] - 1.0f,
						temp[1] - 1.0f,
					};
					cut_vertex[0].x = 
						_FLT_EQUAL_FLT(more_vertex->x, less_vertex[0]->x) ? more_vertex->x : (temp[0] * less_vertex[0]->x - more_vertex->x) / temp_minus_1[0];
					cut_vertex[0].y =
						_FLT_EQUAL_FLT(more_vertex->y, less_vertex[0]->y) ? more_vertex->y : (temp[0] * less_vertex[0]->y - more_vertex->y) / temp_minus_1[0];
					cut_color[0].x =
						_FLT_EQUAL_FLT(more_color->x, less_color[0]->x) ? more_color->x : (temp[0] * less_color[0]->x - more_color->x) / temp_minus_1[0];
					cut_color[0].y =
						_FLT_EQUAL_FLT(more_color->y, less_color[0]->y) ? more_color->y : (temp[0] * less_color[0]->y - more_color->y) / temp_minus_1[0];
					cut_color[0].z =
						_FLT_EQUAL_FLT(more_color->z, less_color[0]->z) ? more_color->z : (temp[0] * less_color[0]->z - more_color->z) / temp_minus_1[0];
					cut_vertex[1].x =
						_FLT_EQUAL_FLT(more_vertex->x, less_vertex[1]->x) ? more_vertex->x : (temp[1] * less_vertex[1]->x - more_vertex->x) / temp_minus_1[1];
					cut_vertex[1].y =
						_FLT_EQUAL_FLT(more_vertex->y, less_vertex[1]->y) ? more_vertex->y : (temp[1] * less_vertex[1]->y - more_vertex->y) / temp_minus_1[1];
					cut_color[1].x =
						_FLT_EQUAL_FLT(more_color->x, less_color[1]->x) ? more_color->x : (temp[1] * less_color[1]->x - more_color->x) / temp_minus_1[1];
					cut_color[1].y =
						_FLT_EQUAL_FLT(more_color->y, less_color[1]->y) ? more_color->y : (temp[1] * less_color[1]->y - more_color->y) / temp_minus_1[1];
					cut_color[1].z =
						_FLT_EQUAL_FLT(more_color->z, less_color[1]->z) ? more_color->z : (temp[1] * less_color[1]->z - more_color->z) / temp_minus_1[1];

					//新点入表并得到下标
					int cut_vertex_index[2] = { -1, -1 };
					m_VertexInCamera.push_back(cut_vertex[0]);
					m_ColorAfterIlluminationCompute.push_back(cut_color[0]);
					cut_vertex_index[0] = (int)m_VertexInCamera.size() - 1;
					m_VertexInCamera.push_back(cut_vertex[1]);
					m_ColorAfterIlluminationCompute.push_back(cut_color[1]);
					cut_vertex_index[1] = (int)m_VertexInCamera.size() - 1;

					//得到1个新三角索引
					int cut_triangle_index[3] =
					{
						index[0],
						index[1],
						index[2],
					};
					//新三角索引：保留1大点的索引，加入2截点索引
					cut_triangle_index[less_index[0]] = cut_vertex_index[0];
					cut_triangle_index[less_index[1]] = cut_vertex_index[1];

					//放入更新线段索引
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[2]);
				}
			}
		}

		m_pTriangleAfterNearPlaneClip = &m_TriangleAfterNearPlaneClip;
	}
	void Render::Draw3DMeshTriangleNearPlaneClip_ts1_ic0(
		float sphere_radius,
		const std::vector<int>* triangle_origin)
	{
		//得到摄像机坐标系下面包围球球心
		vector3 center_camera = ComputerCenterInCamera();

		//无需进行近截面裁剪
		if (_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, center_camera.z - sphere_radius))
		{
			m_pTriangleAfterNearPlaneClip = triangle_origin;
			return;
		}

		//更新原始线段索引到更新线段索引
		m_TriangleAfterNearPlaneClip.clear();
		int triangle_count = (int)triangle_origin->size();
		for (int i = 0; i < triangle_count; i += 3)
		{
			//得到三角索引
			int index[3] =
			{
				triangle_origin->at(i),
				triangle_origin->at(i + 1),
				triangle_origin->at(i + 2),
			};

			//得到顶点
			vector3* vertex[3] =
			{
				&m_VertexInCamera[index[0]],
				&m_VertexInCamera[index[1]],
				&m_VertexInCamera[index[2]],
			};

			//得到纹理
			vector2* texture[3] =
			{
				&m_TextureCopy[index[0]],
				&m_TextureCopy[index[1]],
				&m_TextureCopy[index[2]],
			};

			//三点的z值同时小于等于近截面，4种情况：3点小、2点小1点等、1点小2点等、3点等
			if (_FLT_LESS_EQUAL_FLT(vertex[0]->z, m_NearPlaneZInCamera) &&
				_FLT_LESS_EQUAL_FLT(vertex[1]->z, m_NearPlaneZInCamera) &&
				_FLT_LESS_EQUAL_FLT(vertex[2]->z, m_NearPlaneZInCamera))
			{
				//不放入更新三角索引
			}
			//三点的z值同时大于等于近截面，3种情况：3点大、2点大1点等、1点大2点等
			else if (
				_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[0]->z) &&
				_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[1]->z) &&
				_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[2]->z))
			{
				//放入更新线段索引
				m_TriangleAfterNearPlaneClip.push_back(index[0]);
				m_TriangleAfterNearPlaneClip.push_back(index[1]);
				m_TriangleAfterNearPlaneClip.push_back(index[2]);
			}
			//2种情况：1点小2点大、2点小1点大
			else
			{
				//得到具体情况
				int less_near_count =
					(_FLT_LESS_FLT(vertex[0]->z, m_NearPlaneZInCamera) ? 1 : 0) +
					(_FLT_LESS_FLT(vertex[1]->z, m_NearPlaneZInCamera) ? 1 : 0) +
					(_FLT_LESS_FLT(vertex[2]->z, m_NearPlaneZInCamera) ? 1 : 0);

				//1点小2点大
				if (1 == less_near_count)
				{
					//得到2大点和1小点的下标
					int more_index[2] = {-1, -1};
					int less_index = -1;
					if (_FLT_LESS_FLT(vertex[0]->z, m_NearPlaneZInCamera))
					{
						less_index = 0;
						more_index[0] = 1;
						more_index[1] = 2;
					}
					else if (_FLT_LESS_FLT(vertex[1]->z, m_NearPlaneZInCamera))
					{
						less_index = 1;
						more_index[0] = 2;
						more_index[1] = 0;
					}
					else
					{
						less_index = 2;
						more_index[0] = 0;
						more_index[1] = 1;
					}

					//得到2大点和1小点，及其纹理
					vector3* less_vertex = vertex[less_index];
					vector3* more_vertex[2] =
					{
						vertex[more_index[0]],
						vertex[more_index[1]],
					};
					vector2* less_texture = texture[less_index];
					vector2* more_texture[2] =
					{
						texture[more_index[0]],
						texture[more_index[1]],
					};

					//计算截点xy坐标，因为摄像机坐标系下面任意顶点的xyz都是线性关系，故有以下等式
					//m_CoordinateCameraNearPlane - 大->z   截->x - 大->x   截->y - 大->y
					//----------------------------------- = ------------- = -------------
					//m_CoordinateCameraNearPlane - 小->z   截->x - 小->x   截->y - 小->y
					vector3 cut_vertex[2] =
					{
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
					};
					vector2 cut_texture[2] =
					{
						vector2(0.0f, 0.0f),
						vector2(0.0f, 0.0f),
					};
					float temp[2] =
					{
						(cut_vertex[0].z - more_vertex[0]->z) / (cut_vertex[0].z - less_vertex->z),
						(cut_vertex[1].z - more_vertex[1]->z) / (cut_vertex[1].z - less_vertex->z),
					};
					float temp_minus_1[2] =
					{
						temp[0] - 1.0f,
						temp[1] - 1.0f,
					};
					cut_vertex[0].x =
						_FLT_EQUAL_FLT(more_vertex[0]->x, less_vertex->x) ? less_vertex->x : (temp[0] * less_vertex->x - more_vertex[0]->x) / temp_minus_1[0];
					cut_vertex[0].y =
						_FLT_EQUAL_FLT(more_vertex[0]->y, less_vertex->y) ? less_vertex->y : (temp[0] * less_vertex->y - more_vertex[0]->y) / temp_minus_1[0];
					cut_texture[0].x =
						_FLT_EQUAL_FLT(more_texture[0]->x, less_texture->x) ? less_texture->x : (temp[0] * less_texture->x - more_texture[0]->x) / temp_minus_1[0];
					cut_texture[0].y =
						_FLT_EQUAL_FLT(more_texture[0]->y, less_texture->y) ? less_texture->y : (temp[0] * less_texture->y - more_texture[0]->y) / temp_minus_1[0];
					cut_vertex[1].x =
						_FLT_EQUAL_FLT(more_vertex[1]->x, less_vertex->x) ? less_vertex->x : (temp[1] * less_vertex->x - more_vertex[1]->x) / temp_minus_1[1];
					cut_vertex[1].y =
						_FLT_EQUAL_FLT(more_vertex[1]->y, less_vertex->y) ? less_vertex->y : (temp[1] * less_vertex->y - more_vertex[1]->y) / temp_minus_1[1];
					cut_texture[1].x =
						_FLT_EQUAL_FLT(more_texture[1]->x, less_texture->x) ? less_texture->x : (temp[1] * less_texture->x - more_texture[1]->x) / temp_minus_1[1];
					cut_texture[1].y =
						_FLT_EQUAL_FLT(more_texture[1]->y, less_texture->y) ? less_texture->y : (temp[1] * less_texture->y - more_texture[1]->y) / temp_minus_1[1];

					//新点入表并得到下标
					int cut_vertex_index[2] = { -1, -1 };
					m_VertexInCamera.push_back(cut_vertex[0]);
					m_TextureCopy.push_back(cut_texture[0]);
					cut_vertex_index[0] = (int)m_VertexInCamera.size() - 1;
					m_VertexInCamera.push_back(cut_vertex[1]);
					m_TextureCopy.push_back(cut_texture[1]);
					cut_vertex_index[1] = (int)m_VertexInCamera.size() - 1;

					//得到2个新三角索引
					int cut_triangle_index[2][3] =
					{
						{
							index[0],
							index[1],
							index[2],
						},
						{
							index[0],
							index[1],
							index[2],
						},
					};
					//新三角索引0：保留2大点的索引，加入1截点索引
					cut_triangle_index[0][less_index] = cut_vertex_index[0];
					//新三角索引1：保留1大点的索引，加入2截点索引
					cut_triangle_index[1][more_index[0]] = cut_vertex_index[0];
					cut_triangle_index[1][less_index] = cut_vertex_index[1];

					//放入更新线段索引
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][2]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][2]);
				}
				//2点小1点大
				else
				{
					//得到1大点和2小点的下标
					int more_index = -1;
					int less_index[2] = {-1, -1};
					if (_FLT_LESS_FLT(m_NearPlaneZInCamera, vertex[0]->z))
					{
						more_index = 0;
						less_index[0] = 1;
						less_index[1] = 2;
					}
					else if (_FLT_LESS_FLT(m_NearPlaneZInCamera, vertex[1]->z))
					{
						more_index = 1;
						less_index[0] = 2;
						less_index[1] = 0;
					}
					else
					{
						more_index = 2;
						less_index[0] = 0;
						less_index[1] = 1;
					}

					//得到1大点和2小点，及其纹理
					vector3* more_vertex = vertex[more_index];
					vector3* less_vertex[2] =
					{
						vertex[less_index[0]],
						vertex[less_index[1]],
					};
					vector2* more_texture = texture[more_index];
					vector2* less_texture[2] =
					{
						texture[less_index[0]],
						texture[less_index[1]],
					};

					//计算截点xy坐标，因为摄像机坐标系下面任意顶点的xyz都是线性关系，故有以下等式
					//m_CoordinateCameraNearPlane - 大->z   截->x - 大->x   截->y - 大->y
					//----------------------------------- = ------------- = -------------
					//m_CoordinateCameraNearPlane - 小->z   截->x - 小->x   截->y - 小->y
					vector3 cut_vertex[2] =
					{
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
					};
					vector2 cut_texture[2] =
					{
						vector2(0.0f, 0.0f),
						vector2(0.0f, 0.0f),
					};
					float temp[2] =
					{
						(cut_vertex[0].z - more_vertex->z) / (cut_vertex[0].z - less_vertex[0]->z),
						(cut_vertex[1].z - more_vertex->z) / (cut_vertex[1].z - less_vertex[1]->z),
					};
					float temp_minus_1[2] =
					{
						temp[0] - 1.0f,
						temp[1] - 1.0f,
					};
					cut_vertex[0].x = 
						_FLT_EQUAL_FLT(more_vertex->x, less_vertex[0]->x) ? more_vertex->x : (temp[0] * less_vertex[0]->x - more_vertex->x) / temp_minus_1[0];
					cut_vertex[0].y =
						_FLT_EQUAL_FLT(more_vertex->y, less_vertex[0]->y) ? more_vertex->y : (temp[0] * less_vertex[0]->y - more_vertex->y) / temp_minus_1[0];
					cut_texture[0].x =
						_FLT_EQUAL_FLT(more_texture->x, less_texture[0]->x) ? more_texture->x : (temp[0] * less_texture[0]->x - more_texture->x) / temp_minus_1[0];
					cut_texture[0].y =
						_FLT_EQUAL_FLT(more_texture->y, less_texture[0]->y) ? more_texture->y : (temp[0] * less_texture[0]->y - more_texture->y) / temp_minus_1[0];
					cut_vertex[1].x =
						_FLT_EQUAL_FLT(more_vertex->x, less_vertex[1]->x) ? more_vertex->x : (temp[1] * less_vertex[1]->x - more_vertex->x) / temp_minus_1[1];
					cut_vertex[1].y =
						_FLT_EQUAL_FLT(more_vertex->y, less_vertex[1]->y) ? more_vertex->y : (temp[1] * less_vertex[1]->y - more_vertex->y) / temp_minus_1[1];
					cut_texture[1].x =
						_FLT_EQUAL_FLT(more_texture->x, less_texture[1]->x) ? more_texture->x : (temp[1] * less_texture[1]->x - more_texture->x) / temp_minus_1[1];
					cut_texture[1].y =
						_FLT_EQUAL_FLT(more_texture->y, less_texture[1]->y) ? more_texture->y : (temp[1] * less_texture[1]->y - more_texture->y) / temp_minus_1[1];
					
					//新点入表并得到下标
					int cut_vertex_index[2] = { -1, -1 };
					m_VertexInCamera.push_back(cut_vertex[0]);
					m_TextureCopy.push_back(cut_texture[0]);
					cut_vertex_index[0] = (int)m_VertexInCamera.size() - 1;
					m_VertexInCamera.push_back(cut_vertex[1]);
					m_TextureCopy.push_back(cut_texture[1]);
					cut_vertex_index[1] = (int)m_VertexInCamera.size() - 1;

					//得到1个新三角索引
					int cut_triangle_index[3] =
					{
						index[0],
						index[1],
						index[2],
					};
					//新三角索引：保留1大点的索引，加入2截点索引
					cut_triangle_index[less_index[0]] = cut_vertex_index[0];
					cut_triangle_index[less_index[1]] = cut_vertex_index[1];

					//放入更新线段索引
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[2]);
				}
			}
		}

		m_pTriangleAfterNearPlaneClip = &m_TriangleAfterNearPlaneClip;
	}
	void Render::Draw3DMeshTriangleNearPlaneClip_ts1_ic1(
		float sphere_radius,
		const std::vector<int>* triangle_origin)
	{
		//得到摄像机坐标系下面包围球球心
		vector3 center_camera = ComputerCenterInCamera();

		//无需进行近截面裁剪
		if (_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, center_camera.z - sphere_radius))
		{
			m_pTriangleAfterNearPlaneClip = triangle_origin;
			return;
		}

		//更新原始线段索引到更新线段索引
		m_TriangleAfterNearPlaneClip.clear();
		int triangle_count = (int)triangle_origin->size();
		for (int i = 0; i < triangle_count; i += 3)
		{
			//得到三角索引
			int index[3] =
			{
				triangle_origin->at(i),
				triangle_origin->at(i + 1),
				triangle_origin->at(i + 2),
			};

			//得到顶点
			vector3* vertex[3] =
			{
				&m_VertexInCamera[index[0]],
				&m_VertexInCamera[index[1]],
				&m_VertexInCamera[index[2]],
			};

			//得到颜色
			vector3* color[3] =
			{
				&m_ColorAfterIlluminationCompute[index[0]],
				&m_ColorAfterIlluminationCompute[index[1]],
				&m_ColorAfterIlluminationCompute[index[2]],
			};

			//得到纹理
			vector2* texture[3] =
			{
				&m_TextureCopy[index[0]],
				&m_TextureCopy[index[1]],
				&m_TextureCopy[index[2]],
			};

			//三点的z值同时小于等于近截面，4种情况：3点小、2点小1点等、1点小2点等、3点等
			if (_FLT_LESS_EQUAL_FLT(vertex[0]->z, m_NearPlaneZInCamera) &&
				_FLT_LESS_EQUAL_FLT(vertex[1]->z, m_NearPlaneZInCamera) &&
				_FLT_LESS_EQUAL_FLT(vertex[2]->z, m_NearPlaneZInCamera))
			{
				//不放入更新三角索引
			}
			//三点的z值同时大于等于近截面，3种情况：3点大、2点大1点等、1点大2点等
			else if (
				_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[0]->z) &&
				_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[1]->z) &&
				_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, vertex[2]->z))
			{
				//放入更新线段索引
				m_TriangleAfterNearPlaneClip.push_back(index[0]);
				m_TriangleAfterNearPlaneClip.push_back(index[1]);
				m_TriangleAfterNearPlaneClip.push_back(index[2]);
			}
			//2种情况：1点小2点大、2点小1点大
			else
			{
				//得到具体情况
				int less_near_count =
					(_FLT_LESS_FLT(vertex[0]->z, m_NearPlaneZInCamera) ? 1 : 0) +
					(_FLT_LESS_FLT(vertex[1]->z, m_NearPlaneZInCamera) ? 1 : 0) +
					(_FLT_LESS_FLT(vertex[2]->z, m_NearPlaneZInCamera) ? 1 : 0);

				//1点小2点大
				if (1 == less_near_count)
				{
					//得到2大点和1小点的下标
					int more_index[2] = {-1, -1};
					int less_index = -1;
					if (_FLT_LESS_FLT(vertex[0]->z, m_NearPlaneZInCamera))
					{
						less_index = 0;
						more_index[0] = 1;
						more_index[1] = 2;
					}
					else if (_FLT_LESS_FLT(vertex[1]->z, m_NearPlaneZInCamera))
					{
						less_index = 1;
						more_index[0] = 2;
						more_index[1] = 0;
					}
					else
					{
						less_index = 2;
						more_index[0] = 0;
						more_index[1] = 1;
					}

					//得到2大点和1小点，及其颜色、纹理
					vector3* less_vertex = vertex[less_index];
					vector3* more_vertex[2] =
					{
						vertex[more_index[0]],
						vertex[more_index[1]],
					};
					vector3* less_color = color[less_index];
					vector3* more_color[2] =
					{
						color[more_index[0]],
						color[more_index[1]],
					};
					vector2* less_texture = texture[less_index];
					vector2* more_texture[2] =
					{
						texture[more_index[0]],
						texture[more_index[1]],
					};

					//计算截点xy坐标，因为摄像机坐标系下面任意顶点的xyz都是线性关系，故有以下等式
					//m_CoordinateCameraNearPlane - 大->z   截->x - 大->x   截->y - 大->y
					//----------------------------------- = ------------- = -------------
					//m_CoordinateCameraNearPlane - 小->z   截->x - 小->x   截->y - 小->y
					vector3 cut_vertex[2] =
					{
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
					};
					vector3 cut_color[2] =
					{
						vector3(0.0f, 0.0f, 0.0f),
						vector3(0.0f, 0.0f, 0.0f),
					};
					vector2 cut_texture[2] =
					{
						vector2(0.0f, 0.0f),
						vector2(0.0f, 0.0f),
					};
					float temp[2] =
					{
						(cut_vertex[0].z - more_vertex[0]->z) / (cut_vertex[0].z - less_vertex->z),
						(cut_vertex[1].z - more_vertex[1]->z) / (cut_vertex[1].z - less_vertex->z),
					};
					float temp_minus_1[2] =
					{
						temp[0] - 1.0f,
						temp[1] - 1.0f,
					};
					cut_vertex[0].x =
						_FLT_EQUAL_FLT(more_vertex[0]->x, less_vertex->x) ? less_vertex->x : (temp[0] * less_vertex->x - more_vertex[0]->x) / temp_minus_1[0];
					cut_vertex[0].y =
						_FLT_EQUAL_FLT(more_vertex[0]->y, less_vertex->y) ? less_vertex->y : (temp[0] * less_vertex->y - more_vertex[0]->y) / temp_minus_1[0];
					cut_color[0].x =
						_FLT_EQUAL_FLT(more_color[0]->x, less_color->x) ? less_color->x : (temp[0] * less_color->x - more_color[0]->x) / temp_minus_1[0];
					cut_color[0].y =
						_FLT_EQUAL_FLT(more_color[0]->y, less_color->y) ? less_color->y : (temp[0] * less_color->y - more_color[0]->y) / temp_minus_1[0];
					cut_color[0].z =
						_FLT_EQUAL_FLT(more_color[0]->z, less_color->z) ? less_color->z : (temp[0] * less_color->z - more_color[0]->z) / temp_minus_1[0];
					cut_texture[0].x =
						_FLT_EQUAL_FLT(more_texture[0]->x, less_texture->x) ? less_texture->x : (temp[0] * less_texture->x - more_texture[0]->x) / temp_minus_1[0];
					cut_texture[0].y =
						_FLT_EQUAL_FLT(more_texture[0]->y, less_texture->y) ? less_texture->y : (temp[0] * less_texture->y - more_texture[0]->y) / temp_minus_1[0];
					cut_vertex[1].x =
						_FLT_EQUAL_FLT(more_vertex[1]->x, less_vertex->x) ? less_vertex->x : (temp[1] * less_vertex->x - more_vertex[1]->x) / temp_minus_1[1];
					cut_vertex[1].y =
						_FLT_EQUAL_FLT(more_vertex[1]->y, less_vertex->y) ? less_vertex->y : (temp[1] * less_vertex->y - more_vertex[1]->y) / temp_minus_1[1];
					cut_color[1].x =
						_FLT_EQUAL_FLT(more_color[1]->x, less_color->x) ? less_color->x : (temp[1] * less_color->x - more_color[1]->x) / temp_minus_1[1];
					cut_color[1].y =
						_FLT_EQUAL_FLT(more_color[1]->y, less_color->y) ? less_color->y : (temp[1] * less_color->y - more_color[1]->y) / temp_minus_1[1];
					cut_color[1].z =
						_FLT_EQUAL_FLT(more_color[1]->z, less_color->z) ? less_color->z : (temp[1] * less_color->z - more_color[1]->z) / temp_minus_1[1];
					cut_texture[1].x =
						_FLT_EQUAL_FLT(more_texture[1]->x, less_texture->x) ? less_texture->x : (temp[1] * less_texture->x - more_texture[1]->x) / temp_minus_1[1];
					cut_texture[1].y =
						_FLT_EQUAL_FLT(more_texture[1]->y, less_texture->y) ? less_texture->y : (temp[1] * less_texture->y - more_texture[1]->y) / temp_minus_1[1];
					
					//新点入表并得到下标
					int cut_vertex_index[2] = { -1, -1 };
					m_VertexInCamera.push_back(cut_vertex[0]);
					m_ColorAfterIlluminationCompute.push_back(cut_color[0]);
					m_TextureCopy.push_back(cut_texture[0]);
					cut_vertex_index[0] = (int)m_VertexInCamera.size() - 1;
					m_VertexInCamera.push_back(cut_vertex[1]);
					m_ColorAfterIlluminationCompute.push_back(cut_color[1]);
					m_TextureCopy.push_back(cut_texture[1]);
					cut_vertex_index[1] = (int)m_VertexInCamera.size() - 1;

					//得到2个新三角索引
					int cut_triangle_index[2][3] =
					{
						{
							index[0],
							index[1],
							index[2],
						},
						{
							index[0],
							index[1],
							index[2],
						},
					};
					//新三角索引0：保留2大点的索引，加入1截点索引
					cut_triangle_index[0][less_index] = cut_vertex_index[0];
					//新三角索引1：保留1大点的索引，加入2截点索引
					cut_triangle_index[1][more_index[0]] = cut_vertex_index[0];
					cut_triangle_index[1][less_index] = cut_vertex_index[1];

					//放入更新线段索引
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0][2]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1][2]);
				}
				//2点小1点大
				else
				{
					//得到1大点和2小点的下标
					int more_index = -1;
					int less_index[2] = {-1, -1};
					if (_FLT_LESS_FLT(m_NearPlaneZInCamera, vertex[0]->z))
					{
						more_index = 0;
						less_index[0] = 1;
						less_index[1] = 2;
					}
					else if (_FLT_LESS_FLT(m_NearPlaneZInCamera, vertex[1]->z))
					{
						more_index = 1;
						less_index[0] = 2;
						less_index[1] = 0;
					}
					else
					{
						more_index = 2;
						less_index[0] = 0;
						less_index[1] = 1;
					}

					//得到1大点和2小点，及其颜色、纹理
					vector3* more_vertex = vertex[more_index];
					vector3* less_vertex[2] =
					{
						vertex[less_index[0]],
						vertex[less_index[1]],
					};
					vector3* more_color = color[more_index];
					vector3* less_color[2] =
					{
						color[less_index[0]],
						color[less_index[1]],
					};
					vector2* more_texture = texture[more_index];
					vector2* less_texture[2] =
					{
						texture[less_index[0]],
						texture[less_index[1]],
					};

					//计算截点xy坐标，因为摄像机坐标系下面任意顶点的xyz都是线性关系，故有以下等式
					//m_CoordinateCameraNearPlane - 大->z   截->x - 大->x   截->y - 大->y
					//----------------------------------- = ------------- = -------------
					//m_CoordinateCameraNearPlane - 小->z   截->x - 小->x   截->y - 小->y
					vector3 cut_vertex[2] =
					{
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
						vector3(0.0f, 0.0f, m_NearPlaneZInCamera),
					};
					vector3 cut_color[2] =
					{
						vector3(0.0f, 0.0f, 0.0f),
						vector3(0.0f, 0.0f, 0.0f),
					};
					vector2 cut_texture[2] =
					{
						vector2(0.0f, 0.0f),
						vector2(0.0f, 0.0f),
					};
					float temp[2] =
					{
						(cut_vertex[0].z - more_vertex->z) / (cut_vertex[0].z - less_vertex[0]->z),
						(cut_vertex[1].z - more_vertex->z) / (cut_vertex[1].z - less_vertex[1]->z),
					};
					float temp_minus_1[2] =
					{
						temp[0] - 1.0f,
						temp[1] - 1.0f,
					};
					cut_vertex[0].x = 
						_FLT_EQUAL_FLT(more_vertex->x, less_vertex[0]->x) ? more_vertex->x : (temp[0] * less_vertex[0]->x - more_vertex->x) / temp_minus_1[0];
					cut_vertex[0].y =
						_FLT_EQUAL_FLT(more_vertex->y, less_vertex[0]->y) ? more_vertex->y : (temp[0] * less_vertex[0]->y - more_vertex->y) / temp_minus_1[0];
					cut_color[0].x =
						_FLT_EQUAL_FLT(more_color->x, less_color[0]->x) ? more_color->x : (temp[0] * less_color[0]->x - more_color->x) / temp_minus_1[0];
					cut_color[0].y =
						_FLT_EQUAL_FLT(more_color->y, less_color[0]->y) ? more_color->y : (temp[0] * less_color[0]->y - more_color->y) / temp_minus_1[0];
					cut_color[0].z =
						_FLT_EQUAL_FLT(more_color->z, less_color[0]->z) ? more_color->z : (temp[0] * less_color[0]->z - more_color->z) / temp_minus_1[0];
					cut_texture[0].x =
						_FLT_EQUAL_FLT(more_texture->x, less_texture[0]->x) ? more_texture->x : (temp[0] * less_texture[0]->x - more_texture->x) / temp_minus_1[0];
					cut_texture[0].y =
						_FLT_EQUAL_FLT(more_texture->y, less_texture[0]->y) ? more_texture->y : (temp[0] * less_texture[0]->y - more_texture->y) / temp_minus_1[0];
					cut_vertex[1].x =
						_FLT_EQUAL_FLT(more_vertex->x, less_vertex[1]->x) ? more_vertex->x : (temp[1] * less_vertex[1]->x - more_vertex->x) / temp_minus_1[1];
					cut_vertex[1].y =
						_FLT_EQUAL_FLT(more_vertex->y, less_vertex[1]->y) ? more_vertex->y : (temp[1] * less_vertex[1]->y - more_vertex->y) / temp_minus_1[1];
					cut_color[1].x =
						_FLT_EQUAL_FLT(more_color->x, less_color[1]->x) ? more_color->x : (temp[1] * less_color[1]->x - more_color->x) / temp_minus_1[1];
					cut_color[1].y =
						_FLT_EQUAL_FLT(more_color->y, less_color[1]->y) ? more_color->y : (temp[1] * less_color[1]->y - more_color->y) / temp_minus_1[1];
					cut_color[1].z =
						_FLT_EQUAL_FLT(more_color->z, less_color[1]->z) ? more_color->z : (temp[1] * less_color[1]->z - more_color->z) / temp_minus_1[1];
					cut_texture[1].x =
						_FLT_EQUAL_FLT(more_texture->x, less_texture[1]->x) ? more_texture->x : (temp[1] * less_texture[1]->x - more_texture->x) / temp_minus_1[1];
					cut_texture[1].y =
						_FLT_EQUAL_FLT(more_texture->y, less_texture[1]->y) ? more_texture->y : (temp[1] * less_texture[1]->y - more_texture->y) / temp_minus_1[1];

					//新点入表并得到下标
					int cut_vertex_index[2] = { -1, -1 };
					m_VertexInCamera.push_back(cut_vertex[0]);
					m_ColorAfterIlluminationCompute.push_back(cut_color[0]);
					m_TextureCopy.push_back(cut_texture[0]);
					cut_vertex_index[0] = (int)m_VertexInCamera.size() - 1;
					m_VertexInCamera.push_back(cut_vertex[1]);
					m_ColorAfterIlluminationCompute.push_back(cut_color[1]);
					m_TextureCopy.push_back(cut_texture[1]);
					cut_vertex_index[1] = (int)m_VertexInCamera.size() - 1;

					//得到1个新三角索引
					int cut_triangle_index[3] =
					{
						index[0],
						index[1],
						index[2],
					};
					//新三角索引：保留1大点的索引，加入2截点索引
					cut_triangle_index[less_index[0]] = cut_vertex_index[0];
					cut_triangle_index[less_index[1]] = cut_vertex_index[1];

					//放入更新线段索引
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[0]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[1]);
					m_TriangleAfterNearPlaneClip.push_back(cut_triangle_index[2]);
				}
			}
		}

		m_pTriangleAfterNearPlaneClip = &m_TriangleAfterNearPlaneClip;
	}

	void Render::FaceCulling()
	{
		int triangle_count = (int)m_pTriangleAfterNearPlaneClip->size() / 3;

		//得到投影坐标系顶点
		std::vector<vector3>* vertex_projection = &m_VertexInProjection;

		//清空可见三角
		m_TriangleAfterFaceCulling.clear();

		//背面拣选
		if (m_FaceCullingBack)
		{
			for (int i = 0; i < triangle_count; ++i)
			{
				//得到三角形索引
				int j = i * 3;
				int i0 = m_pTriangleAfterNearPlaneClip->at(j);
				int i1 = m_pTriangleAfterNearPlaneClip->at(j + 1);
				int i2 = m_pTriangleAfterNearPlaneClip->at(j + 2);

				//计算面法线
				vector3 u1 = vertex_projection->at(i0) - vertex_projection->at(i1);
				vector3 u2 = vertex_projection->at(i1) - vertex_projection->at(i2);
				vector3 normal = u1.Cross(u2);
				normal = normal.Normalize();

				//判断是否可见，注意两个向量的点乘结果的正负只和它们夹角的大小有关系，和
				//向量长度无关，因为(v1 dot v2)结果为|v1||v2|cos(a)，而|v1||v2|的结果为正
				//，简单点说就是两个向量的点乘结果就可以看出它们的夹角大小了
				if (normal.Dot(m_SightLineInProjection) < 0.0f)
					m_TriangleAfterFaceCulling.push_back(i);
			}
		}
		//正面拣选
		else
		{
			for (int i = 0; i < triangle_count; ++i)
			{
				int j = i * 3;
				int i0 = m_pTriangleAfterNearPlaneClip->at(j);
				int i1 = m_pTriangleAfterNearPlaneClip->at(j + 1);
				int i2 = m_pTriangleAfterNearPlaneClip->at(j + 2);

				vector3 u1 = vertex_projection->at(i0) - vertex_projection->at(i1);
				vector3 u2 = vertex_projection->at(i1) - vertex_projection->at(i2);
				vector3 normal = u1.Cross(u2);
				normal = normal.Normalize();

				if (normal.Dot(m_SightLineInProjection) > 0.0f)
					m_TriangleAfterFaceCulling.push_back(i);
			}
		}
	}

	int Render::Draw3DMeshTriangleFill_ts0_ic0(
		int index0, int index1, int index2, float* vertex_data0, float* vertex_data1, float* vertex_data2)
	{
		vector3* v0 = &m_VertexInView[index0];
		vector3* v1 = &m_VertexInView[index1];
		vector3* v2 = &m_VertexInView[index2];

		vertex_data0[0] = v0->y;
		vertex_data0[1] = v0->x;
		vertex_data0[2] = 1.0f / v0->z;

		vertex_data1[0] = v1->y;
		vertex_data1[1] = v1->x;
		vertex_data1[2] = 1.0f / v1->z;

		vertex_data2[0] = v2->y;
		vertex_data2[1] = v2->x;
		vertex_data2[2] = 1.0f / v2->z;

		return 3;
	}
	int Render::Draw3DMeshTriangleFill_ts0_ic1(
		int index0, int index1, int index2, float* vertex_data0, float* vertex_data1, float* vertex_data2)
	{
		vector3* v0 = &m_VertexInView[index0];
		vector3* v1 = &m_VertexInView[index1];
		vector3* v2 = &m_VertexInView[index2];

		vector3* c0 = &m_ColorAfterIlluminationCompute[index0];
		vector3* c1 = &m_ColorAfterIlluminationCompute[index1];
		vector3* c2 = &m_ColorAfterIlluminationCompute[index2];

		vertex_data0[0] = v0->y;
		vertex_data0[1] = v0->x;
		vertex_data0[2] = 1.0f / v0->z;
		vertex_data0[3] = c0->x / v0->z;
		vertex_data0[4] = c0->y / v0->z;
		vertex_data0[5] = c0->z / v0->z;

		vertex_data1[0] = v1->y;
		vertex_data1[1] = v1->x;
		vertex_data1[2] = 1.0f / v1->z;
		vertex_data1[3] = c1->x / v1->z;
		vertex_data1[4] = c1->y / v1->z;
		vertex_data1[5] = c1->z / v1->z;

		vertex_data2[0] = v2->y;
		vertex_data2[1] = v2->x;
		vertex_data2[2] = 1.0f / v2->z;
		vertex_data2[3] = c2->x / v2->z;
		vertex_data2[4] = c2->y / v2->z;
		vertex_data2[5] = c2->z / v2->z;

		return 6;
	}
	int Render::Draw3DMeshTriangleFill_ts1_ic0(
		int index0, int index1, int index2, float* vertex_data0, float* vertex_data1, float* vertex_data2)
	{
		vector3* v0 = &m_VertexInView[index0];
		vector3* v1 = &m_VertexInView[index1];
		vector3* v2 = &m_VertexInView[index2];

		vector2* t0 = &m_TextureCopy[index0];
		vector2* t1 = &m_TextureCopy[index1];
		vector2* t2 = &m_TextureCopy[index2];

		int texture_right = m_pTexture->w - 1;
		int texture_bottom = m_pTexture->h - 1;

		vertex_data0[0] = v0->y;
		vertex_data0[1] = v0->x;
		vertex_data0[2] = 1.0f / v0->z;
		vertex_data0[3] = t0->x * texture_right / v0->z;
		vertex_data0[4] = t0->y * texture_bottom / v0->z;

		vertex_data1[0] = v1->y;
		vertex_data1[1] = v1->x;
		vertex_data1[2] = 1.0f / v1->z;
		vertex_data1[3] = t1->x * texture_right / v1->z;
		vertex_data1[4] = t1->y * texture_bottom / v1->z;

		vertex_data2[0] = v2->y;
		vertex_data2[1] = v2->x;
		vertex_data2[2] = 1.0f / v2->z;
		vertex_data2[3] = t2->x * texture_right / v2->z;
		vertex_data2[4] = t2->y * texture_bottom / v2->z;

		return 5;
	}
	int Render::Draw3DMeshTriangleFill_ts1_ic1(
		int index0, int index1, int index2, float* vertex_data0, float* vertex_data1, float* vertex_data2)
	{
		vector3* v0 = &m_VertexInView[index0];
		vector3* v1 = &m_VertexInView[index1];
		vector3* v2 = &m_VertexInView[index2];

		vector3* c0 = &m_ColorAfterIlluminationCompute[index0];
		vector3* c1 = &m_ColorAfterIlluminationCompute[index1];
		vector3* c2 = &m_ColorAfterIlluminationCompute[index2];

		vector2* t0 = &m_TextureCopy[index0];
		vector2* t1 = &m_TextureCopy[index1];
		vector2* t2 = &m_TextureCopy[index2];

		int texture_right = m_pTexture->w - 1;
		int texture_bottom = m_pTexture->h - 1;

		vertex_data0[0] = v0->y;
		vertex_data0[1] = v0->x;
		vertex_data0[2] = 1.0f / v0->z;
		vertex_data0[3] = c0->x / v0->z;
		vertex_data0[4] = c0->y / v0->z;
		vertex_data0[5] = c0->z / v0->z;
		vertex_data0[6] = t0->x * texture_right / v0->z;
		vertex_data0[7] = t0->y * texture_bottom / v0->z;

		vertex_data1[0] = v1->y;
		vertex_data1[1] = v1->x;
		vertex_data1[2] = 1.0f / v1->z;
		vertex_data1[3] = c1->x / v1->z;
		vertex_data1[4] = c1->y / v1->z;
		vertex_data1[5] = c1->z / v1->z;
		vertex_data0[6] = t1->x * texture_right / v1->z;
		vertex_data0[7] = t1->y * texture_bottom / v1->z;

		vertex_data2[0] = v2->y;
		vertex_data2[1] = v2->x;
		vertex_data2[2] = 1.0f / v2->z;
		vertex_data2[3] = c2->x / v2->z;
		vertex_data2[4] = c2->y / v2->z;
		vertex_data2[5] = c2->z / v2->z;
		vertex_data2[6] = t2->x * texture_right / v2->z;
		vertex_data2[7] = t2->y * texture_bottom / v2->z;

		return 8;
	}

	int Render::TriangleClassify(
		int fill_count,
		const float* vertex_data0,
		const float* vertex_data1,
		const float* vertex_data2,
		float* vertex_data3,
		TRIANGLE_RASTERIZE* triangle_flatbottom,
		TRIANGLE_RASTERIZE* triangle_flattop)
	{
		//如果三点x和y相同
		if (_FLT_EQUAL_FLT(vertex_data0[1], vertex_data1[1]) &&
			_FLT_EQUAL_FLT(vertex_data1[1], vertex_data2[1]) &&
			_FLT_EQUAL_FLT(vertex_data0[0], vertex_data1[0]) &&
			_FLT_EQUAL_FLT(vertex_data1[0], vertex_data2[0]))
			return 0;

		//如果三点共线，即三条线段斜率相同
		if (_FLT_EQUAL_FLT(
			(vertex_data0[0] - vertex_data1[0]) * (vertex_data1[1] - vertex_data2[1]),
			(vertex_data1[0] - vertex_data2[0]) * (vertex_data0[1] - vertex_data1[1])))
			return 0;

		//三点y都不相同
		if (!_FLT_EQUAL_FLT(vertex_data0[0], vertex_data1[0]) &&
			!_FLT_EQUAL_FLT(vertex_data1[0], vertex_data2[0]) &&
			!_FLT_EQUAL_FLT(vertex_data2[0], vertex_data0[0]))
		{
			//按y坐标从小到大排序
			const float* vertex_order_by_y[] =
			{
				vertex_data0,
				vertex_data1,
				vertex_data2,
			};
			for (int i = 2; i > 0; --i)
			{
				for (int j = 0; j < i; ++j)
				{
					if (vertex_order_by_y[j][0] > vertex_order_by_y[j + 1][0])
					{
						const float* temp = vertex_order_by_y[j];
						vertex_order_by_y[j] = vertex_order_by_y[j + 1];
						vertex_order_by_y[j + 1] = temp;
					}
				}
			}

			//y值
			vertex_data3[0] = vertex_order_by_y[1][0];

			//y递增量
			int y_offset_top_bottom = (int)vertex_order_by_y[2][0] - (int)vertex_order_by_y[0][0];
			int y_offset_top_middle = (int)vertex_data3[0] - (int)vertex_order_by_y[0][0];

			//其它值
			for (int i = 1; i < fill_count; ++i)
			{
				float change = 
					(vertex_order_by_y[2][i] - vertex_order_by_y[0][i]) / y_offset_top_bottom;
				vertex_data3[i] = 
					vertex_order_by_y[0][i] + y_offset_top_middle * change;
			}

			//平底平顶分割
			if (_FLT_LESS_FLT(vertex_data3[1], vertex_order_by_y[1][1]))
			{
				//平底
				triangle_flatbottom->left_top = vertex_order_by_y[0];
				triangle_flatbottom->right_top = vertex_order_by_y[0];
				triangle_flatbottom->left_bottom = vertex_data3;
				triangle_flatbottom->right_bottom = vertex_order_by_y[1];

				//平顶
				triangle_flattop->left_top = vertex_data3;
				triangle_flattop->right_top = vertex_order_by_y[1];
				triangle_flattop->left_bottom = vertex_order_by_y[2];
				triangle_flattop->right_bottom = vertex_order_by_y[2];
			}
			else
			{
				//平底
				triangle_flatbottom->left_top = vertex_order_by_y[0];
				triangle_flatbottom->right_top = vertex_order_by_y[0];
				triangle_flatbottom->left_bottom = vertex_order_by_y[1];
				triangle_flatbottom->right_bottom = vertex_data3;

				//平顶
				triangle_flattop->left_top = vertex_order_by_y[1];
				triangle_flattop->right_top = vertex_data3;
				triangle_flattop->left_bottom = vertex_order_by_y[2];
				triangle_flattop->right_bottom = vertex_order_by_y[2];
			}

			return 1 | 2;
		}
		//两点y相同
		else
		{
			const float* left = NULL;
			const float* centre = NULL;
			const float* right = NULL;
			if (_FLT_EQUAL_FLT(vertex_data0[0], vertex_data1[0]))
			{
				if (_FLT_LESS_FLT(vertex_data0[1], vertex_data1[1]))
				{
					left = vertex_data0;
					centre = vertex_data2;
					right = vertex_data1;
				}
				else
				{
					left = vertex_data1;
					centre = vertex_data2;
					right = vertex_data0;
				}
			}
			else if (_FLT_EQUAL_FLT(vertex_data1[0], vertex_data2[0]))
			{
				if (_FLT_LESS_FLT(vertex_data1[1], vertex_data2[1]))
				{
					left = vertex_data1;
					centre = vertex_data0;
					right = vertex_data2;
				}
				else
				{
					left = vertex_data2;
					centre = vertex_data0;
					right = vertex_data1;
				}
			}
			else
			{
				if (_FLT_LESS_FLT(vertex_data2[1], vertex_data0[1]))
				{
					left = vertex_data2;
					centre = vertex_data1;
					right = vertex_data0;
				}
				else
				{
					left = vertex_data0;
					centre = vertex_data1;
					right = vertex_data2;
				}
			}

			//平底
			if (_FLT_LESS_FLT(centre[0], left[0]))
			{
				triangle_flatbottom->left_top = centre;
				triangle_flatbottom->right_top = centre;
				triangle_flatbottom->left_bottom = left;
				triangle_flatbottom->right_bottom = right;

				return 1;
			}
			//平顶
			else
			{
				triangle_flattop->left_top = left;
				triangle_flattop->right_top = right;
				triangle_flattop->left_bottom = centre;
				triangle_flattop->right_bottom = centre;

				return 2;
			}
		}
	}

#define _RASTERIZE_TRAVERSE_Y_BEGIN \
	RECTANGLE rect_w =  \
	{ \
		(int)triangle_rasterize->left_top[1], \
		(int)triangle_rasterize->left_top[0], \
		(int)triangle_rasterize->left_top[1], \
		(int)triangle_rasterize->left_bottom[0] \
	}; \
	if (rect_w.x1 > (int)triangle_rasterize->left_bottom[1]) \
		rect_w.x1 = (int)triangle_rasterize->left_bottom[1]; \
	if (rect_w.x2 < (int)triangle_rasterize->left_bottom[1]) \
		rect_w.x2 = (int)triangle_rasterize->left_bottom[1]; \
	if (rect_w.x1 > (int)triangle_rasterize->right_top[1]) \
		rect_w.x1 = (int)triangle_rasterize->right_top[1]; \
	if (rect_w.x2 < (int)triangle_rasterize->right_top[1]) \
		rect_w.x2 = (int)triangle_rasterize->right_top[1]; \
	if (rect_w.x1 > (int)triangle_rasterize->right_bottom[1]) \
		rect_w.x1 = (int)triangle_rasterize->right_bottom[1]; \
	if (rect_w.x2 < (int)triangle_rasterize->right_bottom[1]) \
		rect_w.x2 = (int)triangle_rasterize->right_bottom[1]; \
	if (!RectangleIntersect(&rect_w, &m_RectangleView, NULL)) \
		return; \
	int y_top = rect_w.y1; \
	int y_bottom = rect_w.y2; \
	int y_offset = y_bottom - y_top; \
	if (0 == y_offset) \
		return; \
	const int data_ey_size = _DATA_SIZE - 1; \
	float data_ey_left[data_ey_size] = {}; \
	float data_ey_right[data_ey_size] = {}; \
	for (int i = 0; i < data_ey_size; ++i) \
	{ \
		data_ey_left[i] = triangle_rasterize->left_top[i + 1]; \
		data_ey_right[i] = triangle_rasterize->right_top[i + 1]; \
	} \
	float change_ey_left[data_ey_size] = {}; \
	float change_ey_right[data_ey_size] = {}; \
	for (int i = 0; i < data_ey_size; ++i) \
	{ \
		change_ey_left[i] = \
			(triangle_rasterize->left_bottom[i + 1] - triangle_rasterize->left_top[i + 1]) / y_offset; \
		change_ey_right[i] = \
			(triangle_rasterize->right_bottom[i + 1] - triangle_rasterize->right_top[i + 1]) / y_offset; \
	} \
	if (y_top < m_RectangleView.y1) \
	{ \
		int y_offset_clip = m_RectangleView.y1 - y_top; \
		for (int i = 0; i < data_ey_size; ++i) \
		{ \
			data_ey_left[i] += y_offset_clip * change_ey_left[i]; \
			data_ey_right[i] += y_offset_clip * change_ey_right[i]; \
		} \
		y_top = m_RectangleView.y1; \
	} \
	if (y_bottom > m_RectangleView.y2) \
		y_bottom = m_RectangleView.y2; \
	const int data_eyx_size = data_ey_size - 1; \
	for (int y = y_top; y < y_bottom; ++y) \
	{

#define _RASTERIZE_TRAVERSE_X_BEGIN \
		int x_left = (int)data_ey_left[0]; \
		int x_right = (int)data_ey_right[0]; \
		int x_offset = x_right - x_left; \
		if (0 < x_offset) \
		{ \
			float data_eyx[data_eyx_size] = {}; \
			for (int i = 0; i < data_eyx_size; ++i) \
				data_eyx[i] = data_ey_left[i + 1]; \
			float change_eyx[data_eyx_size] = {}; \
			for (int i = 0; i < data_eyx_size; ++i) \
				change_eyx[i] = (data_ey_right[i + 1] - data_ey_left[i + 1]) / x_offset; \
			if (x_left < m_RectangleView.x1) \
			{ \
				int x_offset_clip = m_RectangleView.x1 - x_left; \
				for (int i = 0; i < data_eyx_size; ++i) \
					data_eyx[i] += x_offset_clip * change_eyx[i]; \
				x_left = m_RectangleView.x1; \
			} \
			if (x_right > m_RectangleView.x2) \
				x_right = m_RectangleView.x2; \
			for (int x = x_left; x < x_right; ++x) \
			{ \
				int pixel_idx = x + y * m_BufferWidth;

#define _RASTERIZE_TRAVERSE_X_END \
				for (int i = 0; i < data_eyx_size; ++i) \
					data_eyx[i] += change_eyx[i]; \
			} \
		}

#define _RASTERIZE_TRAVERSE_Y_END \
		for (int i = 0; i < data_ey_size; ++i) \
		{ \
			data_ey_left[i] += change_ey_left[i]; \
			data_ey_right[i] += change_ey_right[i]; \
		} \
	}

	void Render::Draw3DMeshTriangleRasterize_ts0_ic1_ab0_dt0(const TRIANGLE_RASTERIZE* triangle_rasterize)
	{
		//y、x、1/z、c.x/z、c.y/z、c.z/z
#define _DATA_SIZE 6
		_RASTERIZE_TRAVERSE_Y_BEGIN
		{
			_RASTERIZE_TRAVERSE_X_BEGIN
			{
				m_pVideoBuffer[pixel_idx] = _COLOR_SET(
					(unsigned char)(data_eyx[1] / data_eyx[0]),
					(unsigned char)(data_eyx[2] / data_eyx[0]),
					(unsigned char)(data_eyx[3] / data_eyx[0]));
			}
			_RASTERIZE_TRAVERSE_X_END
		}
		_RASTERIZE_TRAVERSE_Y_END
#undef _DATA_SIZE
	}
	void Render::Draw3DMeshTriangleRasterize_ts0_ic1_ab0_dt1(const TRIANGLE_RASTERIZE* triangle_rasterize)
	{
		//y、x、1/z、c.x/z、c.y/z、c.z/z
#define _DATA_SIZE 6
		_RASTERIZE_TRAVERSE_Y_BEGIN
		{
			_RASTERIZE_TRAVERSE_X_BEGIN
			{
				if (_FLT_LESS_FLT(m_pDepthBuffer[pixel_idx], data_eyx[0]))
				{
					m_pVideoBuffer[pixel_idx] =_COLOR_SET(
						(unsigned char)(data_eyx[1] / data_eyx[0]),
						(unsigned char)(data_eyx[2] / data_eyx[0]),
						(unsigned char)(data_eyx[3] / data_eyx[0]));

					//设置深度
					m_pDepthBuffer[pixel_idx] = data_eyx[0];
				}
			}
			_RASTERIZE_TRAVERSE_X_END
		}
		_RASTERIZE_TRAVERSE_Y_END
#undef _DATA_SIZE
	}
	void Render::Draw3DMeshTriangleRasterize_ts0_ic1_ab1_dt0(const TRIANGLE_RASTERIZE* triangle_rasterize)
	{
		//y、x、1/z、c.x/z、c.y/z、c.z/z
#define _DATA_SIZE 6
		_RASTERIZE_TRAVERSE_Y_BEGIN
		{
			_RASTERIZE_TRAVERSE_X_BEGIN
			{
				//设置混合颜色
				m_pVideoBuffer[pixel_idx] = _COLOR_SET(
					(int)(_COLOR_GET_R(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + data_eyx[1] / data_eyx[0] * m_ForegroundAlphaBlendValue),
					(int)(_COLOR_GET_G(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + data_eyx[2] / data_eyx[0] * m_ForegroundAlphaBlendValue),
					(int)(_COLOR_GET_B(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + data_eyx[3] / data_eyx[0] * m_ForegroundAlphaBlendValue));
			}
			_RASTERIZE_TRAVERSE_X_END
		}
		_RASTERIZE_TRAVERSE_Y_END
#undef _DATA_SIZE
	}
	void Render::Draw3DMeshTriangleRasterize_ts0_ic1_ab1_dt1(const TRIANGLE_RASTERIZE* triangle_rasterize)
	{
		//y、x、1/z、c.x/z、c.y/z、c.z/z
#define _DATA_SIZE 6
		_RASTERIZE_TRAVERSE_Y_BEGIN
		{
			_RASTERIZE_TRAVERSE_X_BEGIN
			{
				if (_FLT_LESS_FLT(m_pDepthBuffer[pixel_idx], data_eyx[0]))
				{
					//设置混合颜色
					m_pVideoBuffer[pixel_idx] = _COLOR_SET(
						(int)(_COLOR_GET_R(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + data_eyx[1] / data_eyx[0] * m_ForegroundAlphaBlendValue),
						(int)(_COLOR_GET_G(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + data_eyx[2] / data_eyx[0] * m_ForegroundAlphaBlendValue),
						(int)(_COLOR_GET_B(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + data_eyx[3] / data_eyx[0] * m_ForegroundAlphaBlendValue));

					//设置深度
					m_pDepthBuffer[pixel_idx] = data_eyx[0];
				}
			}
			_RASTERIZE_TRAVERSE_X_END
		}
		_RASTERIZE_TRAVERSE_Y_END
#undef _DATA_SIZE
	}
	void Render::Draw3DMeshTriangleRasterize_ts1_ic0_ab0_dt0(const TRIANGLE_RASTERIZE* triangle_rasterize)
	{
		//y、x、1/z、t.x/z、t.y/z
#define _DATA_SIZE 5
		_RASTERIZE_TRAVERSE_Y_BEGIN
		{
			_RASTERIZE_TRAVERSE_X_BEGIN
			{
				//设置显示
				m_pVideoBuffer[pixel_idx] =
					m_pTexture->c[(int)(data_eyx[1] / data_eyx[0]) + (int)(data_eyx[2] / data_eyx[0]) * m_pTexture->w];
			}
			_RASTERIZE_TRAVERSE_X_END
		}
		_RASTERIZE_TRAVERSE_Y_END
#undef _DATA_SIZE
	}

	void Render::Draw3DMeshTriangleRasterize_ts1_ic0_ab0_dt1(const TRIANGLE_RASTERIZE* triangle_rasterize)
	{
		//y、x、1/z、t.x/z、t.y/z
#define _DATA_SIZE 5

		//得到包围矩形并进行可见性测试
		RECTANGLE rect_w = 
		{
			(int)triangle_rasterize->left_top[1],
			(int)triangle_rasterize->left_top[0],
			(int)triangle_rasterize->left_top[1],
			(int)triangle_rasterize->left_bottom[0]
		};
		if (rect_w.x1 > (int)triangle_rasterize->left_bottom[1])
			rect_w.x1 = (int)triangle_rasterize->left_bottom[1];
		if (rect_w.x2 < (int)triangle_rasterize->left_bottom[1])
			rect_w.x2 = (int)triangle_rasterize->left_bottom[1];
		if (rect_w.x1 > (int)triangle_rasterize->right_top[1])
			rect_w.x1 = (int)triangle_rasterize->right_top[1];
		if (rect_w.x2 < (int)triangle_rasterize->right_top[1])
			rect_w.x2 = (int)triangle_rasterize->right_top[1];
		if (rect_w.x1 > (int)triangle_rasterize->right_bottom[1])
			rect_w.x1 = (int)triangle_rasterize->right_bottom[1];
		if (rect_w.x2 < (int)triangle_rasterize->right_bottom[1])
			rect_w.x2 = (int)triangle_rasterize->right_bottom[1];
		if (!RectangleIntersect(&rect_w, &m_RectangleView, NULL))
			return;

		//得到整数化y值
		int y_top = rect_w.y1;
		int y_bottom = rect_w.y2;

		//得到整数化y值递增量
		int y_offset = y_bottom - y_top;

		//y值递增量为0不绘制
		if (0 == y_offset)
			return;

		//得到除y之外数据数量
		const int data_ey_size = _DATA_SIZE - 1;

		//得到除y之外数据初始量
		float data_ey_left[data_ey_size] = {};
		float data_ey_right[data_ey_size] = {};
		for (int i = 0; i < data_ey_size; ++i)
		{
			data_ey_left[i] = triangle_rasterize->left_top[i + 1];
			data_ey_right[i] = triangle_rasterize->right_top[i + 1];
		}

		//得到除y之外数据随y递增变化量
		float change_ey_left[data_ey_size] = {};
		float change_ey_right[data_ey_size] = {};
		for (int i = 0; i < data_ey_size; ++i)
		{
			change_ey_left[i] =
				(triangle_rasterize->left_bottom[i + 1] - triangle_rasterize->left_top[i + 1]) / y_offset;
			change_ey_right[i] =
				(triangle_rasterize->right_bottom[i + 1] - triangle_rasterize->right_top[i + 1]) / y_offset;
		}

		//y方向裁剪
		if (y_top < m_RectangleView.y1)
		{
			int y_offset_clip = m_RectangleView.y1 - y_top;
			for (int i = 0; i < data_ey_size; ++i)
			{
				data_ey_left[i] += y_offset_clip * change_ey_left[i];
				data_ey_right[i] += y_offset_clip * change_ey_right[i];
			}
			y_top = m_RectangleView.y1;
		}
		if (y_bottom > m_RectangleView.y2)
			y_bottom = m_RectangleView.y2;

		//得到除yx之外数据数量
		const int data_eyx_size = data_ey_size - 1;

		//光栅化
		for (int y = y_top; y < y_bottom; ++y)
		{
			//得到整数化x值
			int x_left = (int)data_ey_left[0];
			int x_right = (int)data_ey_right[0];

			//得到整数化x值递增量
			int x_offset = x_right - x_left;

			//x值递增量大于0绘制
			if (0 < x_offset)
			{
				//得到除yx之外数据初始量
				float data_eyx[data_eyx_size] = {};
				for (int i = 0; i < data_eyx_size; ++i)
					data_eyx[i] = data_ey_left[i + 1];

				//得到除yx之外数据随x递增变化量
				float change_eyx[data_eyx_size] = {};
				for (int i = 0; i < data_eyx_size; ++i)
					change_eyx[i] = (data_ey_right[i + 1] - data_ey_left[i + 1]) / x_offset;

				//x方向裁剪
				if (x_left < m_RectangleView.x1)
				{
					int x_offset_clip = m_RectangleView.x1 - x_left;
					for (int i = 0; i < data_eyx_size; ++i)
						data_eyx[i] += x_offset_clip * change_eyx[i];
					x_left = m_RectangleView.x1;
				}
				if (x_right > m_RectangleView.x2)
					x_right = m_RectangleView.x2;

				//光栅化
				for (int x = x_left; x < x_right; ++x)
				{
					//得到像素下标
					int pixel_idx = x + y * m_BufferWidth;

					//深度测试
					if (_FLT_LESS_FLT(m_pDepthBuffer[pixel_idx], data_eyx[0]))
					{
						//设置颜色
						m_pVideoBuffer[pixel_idx] = 
							m_pTexture->c[(int)(data_eyx[1] / data_eyx[0]) + (int)(data_eyx[2] / data_eyx[0]) * m_pTexture->w];

						//设置深度
						m_pDepthBuffer[pixel_idx] = data_eyx[0];
					}

					//除yx之外数据量变化
					for (int i = 0; i < data_eyx_size; ++i)
						data_eyx[i] += change_eyx[i];
				}
			}

			//除y之外数据量变化
			for (int i = 0; i < data_ey_size; ++i)
			{
				data_ey_left[i] += change_ey_left[i];
				data_ey_right[i] += change_ey_right[i];
			}
		}
#undef _DATA_SIZE
	}
	void Render::Draw3DMeshTriangleRasterize_ts1_ic0_ab1_dt0(const TRIANGLE_RASTERIZE* triangle_rasterize)
	{
		//y、x、1/z、t.x/z、t.y/z
#define _DATA_SIZE 5
		_RASTERIZE_TRAVERSE_Y_BEGIN
		{
			_RASTERIZE_TRAVERSE_X_BEGIN
			{
				//得到纹理颜色
				int color_texture = 
					m_pTexture->c[(int)(data_eyx[1] / data_eyx[0]) + (int)(data_eyx[2] / data_eyx[0]) * m_pTexture->w];

				//设置混合颜色
				m_pVideoBuffer[pixel_idx] = _COLOR_SET(
					(int)(_COLOR_GET_R(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + _COLOR_GET_R(color_texture) * m_ForegroundAlphaBlendValue),
					(int)(_COLOR_GET_G(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + _COLOR_GET_G(color_texture) * m_ForegroundAlphaBlendValue),
					(int)(_COLOR_GET_B(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + _COLOR_GET_B(color_texture) * m_ForegroundAlphaBlendValue));
			}
			_RASTERIZE_TRAVERSE_X_END
		}
		_RASTERIZE_TRAVERSE_Y_END
#undef _DATA_SIZE
	}
	void Render::Draw3DMeshTriangleRasterize_ts1_ic0_ab1_dt1(const TRIANGLE_RASTERIZE* triangle_rasterize)
	{
		//y、x、1/z、t.x/z、t.y/z
#define _DATA_SIZE 5
		_RASTERIZE_TRAVERSE_Y_BEGIN
		{
			_RASTERIZE_TRAVERSE_X_BEGIN
			{
				//深度测试
				if (_FLT_LESS_FLT(m_pDepthBuffer[pixel_idx], data_eyx[0]))
				{
					//得到纹理颜色
					int color_texture =
						m_pTexture->c[(int)(data_eyx[1] / data_eyx[0]) + (int)(data_eyx[2] / data_eyx[0]) * m_pTexture->w];

					//设置混合颜色
					m_pVideoBuffer[pixel_idx] = _COLOR_SET(
						(int)(_COLOR_GET_R(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + _COLOR_GET_R(color_texture) * m_ForegroundAlphaBlendValue),
						(int)(_COLOR_GET_G(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + _COLOR_GET_G(color_texture) * m_ForegroundAlphaBlendValue),
						(int)(_COLOR_GET_B(m_pVideoBuffer[pixel_idx]) * m_BackgroundAlphaBlendValue + _COLOR_GET_B(color_texture) * m_ForegroundAlphaBlendValue));

					//设置深度
					m_pDepthBuffer[pixel_idx] = data_eyx[0];
				}
			}
			_RASTERIZE_TRAVERSE_X_END
		}
		_RASTERIZE_TRAVERSE_Y_END
#undef _DATA_SIZE
	}

	Render::Render(const Render& that)
	{}

	Render& Render::operator = (const Render& that)
	{
		return *this;
	}

	Render::Render()
		: m_SightLineInProjection(0.0f, 0.0f, 1.0f)
		, m_pVideoBuffer(NULL)
		, m_pDepthBuffer(NULL)
		, m_pTexture(NULL)
		, m_pSegmentAfterNearPlaneClip(NULL)
		, m_pTriangleAfterNearPlaneClip(NULL)
	{
		m_fDraw3DMeshSegmentRasterize[0] = &Render::Draw3DMeshSegmentRasterize_ab0_dt0;
		m_fDraw3DMeshSegmentRasterize[1] = &Render::Draw3DMeshSegmentRasterize_ab0_dt1;
		m_fDraw3DMeshSegmentRasterize[2] = &Render::Draw3DMeshSegmentRasterize_ab1_dt0;
		m_fDraw3DMeshSegmentRasterize[3] = &Render::Draw3DMeshSegmentRasterize_ab1_dt1;

		m_fDraw3DMeshTriangleNearPlaneClip[0] = &Render::Draw3DMeshTriangleNearPlaneClip_ts0_ic0;
		m_fDraw3DMeshTriangleNearPlaneClip[1] = &Render::Draw3DMeshTriangleNearPlaneClip_ts0_ic1;
		m_fDraw3DMeshTriangleNearPlaneClip[2] = &Render::Draw3DMeshTriangleNearPlaneClip_ts1_ic0;
		m_fDraw3DMeshTriangleNearPlaneClip[3] = &Render::Draw3DMeshTriangleNearPlaneClip_ts1_ic1;

		m_fDraw3DMeshTriangleFill[0] = &Render::Draw3DMeshTriangleFill_ts0_ic0;
		m_fDraw3DMeshTriangleFill[1] = &Render::Draw3DMeshTriangleFill_ts0_ic1;
		m_fDraw3DMeshTriangleFill[2] = &Render::Draw3DMeshTriangleFill_ts1_ic0;
		m_fDraw3DMeshTriangleFill[3] = &Render::Draw3DMeshTriangleFill_ts1_ic1;

		m_fDraw3DMeshTriangleRasterize[0x0] = NULL;
		m_fDraw3DMeshTriangleRasterize[0x1] = NULL;
		m_fDraw3DMeshTriangleRasterize[0x2] = NULL;
		m_fDraw3DMeshTriangleRasterize[0x3] = NULL;
		m_fDraw3DMeshTriangleRasterize[0x4] = &Render::Draw3DMeshTriangleRasterize_ts0_ic1_ab0_dt0;
		m_fDraw3DMeshTriangleRasterize[0x5] = &Render::Draw3DMeshTriangleRasterize_ts0_ic1_ab0_dt1;
		m_fDraw3DMeshTriangleRasterize[0x6] = &Render::Draw3DMeshTriangleRasterize_ts0_ic1_ab1_dt0;
		m_fDraw3DMeshTriangleRasterize[0x7] = &Render::Draw3DMeshTriangleRasterize_ts0_ic1_ab1_dt1;
		m_fDraw3DMeshTriangleRasterize[0x8] = &Render::Draw3DMeshTriangleRasterize_ts1_ic0_ab0_dt0;
		m_fDraw3DMeshTriangleRasterize[0x9] = &Render::Draw3DMeshTriangleRasterize_ts1_ic0_ab0_dt1;
		m_fDraw3DMeshTriangleRasterize[0xa] = &Render::Draw3DMeshTriangleRasterize_ts1_ic0_ab1_dt0;
		m_fDraw3DMeshTriangleRasterize[0xb] = &Render::Draw3DMeshTriangleRasterize_ts1_ic0_ab1_dt1;
		m_fDraw3DMeshTriangleRasterize[0xc] = NULL;
		m_fDraw3DMeshTriangleRasterize[0xd] = NULL;
		m_fDraw3DMeshTriangleRasterize[0xe] = NULL;
		m_fDraw3DMeshTriangleRasterize[0xf] = NULL;
	}

	//析构
	Render::~Render()
	{
		End();
	}

	void Render::Init(
		int buffer_width,
		int buffer_height,
		float near_plane_z_in_camera,
		float far_plane_z_in_camera,
		float foreground_alpha_blend_value,
		int default_texture_color,
		const vector3* eye,
		const vector3* at,
		const vector3* up)
	{
		End();

		m_BufferWidth = buffer_width;
		m_BufferHeight = buffer_height;
		m_BufferSize = m_BufferWidth * m_BufferHeight;

		m_pVideoBuffer = (int*)malloc(sizeof(int) * m_BufferSize);

		m_pDepthBuffer = (float*)malloc(sizeof(float) * m_BufferSize);

		m_TransformWorld.Indentity();
		matrix4 transform_camera;
		SetTransform(
			_COORDINATE_CAMERA,
			ComputeTransformCamera(&transform_camera, eye, at, up));
		m_TransformProjection.Indentity();
		matrix4 transform_view;
		SetTransform(
			_COORDINATE_VIEW,
			ComputeTransformView(&transform_view, 0, 0, m_BufferWidth, m_BufferHeight));

		m_NearPlaneZInCamera = near_plane_z_in_camera;
		m_FarPlaneZInCamera = far_plane_z_in_camera;

		m_VertexInWorld.clear();
		m_VertexInCamera.clear();
		m_VertexInProjection.clear();
		m_VertexInView.clear();

		m_EnableRenderStateDepthTest = false;

		m_EnableRenderStateAlphaBlend = false;
		m_ForegroundAlphaBlendValue = foreground_alpha_blend_value;
		m_BackgroundAlphaBlendValue = 1.0f - m_ForegroundAlphaBlendValue;

		m_EnableRenderStateFaceCulling = false;
		m_FaceCullingBack = true;

		m_EnableRenderStateIlluminationCompute = false;
		m_ColorLightAmbient.Set(0.0f, 0.0f, 0.0f);
		m_LightWorld.clear();
		m_Material.emissive.Set(0.0f, 0.0f, 0.0f);
		m_Material.ambient.Set(0.0f, 0.0f, 0.0f);
		m_Material.diffuse.Set(0.0f, 0.0f, 0.0f);
		m_Material.specular.Set(0.0f, 0.0f, 0.0f);
		m_Material.power = 0.0f;

		m_EnableRenderStateTextureSample = false;
		m_DefaultTexture.w = 1;
		m_DefaultTexture.h = 1;
		m_DefaultTexture.c[0] = default_texture_color;
		m_pTexture = &m_DefaultTexture;

		m_SegmentByNearPlaneClip.clear();
		m_pSegmentAfterNearPlaneClip = NULL;

		m_NormalInWorld.clear();
		
		m_ColorAfterIlluminationCompute.clear();

		m_TextureCopy.clear();

		m_TriangleAfterNearPlaneClip.clear();
		m_pTriangleAfterNearPlaneClip = NULL;
		
		m_TriangleAfterFaceCulling.clear();
	}

	int Render::GetBufferSize(
		int* buffer_width,
		int* buffer_height)
	{
		if (NULL != buffer_width)
			*buffer_width = m_BufferWidth;

		if (NULL != buffer_height)
			*buffer_height = m_BufferHeight;

		return m_BufferSize;
	}

	const int* Render::GetVideoBuffer()
	{
		return m_pVideoBuffer;
	}

	void Render::End()
	{
		if (NULL != m_pDepthBuffer)
		{
			free(m_pDepthBuffer);
			m_pDepthBuffer = NULL;
		}
			
		if (NULL != m_pVideoBuffer)
		{
			free(m_pVideoBuffer);
			m_pVideoBuffer = NULL;
		}
	}

	void Render::Draw2DSegment(const SEGMENT* seg, int color)
	{
		//得到缓冲矩形
		RECTANGLE buffer_rectangle =
			{ 0, 0, m_BufferWidth, m_BufferHeight };

		//创建结果
		SEGMENT r_seg = *seg;

		//线段裁剪
		if (!SegmentClip(&buffer_rectangle, &r_seg))
			return;

		//得到起始显示缓冲地址
		int* current_video_buffer
			= m_pVideoBuffer + r_seg.y1 * m_BufferWidth + r_seg.x1;

		//得到x、y方向的差值
		int delta_x = r_seg.x2 - r_seg.x1;
		int delta_y = r_seg.y2 - r_seg.y1;

		//得到x、y方向递增1个像素对应的字节递增量
		int add_x, add_y;
		if (delta_x < 0)
		{
			delta_x = -delta_x;
			add_x = -1;
		}
		else
			add_x = 1;
		if (delta_y < 0)
		{
			delta_y = -delta_y;
			add_y = -m_BufferWidth;
		}
		else
			add_y = m_BufferWidth;

		//得到x、y方向的差值的2倍
		int delta_2x = delta_x << 1;
		int delta_2y = delta_y << 1;

		//分情况进行绘制
		if (delta_x > delta_y)
		{
			//得到最初的因子
			int p = delta_2y - delta_x;

			//循环绘制
			for (int i = delta_x; i >= 0; --i)
			{
				*current_video_buffer = color;

				//根据p值进行处理
				if (p >= 0)
				{
					current_video_buffer += add_y;
					p -= delta_2x;
				}

				current_video_buffer += add_x;
				p += delta_2y;
			}
		}
		else
		{
			//得到最初的因子
			int p = delta_2x - delta_y;

			//循环绘制
			for (int i = delta_y; i >= 0; --i)
			{
				*current_video_buffer = color;

				//根据p值进行处理
				if (p >= 0)
				{
					current_video_buffer += add_x;
					p -= delta_2y;
				}

				current_video_buffer += add_y;
				p += delta_2x;
			}
		}
	}

	void Render::Draw2DRectangle(const RECTANGLE* rect, int color)
	{
		//得到缓冲矩形
		RECTANGLE buffer_rectangle =
		{ 0, 0, m_BufferWidth, m_BufferHeight };

		//创建结果
		RECTANGLE r_rect = {};

		//相交测试
		if (!RectangleIntersect(rect, &buffer_rectangle, &r_rect))
			return;

		for (int y = r_rect.y1; y < r_rect.y2; ++y)
		{
			for (int x = r_rect.x1; x < r_rect.x2; ++x)
			{
				m_pVideoBuffer[x + y * m_BufferWidth] = color;
			}
		}
	}

	void Render::Draw2DTexture(
		const TEXTURE* texture,
		int sx, int sy, int sw, int sh,
		int dx, int dy,
		int tc)
	{
		//得到纹理矩形
		RECTANGLE t_rect =
		{ 0, 0, texture->w, texture->h };

		//得到源矩形
		RECTANGLE s_rect =
		{ sx, sy, sx + sw, sy + sh };

		//创建结果
		RECTANGLE r1_rect = {};

		//相交测试
		if (!RectangleIntersect(&t_rect, &s_rect, &r1_rect))
			return;

		//创建目标矩形
		RECTANGLE d_rect =
		{ dx, dy, dx + sw, dy + sh };

		//更新目标矩形
		d_rect.x1 += r1_rect.x1 - s_rect.x1;
		d_rect.y1 += r1_rect.y1 - s_rect.y1;
		d_rect.x2 += r1_rect.x2 - s_rect.x2;
		d_rect.y2 += r1_rect.y2 - s_rect.y2;

		//得到缓冲矩形
		RECTANGLE b_rect =
		{ 0, 0, m_BufferWidth, m_BufferHeight };

		//创建结果
		RECTANGLE r2_rect;

		//相交测试
		if (!RectangleIntersect(&d_rect, &b_rect, &r2_rect))
			return;

		//更新源矩形
		r1_rect.x1 += r2_rect.x1 - d_rect.x1;
		r1_rect.y1 += r2_rect.y1 - d_rect.y1;
		r1_rect.x2 += r2_rect.x2 - d_rect.x2;
		r1_rect.y2 += r2_rect.y2 - d_rect.y2;

		//不做过滤
		if (0x00000000 == (tc & 0xff000000))
		{
			for (int sy = r1_rect.y1, dy = r2_rect.y1; sy < r1_rect.y2; ++sy, ++dy)
			{
				for (int sx = r1_rect.x1, dx = r2_rect.x1; sx < r1_rect.x2; ++sx, ++dx)
				{
					m_pVideoBuffer[dx + dy * m_BufferWidth] = texture->c[sx + sy * texture->w];
				}
			}
		}
		//要做过滤
		else
		{
			tc |= 0xff000000;
			for (int sy = r1_rect.y1, dy = r2_rect.y1; sy < r1_rect.y2; ++sy, ++dy)
			{
				for (int sx = r1_rect.x1, dx = r2_rect.x1; sx < r1_rect.x2; ++sx, ++dx)
				{
					int color = texture->c[sx + sy * texture->w];
					if (tc != color)
						m_pVideoBuffer[dx + dy * m_BufferWidth] = color;
				}
			}
		}
	}

	void Render::Draw2DAsciiString(
		const ASCII_FONT* ascii_font,
		int x_max_count, int y_max_count,
		int x, int y,
		const char* str)
	{
		//得到字体长度
		int len = (int)strlen(str);

		//得到过滤颜色
		int tc = (~ascii_font->c) | 0xff000000;

		//绘制字体
		int i = 0;
		for (int cy = 0; cy < y_max_count; ++cy)
		{
			for (int cx = 0; cx < x_max_count; ++cx)
			{
				//绘制
				Draw2DTexture(
					ascii_font->t,
					str[i++] * ascii_font->w,
					0,
					ascii_font->w,
					ascii_font->h,
					x + ascii_font->w * cx,
					y + ascii_font->h * cy,
					tc);

				if (i == len)
					return;
			}
		}
	}

	bool Render::SetCoordinateCameraPlane(float near_plane, float far_plane)
	{
		if (_FLT_LESS_FLT(near_plane, 1.0f) ||
			_FLT_LESS_EQUAL_FLT(far_plane, near_plane))
			return false;

		m_NearPlaneZInCamera = near_plane;
		m_FarPlaneZInCamera = far_plane;

		return true;
	}

	bool Render::SetTransform(
		int transform_type,
		const matrix4* mat4)
	{
		switch (transform_type)
		{
		case _COORDINATE_WORLD:
			{
				m_TransformWorld = *mat4;
				break;
			}
		case _COORDINATE_CAMERA:
			{
				m_TransformCamera = *mat4;
				break;
			}
		case _COORDINATE_PROJECTION:
			{
				m_TransformProjection = *mat4;
				break;
			}
		case _COORDINATE_VIEW:
		{
			//视口变换矩阵记录视口矩形
			m_RectangleView.x1 = (int)(mat4->e[_M4_41] - mat4->e[_M4_11]);
			m_RectangleView.y1 = (int)(mat4->e[_M4_42] + mat4->e[_M4_22]);
			m_RectangleView.x2 = m_RectangleView.x1 + (int)(mat4->e[_M4_11] * 2.0f);
			m_RectangleView.y2 = m_RectangleView.y1 + (int)(-mat4->e[_M4_22] * 2.0f);

			m_TransformView = *mat4;
			break;
		}
		default:
			return false;
		}

		return true;
	}

	bool Render::EnableRenderState(
		int render_state_type,
		bool enable)
	{
		switch (render_state_type)
		{
		case _RENDER_STATE_DEPTH_TEST:
			{
				m_EnableRenderStateDepthTest = enable;
				break;
			}
		case _RENDER_STATE_ALPHA_BLEND:
			{
				m_EnableRenderStateAlphaBlend = enable;
				break;
			}
		case _RENDER_STATE_FACE_CULLING:
			{
				m_EnableRenderStateFaceCulling = enable;
				break;
			}
		case _RENDER_STATE_ILLUMINATION_COMPUTE:
			{
				m_EnableRenderStateIlluminationCompute = enable;
				break;
			}
		case _RENDER_STATE_TEXTURE_SAMPLE:
			{
				m_EnableRenderStateTextureSample = enable;
				break;
			}
		default:
			return false;
		}

		return true;
	}

	void Render::FillBuffer(
		bool video,
		int color,
		bool depth)
	{
		if (video)
		{
			for (int i = m_BufferSize - 1; i >= 0; --i)
				m_pVideoBuffer[i] = color;
		}
		if (depth)
		{
			float one_div_far = 1.0f / m_FarPlaneZInCamera;
			for (int i = m_BufferSize - 1; i >= 0; --i)
				m_pDepthBuffer[i] = one_div_far;
		}
	}

	void Render::SetRenderStateFaceCullingBack(bool face_culling_back)
	{
		m_FaceCullingBack = face_culling_back;
	}

	void Render::SetRenderStateForegroundAlphaBlendValue(float foreground_alpha_blend_value)
	{
		m_ForegroundAlphaBlendValue = foreground_alpha_blend_value;
		m_BackgroundAlphaBlendValue = 1.0f - m_ForegroundAlphaBlendValue;
	}

	void Render::SetLightAmbientColor(const vector3* light_ambient_color)
	{
		m_ColorLightAmbient = *light_ambient_color;
	}
	bool Render::AddLight(const LIGHT* light, int id, bool enable)
	{
		int light_world_count = (int)m_LightWorld.size();
		for (int i = 0; i < light_world_count; ++i)
		{
			if (m_LightWorld[i].id == id)
				return false;
		}

		LIGHT_WORLD light_world = {*light, id, enable};
		if (_LIGHT_DIRECTION == light_world.light.type)
			light_world.light.directory = light_world.light.directory.Normalize();
		m_LightWorld.push_back(light_world);
		return true;
	}
	bool Render::DeleteLight(int id)
	{
		int light_world_count = (int)m_LightWorld.size();
		for (int i = 0; i < light_world_count; ++i)
		{
			if (m_LightWorld[i].id == id)
			{
				m_LightWorld.erase(m_LightWorld.begin() + i);
				return true;
			}
		}

		return false;
	}
	LIGHT* Render::GetLight(int id)
	{
		int light_world_count = (int)m_LightWorld.size();
		for (int i = 0; i < light_world_count; ++i)
		{
			if (m_LightWorld[i].id == id)
				return &m_LightWorld[i].light;
		}

		return NULL;
	}
	bool Render::EnableLight(int id, bool enable)
	{
		int light_world_count = (int)m_LightWorld.size();
		for (int i = 0; i < light_world_count; ++i)
		{
			if (m_LightWorld[i].id == id)
			{
				m_LightWorld[i].enable = enable;
				return true;
			}
		}

		return false;
	}
	void Render::SetMaterial(const MATERIAL* material)
	{
		m_Material = *material;
	}

	void Render::SetRenderStateDefaultTextureColor(int color)
	{
		m_DefaultTexture.c[0] = color;
	}
	void Render::SetRenderStateTexture(const TEXTURE* texture)
	{
		m_pTexture = (NULL == texture) ? &m_DefaultTexture : texture;
	}

	void Render::Draw3DMeshSegment(
		const MESH_SEGMENT* mesh_segment,
		int color)
	{
		//视锥体裁剪
		if (!CoordinateCameraFrustumTest(mesh_segment->radius))
			return;

		//得到本地坐标系下面的顶点数量
		int vertex_count = (int)mesh_segment->vertex.size();

		//重置世界坐标系顶点变换表数量并进行世界变换
		m_VertexInWorld.resize(vertex_count);
		for (int i = 0; i < vertex_count; ++i)
			Vec3MulMat4(&mesh_segment->vertex[i], &m_TransformWorld, &m_VertexInWorld[i]);

		//重置摄像机坐标系顶点变换表数量并进行摄像机变换
		m_VertexInCamera.resize(vertex_count);
		for (int i = 0; i < vertex_count; ++i)
			Vec3MulMat4(&m_VertexInWorld[i], &m_TransformCamera, &m_VertexInCamera[i]);

		//近截面裁剪
		Draw3DMeshSegmentNearPlaneClip(mesh_segment->radius, &mesh_segment->segment);

		//更新顶点数量，因为m_VertexInCamera有可能增加
		vertex_count = (int)m_VertexInCamera.size();

		//重置投影坐标系顶点变换表数量并进行投影变换，有部分顶点z值小于近截面就不进行变换，这些
		//点在投影坐标系顶点表之中为零值向量，注意投影坐标系下面的z值为摄像机坐标系下面的z值倒数
		m_VertexInProjection.resize(vertex_count);
		for (int i = 0; i < vertex_count; ++i)
		{
			//需要包含等的情况，因为1点等1点大的情况是保留到更新线段索引表中了
			if (_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, m_VertexInCamera[i].z))
			{
				m_VertexInProjection[i].x = m_VertexInCamera[i].x / m_VertexInCamera[i].z;
				m_VertexInProjection[i].y = m_VertexInCamera[i].y / m_VertexInCamera[i].z;
				m_VertexInProjection[i].z = 1.0f / m_VertexInCamera[i].z;
			}
		}

		//重置视口坐标系顶点变换表数量并进行视口变换
		m_VertexInView.resize(vertex_count);
		for (int i = 0; i < vertex_count; ++i)
			Vec3MulMat4(&m_VertexInProjection[i], &m_TransformView, &m_VertexInView[i]);

		//根据渲染状态得到渲染函数索引(ab dt)并得到渲染函数
		int rasterization_func_index = 
			((m_EnableRenderStateDepthTest ? 1 : 0) << 0) |
			((m_EnableRenderStateAlphaBlend ? 1 : 0) << 1);
		void (Render::* rasterization)(const vector3*, const vector3*, int color) = 
			m_fDraw3DMeshSegmentRasterize[rasterization_func_index];

		//光栅化
		int segment_count = (int)m_pSegmentAfterNearPlaneClip->size();
		for (int i = 0; i < segment_count; i += 2)
		{
			(this->*rasterization)(
				&m_VertexInView[m_pSegmentAfterNearPlaneClip->at(i)],
				&m_VertexInView[m_pSegmentAfterNearPlaneClip->at(i + 1)],
				color);
		}
	}

	//绘制三角模型
	void Render::Draw3DMeshTriangle(
		const MESH_TRIANGLE* mesh_triangle,
		const vector3* eye)
	{
		//01：数据合法性检测
		if ((mesh_triangle->vertex.size() != mesh_triangle->normal.size()) ||
			(m_EnableRenderStateIlluminationCompute && m_EnableRenderStateTextureSample) ||
			(!m_EnableRenderStateIlluminationCompute && !m_EnableRenderStateTextureSample))
			return;

		//02：纹理采样检测
		if (m_EnableRenderStateTextureSample)
		{
			//复制纹理
			if (mesh_triangle->vertex.size() == mesh_triangle->texture.size())
				m_TextureCopy = mesh_triangle->texture;
			else
				return;
		}

		//03：视锥体测试，如果失败就不进行任何绘制
		if (!CoordinateCameraFrustumTest(mesh_triangle->radius))
			return;

		//得到顶点数量
		int vertex_count = (int)mesh_triangle->vertex.size();

		//根据渲染状态(ts ic dt)得到填充函数
		int fill_func_index =
			((m_EnableRenderStateIlluminationCompute ? 1 : 0) << 0) |
			((m_EnableRenderStateTextureSample ? 1 : 0) << 1);
		int (Render:: * fill)(int, int, int, float*, float*, float*) =
			m_fDraw3DMeshTriangleFill[fill_func_index];

		//根据渲染状态(ts ic ab dt)得到渲染函数
		int rasterization_func_index =
			((m_EnableRenderStateDepthTest ? 1 : 0) << 0) |
			((m_EnableRenderStateAlphaBlend ? 1 : 0) << 1) |
			((m_EnableRenderStateIlluminationCompute ? 1 : 0) << 2) |
			((m_EnableRenderStateTextureSample ? 1 : 0) << 3);
		void (Render:: * rasterization)(const TRIANGLE_RASTERIZE * triangle_rasterize) =
			m_fDraw3DMeshTriangleRasterize[rasterization_func_index];
		
		//04：重置世界坐标系顶点变换表数量，进行世界变换
		m_VertexInWorld.resize(vertex_count);
		for (int i = 0; i < vertex_count; ++i)
			Vec3MulMat4(&mesh_triangle->vertex[i], &m_TransformWorld, &m_VertexInWorld[i]);

		//05：光照运算
		if (m_EnableRenderStateIlluminationCompute)
			IlluminationCompute(&mesh_triangle->normal, eye);

		//06：重置摄像机坐标系顶点变换表数量，进行摄像机变换
		m_VertexInCamera.resize(vertex_count);
		for (int i = 0; i < vertex_count; ++i)
			Vec3MulMat4(&m_VertexInWorld[i], &m_TransformCamera, &m_VertexInCamera[i]);

		//07：根据渲染状态(ts ic)得到近截面裁剪函数
		int near_plane_clip_func_index =
			((m_EnableRenderStateIlluminationCompute ? 1 : 0) << 0) |
			((m_EnableRenderStateTextureSample ? 1 : 0) << 1);
		void (Render::* near_plane_clip)(float, const std::vector<int>*) =
			m_fDraw3DMeshTriangleNearPlaneClip[near_plane_clip_func_index];

		//08：近截面裁剪
		(this->*near_plane_clip)(mesh_triangle->radius, &mesh_triangle->triangle);

		//09：更新顶点数量，因为m_VertexInCamera有可能增加
		vertex_count = (int)m_VertexInCamera.size();

		//10：重置投影坐标系顶点变换表数量，进行投影变换
		m_VertexInProjection.resize(vertex_count);
		for (int i = 0; i < vertex_count; ++i)
		{
			//需要包含等的情况，因为1点等2点大和2点等1点大的情况是保留到更新线段索引表中了
			if (_FLT_LESS_EQUAL_FLT(m_NearPlaneZInCamera, m_VertexInCamera[i].z))
			{
				m_VertexInProjection[i].x = m_VertexInCamera[i].x / m_VertexInCamera[i].z;
				m_VertexInProjection[i].y = m_VertexInCamera[i].y / m_VertexInCamera[i].z;
				m_VertexInProjection[i].z = m_VertexInCamera[i].z;
			}
			else
				m_VertexInProjection[i].Set(0.0f, 0.0f, 0.0f);
		}

		//11：表面拣选
		if (m_EnableRenderStateFaceCulling)
			FaceCulling();
		else
		{
			//全部三角可见
			int triangle_count = (int)m_pTriangleAfterNearPlaneClip->size() / 3;
			m_TriangleAfterFaceCulling.resize(triangle_count);
			for (int i = 0; i < triangle_count; ++i)
				m_TriangleAfterFaceCulling[i] = i;
		}

		//12:重置视口坐标系顶点变换表数量，进行视口变换
		m_VertexInView.resize(vertex_count);
		for (int i = 0; i < vertex_count; ++i)
		{
			//非零值顶点才进行变换，零值的是已经在近截面裁剪中被舍去的顶点
			if (!m_VertexInProjection[i].IsZero())
				Vec3MulMat4(&m_VertexInProjection[i], &m_TransformView, &m_VertexInView[i]);
		}

		//13：光栅化
		float vertex_data0[8] = {};
		float vertex_data1[8] = {};
		float vertex_data2[8] = {};
		float vertex_data3[8] = {};
		TRIANGLE_RASTERIZE triangle_flatbottom = {NULL, NULL, NULL, NULL};
		TRIANGLE_RASTERIZE triangle_flattop = {NULL, NULL, NULL, NULL};
		int triangle_visible_count = (int)m_TriangleAfterFaceCulling.size();
		for (int i = 0; i < triangle_visible_count; ++i)
		{
			//得到三角形三点
			int j = m_TriangleAfterFaceCulling[i] * 3;

			//根据渲染状态填充数据
			int fill_count = (this->*fill)(
				m_pTriangleAfterNearPlaneClip->at(j),
				m_pTriangleAfterNearPlaneClip->at(j + 1),
				m_pTriangleAfterNearPlaneClip->at(j + 2),
				vertex_data0,
				vertex_data1,
				vertex_data2);

			//三角形平底平顶分割
			int classify_result = TriangleClassify(
				fill_count,
				vertex_data0,
				vertex_data1,
				vertex_data2,
				vertex_data3,
				&triangle_flatbottom,
				&triangle_flattop);

			//平底三角形光栅化
			if (classify_result & 0x01)
				(this->*rasterization)(&triangle_flatbottom);

			//平顶三角形光栅化
			if (classify_result & 0x02)
				(this->*rasterization)(&triangle_flattop);
		}
	}

}