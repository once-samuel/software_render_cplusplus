#ifndef _RENDER_H_
#define _RENDER_H_

#include "CommonMacro.h"
#include "Segment.h"
#include "Rectangle.h"
#include "Texture.h"
#include "AsciiFont.h"
#include "vector2.h"
#include "matrix3.h"
#include "vector3.h"
#include "matrix4.h"
#include "Light.h"
#include "Material.h"
#include "MeshSegment.h"
#include "MeshTriangle.h"

#include <vector>

namespace render {

//坐标系：世界索引
#define _COORDINATE_WORLD 0
//坐标系：摄像机索引
#define _COORDINATE_CAMERA 1
//坐标系：投影索引（未使用）
#define _COORDINATE_PROJECTION 2
//坐标系：视口索引
#define _COORDINATE_VIEW 3

//渲染状态：深度测试dt索引
#define _RENDER_STATE_DEPTH_TEST 0
//渲染状态：阿尔法混合ab索引
#define _RENDER_STATE_ALPHA_BLEND 1
//渲染状态：表面拣选fc索引
#define _RENDER_STATE_FACE_CULLING 2
//渲染状态：光照运算ic索引
#define _RENDER_STATE_ILLUMINATION_COMPUTE 3
//渲染状态：纹理采样ts索引
#define _RENDER_STATE_TEXTURE_SAMPLE 4

	//计算摄像机变换矩阵
	matrix4* ComputeTransformCamera(
		matrix4* mat4,
		const vector3* eye,
		const vector3* at,
		const vector3* up);

	//计算视口变换矩阵
	matrix4* ComputeTransformView(
		matrix4* mat4,
		int x,
		int y,
		int w,
		int h);

	//计算本地坐标系包围球（球心为本地坐标系原点(0,0,0)）的半径
	float ComputeLocalShpereRadius(
		const std::vector<vector3>* vertex);

	class Render
	{
		//----------通用----------

		//缓冲尺寸
		int m_BufferWidth;
		int m_BufferHeight;
		int m_BufferSize;

		//显示缓冲
		int* m_pVideoBuffer;

		//深度缓冲
		float* m_pDepthBuffer;

		//变换矩阵
		matrix4 m_TransformWorld;
		matrix4 m_TransformCamera;
		matrix4 m_TransformProjection;
		matrix4 m_TransformView;

		//视口矩形
		RECTANGLE m_RectangleView;

		//近远截面
		float m_NearPlaneZInCamera;
		float m_FarPlaneZInCamera;

		//变换顶点表
		std::vector<vector3> m_VertexInWorld;
		std::vector<vector3> m_VertexInCamera;
		std::vector<vector3> m_VertexInProjection;
		std::vector<vector3> m_VertexInView;

		//渲染状态：深度缓冲
		bool m_EnableRenderStateDepthTest;

		//渲染状态：阿尔法混合
		bool m_EnableRenderStateAlphaBlend;
		float m_ForegroundAlphaBlendValue;
		float m_BackgroundAlphaBlendValue;

		//渲染状态：表面拣选
		bool m_EnableRenderStateFaceCulling;
		bool m_FaceCullingBack;

		//渲染状态：光照运算
		bool m_EnableRenderStateIlluminationCompute;
		vector3 m_ColorLightAmbient;
		struct LIGHT_WORLD
		{
			LIGHT light;
			int id;
			bool enable;
		};
		std::vector<LIGHT_WORLD> m_LightWorld;
		MATERIAL m_Material;

		//渲染状态：纹理采样
		bool m_EnableRenderStateTextureSample;
		TEXTURE m_DefaultTexture;
		const TEXTURE* m_pTexture;

		//计算摄像机坐标系下包围球球心
		vector3 ComputerCenterInCamera();

		//视锥体测试
		bool CoordinateCameraFrustumTest(float sphere_radius);
		
		//----------线段网格相关----------

		//被近截面裁剪线段索引表
		std::vector<int> m_SegmentByNearPlaneClip;
		const std::vector<int>* m_pSegmentAfterNearPlaneClip;

		//近截面线裁剪，裁剪完毕m_pSegmentAfterNearPlaneClip
		//指向有效线段索引表，m_VertexInCamera有可能增加
		void Draw3DMeshSegmentNearPlaneClip(
			float sphere_radius,
			const std::vector<int>* segment_origin);

