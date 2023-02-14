#include "MyApplication.h"
#include <iostream>

#define _TITLE "3d_render_cpp"
#define _PIXEL_WIDTH 800
#define _PIXEL_HEIGHT 600
#define _LOOP_INTERVAL_MILLISECONDES 30

const char* MyApplication::getTitle()
{
	return _TITLE;
}

int MyApplication::getPixelWidth()
{
	return _PIXEL_WIDTH;
}

int MyApplication::getPixelHeight()
{
	return _PIXEL_HEIGHT;
}

int MyApplication::getLoopIntervalMilliseconds()
{
	return _LOOP_INTERVAL_MILLISECONDES;
}

bool MyApplication::getActiveInMinimized()
{
	return false;
}

void MyApplication::OnInit()
{
	eye.Set(152.5, 25, -70);
	at.Set(0, 0, 0);
	up.Set(0, 1, 0);
	pos.Set(0, 0, 0);

	r.Init(_PIXEL_WIDTH, _PIXEL_HEIGHT, 2.0f, 1000.0f, 0.5f, _COLOR_LIME, &eye, &at, &up);
	// t1 = render::TextureLoad("resource/image/a.bmp");
	// t2 = render::TextureLoad("resource/image/b.bmp");
	// t3 = render::TextureLoad("resource/image/c.bmp");
	// t4 = render::TextureLoad("resource/image/d.bmp");
	t5 = render::TextureLoad("../resource/image/tiger.bmp");
	f1 = render::AsciiFontCreate(8, 16, _COLOR_WHITE);
	//f2 = render::AsciiFontCreate(40, 40, _COLOR_BLUE);

	//ms1 = MeshTriangleLoad("mesh/jzt_t1.txt");
	ms1 = render::MeshTriangleCreateSphere(50, 32, 32);
	ms1->texture.resize(ms1->vertex.size());
	//ms2 = render::MeshTriangleCreateCube(20, 30, 40);
	//ms3 = render::MeshTriangleCreateSphere(50, 24, 24);
	//ms3->texture.resize(ms3->vertex.size());
	render::matrix4 m4;
	m4.Scale(60, 60, 60);
	ms4 = render::MeshTriangleLoad("../resource/mesh/tiger.txt", &m4);
	//ms4 = MeshTriangleCreateCube(20, 30, 40);
	//ms4 = MeshTriangleCreateSphere(50, 64, 64);
	//ms4 = MeshTriangleCreateCone(30, 40, 13);
	//ms4 = MeshTriangleCreateCylinder(20, 15, 60, 13);
	//ms4 = MeshTriangleCreatePipe(15, 20, 30, 40, 30, 6);
	//ms4 = MeshTriangleCreateTorus(45, 90, 32, 32);

	//设置近远截面
	r.SetCoordinateCameraPlane(2.0, 1000.0);

	//设置视口变换矩阵
	//view_x = client_pixel_width / 4;
	//view_y = client_pixel_height / 4;
	//view_w = client_pixel_width / 2;
	//view_h = client_pixel_height / 2;
	view_x = 0;
	view_y = 0;
	view_w = 800;
	view_h = 600;
	render::matrix4 tv;
	render::ComputeTransformView(&tv, view_x, view_y, view_w, view_h);
	r.SetTransform(_COORDINATE_VIEW, &tv);

	//设置渲染状态
	r.EnableRenderState(_RENDER_STATE_DEPTH_TEST, 1);
	r.EnableRenderState(_RENDER_STATE_ALPHA_BLEND, 1);
	r.SetRenderStateForegroundAlphaBlendValue(0.5f);
	r.EnableRenderState(_RENDER_STATE_FACE_CULLING, 1);
	r.SetRenderStateFaceCullingBack(1);
	r.EnableRenderState(_RENDER_STATE_ILLUMINATION_COMPUTE, 0);
	render::MATERIAL m = {
		//自发光颜色
		render::vector3(0, 0, 0),
		//环境光照系数
		render::vector3(0.5, 0.5, 0.5),
		//漫反射光照系数
		render::vector3(0.5, 0.5, 0.5),
		//镜面光光照系数
		render::vector3(0.6, 0.6, 0.6),
		//镜面光光照指数
		5
	};
	r.SetMaterial(&m);
	render::LIGHT light1 = {
		_LIGHT_DOT,
		render::vector3(0, 255, 0),
		render::vector3(0, 0, 0),
		render::vector3(0, 0, 0),
		70,
	};
	render::LIGHT light2 = {
		_LIGHT_DIRECTION,
		render::vector3(0, 0, 255),
		render::vector3(0, -1, 0)
	};
	render::LIGHT light3 = {
		_LIGHT_DIRECTION,
		render::vector3(255, 0, 0),
		render::vector3(0, 1, 0)
	};
	r.AddLight(&light1, 1, true);
	r.AddLight(&light2, 2, true);
	r.AddLight(&light3, 3, true);
	dot_light = r.GetLight(1);
	r.EnableRenderState(_RENDER_STATE_TEXTURE_SAMPLE, 1);
	r.SetRenderStateTexture(t5);
	//r.SetRenderStateTexture(0);
}

