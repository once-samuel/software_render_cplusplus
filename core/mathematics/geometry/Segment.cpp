#include "Segment.h"

namespace render {

	bool SegmentClip(const RECTANGLE* rect, SEGMENT* seg)
	{
#define _AREA_C 0x00 //00000000
#define _AREA_N 0x01 //00000001
#define _AREA_S 0x02 //00000010
#define _AREA_W 0x04 //00000100
#define _AREA_E 0x08 //00001000
#define _AREA_EN (_AREA_E | _AREA_N) //00001001
#define _AREA_ES (_AREA_E | _AREA_S) //00001010
#define _AREA_WS (_AREA_W | _AREA_S) //00000110
#define _AREA_WN (_AREA_W | _AREA_N) //00000101

		//得到点1的方位信息
		char a1 = _AREA_C;
		if (seg->x1 < rect->x1)
			a1 |= _AREA_W;
		else if (seg->x1 >= rect->x2)
			a1 |= _AREA_E;
		if (seg->y1 < rect->y1)
			a1 |= _AREA_N;
		else if (seg->y1 >= rect->y2)
			a1 |= _AREA_S;

		//得到点2的方位信息
		char a2 = _AREA_C;
		if (seg->x2 < rect->x1)
			a2 |= _AREA_W;
		else if (seg->x2 >= rect->x2)
			a2 |= _AREA_E;
		if (seg->y2 < rect->y1)
			a2 |= _AREA_N;
		else if (seg->y2 >= rect->y2)
			a2 |= _AREA_S;

		//两个点都在中，则要进行绘制不裁剪
		if (_AREA_C == a1 && _AREA_C == a2)
			return true;

		//两个点含有相同区域，则不进行绘制
		if (0 != (a1 & a2))
			return false;

		//得到原始数据
		int _x1 = seg->x1;
		int _y1 = seg->y1;
		int _x2 = seg->x2;
		int _y2 = seg->y2;

		//点1裁剪
		switch (a1)
		{
		case _AREA_C:
		{
			break;
		}
		case _AREA_N:
		{
			_y1 = rect->y1;
			_x1 = (int)(seg->x1 + (_y1 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));

			break;
		}
		case _AREA_S:
		{
			_y1 = rect->y2 - 1;
			_x1 = (int)(seg->x1 + (_y1 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));

			break;
		}
		case _AREA_W:
		{
			_x1 = rect->x1;
			_y1 = (int)(seg->y1 + (_x1 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			break;
		}
		case _AREA_E:
		{
			_x1 = rect->x2 - 1;
			_y1 = (int)(seg->y1 + (_x1 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			break;
		}
		case _AREA_EN:
		{
			//先按东裁剪
			_x1 = rect->x2 - 1;
			_y1 = (int)(seg->y1 + (_x1 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			//东裁剪完新点不在中心，再按北裁剪
			if (_y1 < rect->y1 || _y1 >= rect->y2)
			{
				_y1 = rect->y1;
				_x1 = (int)(seg->x1 + (_y1 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));
			}

			break;
		}
		case _AREA_ES:
		{
			//先按东裁剪
			_x1 = rect->x2 - 1;
			_y1 = (int)(seg->y1 + (_x1 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			//东裁剪完新点不在中心，再按南裁剪
			if (_y1 < rect->y1 || _y1 >= rect->y2)
			{
				_y1 = rect->y2 - 1;
				_x1 = (int)(seg->x1 + (_y1 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));
			}

			break;
		}
		case _AREA_WS:
		{
			//先按西裁剪
			_x1 = rect->x1;
			_y1 = (int)(seg->y1 + (_x1 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			//西裁剪完新点不在中心，再按南裁剪
			if (_y1 < rect->y1 || _y1 >= rect->y2)
			{
				_y1 = rect->y2 - 1;
				_x1 = (int)(seg->x1 + (_y1 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));
			}

			break;
		}
		case _AREA_WN:
		{
			//先按西裁剪
			_x1 = rect->x1;
			_y1 = (int)(seg->y1 + (_x1 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			//西裁剪完新点不在中心，再按北裁剪
			if (_y1 < rect->y1 || _y1 >= rect->y2)
			{
				_y1 = rect->y1;
				_x1 = (int)(seg->x1 + (_y1 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));
			}

			break;
		}
		}
		if (_x1 < rect->x1 || _x1 >= rect->x2 || _y1 < rect->y1 || _y1 >= rect->y2)
			return false;

		//点2裁剪
		switch (a2)
		{
		case _AREA_C:
		{
			break;
		}
		case _AREA_N:
		{
			_y2 = rect->y1;
			_x2 = (int)(seg->x1 + (_y2 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));

			break;
		}
		case _AREA_S:
		{
			_y2 = rect->y2 - 1;
			_x2 = (int)(seg->x1 + (_y2 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));

			break;
		}
		case _AREA_W:
		{
			_x2 = rect->x1;
			_y2 = (int)(seg->y1 + (_x2 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			break;
		}
		case _AREA_E:
		{
			_x2 = rect->x2 - 1;
			_y2 = (int)(seg->y1 + (_x2 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			break;
		}
		case _AREA_EN:
		{
			//先按东裁剪
			_x2 = rect->x2 - 1;
			_y2 = (int)(seg->y1 + (_x2 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			//东裁剪完新点不在中心，再按北裁剪
			if (_y2 < rect->y1 || _y2 >= rect->y2)
			{
				_y2 = rect->y1;
				_x2 = (int)(seg->x1 + (_y2 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));
			}

			break;
		}
		case _AREA_ES:
		{
			//先按东裁剪
			_x2 = rect->x2 - 1;
			_y2 = (int)(seg->y1 + (_x2 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			//东裁剪完新点不在中心，再按南裁剪
			if (_y2 < rect->y1 || _y2 >= rect->y2)
			{
				_y2 = rect->y2 - 1;
				_x2 = (int)(seg->x1 + (_y2 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));
			}

			break;
		}
		case _AREA_WS:
		{
			//先按西裁剪
			_x2 = rect->x1;
			_y2 = (int)(seg->y1 + (_x2 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			//西裁剪完新点不在中心，再按南裁剪
			if (_y2 < rect->y1 || _y2 >= rect->y2)
			{
				_y2 = rect->y2 - 1;
				_x2 = (int)(seg->x1 + (_y2 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));
			}

			break;
		}
		case _AREA_WN:
		{
			//先按西裁剪
			_x2 = rect->x1;
			_y2 = (int)(seg->y1 + (_x2 - seg->x1) * ((float)(seg->y2 - seg->y1)) / (seg->x2 - seg->x1));

			//西裁剪完新点不在中心，再按北裁剪
			if (_y2 < rect->y1 || _y2 >= rect->y2)
			{
				_y2 = rect->y1;
				_x2 = (int)(seg->x1 + (_y2 - seg->y1) * ((float)(seg->x2 - seg->x1)) / (seg->y2 - seg->y1));
			}

			break;
		}
		}
		if (_x2 < rect->x1 || _x2 >= rect->x2 || _y2 < rect->y1 || _y2 >= rect->y2)
			return false;

		//裁剪成功
		seg->x1 = _x1;
		seg->y1 = _y1;
		seg->x2 = _x2;
		seg->y2 = _y2;

		return true;

#undef _AREA_WN
#undef _AREA_WS
#undef _AREA_ES
#undef _AREA_EN
#undef _AREA_E
#undef _AREA_W
#undef _AREA_S
#undef _AREA_N
#undef _AREA_C
	}

}