		//线段光栅化
		//1/z和x、y都是线性关系，可以根据x、y的变化量哪个非0就用哪个来计算
		void Draw3DMeshSegmentRasterize_ab0_dt0(const vector3* v0, const vector3* v1, int color);
		void Draw3DMeshSegmentRasterize_ab0_dt1(const vector3* v0, const vector3* v1, int color);
		void Draw3DMeshSegmentRasterize_ab1_dt0(const vector3* v0, const vector3* v1, int color);
		void Draw3DMeshSegmentRasterize_ab1_dt1(const vector3* v0, const vector3* v1, int color);
		void (Render::* m_fDraw3DMeshSegmentRasterize[4])(const vector3*, const vector3*, int);

		//----------三角网格相关----------

		//顶点法线表，从本地坐标系转换到世界坐标系
		std::vector<vector3> m_NormalInWorld;

		//顶点颜色表，光照运算生成
		std::vector<vector3> m_ColorAfterIlluminationCompute;

		//顶点纹理表，从原始网格顶点纹理表复制
		std::vector<vector2> m_TextureCopy;

		//被近截面裁剪三角索引表
		std::vector<int> m_TriangleAfterNearPlaneClip;
		const std::vector<int>* m_pTriangleAfterNearPlaneClip;

		//被表面拣选三角索引表，存储可见三角形索引
		std::vector<int> m_TriangleAfterFaceCulling;

		//投影坐标系下视线向量
		const vector3 m_SightLineInProjection;

		//光源表中是否存在有效光源
		bool IsLightWorldEnable();

		//光照运算，计算结果存储到m_VertexColor
		void IlluminationCompute(
			const std::vector<vector3>* normal,
			const vector3* eye);

		//近截面线裁剪，裁剪完毕m_pTriangleAfterNearPlaneClip
		//指向有效三角索引表，m_VertexInCamera有可能增加
		void Draw3DMeshTriangleNearPlaneClip_ts0_ic0(float sphere_radius, const std::vector<int>* triangle_origin);
		void Draw3DMeshTriangleNearPlaneClip_ts0_ic1(float sphere_radius, const std::vector<int>* triangle_origin);
		void Draw3DMeshTriangleNearPlaneClip_ts1_ic0(float sphere_radius, const std::vector<int>* triangle_origin);
		void Draw3DMeshTriangleNearPlaneClip_ts1_ic1(float sphere_radius, const std::vector<int>* triangle_origin);
		void (Render::* m_fDraw3DMeshTriangleNearPlaneClip[4])(float, const std::vector<int>*);

		//表面拣选，完毕之后可见三角放入m_TriangleVisible
		void FaceCulling();

		//填充，{y, x, 1/z, c.x/z, c.y/z, c.z/z, t.x/z, t.y/z}，其中y、x、1/z必填
		//返回填充数据数量
		int Draw3DMeshTriangleFill_ts0_ic0(
			int index0, int index1, int index2, float* vertex_data0, float* vertex_data1, float* vertex_data2);
		int Draw3DMeshTriangleFill_ts0_ic1(
			int index0, int index1, int index2, float* vertex_data0, float* vertex_data1, float* vertex_data2);
		int Draw3DMeshTriangleFill_ts1_ic0(
			int index0, int index1, int index2, float* vertex_data0, float* vertex_data1, float* vertex_data2);
		int Draw3DMeshTriangleFill_ts1_ic1(
			int index0, int index1, int index2, float* vertex_data0, float* vertex_data1, float* vertex_data2);
		int (Render::* m_fDraw3DMeshTriangleFill[4])(int, int, int, float*, float*, float*);

