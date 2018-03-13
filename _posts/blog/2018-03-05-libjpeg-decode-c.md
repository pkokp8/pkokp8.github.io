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
## jpeg_std_error ##

这是一个异常处理函数相关的注册。libjpeg所有的异常返回，默认都是直接调用exit退出。也就是说如果发生了错误，程序就直接挂了。这在本地测试、使用时没什么问题，出错了定位一下，然后接着用。但生产环境中动不动就可能程序退出，还没什么日志，这怎么行？libjpeg提供了一种方法，先覆盖默认的error_exit函数指针。  
例如提供一个my_error_exit函数通过jpeg_std_error注册进libjpeg。  
在my_error_exit以及解码接口中通过setjmp/longjmp实现函数间的跳转。具体可以看一下example.c的实现。不难


## jpeg_create_decompress ##
<pre>
/* jpeglib.h */
#define jpeg_create_decompress(cinfo) \
    jpeg_CreateDecompress((cinfo), JPEG_LIB_VERSION, \
			  (size_t) sizeof(struct jpeg_decompress_struct))

</pre>
可见，jpeg_create_decompress接口就是jpeg_CreateDecompress。函数实现在jdapimin.c中。有一点需要提一下，源码文件前缀的命名，j是jpeg，c是compress，d是decompress。现在看解码相关的代码时，搜函数搜到的jc开头的文件里时就可以忽略了。既不是c也不是d，但是j开头的，说明是公共文件。如果都不是，有可能是一个demo，有可能是一些扩展功能，例如bmp函数相关的封装等。  
进入这个函数内部一步步的看可以发现，这个create接口仅仅是向解码结构体j_decompress_ptr中注册一些函数指针，初始化一些数据结构，并且标记这个解码结构体的解码状态为DSTATE_START。解码状态是个一次解码中的全局标志位，可以通过解码结构体查询和设置。因为是单线程解码，因此没有锁的问题。  
<pre>
/* Values of global_state field (jdapi.c has some dependencies on ordering!) */
#define CSTATE_START	100	/* after create_compress */
#define CSTATE_SCANNING	101	/* start_compress done, write_scanlines OK */
#define CSTATE_RAW_OK	102	/* start_compress done, write_raw_data OK */
#define CSTATE_WRCOEFS	103	/* jpeg_write_coefficients done */
#define DSTATE_START	200	/* after create_decompress */
#define DSTATE_INHEADER	201	/* reading header markers, no SOS yet */
#define DSTATE_READY	202	/* found SOS, ready for start_decompress */
#define DSTATE_PRELOAD	203	/* reading multiscan file in start_decompress*/
#define DSTATE_PRESCAN	204	/* performing dummy pass for 2-pass quant */
#define DSTATE_SCANNING	205	/* start_decompress done, read_scanlines OK */
#define DSTATE_RAW_OK	206	/* start_decompress done, read_raw_data OK */
#define DSTATE_BUFIMAGE	207	/* expecting jpeg_start_output */
#define DSTATE_BUFPOST	208	/* looking for SOS/EOI in jpeg_finish_output */
#define DSTATE_RDCOEFS	209	/* reading file in jpeg_read_coefficients */
#define DSTATE_STOPPING	210	/* looking for EOI in jpeg_finish_decompress */
</pre>


## jpeg_mem_src ##
jpeg_mem_src在libjpeg.so.62是不存在的。但在较新版本中存在这个接口。较新版本的libjpeg使用时遇到了一些问题，暂时没找到解决办法，因此就把新版本库中的这个接口移植了过来。移植方法很简单，新搜一下这个函数，直接拿过来放在62版本同样的位置。编译一下，提示少定义函数了，再继续移植。提示少声明了，再继续移植。几分钟就搞定了。  
这个接口的入参除了解码结构体，还有一个buffer和一个size，就是需要解码的jpegbuffer的指针和长度。函数会把buffer和size注册给解码结构体内部的一个jpeg内存管理结构。此外就是注册一些内存管理的函数指针了。  


