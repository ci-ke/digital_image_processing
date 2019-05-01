#include "bmp_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <string.h>

RGB BLACK = {0, 0, 0, 0};
RGB WHITE = {255, 255, 255, 0};
RGB GRAY127 = {127, 127, 127, 0};

void showBmpHead(BITMAPFILEHEADER *pBmpHead)
{
    printf("位图文件头:\n");
    printf("文件类型:%x\n", pBmpHead->bfType);
    printf("文件大小:%d\n", pBmpHead->bfSize);
    printf("保留字:%d\n", pBmpHead->bfReserved1);
    printf("保留字:%d\n", pBmpHead->bfReserved2);
    printf("实际位图数据的偏移字节数:%d\n", pBmpHead->bfOffBits);
}

void showBmpInforHead(BITMAPINFOHEADER *pBmpInforHead)
{
    printf("位图信息头:\n");
    printf("结构体的长度:%d\n", pBmpInforHead->biSize);
    printf("位图宽:%d\n", pBmpInforHead->biWidth);
    printf("位图高:%d\n", pBmpInforHead->biHeight);
    printf("biPlanes平面数:%d\n", pBmpInforHead->biPlanes);
    printf("biBitCount采用颜色位数:%d\n", pBmpInforHead->biBitCount);
    printf("压缩方式:%d\n", pBmpInforHead->biCompression);
    printf("biSizeImage实际位图数据占用的字节数:%d\n", pBmpInforHead->biSizeImage);
    printf("X方向分辨率:%d\n", pBmpInforHead->biXPelsPerMeter);
    printf("Y方向分辨率:%d\n", pBmpInforHead->biYPelsPerMeter);
    printf("使用的颜色数:%d\n", pBmpInforHead->biClrUsed);
    printf("重要颜色数:%d\n", pBmpInforHead->biClrImportant);
}

void showRgbQuan(RGB *pRGB, int num)
{
    for (int i = 0; i < num; i++)
    {
        if (i % 5 == 0)
        {
            printf("\n");
        }
        printf("(%-3d,%-3d,%-3d) ", (pRGB + i)->red, (pRGB + i)->green, (pRGB + i)->blue);
    }
    printf("\n");
}

