#include "Texture.h"
#include "CommonMacro.h"
#include <cstdio>
#include <cstdlib>

namespace render {

	struct BITMAP_FILE_HEADER
	{
		unsigned short type;
		unsigned char size[sizeof(unsigned int)];
		unsigned short reserved1;
		unsigned short reserved2;
		unsigned char offbits[sizeof(unsigned int)];
	};

	struct BITMAP_INFO_HEADER
	{
		unsigned int size;
		int width;
		int height;
		unsigned short planes;
		unsigned short bit_count;
		unsigned int compression;
		unsigned int size_image;
		int x_pels_per_meter;
		int y_pels_per_meter;
		unsigned int clr_used;
		unsigned int clr_important;
	};

	TEXTURE* TextureLoad(const char* file_name)
	{
		//打开文件
		FILE* file = fopen(file_name, "rb");
		if (NULL == file)
			return NULL;

		//读取文件
		fseek(file, 0, SEEK_END);
		int file_size = ftell(file);
		rewind(file);
		unsigned char* file_data = (unsigned char*)malloc(sizeof(unsigned char) * file_size);
		fread(file_data, sizeof(unsigned char), file_size, file);

		//关闭文件
		fclose(file);

		//得到位图文件头
		BITMAP_FILE_HEADER* bfh = (BITMAP_FILE_HEADER*)file_data;

		//得到位图信息头
		BITMAP_INFO_HEADER* bih = (BITMAP_INFO_HEADER*)(bfh + 1);

		//位图格式检查
		if (*((unsigned short*)"BM") != bfh->type || (8 != bih->bit_count && 24 != bih->bit_count))
		{
			free(file_data);
			return NULL;
		}

		//创建纹理对象
		TEXTURE* t = (TEXTURE*)malloc(
			sizeof(int) + sizeof(int) + sizeof(int) * bih->width * bih->height);

		//24位位图
		if (24 == bih->bit_count)
		{
			//得到颜色数据地址
			unsigned char* start_color = (unsigned char*)(bih + 1);

			//得到每行颜色数据总字节数
			int bytes = bih->width * 3;
			if (0 != bytes % 4)
				bytes += 4 - bytes % 4;

			//填充纹理
			t->w = bih->width;
			t->h = bih->height;
			for (int y = 0; y < bih->height; ++y)
			{
				for (int x = 0; x < bih->width; ++x)
				{
					//得到位图当前颜色
					unsigned char* current_color =
						start_color + (bih->height - 1 - y) * bytes + x * 3;

					//设置到纹理中
					t->c[x + y * t->w] =
						_COLOR_SET(current_color[2], current_color[1], current_color[0]);
				}
			}
		}
		//256色位图
		else
		{
			//得到色彩表
			int* color_table = (int*)(bih + 1);

			//得到颜色数据地址
			unsigned char* start_color_index = (unsigned char*)(color_table + 256);

			//得到每行颜色总字节数
			int bytes = bih->width;
			if (0 != bytes % 4)
				bytes += 4 - bytes % 4;

			//填充纹理对象
			t->w = bih->width;
			t->h = bih->height;
			for (int y = 0; y < bih->height; ++y)
			{
				for (int x = 0; x < bih->width; ++x)
				{
					//得到位图当前颜色下标
					unsigned char* current_color_index =
						start_color_index + (bih->height - 1 - y) * bytes + x;

					//得到当前颜色
					unsigned char* current_color =
						(unsigned char*)(color_table + *current_color_index);

					//设置到纹理对象中
					t->c[x + y * t->w] =
						_COLOR_SET(current_color[2], current_color[1], current_color[0]);
				}
			}
		}

		//释放位图
		free(file_data);

		return t;
	}

	void TextureUnload(TEXTURE* texture)
	{
		if (NULL != texture)
			free(texture);
	}

}