## jpeg_read_header ##
看名字就可以知道，这个函数的功能是读取一下jpeg头。jpeg头中包含了很多解码需要的信息，例如huffman表，jpeg的长宽、位宽等等。  
那么具体看一下，首先这个接口调用了jpeg_consume_input，接着根据返回值就返回了。联系函数名叫做readheader，那么jpeg_consume_input的返回值必定是JPEG_REACHED_SOS。否则如果是JPEG_REACHED_EOI（EOI，end of image），那干脆叫jpeg_decoder算了。  
# jpeg_consume_input #
<pre>
GLOBAL(int)
jpeg_consume_input (j_decompress_ptr cinfo)
{
  int retcode = JPEG_SUSPENDED;

  /* NB: every possible DSTATE value should be listed in this switch */
  switch (cinfo->global_state) {
  case DSTATE_START:
    /* Start-of-datastream actions: reset appropriate modules */
    (*cinfo->inputctl->reset_input_controller) (cinfo);
    /* Initialize application's data source module */
    (*cinfo->src->init_source) (cinfo);
    cinfo->global_state = DSTATE_INHEADER;
    /*FALLTHROUGH*/
  case DSTATE_INHEADER:
    retcode = (*cinfo->inputctl->consume_input) (cinfo);
    if (retcode == JPEG_REACHED_SOS) { /* Found SOS, prepare to decompress */
      /* Set up default parameters based on header data */
      default_decompress_parms(cinfo);
      /* Set global state: ready for start_decompress */
      cinfo->global_state = DSTATE_READY;
    }
    break;
</pre>
里面就是个switch...case。首先解码状态在jpeg_create_decompress时被设为DSTATE_START，因此调用了reset_input_controller以及init_source，并标记解码状态为DSTATE_INHEADER。  
接着调用consume_input，以及default_decompress_parms，并标记解码状态为DSTATE_READY。  
reset_marker_reader看名字，叫做reset marker reader，外层函数叫做read header，显然就是用来重置解码结构体中marker相关的内容的。看进去也可以发现，没有实际的读取jpegbuffer。  
init_source更简单，标记 ((my_src_ptr)cinfo->src)->start_of_file为true。意味着获取到了数据。那么显然主要操作在consume_input和default_decompress_parms中。  
consume_input函数指针是jdinput.c--consume_markers函数。调用read_markers并返回JPEG_REACHED_SOS，接着调用initial_setup判断一下之前读出来的一些信息的合法性。  
# read_markers #
可以看到，这个函数就是个死循环。一开始unread_marker和saw_SOI都是0，先用first_marker读第一个标记M_SOI，并标记saw_SOI为TRUE，于是下一次就会调用next_marker，每次通过switch...case处理当前读到的标记下面的一段数据，直到读到M_SOS就会返回JPEG_REACHED_SOS。至于如何解析各个标记，可以去找一下jpeg的头的格式。网上资料还是比较详细的。
<pre>
	printf("unread_marker = %x\n", cinfo->unread_marker);		//此处加一句打印
    /* At this point cinfo->unread_marker contains the marker code and the
     * input point is just past the marker proper, but before any parameters.
     * A suspension will cause us to return with this state still true.
     */
    switch (cinfo->unread_marker) {
</pre>
就可以看到文章最开始的那张图像会打印如下信息。
<pre>
unread_marker = d8
unread_marker = e0
unread_marker = db
unread_marker = db
unread_marker = c0
unread_marker = c4
unread_marker = c4
unread_marker = c4
unread_marker = c4
unread_marker = da
</pre>
例如根据jpeg的格式，一张jpeg的宽高可以从sof标记后面的数据得到：
![avatar](/images/libjpegmaterial/1.jpg)
1920=0x0780  
1080=0x0438  
当然并不是所有jpeg图像都有上述的各个标记。具体图像具体分析。  
最终read_markers将返回值JPEG_REACHED_SOS返回到jpeg_consume_input中，会接着执行default_decompress_parms。  
根据default_decompress_parms的注释，这段代码是一个猜测的经验值，并没有规定一定是这样。jpeg_color_space是输入的jpeg的色彩空间。out_color_space是输出的。
<pre>
![avatar](/images/libjpegmaterial/2.jpg)
</pre>
最上方的这张图的输入是JCS_YCbCr，输出在此处被设为了JCS_RGB。理论上如果需要实现解码成yuv，那么此处应该设置输出为JCS_YCbCr。这个后面再讲。(1)  
最终设置一些默认解码参数，例如raw_data_out为false，dct_method为JDCT_DEFAULT，scale比例为1等等，之后函数就结束了。  
最终jpeg_consume_input标记解码状态为DSTATE_READY，jpeg_read_header就正常的结束了。  



## jpeg_start_decompress ##
调用jinit_master_decompress初始化master。这是个私有数据，可以暂不关心。并标记解码状态为DSTATE_PRELOAD。  
progress这个函数指针是注册一个进度处理函数，可以显示进度。一般可以不注册，那么就忽略。  
而has_multiple_scans标记位，看名字意思是jpeg图像中有多个扫描。上面这张图并没有这个特征。  
接着调用output_pass_setup就可以返回了。
# output_pass_setup #
注意此处的解码状态依然是DSTATE_PRELOAD，因此会调用prepare_for_output_pass。is_dummy_pass在jinit_master_decompress时被初始化为false，因此下面其实是个不执行的循环，意思是prepare_for_output_pass执行完就基本返回了。  
prepare_for_output_pass中首先和上面说的一样，is_dummy_pass是false，default_decompress_parms会把quantize_colors设为false，colormap设为NULL；jpeg_create_decompress会把progress设为NULL。  
于是，此处相当于只执行了以下内容：
<pre>
    (*cinfo->idct->start_pass) (cinfo);
    (*cinfo->coef->start_output_pass) (cinfo);
    if (! cinfo->raw_data_out) {
      if (! master->using_merged_upsample)
	(*cinfo->cconvert->start_pass) (cinfo);
      (*cinfo->upsample->start_pass) (cinfo);
      if (cinfo->quantize_colors)
	(*cinfo->cquantize->start_pass) (cinfo, master->pub.is_dummy_pass);
      (*cinfo->post->start_pass) (cinfo,
	    (master->pub.is_dummy_pass ? JBUF_SAVE_AND_PASS : JBUF_PASS_THRU));
      (*cinfo->main->start_pass) (cinfo, JBUF_PASS_THRU);
}
</pre>
可以看出都是一些start pass。说明并不是实际的进行一些解码变换。否则应该叫idct->process。  
举个例子，idct->start_pass注册为jddctmgr.c--start_pass，看一下大致的流程，仅仅是初始化一些查询表，不作解码动作。  
raw_data_out在default_decompress_parms时被设为false，因此最终解码状态为DSTATE_SCANNING。如果手动设置了raw_data_out（见上(1)），那么此处可能是DSTATE_RAW_OK。  
不管怎么说，jpeg_start_decompress就正确的返回了。


## jpeg_read_scanlines/jpeg_read_raw_data ##
根据解码状态，显然接下去应该调用jpeg_read_scanlines解码。也有可能调用jpeg_read_raw_data解码。  
如果要详细讲这两个函数，那么显然要了解jpeg的编码、解码方式。由于我的能力有限，暂未能详细了解jpeg的编解码流程，因此此处暂且不讲具体流程，仅仅简单的分析一下。  
这两个函数处于jdapistd.c中，并且是紧挨着的两个函数。说明功能其实差不多。
# jpeg_read_scanlines #
调用process_data--process_data_simple_main。而process_data_simple_main调用decompress_data和post_process_data。
# jpeg_read_raw_data #
直接调用decompress_data。

# post_process_data #
虽然细节上还有差异，但显然差异就在于post_process_data。  
post->pub.post_process_data ---- cinfo->upsample->upsample ---- (jdsample.c)sep_upsample  
sep_upsample中调用了两个接口，methods以及color_convert。methods是一些采样方法，暂且忽略。color_convert就是色彩空间的转换。  
jinit_master_decompress-master_selection-jinit_color_deconverter中会根据输入的jpeg色彩空间以及输出的色彩空间（之前默认rgb），设置color_convert函数指针为  
grayscale_convert  
ycc_rgb_convert  
gray_rgb_convert  
null_convert  
ycck_cmyk_convert  
之一。那么如果要解码成JCS_YCbCr（见上(1)），输入又是JCS_YCbCr，那么显然调用jpeg_read_raw_data速度会更快。否则jpeg_read_scanlines会将yuv变换为rgb。后面又要通过rgb转换回yuv(demo写的不够好)。


## 色彩空间 ##
上面这段demo支持rgb、gray、cmyk的解码。其中rgb和gray的解码效果基本没有差异，唯独cmyk有点缺陷。

demo解码  :
![avatar](/images/libjpegmaterial/8.jpg)

XnView解码  :
![avatar](/images/libjpegmaterial/9.jpg)

windows照片查看器  :
![avatar](/images/libjpegmaterial/10.jpg)

ffplay CactiIslaPescado_ZH-CN11317505000_1920x1080_cmyk.jpg -x 1280 -y 720  :
![avatar](/images/libjpegmaterial/12.jpg)

photoshop  :
![avatar](/images/libjpegmaterial/13.jpg)

看得出，除了photoshop和windows，其他的解码效果都与原图色彩较深。这是因为rgb于cmyk的转换并不是点对点的无损转换。
![avatar](/images/libjpegmaterial/14.jpg)

有一种做法是将当前cmyk的色域扩充到lab色域。然后再缩回rgb色域。这样效果会较好（未经过验证）。


## jpeg_finish_decompress/jpeg_destroy_decompress ##
一般来说，完成解码会调用这两个接口进行资源回收。但如果需要反复的解码，建议jpeg_create_decompress之前的代码只执行一次。此处只执行jpeg_finish_decompress，不执行jpeg_destroy_decompress。  
jpeg_finish_decompress的内部会调用jpeg_abort进行一些资源回收。不调用jpeg_abort会出现内存泄漏问题。  
jpeg_abort很奇怪的位于jcomapi.c中



至此，一段简单的jpeg解码流程就结束了。欢迎和我讨论: pkokp8@gmail.com  
编码流程有空继续补充。