BMP *readBMP(char *strFile)
{
    BITMAPFILEHEADER bitHead;
    BITMAPINFOHEADER bitInfoHead;
    if (sizeof(BITMAPFILEHEADER) != 14 || sizeof(BITMAPINFOHEADER) != 40)
    {
        printf("program cannot run on this machine.\n");
        return NULL;
    }
    FILE *pfile;
    pfile = fopen(strFile, "rb");
    if (pfile != NULL)
    {
        fread(&bitHead, sizeof(BITMAPFILEHEADER), 1, pfile); //需要保证文件头单字节对齐
        if (bitHead.bfType != 0x4d42)
        {
            printf("file %s is not .bmp file.\n", strFile);
            return NULL;
        }
        fread(&bitInfoHead, sizeof(BITMAPINFOHEADER), 1, pfile); //需要保证信息头单字节对齐
    }
    else
    {
        printf("file %s open fail.\n", strFile);
        return NULL;
    }
#if SHOWHEADER == 1
    showBmpHead(&bitHead);
    printf("\n");
    showBmpInforHead(&bitInfoHead);
    printf("\n");
#endif
    RGB *pRgb = NULL;
    if (bitInfoHead.biBitCount < 24) //有调色板
    {
        int nPlantNum = bitInfoHead.biClrUsed;
        if (!nPlantNum)
        {
            nPlantNum = pow(2, bitInfoHead.biBitCount);
        }
        pRgb = (RGB *)malloc(sizeof(RGB) * nPlantNum);
        memset(pRgb, 0, nPlantNum * sizeof(RGB));
        fread(pRgb, sizeof(pRgb), nPlantNum, pfile); //需要保证RGB单字节对齐
#if SHOWHEADER == 1
        printf("Color Plate Number:%d\n", nPlantNum);
        printf("颜色板信息:\n");
        showRgbQuan(pRgb, nPlantNum);
#endif
    }
    int width = bitInfoHead.biWidth;
    int height = bitInfoHead.biHeight;
    int l_width = WIDTHcharS(width * bitInfoHead.biBitCount); //计算位图的实际宽度并确保它为32的倍数
    int nData = height * l_width;
    char *pColorData = (char *)malloc(nData);
    memset(pColorData, 0, nData);
    fread(pColorData, nData, 1, pfile); //把位图数据信息读到数组里
    RGB **dataOfBmp_src = NULL;         //将位图数据转化为RGB数据
    dataOfBmp_src = (RGB **)malloc(sizeof(RGB *) * height);
    for (int i = 0; i < height; i++)
    {
        dataOfBmp_src[i] = (RGB *)malloc(sizeof(RGB) * width);
    }
    if (bitInfoHead.biBitCount < 24) //有调色板，即位图为非真彩色
    {
        if (bitInfoHead.biBitCount <= 8 && !bitInfoHead.biCompression)
        {
            int pnum = 8 / bitInfoHead.biBitCount;
            int mbnum = 8 - bitInfoHead.biBitCount;
            for (int i = 0; i < height; i++)
            {
                int k0 = (height - i - 1) * l_width; //k:取得该像素颜色数据在实际数据数组中的序号
                for (int j = 0; j < width; j++)
                {
                    char mixIndex = 0;
                    int k = k0 + (j / pnum);
                    mixIndex = pColorData[k]; //提取当前像素的颜色的在颜色表中的索引值
                    if (bitInfoHead.biBitCount < 8)
                    {
                        mixIndex = mixIndex << ((j % pnum) * bitInfoHead.biBitCount);
                        mixIndex = mixIndex >> mbnum;
                    }
                    dataOfBmp_src[i][j].red = pRgb[mixIndex].red;
                    dataOfBmp_src[i][j].green = pRgb[mixIndex].green;
                    dataOfBmp_src[i][j].blue = pRgb[mixIndex].blue;
                    dataOfBmp_src[i][j].reserved = pRgb[mixIndex].reserved;
                }
            }
        }
        if (bitInfoHead.biBitCount == 16)
        {
            if (!bitInfoHead.biCompression)
            {
                for (int i = 0; i < height; i++)
                {
                    int k0 = (height - i - 1) * l_width;
                    for (int j = 0; j < width; j++)
                    {
                        short mixIndex = 0;
                        int k = k0 + j * 2;
                        short shortTemp;
                        shortTemp = pColorData[k + 1];
                        shortTemp = shortTemp << 8;
                        mixIndex = pColorData[k] + shortTemp;
                        dataOfBmp_src[i][j].red = pRgb[mixIndex].red;
                        dataOfBmp_src[i][j].green = pRgb[mixIndex].green;
                        dataOfBmp_src[i][j].blue = pRgb[mixIndex].blue;
                        dataOfBmp_src[i][j].reserved = pRgb[mixIndex].reserved;
                    }
                }
            }
        }
    }
    else //位图为24/32位真彩色
    {
        if (bitInfoHead.biBitCount == 16)
        {
            for (int i = 0; i < height; i++)
            {
                int k0 = (height - i - 1) * l_width;
                for (int j = 0; j < width; j++)
                {
                    int k = k0 + j * 2;
                    if (!bitInfoHead.biCompression) //555格式
                    {
                        dataOfBmp_src[i][j].blue = pColorData[k] & 0x1F;
                        dataOfBmp_src[i][j].green = (((pColorData[k + 1] << 6) & 0xFF) >> 3) + (pColorData[k] >> 5);
                        dataOfBmp_src[i][j].red = (pColorData[k + 1] << 1) >> 3;
                        dataOfBmp_src[i][j].reserved = 0;
                    }
                }
            }
        }
        if (bitInfoHead.biBitCount == 24 && !bitInfoHead.biCompression)
        {
            for (int i = 0; i < height; i++)
            {
                int k0 = (height - i - 1) * l_width;
                for (int j = 0; j < width; j++)
                {
                    int k = k0 + (j * 3);
                    dataOfBmp_src[i][j].red = pColorData[k + 2];
                    dataOfBmp_src[i][j].green = pColorData[k + 1];
                    dataOfBmp_src[i][j].blue = pColorData[k];
                    dataOfBmp_src[i][j].reserved = 0;
                }
            }
        }
        if (bitInfoHead.biBitCount == 32 && !bitInfoHead.biCompression)
        {
            for (int i = 0; i < height; i++)
            {
                int k0 = (height - i - 1) * l_width;
                for (int j = 0; j < width; j++)
                {
                    int k = k0 + (j * 4);
                    dataOfBmp_src[i][j].red = pColorData[k + 2];
                    dataOfBmp_src[i][j].green = pColorData[k + 1];
                    dataOfBmp_src[i][j].blue = pColorData[k];
                    dataOfBmp_src[i][j].reserved = pColorData[k + 3];
                }
            }
        }
    }
    if (bitInfoHead.biBitCount < 24 && pRgb)
    {
        free(pRgb);
    }
    if (pColorData)
    {
        free(pColorData);
    }
    fclose(pfile);
    BMP *bmp = (BMP *)malloc(sizeof(BMP));
    bmp->data = dataOfBmp_src;
    bmp->width = width;
    bmp->height = height;
    printf("file %s read success.\n", strFile);
    return bmp;
}

