/// bmp I/O
/// \author aa11285
/// \date 2018-06-21

#pragma once

#include "Windows.h"
#include <stdio.h>

// from DIB


///
/// \brief      write bmp file
///
/// \param[in]  filename  The filename
/// \param[in]  imgBuf    The image buffer
/// \param[in]  width     The width
/// \param[in]  height    The height
/// \param[in]  nBits     The bit depth (grayscale 8 or RGB 24)
///
/// \return     0 error, 1 okay
///
int savebmp(const char *filename, const unsigned char *imgBuf, int width, int height, int nBits /*= 8*/)
{
	if (!filename || !imgBuf || !(nBits == 8 || nBits == 24)) {
		return 0;
	}

	int nPaletteSize = 0;
	if (nBits == 8) {
		nPaletteSize = 256;
	}

	//unsigned long alignedWidth = (width * nBits + 31) / 32 * 4;
	int alignedWidth = ((width * nBits) / 8 + 3) / 4 * 4;

	unsigned long dibLength = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
		sizeof(RGBQUAD) * nPaletteSize + alignedWidth * height;

	unsigned char* dibBuf = (unsigned char*)malloc(dibLength);
	LPBITMAPFILEHEADER filehead = (LPBITMAPFILEHEADER)dibBuf;
	LPBITMAPINFOHEADER infohead = (LPBITMAPINFOHEADER)(dibBuf + sizeof(BITMAPFILEHEADER));

	infohead->biSize = sizeof(BITMAPINFOHEADER);
	infohead->biWidth = width;
	infohead->biHeight = height;
	infohead->biPlanes = 1;
	infohead->biBitCount = nBits;
	infohead->biCompression = BI_RGB;
	infohead->biSizeImage = 0;
	infohead->biXPelsPerMeter = 0;
	infohead->biYPelsPerMeter = 0;
	infohead->biClrUsed = 0;
	infohead->biClrImportant = 0;

	unsigned char* ptrData = (unsigned char*)infohead + sizeof(BITMAPINFOHEADER) +
		sizeof(RGBQUAD) * nPaletteSize;

	filehead->bfType = 0x4d42;	//'BM'
	filehead->bfSize = dibLength;
	filehead->bfReserved1 = 0;
	filehead->bfReserved2 = 0;
	filehead->bfOffBits = (unsigned char*)ptrData - dibBuf;

	// Palette
	unsigned char* ptrPalette = (unsigned char*)infohead + sizeof(BITMAPINFOHEADER);
	if (nBits == 8) {
		for (int i = 0; i < nPaletteSize; i++) {
			ptrPalette[4 * i] = (unsigned char)i;
			ptrPalette[4 * i + 1] = (unsigned char)i;
			ptrPalette[4 * i + 2] = (unsigned char)i;
			ptrPalette[4 * i + 3] = (unsigned char)i;
		}
	}

	memset(ptrData, 0, alignedWidth*height);
	int bufferWidth = width * nBits / 8;

	int i = 0, j = 0, k = 0;
	for (i = height - 1, k = 0; i >= 0; i--, k++) {
		for (j = 0; j < bufferWidth; j++) {
			ptrData[k*alignedWidth + j] = imgBuf[i*bufferWidth + j];
		}
	}

	// Write BMP
	FILE* fp = fopen(filename, "wb");
	fwrite(dibBuf, dibLength, 1, fp);
	fclose(fp);

	if (dibBuf) {
		free(dibBuf);
		dibBuf = 0;
	}

	return 1;
}

///
/// \brief      read bmp file
///
/// \param[in]  filename  The filename
/// \param[out] imgBuf    The image buffer
/// \param[out] width     The width
/// \param[out] height    The height
/// \param[out] nBits     The bit depth (grayscale 8 or RGB 24)
///
/// \return     0 error, 1 okay
/// 
///	Example:
///	```
///	unsigned char* buffer = 0;
///	int width = 0, height = 0, bitdepth = 0;
///	readbmp(filename, &buffer, &width, &height, &bitdepth);
///	```
///
int readbmp(const char *filename, unsigned char /**&imgBuf*/**imgBuf, int* width, int* height, int* nBits)
{
	FILE *fp = fopen(filename, "rb");
	if (fp == 0) {
		return 0;
	}

	fseek(fp, sizeof(BITMAPFILEHEADER), 0);
	BITMAPINFOHEADER head;
	fread(&head, sizeof(BITMAPINFOHEADER), 1, fp);

	*width = head.biWidth;
	*height = head.biHeight;
	*nBits = head.biBitCount;
	
	RGBQUAD* pColorTable = 0;
	if (*nBits == 8) {
		pColorTable = (RGBQUAD*)malloc(sizeof(RGBQUAD) * 256);
		fread(pColorTable, sizeof(RGBQUAD), 256, fp);
	}

	int alignedWidth = ((*width) * (*nBits) / 8 + 3) / 4 * 4;
	unsigned char* dataBuf = (unsigned char*)malloc(alignedWidth * (*height));
	fread(dataBuf, 1, alignedWidth * (*height), fp);

	if (*imgBuf) {
		free(*imgBuf);
		*imgBuf = 0;
	}
	*imgBuf = (unsigned char*)malloc(alignedWidth * (*height));

	int bufferWidth = (*width) * (*nBits) / 8;
	int i, j, k;
	if (head.biHeight > 0) {
		for (i = (*height) - 1, k = 0; i >= 0; i--, k++) {
			memcpy(*imgBuf + k*bufferWidth, dataBuf + i*alignedWidth, bufferWidth * sizeof(unsigned char));
			//for (j = 0; j < bufferWidth; j++) {
			//	imgBuf[k*bufferWidth + j] = dataBuf[i*alignedWidth + j];
			//}
		}
	}
	else {
		for (i = 0; i < -(head.biHeight); i++) {
			//for (j = 0; j < bufferWidth; j++) {
			//	imgBuf[i*bufferWidth + j] = dataBuf[i*alignedWidth + j];
			//}
			memcpy(*imgBuf + i*bufferWidth, dataBuf + i*alignedWidth, bufferWidth * sizeof(unsigned char));
		}
	}

	free(dataBuf);
	fclose(fp);
	return 1;
}