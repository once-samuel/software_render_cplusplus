#ifndef _COLOR_H_
#define _COLOR_H_

namespace render {

//空地址
#ifndef NULL
#define NULL 0
#endif

//浮点数判断
#define _FLT_DECIMAL_DIGITS 1000000.0f
#define _FLT_EQUAL_ZERO(flt) \
	(0==(long long)((flt)*_FLT_DECIMAL_DIGITS))
#define _FLT_LESS_FLT(flt1,flt2) \
	((long long)((flt1)*_FLT_DECIMAL_DIGITS)<(long long)((flt2)*_FLT_DECIMAL_DIGITS))
#define _FLT_EQUAL_FLT(flt1,flt2) \
	((long long)((flt1)*_FLT_DECIMAL_DIGITS)==(long long)((flt2)*_FLT_DECIMAL_DIGITS))
#define _FLT_LESS_EQUAL_FLT(flt1,flt2) \
	(_FLT_LESS_FLT(flt1,flt2) || _FLT_EQUAL_FLT(flt1,flt2))

//颜色
#define _COLOR_SET(r,g,b) \
	(0xff000000|((r)<<16)|((g)<<8)|(b))
#define _COLOR_GET_R(c) \
	(((c)&0x00ff0000)>>16)
#define _COLOR_GET_G(c) \
	(((c)&0x0000ff00)>>8)
#define _COLOR_GET_B(c) \
	((c)&0x000000ff)
#define _COLOR_BLACK \
	0xff000000
#define _COLOR_RED \
	0xffff0000
#define _COLOR_LIME \
	0xff00ff00
#define _COLOR_BLUE \
	0xff0000ff
#define _COLOR_YELLOW \
	0xffffff00
#define _COLOR_MAGENTA \
	0xffff00ff
#define _COLOR_CYAN \
	0xff00ffff
#define _COLOR_WHITE \
	0xffffffff

//圆周率
#define _PI_D2 \
	1.5707963f
#define _PI \
	3.1415926f
#define _PI_M2 \
	6.2831852f

}

#endif