#include "dipcode.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

RGBQUAD BLACK = {0, 0, 0, 0};
RGBQUAD WHITE = {255, 255, 255, 0};
RGBQUAD GRAY127 = {127, 127, 127, 0};

BMP *readBMP(char *strFile)
{
    BITMAPFILEHEADER bitHead;
    BITMAPINFOHEADER bitInfoHead;
    BMP *bmp = (BMP *)malloc(sizeof(BMP));
    int i, j;
    FILE *pfile;
    pfile = fopen(strFile, "rb");
    if (pfile != NULL)
    {
        printf("file %s open success.\n", strFile);
        fread(&bitHead, 1, sizeof(BITMAPFILEHEADER), pfile);
        if (bitHead.bfType != 0x4d42)
        {
            printf("file is not .bmp file!");
            return NULL;
        }
        if (SHOW)
        {
            showBmpHead(&bitHead);
            printf("\n");
        }
        fread(&bitInfoHead, 1, sizeof(BITMAPINFOHEADER), pfile);
        if (SHOW)
        {
            showBmpInforHead(&bitInfoHead);
            printf("\n");
        }
    }
    else
    {
        printf("file open fail!\n");
        return NULL;
    }
    RGBQUAD *pRgb = NULL;
    //有调色板
    if (bitInfoHead.biBitCount < 24)
    {
        long nPlantNum = bitInfoHead.biClrUsed;
        if (!nPlantNum)
        {
            nPlantNum = pow(2, bitInfoHead.biBitCount);
        }
        pRgb = (RGBQUAD *)malloc(sizeof(RGBQUAD) * nPlantNum);
        memset(pRgb, 0, nPlantNum * sizeof(RGBQUAD));
        int num = fread(pRgb, 4, nPlantNum, pfile);
        if (SHOW)
        {
            printf("Color Plate Number: %d\n", nPlantNum);
            printf("颜色板信息:\n");
            showRgbQuan(pRgb, nPlantNum);
        }
    }
    int width = bitInfoHead.biWidth;
    int height = bitInfoHead.biHeight;
    //分配内存空间把源图存入内存
    int l_width = WIDTHcharS(width * bitInfoHead.biBitCount);
    //计算位图的实际宽度并确保它为32的倍数
    long nData = height * l_width;
    char *pColorData = (char *)malloc(nData);
    memset(pColorData, 0, nData);
    //把位图数据信息读到数组里
    fread(pColorData, 1, nData, pfile);
    //将位图数据转化为RGB数据
    //用于保存各像素对应的RGB数据
    RGBQUAD **dataOfBmp_src = NULL;
    dataOfBmp_src = (RGBQUAD **)malloc(sizeof(RGBQUAD *) * height);
    for (i = 0; i < height; i++)
    {
        dataOfBmp_src[i] = (RGBQUAD *)malloc(sizeof(RGBQUAD) * width);
    }
    //有调色板，即位图为非真彩色
    if (bitInfoHead.biBitCount < 24)
    {
        int k;
        if (bitInfoHead.biBitCount <= 8 && !bitInfoHead.biCompression)
        {
            int pnum = 8 / bitInfoHead.biBitCount;
            int mbnum = 8 - bitInfoHead.biBitCount;
            for (int i = 0; i < height; i++)
            {
                //k:取得该像素颜色数据在实际数据数组中的序号
                int k0 = (height - i - 1) * l_width;
                for (int j = 0; j < width; j++)
                {
                    char mixIndex = 0;
                    k = k0 + (j / pnum);
                    //mixIndex:提取当前像素的颜色的在颜色表中的索引值
                    mixIndex = pColorData[k];
                    if (bitInfoHead.biBitCount < 8)
                    {
                        mixIndex = mixIndex << ((j % pnum) * bitInfoHead.biBitCount);
                        mixIndex = mixIndex >> mbnum;
                    }
                    //将像素颜色数据（RGBA）保存到数组中对应的位置
                    dataOfBmp_src[i][j].rgbRed = pRgb[mixIndex].rgbRed;
                    dataOfBmp_src[i][j].rgbGreen = pRgb[mixIndex].rgbGreen;
                    dataOfBmp_src[i][j].rgbBlue = pRgb[mixIndex].rgbBlue;
                    dataOfBmp_src[i][j].rgbReserved = pRgb[mixIndex].rgbReserved;
                }
            }
        }
        if (bitInfoHead.biBitCount == 16)
        {
            if (!bitInfoHead.biCompression)
            {
                for (i = 0; i < height; i++)
                {
                    int k0 = (height - i - 1) * l_width;
                    for (j = 0; j < width; j++)
                    {
                        short mixIndex = 0;
                        k = k0 + j * 2;
                        short shortTemp;
                        shortTemp = pColorData[k + 1];
                        shortTemp = shortTemp << 8;
                        mixIndex = pColorData[k] + shortTemp;
                        dataOfBmp_src[i][j].rgbRed = pRgb[mixIndex].rgbRed;
                        dataOfBmp_src[i][j].rgbGreen = pRgb[mixIndex].rgbGreen;
                        dataOfBmp_src[i][j].rgbBlue = pRgb[mixIndex].rgbBlue;
                        dataOfBmp_src[i][j].rgbReserved = pRgb[mixIndex].rgbReserved;
                    }
                }
            }
        }
    }
    //位图为24/32位真彩色
    else
    {
        int k;
        int index = 0;
        if (bitInfoHead.biBitCount == 16)
        {
            for (i = 0; i < height; i++)
            {
                int k0 = (height - i - 1) * l_width;
                for (j = 0; j < width; j++)
                {
                    k = k0 + j * 2;
                    //555格式
                    if (!bitInfoHead.biCompression)
                    {
                        dataOfBmp_src[i][j].rgbBlue = pColorData[k] & 0x1F;
                        dataOfBmp_src[i][j].rgbGreen = (((pColorData[k + 1] << 6) & 0xFF) >> 3) + (pColorData[k] >> 5);
                        dataOfBmp_src[i][j].rgbRed = (pColorData[k + 1] << 1) >> 3;
                        dataOfBmp_src[i][j].rgbReserved = 0;
                    }
                }
            }
        }
        if (bitInfoHead.biBitCount == 24 && !bitInfoHead.biCompression)
        {
            for (i = 0; i < height; i++)
            {
                int k0 = (height - i - 1) * l_width;
                for (j = 0; j < width; j++)
                {
                    k = k0 + (j * 3);
                    dataOfBmp_src[i][j].rgbRed = pColorData[k + 2];
                    dataOfBmp_src[i][j].rgbGreen = pColorData[k + 1];
                    dataOfBmp_src[i][j].rgbBlue = pColorData[k];
                    dataOfBmp_src[i][j].rgbReserved = 0;
                }
            }
        }
        if (bitInfoHead.biBitCount == 32 && !bitInfoHead.biCompression)
        {
            for (i = 0; i < height; i++)
            {
                int k0 = (height - i - 1) * l_width;
                for (j = 0; j < width; j++)
                {
                    k = k0 + (j * 4);
                    dataOfBmp_src[i][j].rgbRed = pColorData[k + 2];
                    dataOfBmp_src[i][j].rgbGreen = pColorData[k + 1];
                    dataOfBmp_src[i][j].rgbBlue = pColorData[k];
                    dataOfBmp_src[i][j].rgbReserved = pColorData[k + 3];
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
    bmp->data = dataOfBmp_src;
    bmp->width = width;
    bmp->height = height;
    return bmp;
}

void saveBMP(BMP *bmp, char *strFile)
{
    RGBQUAD **dataOfBmp = bmp->data;
    int width = bmp->width;
    int height = bmp->height;
    int i, j;
    BITMAPFILEHEADER bitHead;
    BITMAPINFOHEADER bitInfoHead;
    short biBitCount = 32;
    FILE *pfile;
    pfile = fopen(strFile, "wb");
    if (pfile != NULL)
    {
        bitHead.bfType = 0x4d42;
        bitHead.bfSize = 0;
        bitHead.bfReserved1 = 0;
        bitHead.bfReserved2 = 0;
        bitHead.bfOffBits = 54;
        if (biBitCount <= 8)
        {
            bitHead.bfOffBits += (int)pow(2, biBitCount) * 4;
        }
        fwrite(&bitHead, 1, sizeof(BITMAPFILEHEADER), pfile);
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
        fwrite(&bitInfoHead, 1, sizeof(BITMAPINFOHEADER), pfile);
        if (biBitCount <= 8)
        {
            char tmp = 0;
            for (i = 0; i < (int)pow(2, biBitCount); i++)
            {
                tmp = (char)i;
                fwrite(&tmp, 1, 4, pfile);
            }
        }
        //计算为确保位图数据区的实际宽度为32字节的整数倍需添加的0字节个数
        int l_width = WIDTHcharS(width * biBitCount) - width * 4;
        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j += 1)
            {
                fwrite(&dataOfBmp[height - i - 1][j], 1, 4, pfile);
            }
            char tmp = 0;
            for (j = 0; j < l_width; j++)
            {
                fwrite(&tmp, 1, 1, pfile);
            }
        }
    }
    fclose(pfile);
    printf("file %s save success.\n", strFile);
}

BMP *initBMP(int height, int width, RGBQUAD defaultRGB)
{
    BMP *bmpnew = (BMP *)malloc(sizeof(BMP));
    RGBQUAD **data = (RGBQUAD **)malloc(sizeof(RGBQUAD *) * height);
    for (int i = 0; i < height; i++)
    {
        data[i] = (RGBQUAD *)malloc(sizeof(RGBQUAD) * width);
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

BMPINT *initBMPINT(int height, int width, RGBQUAD defaultRGB)
{
    BMPINT *bmpnew = (BMPINT *)malloc(sizeof(BMPINT));
    RGBQUADINT **data = (RGBQUADINT **)malloc(sizeof(RGBQUADINT *) * height);
    for (int i = 0; i < height; i++)
    {
        data[i] = (RGBQUADINT *)malloc(sizeof(RGBQUADINT) * width);
        for (int j = 0; j < width; j++)
        {
            data[i][j] = RGBtoRGBINT(defaultRGB);
        }
    }
    bmpnew->data = data;
    bmpnew->height = height;
    bmpnew->width = width;
    return bmpnew;
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

void deleteBMPINT(BMPINT *bmp)
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

void showRgbQuan(RGBQUAD *pRGB, int num)
{
    for (int i = 0; i < num; i++)
    {
        if (i % 5 == 0)
        {
            printf("\n");
        }
        printf("(%-3d,%-3d,%-3d)   ", (pRGB + i)->rgbRed, (pRGB + i)->rgbGreen, (pRGB + i)->rgbBlue);
    }
    printf("\n");
}

double contrast(char **dataOfBmp_gray, int width, int height, BOOL flag)
{
    int i, j;
    double contrast_sum = 0;
    int tmp0 = 2, tmp1 = 3, tmp2 = 4;
    int num = 0;
    if (flag)
    {
        tmp0 = 3;
        tmp1 = 5;
        tmp2 = 8;
    }
    num = 4 * tmp0 + ((width - 2) + (height - 2)) * 2 * tmp1 + ((width - 2) * (height - 2)) * tmp2;
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            if (i > 0)
            {
                contrast_sum += pow((dataOfBmp_gray[i - 1][j] - dataOfBmp_gray[i][j]), 2.0);
                if (flag)
                {
                    if (j > 0)
                    {
                        contrast_sum += pow((dataOfBmp_gray[i - 1][j - 1] - dataOfBmp_gray[i][j]), 2.0);
                    }
                    if (j < width - 1)
                    {
                        contrast_sum += pow((dataOfBmp_gray[i - 1][j + 1] - dataOfBmp_gray[i][j]), 2.0);
                    }
                }
            }
            if (i < height - 1)
            {
                contrast_sum += pow((dataOfBmp_gray[i + 1][j] - dataOfBmp_gray[i][j]), 2.0);
                if (flag)
                {
                    if (j > 0)
                    {
                        contrast_sum += pow((dataOfBmp_gray[i + 1][j - 1] - dataOfBmp_gray[i][j]), 2.0);
                    }
                    if (j < width - 1)
                    {
                        contrast_sum += pow((dataOfBmp_gray[i + 1][j + 1] - dataOfBmp_gray[i][j]), 2.0);
                    }
                }
            }
            if (j > 0)
            {
                contrast_sum += pow((dataOfBmp_gray[i][j - 1] - dataOfBmp_gray[i][j]), 2.0);
            }
            if (j < width - 1)
            {
                contrast_sum += pow((dataOfBmp_gray[i][j + 1] - dataOfBmp_gray[i][j]), 2.0);
            }
        }
    }
    return contrast_sum / num;
}

RGBQUADINT RGBtoRGBINT(RGBQUAD rgb)
{
    RGBQUADINT rgbnew;
    rgbnew.rgbBlue = rgb.rgbBlue;
    rgbnew.rgbGreen = rgb.rgbGreen;
    rgbnew.rgbRed = rgb.rgbRed;
    rgbnew.rgbReserved = rgb.rgbReserved;
    return rgbnew;
}

RGBQUAD RGBINTtoRGB(RGBQUADINT rgb)
{
    RGBQUAD rgbnew;
    if (rgb.rgbBlue > 255)
    {
        rgbnew.rgbBlue = 255;
    }
    else if (rgb.rgbBlue < 0)
    {
        rgbnew.rgbBlue = 0;
    }
    else
    {
        rgbnew.rgbBlue = rgb.rgbBlue;
    }

    if (rgb.rgbGreen > 255)
    {
        rgbnew.rgbGreen = 255;
    }
    else if (rgb.rgbGreen < 0)
    {
        rgbnew.rgbGreen = 0;
    }
    else
    {
        rgbnew.rgbGreen = rgb.rgbGreen;
    }

    if (rgb.rgbRed > 255)
    {
        rgbnew.rgbRed = 255;
    }
    else if (rgb.rgbRed < 0)
    {
        rgbnew.rgbRed = 0;
    }
    else
    {
        rgbnew.rgbRed = rgb.rgbRed;
    }
    rgbnew.rgbReserved = rgb.rgbReserved;
    return rgbnew;
}

RGBQUAD RGBand(RGBQUAD rgb1, RGBQUAD rgb2)
{
    RGBQUAD rgbnew;
    rgbnew.rgbBlue = rgb1.rgbBlue & rgb2.rgbBlue;
    rgbnew.rgbGreen = rgb1.rgbGreen & rgb2.rgbGreen;
    rgbnew.rgbRed = rgb1.rgbRed & rgb2.rgbRed;
    rgbnew.rgbReserved = 0;
    return rgbnew;
}

RGBQUAD RGBor(RGBQUAD rgb1, RGBQUAD rgb2)
{
    RGBQUAD rgbnew;
    rgbnew.rgbBlue = rgb1.rgbBlue | rgb2.rgbBlue;
    rgbnew.rgbGreen = rgb1.rgbGreen | rgb2.rgbGreen;
    rgbnew.rgbRed = rgb1.rgbRed | rgb2.rgbRed;
    rgbnew.rgbReserved = 0;
    return rgbnew;
}

RGBQUAD RGBnot(RGBQUAD rgb)
{
    RGBQUAD rgbnew;
    rgbnew.rgbBlue = ~rgb.rgbBlue;
    rgbnew.rgbGreen = ~rgb.rgbGreen;
    rgbnew.rgbRed = ~rgb.rgbRed;
    rgbnew.rgbReserved = 0;
    return rgbnew;
}

RGBQUAD RGBaverage(int n, RGBQUAD rgbs[])
{
    RGBQUAD rgbnew;
    int tempb = 0;
    int tempg = 0;
    int tempr = 0;
    for (int i = 0; i < n; i++)
    {
        tempb += rgbs[i].rgbBlue;
        tempg += rgbs[i].rgbGreen;
        tempr += rgbs[i].rgbRed;
    }
    rgbnew.rgbBlue = (uchar)(tempb / n);
    rgbnew.rgbGreen = (uchar)(tempg / n);
    rgbnew.rgbRed = (uchar)(tempr / n);
    rgbnew.rgbReserved = 0;
    return rgbnew;
}

RGBQUAD RGBQUADget(BMP *bmp, int i, int j, RGBQUAD defaultRGB, int *flag)
{
    RGBQUAD rgbgot;
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

RGBQUADINT RGBINTadd(RGBQUADINT rgb1, RGBQUADINT rgb2)
{
    RGBQUADINT rgbnew;
    rgbnew.rgbBlue = rgb1.rgbBlue + rgb2.rgbBlue;
    rgbnew.rgbGreen = rgb1.rgbGreen + rgb2.rgbGreen;
    rgbnew.rgbRed = rgb1.rgbRed + rgb2.rgbRed;
    rgbnew.rgbReserved = 0;
    return rgbnew;
}

RGBQUADINT RGBINTsub(RGBQUADINT rgb1, RGBQUADINT rgb2)
{
    RGBQUADINT rgbnew;
    rgbnew.rgbBlue = rgb1.rgbBlue - rgb2.rgbBlue;
    rgbnew.rgbGreen = rgb1.rgbGreen - rgb2.rgbGreen;
    rgbnew.rgbRed = rgb1.rgbRed - rgb2.rgbRed;
    rgbnew.rgbReserved = 0;
    return rgbnew;
}

RGBQUADINT RGBINTmultipy(RGBQUADINT rgb1, RGBQUADINT rgb2)
{
    RGBQUADINT rgbnew;
    rgbnew.rgbBlue = rgb1.rgbBlue * rgb2.rgbBlue;
    rgbnew.rgbGreen = rgb1.rgbGreen * rgb2.rgbGreen;
    rgbnew.rgbRed = rgb1.rgbRed * rgb2.rgbRed;
    rgbnew.rgbReserved = 0;
    return rgbnew;
}

RGBQUADINT RGBINTdivide(RGBQUADINT rgb1, RGBQUADINT rgb2, RGBQUAD defaultRGB)
{
    RGBQUADINT rgbnew;
    if (rgb2.rgbBlue != 0)
    {
        rgbnew.rgbBlue = rgb1.rgbBlue / rgb2.rgbBlue;
    }
    else
    {
        rgbnew.rgbBlue = defaultRGB.rgbBlue;
    }
    if (rgb2.rgbGreen != 0)
    {
        rgbnew.rgbGreen = rgb1.rgbGreen / rgb2.rgbGreen;
    }
    else
    {
        rgbnew.rgbGreen = defaultRGB.rgbGreen;
    }
    if (rgb2.rgbRed != 0)
    {
        rgbnew.rgbRed = rgb1.rgbRed / rgb2.rgbRed;
    }
    else
    {
        rgbnew.rgbRed = defaultRGB.rgbRed;
    }
    rgbnew.rgbReserved = 0;
    return rgbnew;
}

RGBQUADINT RGBINTaddarray(int n, RGBQUADINT rgbs[])
{
    RGBQUADINT rgbnew;
    rgbnew.rgbBlue = 0;
    rgbnew.rgbGreen = 0;
    rgbnew.rgbRed = 0;
    rgbnew.rgbReserved = 0;
    for (int i = 0; i < n; i++)
    {
        rgbnew.rgbBlue += rgbs[i].rgbBlue;
        rgbnew.rgbGreen += rgbs[i].rgbGreen;
        rgbnew.rgbRed += rgbs[i].rgbRed;
    }
    return rgbnew;
}

RGBQUADINT RGBQUADINTcoefficient(double n, RGBQUADINT rgb)
{
    RGBQUADINT rgbnew;
    rgbnew.rgbBlue = (int)(n * rgb.rgbBlue);
    rgbnew.rgbGreen = (int)(n * rgb.rgbGreen);
    rgbnew.rgbRed = (int)(n * rgb.rgbRed);
    rgbnew.rgbReserved = 0;
    return rgbnew;
}

RGBQUADINT RGBcoefficentaddmodel(BMPINT *bmp, int a, int b, double model[][3])
{
    RGBQUADINT rgbsquare[3][3];
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            rgbsquare[i][j] = RGBQUADINTcoefficient(model[i][j], bmp->data[a - 1 + i][b - 1 + j]);
        }
    }
    RGBQUADINT rgbtmp[9];
    int index = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            rgbtmp[index] = rgbsquare[i][j];
            index += 1;
        }
    }
    RGBQUADINT rgbnew = RGBINTaddarray(9, rgbtmp);
    return rgbnew;
}