void saveBMP(BMP *bmp, char *strFile)
{
    FILE *pfile = fopen(strFile, "wb");
    if (pfile != NULL)
    {
        RGB **dataOfBmp = bmp->data;
        int width = bmp->width;
        int height = bmp->height;
        BITMAPFILEHEADER bitHead;
        BITMAPINFOHEADER bitInfoHead;
        short biBitCount = 32;
        bitHead.bfType = 0x4d42;
        bitHead.bfSize = 0;
        bitHead.bfReserved1 = 0;
        bitHead.bfReserved2 = 0;
        bitHead.bfOffBits = 54;
        if (biBitCount <= 8)
        {
            bitHead.bfOffBits += (int)pow(2, biBitCount) * 4;
        }
        fwrite(&bitHead.bfType, sizeof(BITMAPFILEHEADER), 1, pfile);
        bitInfoHead.biSize = 40;
        bitInfoHead.biWidth = width;
        bitInfoHead.biHeight = height;
        bitInfoHead.biPlanes = 1;
        bitInfoHead.biBitCount = biBitCount;
        bitInfoHead.biCompression = 0;
        bitInfoHead.biSizeImage = 0;
        bitInfoHead.biXPelsPerMeter = 0;
        bitInfoHead.biYPelsPerMeter = 0;
        bitInfoHead.biClrImportant = 0;
        bitInfoHead.biClrUsed = 0;
        fwrite(&bitInfoHead, sizeof(BITMAPINFOHEADER), 1, pfile);
        if (biBitCount <= 8)
        {
            char tmp = 0;
            for (int i = 0; i < (int)pow(2, biBitCount); i++)
            {
                tmp = (char)i;
                fwrite(&tmp, 1, 4, pfile);
            }
        }
        int l_width = WIDTHcharS(width * biBitCount) - width * 4; //计算为确保位图数据区的实际宽度为32字节的整数倍需添加的'0'字节数
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                fwrite(&dataOfBmp[height - i - 1][j], 4, 1, pfile);
            }
            char tmp = 0;
            for (int j = 0; j < l_width; j++)
            {
                fwrite(&tmp, 1, 1, pfile);
            }
        }
        fclose(pfile);
        printf("file %s save success.\n", strFile);
    }
    else
    {
        printf("file %s save fail.\n", strFile);
    }
}

BMP *initBMP(int height, int width, RGB defaultRGB)
{
    BMP *bmpnew = (BMP *)malloc(sizeof(BMP));
    RGB **data = (RGB **)malloc(sizeof(RGB *) * height);
    for (int i = 0; i < height; i++)
    {
        data[i] = (RGB *)malloc(sizeof(RGB) * width);
        for (int j = 0; j < width; j++)
        {
            data[i][j] = defaultRGB;
        }
    }
    bmpnew->data = data;
    bmpnew->height = height;
    bmpnew->width = width;
    return bmpnew;
}

BMPINT *initBMPINT(int height, int width, RGB defaultRGB)
{
    BMPINT *bmpintnew = (BMPINT *)malloc(sizeof(BMPINT));
    RGBINT **data = (RGBINT **)malloc(sizeof(RGBINT *) * height);
    for (int i = 0; i < height; i++)
    {
        data[i] = (RGBINT *)malloc(sizeof(RGBINT) * width);
        for (int j = 0; j < width; j++)
        {
            data[i][j] = RGBtoRGBINT(defaultRGB);
        }
    }
    bmpintnew->data = data;
    bmpintnew->height = height;
    bmpintnew->width = width;
    return bmpintnew;
}

void deleteBMP(BMP *bmp)
{
    for (int i = 0; i < bmp->height; i++)
    {
        if (bmp->data[i])
        {
            free(bmp->data[i]);
        }
    }
    if (bmp->data)
    {
        free(bmp->data);
    }
    free(bmp);
}

void deleteBMPINT(BMPINT *bmpint)
{
    for (int i = 0; i < bmpint->height; i++)
    {
        if (bmpint->data[i])
        {
            free(bmpint->data[i]);
        }
    }
    if (bmpint->data)
    {
        free(bmpint->data);
    }
    free(bmpint);
}

