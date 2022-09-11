#include "pch.h"

#include <windowsx.h>

#include "gdrv.h"
#include "memory.h"
#include "partman.h"
#include "pinball.h"
#include "render.h"
#include "winmain.h"

#include <algorithm>

using namespace std;

HINSTANCE gdrv::hinst;
HWND gdrv::hwnd;
int gdrv::sequence_handle;
HDC gdrv::sequence_hdc;
int gdrv::use_wing = 0;
int gdrv::grtext_blue = 0;
int gdrv::grtext_green = 0;
int gdrv::grtext_red = -1;
RGBQUAD gdrv::palette[256];


int gdrv::init(HINSTANCE hInst, HWND hWnd)
{
	hinst = hInst;
	hwnd = hWnd;
	char datFilePath[300];
	pinball::make_path_name(datFilePath, winmain::DatFileName, 300);
	auto record_table = partman::load_records(datFilePath);
	auto plt = (PALETTEENTRY*)partman::field_labeled(record_table, "background", datFieldTypes::Palette);
	for (size_t i = 0; i < 256; i++)
	{
		palette[i].rgbBlue = plt[i].peRed;
		palette[i].rgbGreen = plt[i].peGreen;
		palette[i].rgbRed = plt[i].peBlue;
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
	auto buf = GlobalAlloc(0x42u, sizeBytes + 1064);
	auto dib = static_cast<BITMAPINFO*>(GlobalLock(buf));

	if (!dib)
		return nullptr;
	dib->bmiHeader.biSizeImage = sizeBytes;
	dib->bmiHeader.biWidth = width;
	dib->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
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

	memcpy(dib->bmiColors, palette, sizeof(RGBQUAD) * 256);
	
	return dib;
}


int gdrv::create_bitmap_dib(gdrv_bitmap8* bmp, int width, int height)
{
	char* bmpBufPtr;
	auto dib = DibCreate(8, width, height);

	bmp->Dib = dib;
	bmp->Width = width;
	bmp->Stride = width;
	if (width % 4)
		bmp->Stride = 4 - width % 4 + width;

	bmp->Height = height;
	bmp->BitmapType = BitmapType::DibBitmap;

	bmp->Handle = CreateDIBSection(
		winmain::_GetDC(winmain::hwnd_frame),
		bmp->Dib,
		DIB_RGB_COLORS,
		(void **)&bmp->BmpBufPtr1,
		nullptr,
		0);
	return 0;
}

int gdrv::create_bitmap(gdrv_bitmap8* bmp, int width, int height)
{
	return create_bitmap_dib(bmp, width, height);
}

int gdrv::create_raw_bitmap(gdrv_bitmap8* bmp, int width, int height, int flag)
{
	return create_bitmap_dib(bmp, width, height);
}


int gdrv::destroy_bitmap(gdrv_bitmap8* bmp)
{
	if (!bmp)
		return -1;

	if (bmp->Handle)
	{
		DeleteObject(bmp->Handle);
	}

	if (bmp->Dib)
	{
		GlobalUnlock(GlobalHandle(bmp->Dib));
		GlobalFree(GlobalHandle(bmp->Dib));
	}

	memset(bmp, 0, sizeof(gdrv_bitmap8));
	return 0;
}

UINT gdrv::start_blit_sequence()
{
	sequence_handle = 0;
	sequence_hdc = render::vscreen_dc;
	return 0;
}

void gdrv::blit_sequence(gdrv_bitmap8* bmp, int xSrc, int ySrcOff, int xDest, int yDest, int DestWidth, int DestHeight)
{
	HDC dcSrc = CreateCompatibleDC(sequence_hdc);
	if (dcSrc)
	{
		SelectObject(dcSrc, bmp->Handle);
		StretchBlt(
			sequence_hdc,
			xDest,
			yDest,
			DestWidth,
			DestHeight,
			dcSrc,
			xSrc,
			bmp->Height - ySrcOff - DestHeight,
			DestWidth,
			DestHeight,
			SRCCOPY);
			DeleteDC(dcSrc);
	}
}


void gdrv::end_blit_sequence()
{
	ReleaseDC(hwnd, sequence_hdc);
}

void gdrv::blit(gdrv_bitmap8* bmp, int xSrc, int ySrcOff, int xDest, int yDest, int DestWidth, int DestHeight)
{
	if (render::vscreen_dc)
	{
		HDC dcSrc = CreateCompatibleDC(render::vscreen_dc);
		if (dcSrc)
		{
			SelectObject(dcSrc, bmp->Handle);
			StretchBlt(
				render::vscreen_dc,
				xDest,
				yDest,
				DestWidth,
				DestHeight,
				dcSrc,
				xSrc,
				bmp->Height - ySrcOff - DestHeight,
				DestWidth,
				DestHeight,
				SRCCOPY);
			DeleteDC(dcSrc);
		}
	}
}

void gdrv::blat(gdrv_bitmap8* bmp, int xDest, int yDest)
{
	HDC dc = winmain::_GetDC(hwnd);
	if (dc)
	{
		HDC vscreen32_dc = CreateCompatibleDC(winmain::_GetDC(hwnd));
		HBITMAP vscreen32_bmp_old = SelectBitmap(vscreen32_dc, render::vscreen32_bmp);
		if (vscreen32_dc)
		{
			BitBlt(vscreen32_dc,
				0,
				0,
				render::vscreen.Width,
				render::vscreen.Height,
				render::vscreen_dc,
				0,
				0,
				SRCCOPY);
			StretchBlt(
				dc,
				xDest,
				yDest,
				bmp->Width,
				bmp->Height,
				vscreen32_dc,
				0,
				0,
				bmp->Width,
				bmp->Height,
				SRCCOPY);
			SelectBitmap(vscreen32_dc, vscreen32_bmp_old);
			DeleteDC(vscreen32_dc);
		}
		ReleaseDC(hwnd, dc);
	}
}

void gdrv::fill_bitmap(HDC dc, int width, int height, int xOff, int yOff, char fillChar)
{
	if (dc)
	{
		HBRUSH brush = CreateSolidBrush(RGB(palette[fillChar].rgbRed, palette[fillChar].rgbGreen, palette[fillChar].rgbBlue));
		if (brush)
		{
			HGDIOBJ old_brush = SelectObject(dc, brush);
			const RECT rc{ xOff , yOff, width + 1, height + 1 };
			FillRect(dc, &rc, brush);
			SelectObject(dc, old_brush);
			DeleteObject(brush);
		}
	}
}

void gdrv::fill_bitmap(gdrv_bitmap8* bmp, int width, int height, int xOff, int yOff, char fillChar)
{
	HDC bmpDC = CreateCompatibleDC(render::vscreen_dc);
	if (bmpDC)
	{
		HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
		if (brush)
		{
			HBRUSH old_brush = SelectBrush(bmpDC, brush);

			const RECT bmp_rect{ xOff, yOff, width, height };
			FillRect(bmpDC, &bmp_rect, brush);
			SelectBrush(bmpDC, old_brush);
			DeleteBrush(brush);
		}
		DeleteDC(bmpDC);
	}
}

void gdrv::copy_bitmap(HDC dstDC, int width, int height, int xOff, int yOff, gdrv_bitmap8* srcBmp,
                       int srcXOff, int srcYOff)
{
	if (dstDC)
	{
		HDC srcDC = CreateCompatibleDC(dstDC);
		SelectObject(srcDC, srcBmp->Handle);
		BOOL fRet = BitBlt(
			dstDC,
			xOff,
			yOff,
			width,
			height,
			srcDC,
			srcXOff,
			srcYOff,
			SRCCOPY
		);
		DeleteDC(srcDC);
	}
}

void gdrv::copy_bitmap(gdrv_bitmap8* dstBmp, int width, int height, int xOff, int yOff, HDC srcDC,
                       int srcXOff, int srcYOff)
{
	HDC dstDC = CreateCompatibleDC(nullptr);
	if (dstDC)
	{
		SelectObject(dstDC, dstBmp->Handle);
		if (srcDC)
		{
			BOOL fRet = BitBlt(
				dstDC,
				xOff,
				yOff,
				width,
				height,
				srcDC,
				srcXOff,
				srcYOff,
				SRCCOPY
			);
		}
		DeleteDC(dstDC);
	}
}

void gdrv::copy_bitmap_w_transparency(HDC dstDC, int width, int height, int xOff, int yOff,
                                      gdrv_bitmap8* srcBmp, int srcXOff, int srcYOff)
{
	copy_bitmap(dstDC, width, height, xOff, yOff, srcBmp, srcXOff, srcYOff);
}


void gdrv::grtext_draw_ttext_in_box(LPCSTR text, int xOff, int yOff, int width, int height, int a6)
{
	tagRECT rc{};

	HDC dc = render::vscreen_dc;
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
