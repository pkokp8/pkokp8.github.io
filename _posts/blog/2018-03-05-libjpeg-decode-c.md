---
layout:     post
title: libjpeg不完全解
description: 要说详解肯定要说实际解码部分，还没这么深入。应该算是初解
category: blog
---

先放一张图。

![avatar](/images/libjpegmaterial/CactiIslaPescado_ZH-CN11317505000_1920x1080_orig.jpg)

jpeg有非常多格式，后面会拿上图作示例。这是一张jfif格式的jpeg图像。



这张图的格式是这样的：

<pre>
SOI				(0xFFD8)
APP0			(0xFFE0)
DQT				(0xFFDB)
DQT				(0xFFDB)
SOF0			(0xFFC0)
DHT				(0xFFC4)
DHT				(0xFFC4)
DHT				(0xFFC4)
DHT				(0xFFC4)
SOS				(0xFFDA)
实际图像的压缩数据
EOI				(0xFFD9)
</pre>

<p>
然后放一段可以直接拿来用的libjpeg的sample代码。这段代码并没有经过仔细的优化，部分显示效果也并不是最佳。但作为解码的初解，应该够了吧。
</p>
<pre>
int jpeg_to_yuv422(unsigned char *jpeg_buffer,int insize, unsigned char **yuv_buffer, int *yuvlen)
{
	int a, i;
	int width, height;
	int r, g, b;
	int c,m,y,k;
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	JSAMPARRAY buffer;
	int row_stride;
	unsigned char *curptr = NULL;
	unsigned char *rgb24_buffer = NULL;

	cinfo.err = jpeg_std_error(&jerr.pub);  
//  	jerr.pub.error_exit = my_error_exit;

	if(setjmp(jerr.setjmp_buffer))
	{
		printf("my err %d\n", insize);
		jpeg_destroy_decompress(&cinfo);

		return 1;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, jpeg_buffer, insize);
	(void) jpeg_read_header(&cinfo, TRUE);

	//cinfo.out_color_space = JCS_YCbCr;
	//cinfo.raw_data_out = TRUE;

	
	(void) jpeg_start_decompress(&cinfo);

	int alignwidth = 0;
	if(cinfo.output_width % 16)
	alignwidth = cinfo.output_width / 16 * 16 + 16;
	else
	alignwidth = cinfo.output_width;

	row_stride = alignwidth * cinfo.output_components;
	width = alignwidth;
	height = cinfo.output_height;
	*yuvlen = width * height * 2;

	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	rgb24_buffer = (unsigned char*)malloc(width * height * 3);
	memset(rgb24_buffer, 0, width * height * 3);
	printf("%d, color space %d\n", __LINE__, cinfo.out_color_space);

	if(cinfo.out_color_space == JCS_RGB)
	{
		curptr = rgb24_buffer;
		for(a = 0; a < height; a++)
		{
			(void) jpeg_read_scanlines(&cinfo, buffer, 1);

			for(i = 0; i < row_stride; i += 3)
			{
				r = (int)buffer[0][i];
				g = (int)buffer[0][i + 1];
				b = (int)buffer[0][i + 2];

				*curptr++ = r;
				*curptr++ = g;
				*curptr++ = b;
			}

		}
	}
	else if(cinfo.out_color_space == JCS_GRAYSCALE)
	{
		curptr = rgb24_buffer;
		for(a = 0; a < height; a++)
		{
			(void) jpeg_read_scanlines(&cinfo, buffer, 1);

			for(i = 0; i < row_stride; i += 1)
			{
				g = (int)buffer[0][i];

				*curptr++ = g;
				*curptr++ = g;
				*curptr++ = g;
			}

		}
	}
	else if(cinfo.out_color_space == JCS_CMYK)
	{
		curptr = rgb24_buffer;
		for(a = 0; a < height; a++)
		{
			(void) jpeg_read_scanlines(&cinfo, buffer, 1);

			for(i = 0; i < row_stride; i += 4)
			{
				c = (int)buffer[0][i];
				m = (int)buffer[0][i + 1];
				y = (int)buffer[0][i + 2];
				k = (int)buffer[0][i + 3];

				
				r = (k*c)/255;
				g = (k*m)/255;
				b = (k*y)/255; 
				 
				*curptr++ = r;
				*curptr++ = g;
				*curptr++ = b;
			}

		}
	}
	else
	{
		printf("jpg_to_yuv:unsupported jpeg color space %d, aborting.\n",  cinfo.out_color_space);
		return -1;
	}
	//(*cinfo.mem->free_pool) ((j_common_ptr) &cinfo, JPOOL_IMAGE);

	/* rbgb24 to yuv */
	{
		*yuv_buffer = (unsigned char*)malloc(width * height * 2);
		RGB2YUV422(rgb24_buffer, *yuv_buffer, height * width);
	}
	(void) jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);

	if(jerr.pub.num_warnings)
	{
		printf("jpg_to_yuv: jpeg lib warnings=%ld occurred\n", jerr.pub.num_warnings);
		return -1;
	}

	return 0;
}

</pre>


然后开始讲解一下这段代码。
# jpeg_std_error
这是一个异常处理函数相关的注册。libjpeg所有的异常返回，默认都是直接调用exit退出。也就是说如果发生了错误，程序就直接挂了。这在本地测试、使用时没什么问题，出错了定位一下，然后接着用。但生产环境中动不动就可能程序退出，还没什么日志，这怎么行？libjpeg提供了一种方法，先覆盖默认的error_exit函数指针。  
例如提供一个my_error_exit函数通过jpeg_std_error注册进libjpeg。  
在my_error_exit以及解码接口中通过setjmp/longjmp实现函数间的跳转。具体可以看一下example.c的实现。不难


# jpeg_create_decompress
<pre>
/* jpeglib.h */
#define jpeg_create_decompress(cinfo) \
    jpeg_CreateDecompress((cinfo), JPEG_LIB_VERSION, \
			  (size_t) sizeof(struct jpeg_decompress_struct))

</pre>
可见，实际jpeg_create_decompress接口就是jpeg_CreateDecompress。函数实现在jdapimin.c中。记住，源码文件前缀的命名，j是jpeg，c是compress，d是decompress。  
实际上这个接口仅仅是向解码结构体j_decompress_ptr中注册一些函数指针，初始化一些数据结构，并且标记这个解码结构体的解码状态为DSTATE_START
<pre>
#define DSTATE_START	200	/* after create_decompress */
</pre>
