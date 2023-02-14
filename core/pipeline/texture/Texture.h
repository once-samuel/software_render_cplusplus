#ifndef _TEXTURE_H_
#define _TEXTURE_H_

namespace render {

	struct TEXTURE
	{
		//纹理宽度
		int w;

		//纹理高度
		int h;

		//纹理颜色数组
		int c[1];
	};

	//加载纹理
	TEXTURE* TextureLoad(const char* file_name);

	//卸载纹理
	void TextureUnload(TEXTURE* texture);

}

#endif