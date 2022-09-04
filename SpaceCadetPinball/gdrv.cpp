#include "pch.h"
#include "gdrv.h"
#include "memory.h"
#include "partman.h"
#include "pinball.h"
#include "render.h"
#include "winmain.h"

RGBQUAD gdrv::palette[256];
HINSTANCE gdrv::hinst;
HWND gdrv::hwnd;
int gdrv::use_wing = 0;
int gdrv::grtext_blue = 0;
int gdrv::grtext_green = 0;
int gdrv::grtext_red = -1;


int gdrv::init(HINSTANCE hInst, HWND hWnd)
{
	hinst = hInst;
	hwnd = hWnd;
	char dataFilePath[300];
	pinball::make_path_name(dataFilePath, winmain::DatFileName, 300);
	auto record_table = partman::load_records(dataFilePath);
	auto plt = (PALETTEENTRY*)partman::field_labeled(record_table, "background", datFieldTypes::Palette);
	for (size_t i = 0; i < 255; i++)
	{
		palette[i].rgbRed = plt[i].peBlue;
		palette[i].rgbGreen = plt[i].peGreen;
		palette[i].rgbBlue = plt[i].peRed;
		palette[i].rgbReserved = plt[i].peFlags;
	}
	partman::unload_records(record_table);
	return 0;
}

int gdrv::uninit()
{
	return 0;
}

void gdrv::get_focus()
{
}


BITMAPINFO* gdrv::DibCreate(__int16 bpp, int width, int height)
{
	auto sizeBytes = height * ((width * bpp / 8 + 3) & 0xFFFFFFFC);
	auto buf = GlobalAlloc(GHND, sizeBytes + 1064);
	auto dib = static_cast<BITMAPINFO*>(GlobalLock(buf));

	if (!dib)
		return nullptr;
	dib->bmiHeader.biSizeImage = sizeBytes;
	dib->bmiHeader.biWidth = width;
	dib->bmiHeader.biSize = 40;
	dib->bmiHeader.biHeight = height;
	dib->bmiHeader.biPlanes = 1;
	dib->bmiHeader.biBitCount = bpp;
	dib->bmiHeader.biCompression = 0;
	dib->bmiHeader.biXPelsPerMeter = 0;
	dib->bmiHeader.biYPelsPerMeter = 0;
	dib->bmiHeader.biClrUsed = 0;
	dib->bmiHeader.biClrImportant = 0;
	if (bpp == 4)
	{
		dib->bmiHeader.biClrUsed = 16;
	}
	else if (bpp == 8)
	{
		dib->bmiHeader.biClrUsed = 256;
	}

	int index = 0;
	for (size_t i = 0; i < 255; i++)
	{
		dib->bmiColors[i] = palette[i];
	}
	return dib;
}


int gdrv::create_bitmap_dib(gdrv_bitmap8* bmp, int width, int height)
{
	auto dib = DibCreate(8, width, height);
	bmp->Dib = dib;
	bmp->Width = width;
	bmp->Stride = width;
	if (width % 4)
		bmp->Stride = 4 - width % 4 + width;

	bmp->Height = height;
	bmp->BitmapType = BitmapType::DibBitmap;

	bmp->Handle = CreateDIBSection(GetDC(nullptr), bmp->Dib, DIB_RGB_COLORS, (void**)&bmp->BmpBufPtr1, nullptr, 0);
	return 0;
}

int gdrv::create_bitmap(gdrv_bitmap8* bmp, int width, int height)
{
	return create_bitmap_dib(bmp, width, height);
}

int gdrv::create_raw_bitmap(gdrv_bitmap8* bmp, int width, int height, int flag)
{
	bmp->Dib = DibCreate(8, width, height);
	bmp->Width = width;
	bmp->Stride = width;
	if (flag && width % 4)
		bmp->Stride = width - width % 4 + 4;
	unsigned int sizeInBytes = height * bmp->Stride;
	bmp->Height = height;
	bmp->BitmapType = BitmapType::DibBitmap;
	bmp->Handle = CreateDIBSection(GetDC(nullptr), bmp->Dib, DIB_RGB_COLORS, (void**)&bmp->BmpBufPtr1, nullptr, 0);
	return 0;
}

int gdrv::destroy_bitmap(gdrv_bitmap8* bmp)
{
	if (!bmp)
		return -1;
	if (bmp->BitmapType == BitmapType::RawBitmap)
	{
		memory::free(bmp->BmpBufPtr1);
	}
	else if (bmp->BitmapType == BitmapType::DibBitmap)
	{
		GlobalUnlock(GlobalHandle(bmp->Dib));
		GlobalFree(GlobalHandle(bmp->Dib));
		DeleteObject(bmp->Handle);
	}
	memset(bmp, 0, sizeof(gdrv_bitmap8));
	return 0;
}