BMPINT *BMPtoBMPINT(BMP *bmp)
{
    BMPINT *bmpnew = initBMPINT(bmp->height, bmp->width, BLACK);
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            bmpnew->data[i][j] = RGBtoRGBINT(bmp->data[i][j]);
        }
    }
    deleteBMP(bmp);
    return bmpnew;
}

BMP *BMPINTtoBMP(BMPINT *bmp)
{
    BMP *bmpnew = initBMP(bmp->height, bmp->width, BLACK);
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            bmpnew->data[i][j] = RGBINTtoRGB(bmp->data[i][j]);
        }
    }
    deleteBMPINT(bmp);
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
    BMP* bmpheight=BMPheightenlarge(bmp,heightcof);
    BMP* bmpheightwidth=BMPwidthenlarge(bmpheight,widthcof);
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

BMP *BMPput(BMP *bmp, int largeheight, int largewidth, int heightstart, int widthstart, RGBQUAD defaultRGB)
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
            gray = 0.299 * bmp->data[i][j].rgbRed + 0.587 * bmp->data[i][j].rgbGreen + 0.114 * bmp->data[i][j].rgbBlue;
            bmpnew->data[i][j].rgbRed = (char)gray;
            bmpnew->data[i][j].rgbGreen = (char)gray;
            bmpnew->data[i][j].rgbBlue = (char)gray;
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
            bmpnew->data[i][j].rgbRed = 255 - bmp->data[i][j].rgbRed;
            bmpnew->data[i][j].rgbGreen = 255 - bmp->data[i][j].rgbGreen;
            bmpnew->data[i][j].rgbBlue = 255 - bmp->data[i][j].rgbBlue;
        }
    }
    return bmpnew;
}

