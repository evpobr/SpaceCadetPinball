#include "zdrv.h"

#include "memory.h"


int zdrv::create_zmap(zmap_header_type* zmap, int width, int height)
{
	int stride = pad(width);
	zmap->Stride = stride;
	auto bmpBuf = (unsigned short*)memory::allocate(2 * height * stride);
	zmap->ZPtr1 = bmpBuf;
	if (!bmpBuf)
		return -1;
	zmap->Width = width;
	zmap->Height = height;
	return 0;
}

int zdrv::pad(int width)
{
	int result = width;
	if (width & 3)
		result = width - (width & 3) + 4;
	return result;
}

int zdrv::destroy_zmap(zmap_header_type* zmap)
{
	if (!zmap)
		return -1;
	if (zmap->ZPtr1)
		memory::free(zmap->ZPtr1);
	*zmap = {0};
	return 0;
}

void zdrv::fill(zmap_header_type* zmap, int width, int height, int xOff, int yOff, uint16_t fillChar)
{
	int fillCharInt = fillChar | (fillChar << 16);
	auto zmapPtr = &zmap->ZPtr1[xOff + zmap->Stride * (zmap->Height - height - yOff)];

	for (int y = height; width > 0 && y > 0; y--)
	{
		char widthMod2 = width & 1;
		unsigned int widthDiv2 = static_cast<unsigned int>(width) >> 1;
		memset32(zmapPtr, fillCharInt, widthDiv2);

		auto lastShort = &zmapPtr[2 * widthDiv2];
		for (int i = widthMod2; i; --i)
			*lastShort++ = fillChar;

		zmapPtr += zmap->Stride;
	}
}


void zdrv::paint(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff, zmap_header_type* dstZMap,
                 int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp, int srcBmpXOff, int srcBmpYOff,
                 zmap_header_type* srcZMap, int srcZMapXOff, int srcZMapYOff)
{
	int dstHeightAbs = abs(dstBmp->Height);
	int srcHeightAbs = abs(srcBmp->Height);
	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeightAbs - height - srcBmpYOff) + srcBmpXOff];
	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeightAbs - height - dstBmpYOff) + dstBmpXOff];
	auto srcPtrZ = &srcZMap->ZPtr1[srcZMap->Stride * (srcZMap->Height - height - srcZMapYOff) + srcZMapXOff];
	auto dstPtrZ = &dstZMap->ZPtr1[dstZMap->Stride * (dstZMap->Height - height - dstZMapYOff) + dstZMapXOff];

	auto mask = new unsigned char[srcBmp->Stride * srcBmp->Height]{};
	auto mskPtr = &mask[srcZMap->Stride * (srcZMap->Height - height - srcZMapYOff) + srcZMapXOff];

	for (int y = height; y > 0; y--)
	{
		for (int x = width; x > 0; --x)
		{
			if (*dstPtrZ >= *srcPtrZ)
			{
				*dstPtrZ = *srcPtrZ;
				*mskPtr = 1;
			}
			++srcPtrZ;
			++dstPtrZ;
			++mskPtr;
		}

		srcPtrZ += srcZMap->Stride - width;
		dstPtrZ += dstZMap->Stride - width;
		mskPtr += srcBmp->Stride - width;
	}

	srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeightAbs - height - srcBmpYOff) + srcBmpXOff];
	dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeightAbs - height - dstBmpYOff) + dstBmpXOff];
	mskPtr = &mask[srcZMap->Stride * (srcZMap->Height - height - srcZMapYOff) + srcZMapXOff];

	for (int y = height; y > 0; y--)
	{
		for (int x = width; x > 0; --x)
		{
			if (*mskPtr)
			{
				*dstPtr = *srcPtr;
			}
			++srcPtr;
			++dstPtr;
			++mskPtr;
		}

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
		mskPtr += srcBmp->Stride - width;
	}

	delete [] mask;
}

void zdrv::paint_flat(int width, int height, gdrv_bitmap8* dstBmp, int dstBmpXOff, int dstBmpYOff,
                      zmap_header_type* zMap, int dstZMapXOff, int dstZMapYOff, gdrv_bitmap8* srcBmp, int srcBmpXOff,
                      int srcBmpYOff, uint16_t depth)
{
	int dstHeightAbs = abs(dstBmp->Height);
	int srcHeightAbs = abs(srcBmp->Height);
	auto dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeightAbs - height - dstBmpYOff) + dstBmpXOff];
	auto srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeightAbs - height - srcBmpYOff) + srcBmpXOff];
	auto zPtr = &zMap->ZPtr1[zMap->Stride * (zMap->Height - height - dstZMapYOff) + dstZMapXOff];

	auto mask = new unsigned char[srcBmp->Stride * srcBmp->Height]{0};
	auto mskPtr = &mask[srcBmp->Stride * (srcHeightAbs - height - srcBmpYOff) + srcBmpXOff];

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (*srcPtr && *zPtr > depth)
			{
				*mskPtr = *srcPtr;
			}
			++srcPtr;
			++zPtr;
			++mskPtr;
		}

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
		zPtr += zMap->Stride - width;
		mskPtr += srcBmp->Stride - width;
	}

	dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeightAbs - height - dstBmpYOff) + dstBmpXOff];
	srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeightAbs - height - srcBmpYOff) + srcBmpXOff];
	zPtr = &zMap->ZPtr1[zMap->Stride * (zMap->Height - height - dstZMapYOff) + dstZMapXOff];
	mskPtr = &mask[srcBmp->Stride * (srcHeightAbs - height - srcBmpYOff) + srcBmpXOff];

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (*mskPtr)
			{
				*dstPtr = *mskPtr;
			}
			++srcPtr;
			++dstPtr;
			++mskPtr;
		}

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
		mskPtr += srcBmp->Stride - width;
	}

	delete [] mask;
}
