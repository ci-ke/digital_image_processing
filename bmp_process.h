#ifndef BMP_PROCESS_H
#define BMP_PROCESS_H

#define WIDTHcharS(bits) (((bits) + 31) / 32 * 4)
#define SHOWHEADER 0

typedef unsigned char uchar;
typedef char BOOL;
typedef short TWOBYTES;
typedef int FOURBYTES;

#pragma pack(1) //文件头与信息头需要单字节对齐

//文件头
typedef struct _BITMAPFILEHEADER
{
	TWOBYTES bfType;
	FOURBYTES bfSize;	 //文件大小
	TWOBYTES bfReserved1; //保留字，不考虑
	TWOBYTES bfReserved2; //保留字，同上
	FOURBYTES bfOffBits;  //实际位图数据的偏移字节数，即前三个部分长度之和
} BITMAPFILEHEADER;

//信息头
typedef struct _BITMAPINFOHEADER
{
	FOURBYTES biSize;		   //指定此结构体的长度，为40
	FOURBYTES biWidth;		   //位图宽
	FOURBYTES biHeight;		   //位图高
	TWOBYTES biPlanes;		   //平面数，为1
	TWOBYTES biBitCount;	   //采用颜色位数，可以是1，2，4，8，16，24，新的可以是32
	FOURBYTES biCompression;   //压缩方式，可以是0，1，2，其中0表示不压缩
	FOURBYTES biSizeImage;	 //实际位图数据占用的字节数
	FOURBYTES biXPelsPerMeter; //X方向分辨率
	FOURBYTES biYPelsPerMeter; //Y方向分辨率
	FOURBYTES biClrUsed;	   //使用的颜色数，如果为0，则表示默认值(2^颜色位数)
	FOURBYTES biClrImportant;  //重要颜色数，如果为0，则表示所有颜色都是重要的
} BITMAPINFOHEADER;

#pragma pack()

//调色板Palette，当然，这里是对那些需要调色板的位图文件而言的。24位和32位是不需要调色板的。（似乎是调色板结构体个数等于使用的颜色数。）
typedef struct _RGB
{
	uchar blue;	//该颜色的蓝色分量
	uchar green;   //该颜色的绿色分量
	uchar red;	 //该颜色的红色分量
	char reserved; //保留值
} RGB;

//RGB的扩展，只用于计算
typedef struct _RGBINT
{
	int blue;
	int green;
	int red;
	int reserved;
} RGBINT;

//bmp文件
typedef struct _BMP
{
	RGB **data;
	int width;
	int height;
} BMP;

//使用扩展RGB的bmp文件
typedef struct _BMPINT
{
	RGBINT **data;
	int width;
	int height;
} BMPINT;

//----------------------------------------------------------------------------------------------------

//显示bmp文件文件头
void showBmpHead(BITMAPFILEHEADER *pBmpHead);

//显示bmp文件信息头
void showBmpInforHead(BITMAPINFOHEADER *pBmpInforHead);

//显示颜色板
void showRgbQuan(RGB *pRGB, int num);

//----------------------------------------------------------------------------------------------------

//从文件读取BMP
BMP *readBMP(char *strFile);

//保存BMP到文件
void saveBMP(BMP *bmp, char *strFile);

//输入高宽与默认颜色，获得指向一个BMP动态区域的指针
BMP *initBMP(int height, int width, RGB defaultRGB);

//输入高宽与默认颜色，获得指向一个BMPINT动态区域的指针
BMPINT *initBMPINT(int height, int width, RGB defaultRGB);

//释放BMP动态区域
void deleteBMP(BMP *bmp);

//释放BMPINT动态区域
void deleteBMPINT(BMPINT *bmp);

//----------------------------------------------------------------------------------------------------

//获得RGB色块转换的RGBINT色块
RGBINT RGBtoRGBINT(RGB rgb);

//获得RGBINT色块转换的RGB色块
RGB RGBINTtoRGB(RGBINT rgbint);

//两个RGB色块相与
RGB RGBand(RGB rgb1, RGB rgb2);

//两个RGB色块相或
RGB RGBor(RGB rgb1, RGB rgb2);

//RGB色块取非
RGB RGBnot(RGB rgb);

//n个RGB色块的平均值
RGB RGBaverage(int n, RGB rgbs[]);

//防越界读取BMP中的data，flag记录越界次数
RGB RGBget(BMP *bmp, int i, int j, RGB defaultRGB, int *flag);

//两个RGBINT色块相加
RGBINT RGBINTadd(RGBINT rgbint1, RGBINT rgbint2);

//两个RGBINT色块相减
RGBINT RGBINTsub(RGBINT rgbint1, RGBINT rgbint2);

//两个RGBINT色块相乘
RGBINT RGBINTmultipy(RGBINT rgbint1, RGBINT rgbint2);