BMPINT *BMPINTcopy(BMPINT *bmp)
{
    BMPINT *bmpnew = initBMPINT(bmp->height, bmp->width, BLACK);
    for (int i = 0; i < bmp->height; i++)
    {
        for (int j = 0; j < bmp->width; j++)
        {
            bmpnew->data[i][j] = bmp->data[i][j];
        }
    }
    return bmpnew;
}

BMPINT *BMPINTadd(BMPINT *bmpa, BMPINT *bmpb)
{
    if (bmpa->height != bmpb->height || bmpa->width != bmpb->width)
    {
        printf("image size not equal\n");
    }
    BMPINT *bmpnew = initBMPINT(bmpa->height, bmpa->width, BLACK);
    for (int i = 0; i < bmpa->height; i++)
    {
        for (int j = 0; j < bmpa->width; j++)
        {
            bmpnew->data[i][j] = RGBINTadd(bmpa->data[i][j], bmpb->data[i][j]);
        }
    }
    return bmpnew;
}

BMPINT *BMPINTsub(BMPINT *bmpa, BMPINT *bmpb)
{
    if (bmpa->height != bmpb->height || bmpa->width != bmpb->width)
    {
        printf("image size not equal\n");
    }
    BMPINT *bmpnew = initBMPINT(bmpa->height, bmpa->width, BLACK);
    for (int i = 0; i < bmpa->height; i++)
    {
        for (int j = 0; j < bmpa->width; j++)
        {
            bmpnew->data[i][j] = RGBINTsub(bmpa->data[i][j], bmpb->data[i][j]);
        }
    }
    return bmpnew;
}

