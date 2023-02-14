#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "vector3.h"

namespace render {

	struct MATERIAL
	{
		//自发光颜色：0~255
		vector3 emissive;

		//环境光照系数：0~1
		vector3 ambient;

		//漫反射光照系数：0~1
		vector3 diffuse;

		//镜面光光照系数：0~1
		vector3 specular;

		//镜面光光照指数：>0
		float power;
	};
}

#endif
