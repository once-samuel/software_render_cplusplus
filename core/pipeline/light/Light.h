#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "vector3.h"

namespace render {

//定向光
#define _LIGHT_DIRECTION 0

//点光源
#define _LIGHT_DOT 1

	struct LIGHT
	{
		//类型
		int type;

		//颜色：定向光、点光源，0~255
		vector3 color;

		//方向：定向光
		vector3 directory;

		//位置：点光源
		vector3 position;

		//范围：点光源
		float radius;
	};
}

#endif