RGBINT RGBtoRGBINT(RGB rgb)
{
    RGBINT rgbintnew;
    rgbintnew.blue = rgb.blue;
    rgbintnew.green = rgb.green;
    rgbintnew.red = rgb.red;
    rgbintnew.reserved = rgb.reserved;
    return rgbintnew;
}

RGB RGBINTtoRGB(RGBINT rgbint)
{
    RGB rgbnew;
    if (rgbint.blue > 255)
    {
        rgbnew.blue = 255;
    }
    else if (rgbint.blue < 0)
    {
        rgbnew.blue = 0;
    }
    else
    {
        rgbnew.blue = rgbint.blue;
    }

    if (rgbint.green > 255)
    {
        rgbnew.green = 255;
    }
    else if (rgbint.green < 0)
    {
        rgbnew.green = 0;
    }
    else
    {
        rgbnew.green = rgbint.green;
    }

    if (rgbint.red > 255)
    {
        rgbnew.red = 255;
    }
    else if (rgbint.red < 0)
    {
        rgbnew.red = 0;
    }
    else
    {
        rgbnew.red = rgbint.red;
    }
    rgbnew.reserved = rgbint.reserved;
    return rgbnew;
}

RGB RGBand(RGB rgb1, RGB rgb2)
{
    RGB rgbnew;
    rgbnew.blue = rgb1.blue & rgb2.blue;
    rgbnew.green = rgb1.green & rgb2.green;
    rgbnew.red = rgb1.red & rgb2.red;
    rgbnew.reserved = 0;
    return rgbnew;
}

RGB RGBor(RGB rgb1, RGB rgb2)
{
    RGB rgbnew;
    rgbnew.blue = rgb1.blue | rgb2.blue;
    rgbnew.green = rgb1.green | rgb2.green;
    rgbnew.red = rgb1.red | rgb2.red;
    rgbnew.reserved = 0;
    return rgbnew;
}

RGB RGBnot(RGB rgb)
{
    RGB rgbnew;
    rgbnew.blue = ~rgb.blue;
    rgbnew.green = ~rgb.green;
    rgbnew.red = ~rgb.red;
    rgbnew.reserved = 0;
    return rgbnew;
}

RGB RGBaverage(int n, RGB rgbs[])
{
    RGB rgbnew;
    int tempb = 0;
    int tempg = 0;
    int tempr = 0;
    for (int i = 0; i < n; i++)
    {
        tempb += rgbs[i].blue;
        tempg += rgbs[i].green;
        tempr += rgbs[i].red;
    }
    rgbnew.blue = (uchar)(tempb / n);
    rgbnew.green = (uchar)(tempg / n);
    rgbnew.red = (uchar)(tempr / n);
    rgbnew.reserved = 0;
    return rgbnew;
}

RGB RGBget(BMP *bmp, int i, int j, RGB defaultRGB, int *flag)
{
    RGB rgbgot;
    if (i < 0 || i > bmp->height - 1 || j < 0 || j > bmp->width - 1)
    {
        rgbgot = defaultRGB;
        if (flag)
        {
            *flag += 1;
        }
    }
    else
    {
        rgbgot = bmp->data[i][j];
    }
    return rgbgot;
}

RGBINT RGBINTadd(RGBINT rgbint1, RGBINT rgbint2)
{
    RGBINT rgbintnew;
    rgbintnew.blue = rgbint1.blue + rgbint2.blue;
    rgbintnew.green = rgbint1.green + rgbint2.green;
    rgbintnew.red = rgbint1.red + rgbint2.red;
    rgbintnew.reserved = 0;
    return rgbintnew;
}

RGBINT RGBINTsub(RGBINT rgbint1, RGBINT rgbint2)
{
    RGBINT rgbintnew;
    rgbintnew.blue = rgbint1.blue - rgbint2.blue;
    rgbintnew.green = rgbint1.green - rgbint2.green;
    rgbintnew.red = rgbint1.red - rgbint2.red;
    rgbintnew.reserved = 0;
    return rgbintnew;
}

RGBINT RGBINTmultipy(RGBINT rgbint1, RGBINT rgbint2)
{
    RGBINT rgbintnew;
    rgbintnew.blue = rgbint1.blue * rgbint2.blue;
    rgbintnew.green = rgbint1.green * rgbint2.green;
    rgbintnew.red = rgbint1.red * rgbint2.red;
    rgbintnew.reserved = 0;
    return rgbintnew;
}

