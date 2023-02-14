#ifndef _WINDOW_BASED_ON_QT5_H_
#define _WINDOW_BASED_ON_QT5_H_

namespace window_based_on_qt5
{

#define _RENDER_ARGB(a,r,g,b) ((a)<<24|(r)<<16|(g)<<8|(b))
#define _RENDER_RGB(r,g,b) _RENDER_ARGB(0xff,(r),(g),(b))
#define _RENDER_NIL 0
#define _INPUT_KEY_PRESS 0
#define _INPUT_KEY_RELEASE 1
#define _INPUT_MOUSE_PRESS 2
#define _INPUT_MOUSE_RELEASE 3
#define _INPUT_MOUSE_DOUBLE_CLICK 4
#define _INPUT_MOUSE_MOVE 5

	class IRender
	{
	public:
		virtual void SetPenColor(int rgb) = 0;
		virtual void SetBrushColor(int rgb) = 0;
		virtual void DrawString(int x, int y, const char* str) = 0;
		virtual void DrawLine(int x1, int y1, int x2, int y2) = 0;
		virtual void DrawRectangle(int x1, int y1, int x2, int y2) = 0;
		virtual void DrawEllipse(int x1, int y1, int x2, int y2) = 0;
		virtual void DrawARGB(int dx, int dy, const int* argb, int w, int h) = 0;
		virtual void DrawARGB(int dx, int dy, int dw, int dh, const int* argb, int w, int h) = 0;
	};

	class IUserApp
	{
	public:
		virtual const char* getTitle() = 0;
		virtual int getPixelWidth() = 0;
		virtual int getPixelHeight() = 0;
		virtual int getLoopIntervalMilliseconds() = 0;
		virtual bool getActiveInMinimized() = 0;
		virtual void OnInit() = 0;
		virtual bool OnUpdateLogic() = 0;
		virtual void OnUpdateRender(IRender* render) = 0;
		virtual void OnInput(int type, const int* param) = 0;
		virtual void OnEnd() = 0;
	};

#ifdef WIN32
#ifdef _WINDOW_BASED_ON_QT5_CPP_
#define _DLL_PUBLIC __declspec(dllexport)
#else
#define _DLL_PUBLIC __declspec(dllimport)
#endif
#else
#define _DLL_PUBLIC
#endif

	int _DLL_PUBLIC Launch(int argc, char* argv[], IUserApp* user_app);

}

#endif