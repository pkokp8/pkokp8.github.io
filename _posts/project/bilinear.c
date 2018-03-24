#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/time.h>

#define PRINTF(string,args...) printf("%s[%d]: "string"",__FUNCTION__,__LINE__,##args)
typedef struct __pic{
    unsigned int width;
    unsigned int height;
    unsigned char *pic;
    unsigned int len;
}stpic;


//目标座标映射到源座标，取源座标的四个点，进行权重计算得到目标座标的像素值
int BIpic(stpic *srcrgb24, stpic *dstrgb24)
{
    //目标座标
    int x = 0;
    int y = 0;

    //目标座标在源座标区域的映射,double精度
    double dxsrc, dysrc;
    //上述精度的取整
    int xsrc, ysrc;

    char *srcpic = srcrgb24->pic;

    //abcd为目标座标在源座标区域的映射后double精度所在的座标边上的四个像素的rgb
    unsigned char ra,rb,rc,rd;
    unsigned char ga,gb,gc,gd;
    unsigned char ba,bb,bc,bd;

    //目标座标的rgb
    unsigned char dstr, dstg, dstb;

    char flag = 2;

    for(y = 0;y < dstrgb24->height; y++)
    {
        for(x = 0;x < dstrgb24->width; x++)
        {


            dxsrc = (x*srcrgb24->width)/(dstrgb24->width*1.0);
            dysrc = (y*srcrgb24->height)/(dstrgb24->height*1.0);

            xsrc = (int)dxsrc;
            ysrc = (int)dysrc;
            if(xsrc == srcrgb24->width - 1)
            {
                //右边缘
                //continue;
                dxsrc-=1.0;
                xsrc = (int)dxsrc;

            }
            if(ysrc == srcrgb24->height - 1)
            {
                //下边缘
                dysrc-=1.0;
                ysrc = (int)dysrc;
            }
            ra = srcpic[3*ysrc*srcrgb24->width + 3*xsrc + 0];
            ga = srcpic[3*ysrc*srcrgb24->width + 3*xsrc + 1];
            ba = srcpic[3*ysrc*srcrgb24->width + 3*xsrc + 2];

            rb = srcpic[3*ysrc*srcrgb24->width + 3*(xsrc + 1) + 0];
            gb = srcpic[3*ysrc*srcrgb24->width + 3*(xsrc + 1) + 1];
            bb = srcpic[3*ysrc*srcrgb24->width + 3*(xsrc + 1) + 2];

            rc = srcpic[3*(ysrc + 1)*srcrgb24->width + 3*(xsrc + 1) + 0];
            gc = srcpic[3*(ysrc + 1)*srcrgb24->width + 3*(xsrc + 1) + 1];
            bc = srcpic[3*(ysrc + 1)*srcrgb24->width + 3*(xsrc + 1) + 2];

            rd = srcpic[3*(ysrc + 1)*srcrgb24->width + 3*xsrc + 0];
            gd = srcpic[3*(ysrc + 1)*srcrgb24->width + 3*xsrc + 1];
            bd = srcpic[3*(ysrc + 1)*srcrgb24->width + 3*xsrc + 2];

            double s1, s2, s3, s4;
            s1 = (dxsrc - xsrc)*(dysrc - ysrc);
            s2 = (xsrc + 1.0 - dxsrc)*(dysrc-ysrc);
            s3 = (xsrc + 1.0 - dxsrc)*(ysrc + 1.0 - dysrc);
            s4 = (dxsrc - xsrc)*(ysrc + 1.0 - dysrc);

            dstr = ra*s3 + rb*s4 + rc*s1 + rd*s2;
            dstg = ga*s3 + gb*s4 + gc*s1 + gd*s2;
            dstb = ba*s3 + bb*s4 + bc*s1 + bd*s2;

            dstrgb24->pic[3*y*dstrgb24->width + 3*x + 0] = dstr;
            dstrgb24->pic[3*y*dstrgb24->width + 3*x + 1] = dstg;
            dstrgb24->pic[3*y*dstrgb24->width + 3*x + 2] = dstb;

        }

    }

}


int main()
{
    stpic srcrgb24;
    stpic dstrgb24;

    srcrgb24.width = 352;
    srcrgb24.height = 288;
    srcrgb24.len = srcrgb24.width*srcrgb24.height*3;

    srcrgb24.pic = malloc(srcrgb24.len);
    memset(srcrgb24.pic, 0, srcrgb24.len);

    int fd = open("color.rgb", O_RDONLY);
	if(fd <= 0)
	{
		PRINTF("open src img error\n");
		return -1;
	}
    read(fd, srcrgb24.pic, srcrgb24.len);
    close(fd);

    dstrgb24.width = 1920;
    dstrgb24.height = 1080;
    dstrgb24.len = dstrgb24.width*dstrgb24.height*3;  
    dstrgb24.pic = malloc(dstrgb24.len);
	

    struct timeval start,end;
    long timeuse = 0;
    gettimeofday(&start, NULL );	//计时

    BIpic(&srcrgb24, &dstrgb24);
	
    gettimeofday(&end, NULL );		//计时
    timeuse =1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
    printf("time=%f\n",timeuse /1000000.0);


    system("rm dst.rgb");
    fd = open("dst.rgb", O_RDWR|O_CREAT);
    write(fd, dstrgb24.pic, dstrgb24.len);
    close(fd);
    system("chmod 777 dst.rgb");
}