BMPINT *BMPINTmultiply(BMPINT *bmpa, BMPINT *bmpb)
{
    if (bmpa->height != bmpb->height || bmpa->width != bmpb->width)
    {
        printf("image size not equal\n");
    }
    BMPINT *bmpnew = initBMPINT(bmpa->height, bmpa->width, BLACK);
    for (int i = 0; i < bmpa->height; i++)
    {
        for (int j = 0; j < bmpa->width; j++)
        {
            bmpnew->data[i][j] = RGBINTmultipy(bmpa->data[i][j], bmpb->data[i][j]);
        }
    }
    return bmpnew;
}

BMPINT *BMPINTdivide(BMPINT *bmpa, BMPINT *bmpb, RGBQUAD defaultRGB)
{
    if (bmpa->height != bmpb->height || bmpa->width != bmpb->width)
    {
        printf("image size not equal\n");
    }
    BMPINT *bmpnew = initBMPINT(bmpa->height, bmpa->width, BLACK);
    for (int i = 0; i < bmpa->height; i++)
    {
        for (int j = 0; j < bmpa->width; j++)
        {
            bmpnew->data[i][j] = RGBINTdivide(bmpa->data[i][j], bmpb->data[i][j], defaultRGB);
        }
    }
    return bmpnew;
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

BMP *averagefilter(BMP *bmp, char **model)
{
    BMP *dstbmp = BMPcopy(bmp);
    for (int i = 1; i < bmp->height - 1; i++)
    {
        for (int j = 1; j < bmp->width - 1; j++)
        {
            if (model[i][j] == 1)
            {
                RGBQUAD collects[8] = {bmp->data[i - 1][j - 1],
                                       bmp->data[i - 1][j],
                                       bmp->data[i - 1][j + 1],
                                       bmp->data[i][j - 1],
                                       bmp->data[i][j + 1],
                                       bmp->data[i + 1][j - 1],
                                       bmp->data[i + 1][j],
                                       bmp->data[i + 1][j + 1]};
                RGBQUAD average = RGBaverage(8, collects);
                dstbmp->data[i][j] = average;
            }
        }
    }
    return dstbmp;
}

BMP *sharpen(BMP *bmp, double model[][3])
{
    BMP *bmpgray = BMPtogray(bmp);
    BMPINT *dstbmp = BMPtoBMPINT(bmpgray);
    BMPINT *bmpunchanged = BMPINTcopy(dstbmp);
    int minblue = 0;
    for (int i = 1; i < bmp->height - 1; i++)
    {
        for (int j = 1; j < bmp->width - 1; j++)
        {
            dstbmp->data[i][j] = RGBcoefficentaddmodel(bmpunchanged, i, j, model);
            if (dstbmp->data[i][j].rgbBlue < minblue)
            {
                minblue = dstbmp->data[i][j].rgbBlue;
            }
        }
    }
    deleteBMPINT(bmpunchanged);
    if (minblue < 0)
    {
        for (int i = 0; i < bmp->height; i++)
        {
            for (int j = 0; j < bmp->width; j++)
            {
                dstbmp->data[i][j].rgbBlue -= minblue;
                dstbmp->data[i][j].rgbRed = dstbmp->data[i][j].rgbBlue;
                dstbmp->data[i][j].rgbGreen = dstbmp->data[i][j].rgbBlue;
            }
        }
    }
    bmpgray = BMPINTtoBMP(dstbmp);
    return bmpgray;
}

BMP *horizonsharpen(BMP *bmp)
{
    double model[][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    BMP *bmpnew = sharpen(bmp, model);
    return bmpnew;
}

BMP *verticalsharpen(BMP *bmp)
{
    double model[][3] = {{1, 0, -1}, {2, 0, -2}, {1, 0, -1}};
    BMP *bmpnew = sharpen(bmp, model);
    return bmpnew;
}