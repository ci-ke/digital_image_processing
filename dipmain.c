#include "dipcode.h"

extern RGBQUAD BLACK;
extern RGBQUAD WHITE;
extern RGBQUAD GRAY127;

int main(int argc, char *argv[])
{
	BMP *bmp1 = readBMP(argv[1]);
	BMP *bmp2 = BMPenlarge(bmp1,1.5,1.5);
	saveBMP(bmp2, "pic/save.bmp");
	return 0;
}