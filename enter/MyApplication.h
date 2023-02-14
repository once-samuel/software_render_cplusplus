#ifndef _MY_APPLICATION_H_
#define _MY_APPLICATION_H_

#include "WindowBasedOnQt5.h"
#include "Render.h"
#include "Texture.h"
#include "AsciiFont.h"
#include "MeshSegment.h"
#include "MeshTriangle.h"

#include <string>

class IRender;

class MyApplication : public window_based_on_qt5::IUserApp
{
	render::Render r;
	// render::TEXTURE* t1;
	// render::TEXTURE* t2;
	// render::TEXTURE* t3;
	// render::TEXTURE* t4;
	render::TEXTURE* t5;
	render::ASCII_FONT* f1;
	//render::ASCII_FONT* f2;
	render::MESH_TRIANGLE* ms1;
	//render::MESH_TRIANGLE* ms2;
	//render::MESH_TRIANGLE* ms3;
	render::MESH_TRIANGLE* ms4;

	render::vector3 eye;
	render::vector3 at;
	render::vector3 up;
	render::vector3 pos;
	int view_x, view_y, view_w, view_h;
	render::LIGHT* dot_light;

public:
	virtual const char* getTitle() override;
	virtual int getPixelWidth() override;
	virtual int getPixelHeight() override;
	virtual int getLoopIntervalMilliseconds() override;
	virtual bool getActiveInMinimized() override;
	virtual void OnInit() override;
	virtual bool OnUpdateLogic() override;
	virtual void OnUpdateRender(window_based_on_qt5::IRender* render) override;
	virtual void OnInput(int type, const int* param) override;
	virtual void OnEnd() override;
};

#endif