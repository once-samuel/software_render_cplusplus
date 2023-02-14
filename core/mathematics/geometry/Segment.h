#ifndef _SEGMENT_H_
#define _SEGMENT_H_

#include "Rectangle.h"

namespace render {

	struct SEGMENT
	{
		//起始点x坐标
		int x1;

		//起始点y坐标
		int y1;

		//终止点x坐标
		int x2;

		//终止点y坐标
		int y2;
	};

	//线段裁剪
	bool SegmentClip(const RECTANGLE* rect, SEGMENT* seg);

}

#endif