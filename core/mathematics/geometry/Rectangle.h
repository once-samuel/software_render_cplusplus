#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

namespace render {

	struct RECTANGLE
	{
		//矩形左上角x坐标
		int x1;

		//矩形左上角y坐标
		int y1;

		//矩形右下角x坐标+1
		int x2;

		//矩形右下角y坐标+1
		int y2;
	};

	//矩形相交
	bool RectangleIntersect(
		const RECTANGLE* rect1,
		const RECTANGLE* rect2,
		RECTANGLE* rect);
}

#endif