RGBINT RGBINTdivide(RGBINT rgbint1, RGBINT rgbint2, RGB defaultRGB)
{
    RGBINT rgbintnew;
    if (rgbint2.blue != 0)
    {
        rgbintnew.blue = rgbint1.blue / rgbint2.blue;
    }
    else
    {
        rgbintnew.blue = defaultRGB.blue;
    }
    if (rgbint2.green != 0)
    {
        rgbintnew.green = rgbint1.green / rgbint2.green;
    }
    else
    {
        rgbintnew.green = defaultRGB.green;
    }
    if (rgbint2.red != 0)
    {
        rgbintnew.red = rgbint1.red / rgbint2.red;
    }
    else
    {
        rgbintnew.red = defaultRGB.red;
    }
    rgbintnew.reserved = 0;
    return rgbintnew;
}

RGBINT RGBINTaddarray(int n, RGBINT rgbints[])
{
    RGBINT rgbintnew;
    rgbintnew.blue = 0;
    rgbintnew.green = 0;
    rgbintnew.red = 0;
    rgbintnew.reserved = 0;
    for (int i = 0; i < n; i++)
    {
        rgbintnew.blue += rgbints[i].blue;
        rgbintnew.green += rgbints[i].green;
        rgbintnew.red += rgbints[i].red;
    }
    return rgbintnew;
}

RGBINT RGBINTcoefficient(double n, RGBINT rgbint)
{
    RGBINT rgbintnew;
    rgbintnew.blue = (int)(n * rgbint.blue);
    rgbintnew.green = (int)(n * rgbint.green);
    rgbintnew.red = (int)(n * rgbint.red);
    rgbintnew.reserved = 0;
    return rgbintnew;
}

RGBINT RGBcoefficentaddmodel(BMPINT *bmpint, int a, int b, double model[][3])
{
    RGBINT rgbintsquare[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            rgbintsquare[i][j] = RGBINTcoefficient(model[i][j], bmpint->data[a - 1 + i][b - 1 + j]);
        }
    }
    RGBINT rgbinttmp[9];
    int index = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            rgbinttmp[index] = rgbintsquare[i][j];
            index += 1;
        }
    }
    RGBINT rgbintnew = RGBINTaddarray(9, rgbinttmp);
    return rgbintnew;
}

BMPINT *BMPtoBMPINT(BMP *bmp)
{
    BMPINT *bmpintnew = initBMPINT(bmp->height, bmp->width, BLACK);
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            bmpintnew->data[i][j] = RGBtoRGBINT(bmp->data[i][j]);
        }
    }
    deleteBMP(bmp);
    return bmpintnew;
}

BMP *BMPINTtoBMP(BMPINT *bmpint)
{
    BMP *bmpnew = initBMP(bmpint->height, bmpint->width, BLACK);
    for (int i = 0; i < bmpint->height; i++)
    {
        for (int j = 0; j < bmpint->width; j++)
        {
            bmpnew->data[i][j] = RGBINTtoRGB(bmpint->data[i][j]);
        }
    }
    deleteBMPINT(bmpint);
    return bmpnew;
}

BMP *BMPcopy(BMP *bmp)
{
    BMP *bmpnew = initBMP(bmp->height, bmp->width, BLACK);
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            bmpnew->data[i][j] = bmp->data[i][j];
        }
    }
    return bmpnew;
}

BMP *BMPand(BMP *bmpa, BMP *bmpb)
{
    if (bmpa->height != bmpb->height || bmpa->width != bmpb->width)
    {
        printf("image size not equal\n");
    }
    BMP *bmpnew = initBMP(bmpa->height, bmpa->width, BLACK);
    for (int i = 0; i < bmpa->height; i++)
    {
        for (int j = 0; j < bmpa->width; j++)
        {
            bmpnew->data[i][j] = RGBand(bmpa->data[i][j], bmpb->data[i][j]);
        }
    }
    return bmpnew;
}

BMP *BMPor(BMP *bmpa, BMP *bmpb)
{
    if (bmpa->height != bmpb->height || bmpa->width != bmpb->width)
    {
        printf("image size not equal\n");
    }
    BMP *bmpnew = initBMP(bmpa->height, bmpa->width, BLACK);
    for (int i = 0; i < bmpa->height; i++)
    {
        for (int j = 0; j < bmpa->width; j++)
        {
            bmpnew->data[i][j] = RGBor(bmpa->data[i][j], bmpb->data[i][j]);
        }
    }
    return bmpnew;
}

BMP *BMPnot(BMP *bmp)
{
    BMP *bmpnew = initBMP(bmp->height, bmp->width, BLACK);
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            bmpnew->data[i][j] = RGBnot(bmp->data[i][j]);
        }
    }
    return bmpnew;
}

