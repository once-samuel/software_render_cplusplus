#ifndef _ASCII_FONT_H_
#define _ASCII_FONT_H_

#include "Texture.h"

namespace render {

	struct ASCII_FONT
	{
		//字体宽度
		int w;

		//字体高度
		int h;

		//字体颜色
		int c;

		//字体纹理
		TEXTURE t[1];
	};

	ASCII_FONT* AsciiFontCreate(int w, int h, int c);

	void AsciiFontRelease(ASCII_FONT* ascii_font);
}

#endif