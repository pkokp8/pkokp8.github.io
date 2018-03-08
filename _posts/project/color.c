#include<stdio.h>

#define WIDTH 352
#define HEIGHT 288
unsigned char dstrgb24[WIDTH*HEIGHT*3] = {0};


typedef enum _CHANG{
    ADDGERRN 	= 1,
    MINUSRED 	= 2,
    ADDBLUE 	= 3,
    MINUSGREEN 	= 4,
    ADDRED 		= 5,
    MINUSBLUE 	= 6,
    BUTT
}CHANGE;

int width = WIDTH;
int height = HEIGHT;

#define abs(x) ((x)<0?(-1*(x)):(x))
int main()
{
    int pixel = 0;
    int x = 0;
    int y = 0;
    int startr, startg, startb;
    int endr, endg, endb;
    CHANGE flag = BUTT;
    for(y = 0; y < height; y++)
    {
        int r,g,b;
        startr = (-127) * y / height + 255;
        startg = y * 127 / height;
        startb = y * 127 / height;
        endr = 255 - startr;
        endg = 255 - startg;
        endb = 255 - startg;

        r = startr;
        g = startg;
        b = startb;
        flag = ADDGERRN;
        int cnt = 0;
        for(x = 0; x < width; x++)
        {
            double stepw = abs((endr - startr)) / (width * 1.0);
            if(flag == ADDGERRN)
            {
                g = startg + (int)(stepw * (BUTT - 1) * cnt);
                cnt++;
                if(g >= endg)
                {
                    cnt = 0;
                    g = endg;
                    flag++;
                }
            }
            else if(flag == MINUSRED)
            {
                r = startr - (int)(stepw * (BUTT - 1) * cnt);
                cnt++;
                if(r <= endr)
                {
                    cnt = 0;
                    r = endr;
                    flag++;
                }
            }
            else if(flag == ADDBLUE)
            {
                b = startb + (int)(stepw * (BUTT - 1) * cnt);
                cnt++;
                if(b >= endb)
                {
                    cnt = 0;
                    b = endb;
                    flag++;
                }
            }
            else if(flag == MINUSGREEN)
            {
                g = endg - (int)(stepw * (BUTT - 1) * cnt);
                cnt++;
                if(g <= startg)
                {
                    cnt = 0;
                    g = startg;
                    flag++;
                }
            }
            else if(flag == ADDRED)
            {
                r = endr + (int)(stepw * (BUTT - 1) * cnt);
                cnt++;
                if(r >= startr)
                {
                    cnt = 0;
                    r = startr;
                    flag++;
                }
            }
            else if(flag == MINUSBLUE)
            {
                b = endb - (int)(stepw * (BUTT - 1) * cnt);
                cnt++;
                if(b <= startb)
                {
                    cnt = 0;
                    b = startb;
                    flag++;
                }
            }
            else
            {
                //printf("???\n");
            }

            dstrgb24[pixel++] = r;
            dstrgb24[pixel++] = g;
            dstrgb24[pixel++] = b;
        }
    }
    FILE *fp = fopen("color.rgb", "w+");
    if(fp != NULL)
    {
        fwrite(dstrgb24, width * height * 3, 1, fp);
        fclose(fp);
    }
    printf("%d\n", pixel);
    return 0;

}