BMP *BMPheightenlarge(BMP *bmp, float heightcof)
{
    int newheight = (int)(heightcof * bmp->height);
    BMP *bmpnew = initBMP(newheight, bmp->width, GRAY127);
    for (int j = 0; j < bmp->width; j++)
    {
        for (int i = 0; i < bmp->height; i++)
        {
            for (int k = (int)(i * heightcof); k < (int)((i + 1) * heightcof); k++)
            {
                bmpnew->data[k][j] = bmp->data[i][j];
            }
        }
    }
    return bmpnew;
}

BMP *BMPwidthenlarge(BMP *bmp, float widthcof)
{
    int newwidth = (int)(widthcof * bmp->width);
    BMP *bmpnew = initBMP(bmp->height, newwidth, GRAY127);
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            for (int k = (int)(j * widthcof); k < (int)((j + 1) * widthcof); k++)
            {
                bmpnew->data[i][k] = bmp->data[i][j];
            }
        }
    }
    return bmpnew;
}

BMP *BMPenlarge(BMP *bmp, float heightcof, float widthcof)
{
    BMP *bmpheight = BMPheightenlarge(bmp, heightcof);
    BMP *bmpheightwidth = BMPwidthenlarge(bmpheight, widthcof);
    deleteBMP(bmpheight);
    return bmpheightwidth;
}

BMP *BMPshrink(BMP *bmp, float heightcof, float widthcof)
{
    int newheight, newwidth;
    newheight = (int)(heightcof * bmp->height);
    newwidth = (int)(widthcof * bmp->width);
    BMP *bmpnew = initBMP(newheight, newwidth, BLACK);
    for (int i = 0; i < newheight; i++)
    {
        for (int j = 0; j < newwidth; j++)
        {
            int inew = (int)(i / heightcof);
            if (inew > bmp->height - 1)
            {
                inew = bmp->height - 1;
            }
            int jnew = (int)(j / widthcof);
            if (jnew > bmp->width - 1)
            {
                jnew = bmp->width - 1;
            }
            bmpnew->data[i][j] = bmp->data[inew][jnew];
        }
    }
    return bmpnew;
}

BMP *BMPcut(BMP *bmp, int heightstart, int heightstop, int widthstart, int widthstop)
{
    int newheight, newwidth;
    newheight = heightstop - heightstart;
    newwidth = widthstop - widthstart;
    BMP *bmpnew = initBMP(newheight, newwidth, BLACK);
    for (int i = 0; i < newheight; i++)
    {
        for (int j = 0; j < newwidth; j++)
        {
            bmpnew->data[i][j] = bmp->data[i + heightstart][j + widthstart];
        }
    }
    return bmpnew;
}

BMP *BMPput(BMP *bmp, int largeheight, int largewidth, int heightstart, int widthstart, RGB defaultRGB)
{
    BMP *bmpnew = initBMP(largeheight, largewidth, defaultRGB);
    int tempi, tempj;
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            if (i + heightstart < largeheight && j + widthstart < largewidth)
            {
                bmpnew->data[i + heightstart][j + widthstart] = bmp->data[i][j];
            }
        }
    }
    return bmpnew;
}

BMP *BMPtogray(BMP *bmp)
{
    BMP *bmpnew = initBMP(bmp->height, bmp->width, BLACK);
    double gray;
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            gray = 0.299 * bmp->data[i][j].red + 0.587 * bmp->data[i][j].green + 0.114 * bmp->data[i][j].blue;
            bmpnew->data[i][j].red = (char)gray;
            bmpnew->data[i][j].green = (char)gray;
            bmpnew->data[i][j].blue = (char)gray;
        }
    }
    return bmpnew;
}

BMP *BMPreversecolor(BMP *bmp)
{
    BMP *bmpnew = initBMP(bmp->height, bmp->width, BLACK);
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            bmpnew->data[i][j].red = 255 - bmp->data[i][j].red;
            bmpnew->data[i][j].green = 255 - bmp->data[i][j].green;
            bmpnew->data[i][j].blue = 255 - bmp->data[i][j].blue;
        }
    }
    return bmpnew;
}

BMPINT *BMPINTcopy(BMPINT *bmpint)
{
    BMPINT *bmpintnew = initBMPINT(bmpint->height, bmpint->width, BLACK);
    for (int i = 0; i < bmpint->height; i++)
    {
        for (int j = 0; j < bmpint->width; j++)
        {
            bmpintnew->data[i][j] = bmpint->data[i][j];
        }
    }
    return bmpintnew;
}