//两个RGBINT色块相除，如果除零，则取默认颜色
RGBINT RGBINTdivide(RGBINT rgbint1, RGBINT rgbint2, RGB defaultRGB);

//RGB色块乘以一个系数n
RGBINT RGBINTcoefficient(double n, RGBINT rgbint);

//一组RGBINT相加
RGBINT RGBINTaddarray(int n, RGBINT rgbints[]);

//对bmp中的一个色块应用3*3模板
RGBINT RGBcoefficentaddmodel(BMPINT *bmpint, int i, int j, double model[][3]);

//----------------------------------------------------------------------------------------------------

//BMP转换为BMPINT，获得一块指向新BMPINT动态区域的指针，同时释放原始BMP
BMPINT *BMPtoBMPINT(BMP *bmp);

//BMPINT转换为BMP，获得一块指向新BMP动态区域的指针，同时释放原始BMPINT
BMP *BMPINTtoBMP(BMPINT *bmpint);

//复制一个BMP，获得一块指向新BMP动态区域的指针
BMP *BMPcopy(BMP *bmp);

//两个BMP相与，获得一块指向新BMP动态区域的指针
BMP *BMPand(BMP *bmpa, BMP *bmpb);

//两个BMP相或，获得一块指向新BMP动态区域的指针
BMP *BMPor(BMP *bmpa, BMP *bmpb);

//BMP取非，获得一块指向新BMP动态区域的指针
BMP *BMPnot(BMP *bmp);

//BMP垂直方向放大，获得一块指向新BMP动态区域的指针
BMP *BMPheightenlarge(BMP *bmp, float heightcof);

//BMP水平方向放大，获得一块指向新BMP动态区域的指针
BMP *BMPwidthenlarge(BMP *bmp, float widthcof);

//BMP垂直与水平方向放大，获得一块指向新BMP动态区域的指针
BMP *BMPenlarge(BMP *bmp, float heightcof, float widthcof);

//BMP垂直与水平方向缩小，获得一块指向新BMP动态区域的指针
BMP *BMPshrink(BMP *bmp, float heightcof, float widthcof);

//BMP裁切，获得一块指向新BMP动态区域的指针
BMP *BMPcut(BMP *bmp, int heightstart, int heightstop, int widthstart, int widthstop);

//BMP放置于大画布的制定坐标中，并在剩余位置填充默认颜色，获得一块指向新BMP动态区域的指针
BMP *BMPput(BMP *bmp, int largeheight, int largewidth, int heightstart, int widthstart, RGB defaultRGB);

//将BMP变为灰度图像，此时图像的RGB值相等，获得一块指向新BMP动态区域的指针
BMP *BMPtogray(BMP *bmp);

//将BMP反色，获得一块指向新BMP动态区域的指针
BMP *BMPreversecolor(BMP *bmp);

//复制一个BMPINT，获得一块指向新BMP动态区域的指针
BMPINT *BMPINTcopy(BMPINT *bmpint);

//两个BMPINT相加，获得一块指向新BMP动态区域的指针
BMPINT *BMPINTadd(BMPINT *bmpinta, BMPINT *bmpintb);

//两个BMPINT相减，获得一块指向新BMP动态区域的指针
BMPINT *BMPINTsub(BMPINT *bmpinta, BMPINT *bmpintb);

//两个BMPINT相乘，获得一块指向新BMP动态区域的指针
BMPINT *BMPINTmultiply(BMPINT *bmpinta, BMPINT *bmpintb);

//两个BMPINT相除，如果除零，则取默认颜色，获得一块指向新BMP动态区域的指针
BMPINT *BMPINTdivide(BMPINT *bmpinta, BMPINT *bmpintb, RGB defaultRGB);

//----------------------------------------------------------------------------------------------------

//初始化一个二维char数组动态区域
char **initcmodel(int height, int width, int initial);

//初始化一个二维double数组动态区域
double **initdmodel(int height, int width, double initial);

//释放二维char数组动态区域
void deletecmodel(char **model, int height, int width);

//释放二维double数组动态区域
void deletedmodel(double **model, int height, int width);

//----------------------------------------------------------------------------------------------------

//计算灰度图像四近邻（flag=0）或八近邻（flag=1）对比度
double contrast(BMP *bmpgray, BOOL flag);

//对bmp进行均值滤波，model指示对哪些像素进行
BMP *averagefilter(BMP *bmp, char **model);

//对灰色图像应用3*3模板锐化
BMP *sharpen(BMP *bmpgray, double model[][3]);

//对灰色图像水平锐化
BMP *horizonsharpen(BMP *bmpgray);

//对灰色图像垂直锐化
BMP *verticalsharpen(BMP *bmpgray);

//获得灰色图像灰度直方图，存在levelnum[256]数组中
void graylevelcount(BMP *bmpgray, int *levelnum);

//P参数法对灰色图像进行二值化处理
BMP *pparameterbinaryzation(BMP *bmpgray, int *levelnum, double p);

#endif