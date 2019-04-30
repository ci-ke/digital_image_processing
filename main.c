#include "bmp_process.h"

extern RGB BLACK;
extern RGB WHITE;
extern RGB GRAY127;

int main(int argc, char *argv[])
{
	BMP *bmp1 = readBMP(argv[1]);
	BMP *bmp2 = BMPtogray(bmp1);

	int level[256];
	graylevelcount(bmp2,level);
	BMP *bmp3=pparameterbinaryzation(bmp2,level,0.5);

	saveBMP(bmp3, "pic/save.bmp");
	return 0;
}