BMPINT *BMPINTadd(BMPINT *bmpinta, BMPINT *bmpintb)
{
    if (bmpinta->height != bmpintb->height || bmpinta->width != bmpintb->width)
    {
        printf("image size not equal\n");
    }
    BMPINT *bmpintnew = initBMPINT(bmpinta->height, bmpinta->width, BLACK);
    for (int i = 0; i < bmpinta->height; i++)
    {
        for (int j = 0; j < bmpinta->width; j++)
        {
            bmpintnew->data[i][j] = RGBINTadd(bmpinta->data[i][j], bmpintb->data[i][j]);
        }
    }
    return bmpintnew;
}

BMPINT *BMPINTsub(BMPINT *bmpinta, BMPINT *bmpintb)
{
    if (bmpinta->height != bmpintb->height || bmpinta->width != bmpintb->width)
    {
        printf("image size not equal\n");
    }
    BMPINT *bmpintnew = initBMPINT(bmpinta->height, bmpinta->width, BLACK);
    for (int i = 0; i < bmpinta->height; i++)
    {
        for (int j = 0; j < bmpinta->width; j++)
        {
            bmpintnew->data[i][j] = RGBINTsub(bmpinta->data[i][j], bmpintb->data[i][j]);
        }
    }
    return bmpintnew;
}

BMPINT *BMPINTmultiply(BMPINT *bmpinta, BMPINT *bmpintb)
{
    if (bmpinta->height != bmpintb->height || bmpinta->width != bmpintb->width)
    {
        printf("image size not equal\n");
    }
    BMPINT *bmpintnew = initBMPINT(bmpinta->height, bmpinta->width, BLACK);
    for (int i = 0; i < bmpinta->height; i++)
    {
        for (int j = 0; j < bmpinta->width; j++)
        {
            bmpintnew->data[i][j] = RGBINTmultipy(bmpinta->data[i][j], bmpintb->data[i][j]);
        }
    }
    return bmpintnew;
}

BMPINT *BMPINTdivide(BMPINT *bmpinta, BMPINT *bmpintb, RGB defaultRGB)
{
    if (bmpinta->height != bmpintb->height || bmpinta->width != bmpintb->width)
    {
        printf("image size not equal\n");
    }
    BMPINT *bmpintnew = initBMPINT(bmpinta->height, bmpinta->width, BLACK);
    for (int i = 0; i < bmpinta->height; i++)
    {
        for (int j = 0; j < bmpinta->width; j++)
        {
            bmpintnew->data[i][j] = RGBINTdivide(bmpinta->data[i][j], bmpintb->data[i][j], defaultRGB);
        }
    }
    return bmpintnew;
}

char **initcmodel(int height, int width, int initial)
{
    char **a = (char **)malloc(sizeof(char *) * height);
    for (int i = 0; i < height; i++)
    {
        a[i] = (char *)malloc(width);
        memset(a[i], initial, width);
    }
    return a;
}

double **initdmodel(int height, int width, double initial)
{
    double **a = (double **)malloc(sizeof(double *) * height);
    for (int i = 0; i < height; i++)
    {
        a[i] = (double *)malloc(width);
        for (int j = 0; j < width; j++)
        {
            a[i][j] = initial;
        }
    }
    return a;
}

void deletecmodel(char **model, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        if (model[i])
        {
            free(model[i]);
        }
    }
    if (model)
    {
        free(model);
    }
}

void deletedmodel(double **model, int height, int width)
{
    for (int i = 0; i < height; i++)
    {
        if (model[i])
        {
            free(model[i]);
        }
    }
    if (model)
    {
        free(model);
    }
}

