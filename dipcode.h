#ifndef _DIPCODE_H
#define _DIPCODE_H

#define WIDTHcharS(bits) (((bits) + 31) / 32 * 4)
#define SHOW 1

typedef unsigned char uchar;
typedef char BOOL;

#pragma pack(1)

//文件头
typedef struct _BITMAPFILEHEADER
{
	short bfType;
	long bfSize;	   //文件大小
	short bfReserved1; //保留字，不考虑
	short bfReserved2; //保留字，同上
	long bfOffBits;	//实际位图数据的偏移字节数，即前三个部分长度之和
} BITMAPFILEHEADER;

//信息头
typedef struct _BITMAPINFOHEADER
{
	long biSize;		  //指定此结构体的长度，为40
	long biWidth;		  //位图宽
	long biHeight;		  //位图高
	short biPlanes;		  //平面数，为1
	short biBitCount;	 //采用颜色位数，可以是1，2，4，8，16，24，新的可以是32
	long biCompression;   //压缩方式，可以是0，1，2，其中0表示不压缩
	long biSizeImage;	 //实际位图数据占用的字节数
	long biXPelsPerMeter; //X方向分辨率
	long biYPelsPerMeter; //Y方向分辨率
	long biClrUsed;		  //使用的颜色数，如果为0，则表示默认值(2^颜色位数)
	long biClrImportant;  //重要颜色数，如果为0，则表示所有颜色都是重要的
} BITMAPINFOHEADER;

//调色板Palette，当然，这里是对那些需要调色板的位图文件而言的。24位和32位是不需要调色板的。（似乎是调色板结构体个数等于使用的颜色数。）
typedef struct _RGBQUAD
{
	uchar rgbBlue;	//该颜色的蓝色分量
	uchar rgbGreen;   //该颜色的绿色分量
	uchar rgbRed;	 //该颜色的红色分量
	char rgbReserved; //保留值
} RGBQUAD;

#pragma pack()

typedef struct _RGBQUADINT
{
	int rgbBlue;
	int rgbGreen;
	int rgbRed;
	int rgbReserved;
} RGBQUADINT;

typedef struct _BMP
{
	RGBQUAD **data;
	int width;
	int height;
} BMP;

typedef struct _BMPINT
{
	RGBQUADINT **data;
	int width;
	int height;
} BMPINT;

BMP *readBMP(char *strFile);
void saveBMP(BMP *bmp, char *strFile);

BMP *initBMP(int height, int width, RGBQUAD defaultRGB);
BMPINT *initBMPINT(int height, int width, RGBQUAD defaultRGB);

void deleteBMP(BMP *bmp);
void deleteBMPINT(BMPINT *bmp);

void showBmpHead(BITMAPFILEHEADER *pBmpHead);
void showBmpInforHead(BITMAPINFOHEADER *pBmpInforHead);
void showRgbQuan(RGBQUAD *pRGB, int num);
/*
函数功能：灰度图象四近邻（flag=0）或八近邻（flag=1）对比度
输入参数：char* dataOfBmp_gray——灰度图像所有像素（以行为序）对应的灰度值 int width,int height——原图像和输出图像的宽度和高度（以像素为单位） BOOL flag——四近邻或八近邻标志，flag=0为四近邻，flag=1为八近邻
输出值：四近邻（flag=0）或八近邻（flag=1）对比度
*/
double contrast(char **dataOfBmp_gray, int width, int height, BOOL flag);

RGBQUADINT RGBtoRGBINT(RGBQUAD rgb);
RGBQUAD RGBINTtoRGB(RGBQUADINT rgb);

RGBQUAD RGBand(RGBQUAD rgb1, RGBQUAD rgb2);
RGBQUAD RGBor(RGBQUAD rgb1, RGBQUAD rgb2);
RGBQUAD RGBnot(RGBQUAD rgb);
RGBQUAD RGBaverage(int n, RGBQUAD rgbs[]);
RGBQUAD RGBQUADget(BMP *bmp, int i, int j, RGBQUAD defaultRGB, int *flag);

RGBQUADINT RGBINTadd(RGBQUADINT rgb1, RGBQUADINT rgb2);
RGBQUADINT RGBINTsub(RGBQUADINT rgb1, RGBQUADINT rgb2);
RGBQUADINT RGBINTmultipy(RGBQUADINT rgb1, RGBQUADINT rgb2);
RGBQUADINT RGBINTdivide(RGBQUADINT rgb1, RGBQUADINT rgb2, RGBQUAD defaultRGB);
RGBQUADINT RGBINTcoefficient(double n, RGBQUADINT rgb);
RGBQUADINT RGBINTaddarray(int n, RGBQUADINT rgbs[]);
RGBQUADINT RGBcoefficentaddmodel(BMPINT *bmp, int i, int j, double model[][3]);

BMPINT *BMPtoBMPINT(BMP *bmp);
BMP *BMPINTtoBMP(BMPINT *bmp);

BMP *BMPcopy(BMP *bmp);
BMP *BMPand(BMP *bmpa, BMP *bmpb);
BMP *BMPor(BMP *bmpa, BMP *bmpb);
BMP *BMPnot(BMP *bmp);
BMP *BMPheightenlarge(BMP *bmp, float heightcof);
BMP *BMPwidthenlarge(BMP *bmp, float widthcof);
BMP *BMPenlarge(BMP *bmp, float heightcof, float widthcof);
BMP *BMPshrink(BMP *bmp, float heightcof, float widthcof);
BMP *BMPcut(BMP *bmp, int heightstart, int heightstop, int widthstart, int widthstop);
BMP *BMPput(BMP *bmp, int largeheight, int largewidth, int heightstart, int widthstart, RGBQUAD defaultRGB);
BMP *BMPtogray(BMP *bmp);
BMP *BMPreversecolor(BMP *bmp);

BMPINT *BMPINTcopy(BMPINT *bmp);
BMPINT *BMPINTadd(BMPINT *bmpa, BMPINT *bmpb);
BMPINT *BMPINTsub(BMPINT *bmpa, BMPINT *bmpb);
BMPINT *BMPINTmultiply(BMPINT *bmpa, BMPINT *bmpb);
BMPINT *BMPINTdivide(BMPINT *bmpa, BMPINT *bmpb, RGBQUAD defaultRGB);

char **initcmodel(int height, int width, int initial);
double **initdmodel(int height, int width, double initial);
void deletecmodel(char **model, int height, int width);
void deletedmodel(double **model, int height, int width);

BMP *averagefilter(BMP *bmp, char **model);
BMP *sharpen(BMP *bmp, double model[][3]);
BMP *horizonsharpen(BMP *bmp);
BMP *verticalsharpen(BMP *bmp);

#endif