		//三角光栅
		struct TRIANGLE_RASTERIZE
		{
			const float* left_top;
			const float* right_top;
			const float* left_bottom;
			const float* right_bottom;
		};
		//三角分类
		//返回0：无需进行光栅化
		//返回1：平底三角形被填充
		//返回2：平顶三角形被填充
		//返回3：平底、平顶三角形被填充
		int TriangleClassify(
			int fill_count,
			const float* vertex_data0,
			const float* vertex_data1,
			const float* vertex_data2,
			float* vertex_data3,
			TRIANGLE_RASTERIZE* triangle_flatbottom,
			TRIANGLE_RASTERIZE* triangle_flattop);
		//三角光栅化：光照、纹理必须有且只有一个被激活
		//Draw3DMeshTriangleRasterize_ts0_ic0_ab0_dt0无效
		//Draw3DMeshTriangleRasterize_ts0_ic0_ab0_dt1无效
		//Draw3DMeshTriangleRasterize_ts0_ic0_ab1_dt0无效
		//Draw3DMeshTriangleRasterize_ts0_ic0_ab1_dt1无效
		void Draw3DMeshTriangleRasterize_ts0_ic1_ab0_dt0(const TRIANGLE_RASTERIZE* triangle_rasterize);
		void Draw3DMeshTriangleRasterize_ts0_ic1_ab0_dt1(const TRIANGLE_RASTERIZE* triangle_rasterize);
		void Draw3DMeshTriangleRasterize_ts0_ic1_ab1_dt0(const TRIANGLE_RASTERIZE* triangle_rasterize);
		void Draw3DMeshTriangleRasterize_ts0_ic1_ab1_dt1(const TRIANGLE_RASTERIZE* triangle_rasterize);
		void Draw3DMeshTriangleRasterize_ts1_ic0_ab0_dt0(const TRIANGLE_RASTERIZE* triangle_rasterize);
		void Draw3DMeshTriangleRasterize_ts1_ic0_ab0_dt1(const TRIANGLE_RASTERIZE* triangle_rasterize);
		void Draw3DMeshTriangleRasterize_ts1_ic0_ab1_dt0(const TRIANGLE_RASTERIZE* triangle_rasterize);
		void Draw3DMeshTriangleRasterize_ts1_ic0_ab1_dt1(const TRIANGLE_RASTERIZE* triangle_rasterize);
		//Draw3DMeshTriangleRasterize_ts1_ic1_ab0_dt0无效
		//Draw3DMeshTriangleRasterize_ts1_ic1_ab0_dt1无效
		//Draw3DMeshTriangleRasterize_ts1_ic1_ab1_dt0无效
		//Draw3DMeshTriangleRasterize_ts1_ic1_ab1_dt1无效
		void (Render::* m_fDraw3DMeshTriangleRasterize[16])(const TRIANGLE_RASTERIZE* triangle_rasterize);

		//拷贝构造
		Render(const Render& that);

		//同类赋值
		Render& operator = (const Render& that);

	public:

		//构造
		Render();

		//析构
		~Render();

		//初始
		void Init(
			int buffer_width,
			int buffer_height,
			float near_plane_z_in_camera,
			float far_plane_z_in_camera,
			float foreground_alpha_blend_value,
			int default_texture_color,
			const vector3* eye,
			const vector3* at,
			const vector3* up);

		//得到缓冲尺寸
		int GetBufferSize(
			int* buffer_width = NULL,
			int* buffer_height = NULL);

		//得到渲染结果
		const int* GetVideoBuffer();

		//----------2D绘制相关----------

		//绘制2D线段
		void Draw2DSegment(
			const SEGMENT* seg,
			int color);

		//绘制2D矩形
		void Draw2DRectangle(
			const RECTANGLE* rect,
			int color);

		//绘制2D纹理，tc为过滤颜色，若0x??rrggbb中??为00则不做过滤
		void Draw2DTexture(
			const TEXTURE* texture,
			int sx, int sy, int sw, int sh,
			int dx, int dy,
			int tc = 0x00000000);

		//绘制2D文字，xc为x方向最大字符数量，yc为y方向最大字符数量
		void Draw2DAsciiString(
			const ASCII_FONT* ascii_font,
			int x_max_count, int y_max_count,
			int x, int y,
			const char* str);

		//----------3D绘制相关：通用----------

		//设置近远截面
		bool SetCoordinateCameraPlane(
			float near_plane,
			float far_plane);

		//填充缓冲区
		void FillBuffer(
			bool video,
			int color,
			bool depth);

		//设置顶点变换矩阵
		bool SetTransform(
			int transform_type,
			const matrix4* mat4);

		//激活渲染状态
		bool EnableRenderState(
			int render_state_type,
			bool enable);

		//设置阿尔法混合参数：阿尔法混合前景色混合参数
		void SetRenderStateForegroundAlphaBlendValue(float foreground_alpha_blend_value);

		//设置表面拣选参数：背面拣选标志
		void SetRenderStateFaceCullingBack(bool face_culling_back);

		//添加、删除、获取、激活光源表中的光源，设置当前材质
		void SetLightAmbientColor(const vector3* light_ambient_color);
		bool AddLight(const LIGHT* light, int id, bool enable);
		bool DeleteLight(int id);
		LIGHT* GetLight(int id);
		bool EnableLight(int id, bool enable);
		void SetMaterial(const MATERIAL* material);

		//设置纹理采样参数：默认纹理颜色、当前纹理
		void SetRenderStateDefaultTextureColor(int color);
		void SetRenderStateTexture(const TEXTURE* texture);

		//----------3D绘制相关：线段----------

		//绘制线段模型：深度测试、阿尔法混合
		void Draw3DMeshSegment(
			const MESH_SEGMENT* mesh_segment,
			int color);

		//----------3D绘制相关：三角----------

		//绘制三角模型：深度测试、表面拣选、阿尔法混合、光照运算、纹理采样
		void Draw3DMeshTriangle(
			const MESH_TRIANGLE* mesh_triangle,
			const vector3* eye = NULL);

		//结束
		void End();
	};

}

#endif