double contrast(BMP *bmpgray, BOOL flag)
{
    int height = bmpgray->height;
    int width = bmpgray->width;
    double contrast_sum = 0;
    int tmp0 = 2, tmp1 = 3, tmp2 = 4;
    if (flag)
    {
        tmp0 = 3;
        tmp1 = 5;
        tmp2 = 8;
    }
    int num = 4 * tmp0 + ((width - 2) + (height - 2)) * 2 * tmp1 + ((width - 2) * (height - 2)) * tmp2;
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (i > 0)
            {
                contrast_sum += pow((bmpgray->data[i - 1][j].blue - bmpgray->data[i][j].blue), 2.0);
                if (flag)
                {
                    if (j > 0)
                    {
                        contrast_sum += pow((bmpgray->data[i - 1][j - 1].blue - bmpgray->data[i][j].blue), 2.0);
                    }
                    if (j < width - 1)
                    {
                        contrast_sum += pow((bmpgray->data[i - 1][j + 1].blue - bmpgray->data[i][j].blue), 2.0);
                    }
                }
            }
            if (i < height - 1)
            {
                contrast_sum += pow((bmpgray->data[i + 1][j].blue - bmpgray->data[i][j].blue), 2.0);
                if (flag)
                {
                    if (j > 0)
                    {
                        contrast_sum += pow((bmpgray->data[i + 1][j - 1].blue - bmpgray->data[i][j].blue), 2.0);
                    }
                    if (j < width - 1)
                    {
                        contrast_sum += pow((bmpgray->data[i + 1][j + 1].blue - bmpgray->data[i][j].blue), 2.0);
                    }
                }
            }
            if (j > 0)
            {
                contrast_sum += pow((bmpgray->data[i][j - 1].blue - bmpgray->data[i][j].blue), 2.0);
            }
            if (j < width - 1)
            {
                contrast_sum += pow((bmpgray->data[i][j + 1].blue - bmpgray->data[i][j].blue), 2.0);
            }
        }
    }
    return contrast_sum / num;
}

BMP *averagefilter(BMP *bmp, char **model)
{
    BMP *dstbmp = BMPcopy(bmp);
    for (int i = 1; i < bmp->height - 1; i++)
    {
        for (int j = 1; j < bmp->width - 1; j++)
        {
            if (model[i][j] == 1)
            {
                RGB collects[8] = {bmp->data[i - 1][j - 1],
                                   bmp->data[i - 1][j],
                                   bmp->data[i - 1][j + 1],
                                   bmp->data[i][j - 1],
                                   bmp->data[i][j + 1],
                                   bmp->data[i + 1][j - 1],
                                   bmp->data[i + 1][j],
                                   bmp->data[i + 1][j + 1]};
                RGB average = RGBaverage(8, collects);
                dstbmp->data[i][j] = average;
            }
        }
    }
    return dstbmp;
}

BMP *sharpen(BMP *bmpgray, double model[][3])
{
    BMPINT *dstbmpint = BMPtoBMPINT(bmpgray);
    BMPINT *bmpintbak = BMPINTcopy(dstbmpint);
    int minblue = 0;
    for (int i = 1; i < bmpgray->height - 1; i++)
    {
        for (int j = 1; j < bmpgray->width - 1; j++)
        {
            dstbmpint->data[i][j] = RGBcoefficentaddmodel(bmpintbak, i, j, model);
            if (dstbmpint->data[i][j].blue < minblue)
            {
                minblue = dstbmpint->data[i][j].blue;
            }
        }
    }
    deleteBMPINT(bmpintbak);
    if (minblue < 0)
    {
        for (int i = 0; i < bmpgray->height; i++)
        {
            for (int j = 0; j < bmpgray->width; j++)
            {
                dstbmpint->data[i][j].blue -= minblue;
                dstbmpint->data[i][j].red = dstbmpint->data[i][j].blue;
                dstbmpint->data[i][j].green = dstbmpint->data[i][j].blue;
            }
        }
    }
    BMP *bmpnew = BMPINTtoBMP(dstbmpint);
    return bmpnew;
}

BMP *horizonsharpen(BMP *bmpgray)
{
    double model[][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    BMP *bmpnew = sharpen(bmpgray, model);
    return bmpnew;
}

BMP *verticalsharpen(BMP *bmpgray)
{
    double model[][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
    BMP *bmpnew = sharpen(bmpgray, model);
    return bmpnew;
}

void graylevelcount(BMP *bmpgray, int *graylevelnum)
{
    memset(graylevelnum, 0, sizeof(int) * 256);
    for (int i = 0; i < bmpgray->height; i++)
    {
        for (int j = 0; j < bmpgray->width; j++)
        {
            graylevelnum[bmpgray->data[i][j].blue]++;
        }
    }
}

BMP *pparameterbinaryzation(BMP *bmpgray, int *levelnum, double p)
{
    int num = (int)(p * bmpgray->height * bmpgray->width);
    int count = 0;
    int level = 0;
    while (count < num && level < 256)
    {
        count += levelnum[level];
        level++;
    }
    BMP *bmpnew = initBMP(bmpgray->height, bmpgray->width, GRAY127);
    for (int i = 0; i < bmpgray->height; i++)
    {
        for (int j = 0; j < bmpgray->width; j++)
        {
            if (bmpgray->data[i][j].blue < level)
            {
                bmpnew->data[i][j] = BLACK;
            }
            else
            {
                bmpnew->data[i][j] = WHITE;
            }
        }
    }
    return bmpnew;
}