bool MyApplication::OnUpdateLogic()
{
	static int xx = 255;
	render::vector3 light_ambient_color(xx, xx, xx);
	r.SetLightAmbientColor(&light_ambient_color);
	xx++;
	if (xx == 256)
		xx = 0;

	int bw = 0;
	int bh = 0;
	r.GetBufferSize(&bw, &bh);

	r.FillBuffer(true, _COLOR_BLACK, true);

	//r.Draw2DTexture(t1, 0, 0, 800, 600, 0, 0);

	//设置摄像机变换矩阵
	render::matrix4 tc;
	ComputeTransformCamera(&tc, &eye, &at, &up);
	r.SetTransform(_COORDINATE_CAMERA, &tc);

	r.EnableRenderState(_RENDER_STATE_ILLUMINATION_COMPUTE, 1);
	r.EnableRenderState(_RENDER_STATE_TEXTURE_SAMPLE, 0);

	//1
	render::vector3 pos0;
	render::matrix4 tw1;
	tw1.Translate(pos0);
	r.SetTransform(_COORDINATE_WORLD, &tw1);
	r.Draw3DMeshTriangle(ms1, &eye);

	//2
	//render::matrix4 tw2;
	//tw2.Translate(0, 0, 0);
	//r.SetTransform(_COORDINATE_WORLD, &tw2);
	//r.Draw3DMeshTriangle(ms2, &eye);

	//3
	//tw2.Translate(pos);
	//r.SetTransform(_COORDINATE_WORLD, &tw2);
	//r.Draw3DMeshTriangle(ms3, &eye);

	//4
	render::matrix4 tw2;
	r.EnableRenderState(_RENDER_STATE_ILLUMINATION_COMPUTE, 0);
	r.EnableRenderState(_RENDER_STATE_TEXTURE_SAMPLE, 1);
	static float a = 0.0f;
	tw2.RotateY(a += 0.01f);
	render::matrix4 tw3;
	tw3.Translate(pos);
	render::matrix4 tw4;
	Mat4MulMat4(&tw2, &tw3, &tw4);
	r.SetTransform(_COORDINATE_WORLD, &tw4);
	r.Draw3DMeshTriangle(ms4, &eye);

	//infomation
	char buf[1024];

	r.Draw2DAsciiString(f1, 256, 1, 0, 0, "Up,Down,Left,Right,PageUp,PageDown -> move tiger");
	sprintf(buf, "tiger_pos : (x=%.2f,y=%.2f,z=%.2f)", pos.x, pos.y, pos.z);
	r.Draw2DAsciiString(f1, 256, 1, 0, 32, buf);

	r.Draw2DAsciiString(f1, 256, 1, 0, 64, "W,S,A,D -> move eye");
	sprintf(buf, "eye_pos : (x=%.2f,y=%.2f,z=%.2f)", eye.x, eye.y, eye.z);
	r.Draw2DAsciiString(f1, 256, 1, 0, 96, buf);

	r.Draw2DAsciiString(f1, 256, 1, 0, 128, "T,G,F,H -> move dot light");
	sprintf(buf, "dot_light_pos : (x=%.2f,y=%.2f,z=%.2f)", dot_light->position.x, dot_light->position.y, dot_light->position.z);
	r.Draw2DAsciiString(f1, 256, 1, 0, 160, buf);
	
	return true;
}

void MyApplication::OnUpdateRender(window_based_on_qt5::IRender* render)
{
	//返回绘制结果
	const int* video_buffer = r.GetVideoBuffer();

	render->DrawARGB(0, 0, video_buffer, _PIXEL_WIDTH, _PIXEL_HEIGHT);
}

void MyApplication::OnInput(int type, const int* param)
{
	if (_INPUT_KEY_PRESS == type)
	{
		//上下左右
		if (16777235 == param[0])
			pos.z += 2.5;
		if (16777237 == param[0])
			pos.z -= 2.5;
		if (16777234 == param[0])
			pos.x -= 2.5;
		if (16777236 == param[0])
			pos.x += 2.5;
		//页上、页下
		if (16777238 == param[0])
			pos.y += 2.5;
		if (16777239 == param[0])
			pos.y -= 2.5;

		if (('W') == param[0])
			eye.y += 2.5;
		if (('S') == param[0])
			eye.y -= 2.5;
		if (('A') == param[0])
			eye.x -= 2.5;
		if (('D') == param[0])
			eye.x += 2.5;

		if (('T') == param[0])
			dot_light->position.y += 2.5;
		if (('G') == param[0])
			dot_light->position.y -= 2.5;
		if (('F') == param[0])
			dot_light->position.x -= 2.5;
		if (('H') == param[0])
			dot_light->position.x += 2.5;
	}
}

void MyApplication::OnEnd()
{
	if (ms4)
		MeshTriangleUnload(ms4);
	// if (ms3)
	// 	MeshTriangleUnload(ms3);
	// if (ms2)
	// 	MeshTriangleUnload(ms2);
	if (ms1)
		MeshTriangleUnload(ms1);
	//AsciiFontRelease(f2);
	AsciiFontRelease(f1);
	TextureUnload(t5);
	// TextureUnload(t4);
	// TextureUnload(t3);
	// TextureUnload(t2);
	// TextureUnload(t1);
	r.End();
}