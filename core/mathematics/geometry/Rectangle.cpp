#include "Rectangle.h"
#include "CommonMacro.h"

namespace render {

	bool RectangleIntersect(
		const RECTANGLE* rect1,
		const RECTANGLE* rect2,
		RECTANGLE* rect)
	{
		if (rect1->x1 > rect2->x2 ||
			rect2->x1 > rect1->x2 ||
			rect1->y1 > rect2->y2 ||
			rect2->y1 > rect1->y2)
			return false;
		else
		{
			if (NULL != rect)
			{
				RECTANGLE t_rect = *rect1;
				if (t_rect.x1 < rect2->x1)
					t_rect.x1 = rect2->x1;
				if (t_rect.x2 > rect2->x2)
					t_rect.x2 = rect2->x2;
				if (t_rect.y1 < rect2->y1)
					t_rect.y1 = rect2->y1;
				if (t_rect.y2 > rect2->y2)
					t_rect.y2 = rect2->y2;
				*rect = t_rect;
			}
			
			return true;
		}
	}

}