void gdrv::blit(gdrv_bitmap8* bmp, int xSrc, int ySrcOff, int xDest, int yDest, int DestWidth, int DestHeight)
{
	if (render::memory_dc)
	{
		HDC dcBmp = CreateCompatibleDC(nullptr);
		if (dcBmp)
		{
			HGDIOBJ hbmOld = SelectObject(dcBmp, bmp->Handle);
			if (hbmOld)
			{
				DeleteObject(hbmOld);
			}
			if (!dcBmp)
			{
				StretchBlt(
					render::memory_dc,
					xDest,
					yDest,
					DestWidth,
					DestHeight,
					dcBmp,
					xSrc,
					bmp->Height - ySrcOff - DestHeight,
					DestWidth,
					DestHeight,
					SRCCOPY);
			}
			DeleteDC(dcBmp);
		}
	}
}

void gdrv::blat(gdrv_bitmap8* bmp, int xDest, int yDest)
{
	HDC dc = winmain::_GetDC(winmain::hwnd_frame);
	if (dc)
	{
		if (!use_wing)
			StretchBlt(
				dc,
				xDest,
				yDest,
				bmp->Width,
				bmp->Height,
				render::memory_dc,
				0,
				0,
				bmp->Width,
				bmp->Height,
				SRCCOPY);
		ReleaseDC(winmain::hwnd_frame, dc);
	}
}

void gdrv::fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, char fillChar)
{
	int bmpHeight = bmp->Height;
	if (bmpHeight < 0)
		bmpHeight = -bmpHeight;
	char* bmpPtr = &bmp->BmpBufPtr1[bmp->Width * (bmpHeight - height - yOff) + xOff];
	if (height > 0)
	{
		do
		{
			if (width > 0)
				memset(bmpPtr, fillChar, width);
			bmpPtr += bmp->Stride;
			--height;
		}
		while (height);
	}
}

void gdrv::copy_bitmap(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff, gdrv_bitmap8* srcBmp,
                       int srcXOff, int srcYOff)
{
	int dstHeight = abs(dstBmp->Height);
	int srcHeight = abs(srcBmp->Height);
	char* srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeight - height - srcYOff) + srcXOff];
	char* dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeight - height - yOff) + xOff];

	for (int y = height; y > 0; --y)
	{
		for (int x = width; x > 0; --x)
			*dstPtr++ = *srcPtr++;

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
	}
}

void gdrv::copy_bitmap_w_transparency(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff,
                                      gdrv_bitmap8* srcBmp, int srcXOff, int srcYOff)
{
	int dstHeight = abs(dstBmp->Height);
	int srcHeight = abs(srcBmp->Height);
	char* srcPtr = &srcBmp->BmpBufPtr1[srcBmp->Stride * (srcHeight - height - srcYOff) + srcXOff];
	char* dstPtr = &dstBmp->BmpBufPtr1[dstBmp->Stride * (dstHeight - height - yOff) + xOff];

	for (int y = height; y > 0; --y)
	{
		for (int x = width; x > 0; --x)
		{
			if (*srcPtr)
				*dstPtr = *srcPtr;
			++srcPtr;
			++dstPtr;
		}

		srcPtr += srcBmp->Stride - width;
		dstPtr += dstBmp->Stride - width;
	}
}


void gdrv::grtext_draw_ttext_in_box(LPCSTR text, int xOff, int yOff, int width, int height, int a6)
{
	tagRECT rc{};

	HDC dc = render::memory_dc;
	rc.left = xOff;
	rc.right = width + xOff;
	rc.top = yOff;
	rc.bottom = height + yOff;
	if (grtext_red < 0)
	{
		grtext_blue = 255;
		grtext_green = 255;
		grtext_red = 255;
		const char* fontColor = pinball::get_rc_string(189, 0);
		if (fontColor)
			sscanf_s(fontColor, "%d %d %d", &grtext_red, &grtext_green, &grtext_blue);
	}
	int prevMode = SetBkMode(dc, 1);
	COLORREF color = SetTextColor(dc, (grtext_red) | (grtext_green << 8) | (grtext_blue << 16));
	DrawTextA(dc, text, lstrlenA(text), &rc, 0x810u);
	SetBkMode(dc, prevMode);
	SetTextColor(dc, color);
}
