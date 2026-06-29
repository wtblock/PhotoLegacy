/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ImagePlus.h"
#include "CHelper.h"

/////////////////////////////////////////////////////////////////////////////
// CImagePlus.cpp
//
// Implementation of CImagePlus — a lightweight GDI+ wrapper designed
// specifically for Photo Printer’s rendering pipeline.
//
// This class focuses on:
//   • Correct mapping‑mode‑aware drawing (via StretchDIBits)
//   • Simple encoder lookup and saving
//   • Safe extraction of HBITMAPs
//   • Creation of DIB headers for device‑independent rendering
//
// Unlike CPlusGDI (the full-featured version), this class intentionally
// omits multi-frame support, metafile handling, and complex save modes.
// It is optimized for Photo Printer’s single‑image rendering workflow.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// GetEncoderExtByID
//
// Maps an IMAGE_FORMAT_ID → encoder MIME type (e.g., "image/jpeg").
// Used by Save() to retrieve the correct GDI+ encoder.
//
// Returns true if the format ID is found in ImageFormat_ID_EncoderExt.
/////////////////////////////////////////////////////////////////////////////
bool CImagePlus::GetEncoderExtByID(UINT nImageFormatID, LPWSTR pEncoderExt)
{
	bool value = false;

	const int iCount =
		sizeof(ImageFormat_ID_EncoderExt) /
		sizeof(IMAGEFORMAT_ID_EXT);

	for (int iIndex = 0; iIndex < iCount; ++iIndex)
	{
		if (nImageFormatID == ImageFormat_ID_EncoderExt[iIndex].m_nFormatID)
		{
			wcscpy
			(
				pEncoderExt,
				(LPWSTR)&ImageFormat_ID_EncoderExt[iIndex].m_szExt
			);
			return true;
		}
	}

	return value;
} // GetEncoderExtByID

/////////////////////////////////////////////////////////////////////////////
// GetEncoderClsid
//
// Retrieves the CLSID for the encoder matching the given MIME type.
// Required by GDI+ Image::Save().
//
// Behavior:
//   • Queries GDI+ for all installed encoders
//   • Scans for matching MimeType
//   • Returns index of encoder or -1 on failure
//
// Note: Caller must ensure 'format' is a valid MIME type string.
/////////////////////////////////////////////////////////////////////////////
int CImagePlus::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT numEncoders = 0, size = 0;
	GetImageEncodersSize(&numEncoders, &size);
	if (size == 0)
		return -1;

	ImageCodecInfo* pEncoders = (ImageCodecInfo*)malloc(size);
	if (!pEncoders)
		return -1;

	GetImageEncoders(numEncoders, size, pEncoders);
	for (UINT i = 0; i < numEncoders; ++i)
	{
		if (wcscmp(pEncoders[i].MimeType, format) == 0)
		{
			*pClsid = pEncoders[i].Clsid;
			free(pEncoders);
			return i;
		}
	}

	free(pEncoders);
	return -1;
} // GetEncoderClsid

/////////////////////////////////////////////////////////////////////////////
// GetFormatIDByExt
//
// Maps a file extension (".jpg", "png") → IMAGE_FORMAT_ID.
// Removes leading '.' if present.
//
// Returns true if the extension is recognized.
/////////////////////////////////////////////////////////////////////////////
bool CImagePlus::GetFormatIDByExt(LPCWSTR pExt, UINT& nImageFormatID)
{
	bool value = false;

	CString strExt(pExt);
	strExt.Remove('.');

	const int iCount =
		sizeof(ImageFormat_ID_Ext) /
		sizeof(IMAGEFORMAT_ID_EXT);

	for (int iIndex = 0; iIndex < iCount; ++iIndex)
	{
		if (strExt.CompareNoCase(ImageFormat_ID_Ext[iIndex].m_szExt) == 0)
		{
			nImageFormatID = ImageFormat_ID_Ext[iIndex].m_nFormatID;
			return true;
		}
	}

	return value;
} // GetFormatIDByExt

/////////////////////////////////////////////////////////////////////////////
// Save
//
// Saves the wrapped GDI+ Bitmap to disk using the encoder determined
// by the file extension.
//
// Behavior:
//   • Determines extension (".jpg", ".png", etc.)
//   • Looks up format ID
//   • Looks up encoder MIME type
//   • Retrieves encoder CLSID
//   • Calls GDI+ Image::Save()
//
// Uses GIF89/LZW/Flush flags for compatibility with Photo Printer’s
// multi-device rendering pipeline.
//
// Returns true on success.
/////////////////////////////////////////////////////////////////////////////
bool CImagePlus::Save(LPCTSTR lpszPathName)
{
	USES_CONVERSION;

	bool value = false;

	CString csExt = CHelper::GetExtension(lpszPathName);

	int iValue =
		EncoderValueVersionGif89 |
		EncoderValueCompressionLZW |
		EncoderValueFlush;

	EncoderParameters param;
	param.Count = 1;
	param.Parameter[0].Guid = EncoderSaveFlag;
	param.Parameter[0].Value = &iValue;
	param.Parameter[0].Type = EncoderParameterValueTypeLong;
	param.Parameter[0].NumberOfValues = 1;

	UINT nImageFormatID;
	if (!GetFormatIDByExt(T2CW(csExt), nImageFormatID))
		return value;

	WCHAR EncoderExt[MAX_EXT_LEN];
	if (!GetEncoderExtByID(nImageFormatID, EncoderExt))
		return value;

	CLSID clsid;
	GetEncoderClsid(EncoderExt, &clsid);

	Status status = m_pImage->Save(T2CW(lpszPathName), &clsid, &param);
	value = status == Ok;

	return value;
} // Save

/////////////////////////////////////////////////////////////////////////////
// GetBitsPerPixel
//
// Returns the color depth of the image based on its PixelFormat.
// Supports all common GDI+ formats.
//
// Returns -1 if no image is loaded.
/////////////////////////////////////////////////////////////////////////////
long CImagePlus::GetBitsPerPixel()
{
	long value = -1;
	if (!m_pImage)
		return value;

	PixelFormat pixelFormat = m_pImage->GetPixelFormat();
	switch (pixelFormat)
	{
		case PixelFormat1bppIndexed: value = 1; break;
		case PixelFormat4bppIndexed: value = 4; break;
		case PixelFormat8bppIndexed: value = 8; break;
		case PixelFormat16bppARGB1555:
		case PixelFormat16bppGrayScale:
		case PixelFormat16bppRGB555:
		case PixelFormat16bppRGB565: value = 16; break;
		case PixelFormat24bppRGB: value = 24; break;
		case PixelFormat32bppARGB:
		case PixelFormat32bppPARGB:
		case PixelFormat32bppRGB: value = 32; break;
		case PixelFormat48bppRGB: value = 48; break;
		case PixelFormat64bppARGB:
		case PixelFormat64bppPARGB: value = 64; break;
		default: ASSERT(0); break;
	}

	return value;
} // GetBitsPerPixel

/////////////////////////////////////////////////////////////////////////////
// CreateDIBBitmapInfo
//
// Creates a BITMAPINFO structure describing the image for DIB operations.
// This is essential because StretchDIBits requires a DIB header.
//
// Behavior:
//   • Monochrome images → 1 bpp, 2 palette entries
//   • Color images → 32 bpp true color
//   • Sets DPI based on device caps
//   • Allocates BITMAPINFO dynamically and wraps it in shared_ptr
//
// Used by Draw() and GetHBITMAP().
/////////////////////////////////////////////////////////////////////////////
shared_ptr<BITMAPINFO> CImagePlus::CreateDIBBitmapInfo
(
	CDC* pDC, Size& szSizeInPixel, COLORREF& rgbFG, COLORREF& rgbBG
)
{
	shared_ptr<BITMAPINFO> value;

	bool bMonochrome = IsMonochrome;
	int nBitsPerPixel = bMonochrome ? 1 : 32;
	int nSizeBitmapInfo = SizeDIBBitmapInfo;

	BITMAPINFO* pBitmapInfo = (BITMAPINFO*)new byte[nSizeBitmapInfo];
	memset(pBitmapInfo, 0, nSizeBitmapInfo);
	value = shared_ptr<BITMAPINFO>(pBitmapInfo);

	const float fMillimetersPerInch = 25.4f;
	const float fPixelsPerInchX = (float)pDC->GetDeviceCaps(LOGPIXELSX);
	const float fPixelsPerInchY = (float)pDC->GetDeviceCaps(LOGPIXELSY);

	value->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	value->bmiHeader.biWidth = szSizeInPixel.Width;
	value->bmiHeader.biHeight = szSizeInPixel.Height;
	value->bmiHeader.biPlanes = 1;
	value->bmiHeader.biBitCount = nBitsPerPixel;
	value->bmiHeader.biCompression = BI_RGB;
	value->bmiHeader.biSizeImage = 0;
	value->bmiHeader.biXPelsPerMeter =
		(int)(1000.0f * fPixelsPerInchX / fMillimetersPerInch);
	value->bmiHeader.biYPelsPerMeter =
		(int)(1000.0f * fPixelsPerInchY / fMillimetersPerInch);
	value->bmiHeader.biClrUsed = bMonochrome ? 2 : 0;
	value->bmiHeader.biClrImportant = bMonochrome ? 2 : 0;

	if (bMonochrome)
	{
		value->bmiColors[0].rgbBlue = GetBValue(rgbFG);
		value->bmiColors[0].rgbGreen = GetGValue(rgbFG);
		value->bmiColors[0].rgbRed = GetRValue(rgbFG);

		value->bmiColors[1].rgbBlue = GetBValue(rgbBG);
		value->bmiColors[1].rgbGreen = GetGValue(rgbBG);
		value->bmiColors[1].rgbRed = GetRValue(rgbBG);
	}

	return value;
} // CreateDIBBitmapInfo

/////////////////////////////////////////////////////////////////////////////
// GetHBITMAP
//
// Renders the image into a 32‑bit bitmap and returns an HBITMAP.
// Background is forced to white.
//
// Used for printing, brush creation, and device‑independent rendering.
/////////////////////////////////////////////////////////////////////////////
bool CImagePlus::GetHBITMAP(HBITMAP& hBitmap)
{
	bool value = false;
	if (!m_pImage)
		return false;

	Bitmap image(Width, Height);
	image.SetResolution((REAL)HorizontalResolution, (REAL)VerticalResolution);

	Graphics gp(&image);
	gp.Clear(Color(255, 255, 255, 255));

	Status status = gp.DrawImage(m_pImage.get(), 0, 0);
	Color backGround(255, 255, 255, 255);
	status = image.GetHBITMAP(backGround, &hBitmap);

	value = status == Ok;
	return value;
} // GetHBITMAP

/////////////////////////////////////////////////////////////////////////////
// Draw
//
// Draws a portion of the image into the destination rectangle using
// StretchDIBits — NOT GDI+.
//
// Why this matters:
//   • GDI+ Graphics::DrawImage ignores mapping mode.
//   • StretchDIBits honors mapping mode and device resolution.
//
// This ensures consistent rendering across:
//   • Screen
//   • Print preview
//   • Printer output
//
// Behavior:
//   • Converts image to HBITMAP
//   • Extracts raw bits
//   • Creates BITMAPINFO header
//   • Reverses DIB orientation (negative height)
//   • Calls StretchDIBits with correct stretch mode
/////////////////////////////////////////////////////////////////////////////
void CImagePlus::Draw(CDC* pDC, CRect& rectDest, CRect& rectSrc)
{
	Rect destRect(rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height());

	HBITMAP hBitmap = NULL;
	GetHBITMAP(hBitmap);
	CBitmap* pBitmap = CBitmap::FromHandle(hBitmap);

	COLORREF rgbFG = RGB(0, 0, 0);
	COLORREF rgbBG = RGB(255, 255, 255);

	const int nWidth = Width;
	const int nHeight = Height;
	const bool bIsMonochrome = IsMonochrome;
	const int nImagePaddingWidth = ImagePaddingWidth[nWidth];

	Size size(nWidth, nHeight);
	shared_ptr<BITMAPINFO> pSourceBitmapInfo =
		CreateDIBBitmapInfo(pDC, size, rgbFG, rgbBG);

	const int iSizeData = bIsMonochrome ?
		(int)ceil((double)nImagePaddingWidth * (double)nHeight / 8.0) :
		nWidth * nHeight * 4;

	shared_ptr<byte> lpSourceBits(new byte[iSizeData]);

	pBitmap->GetBitmapBits(iSizeData, lpSourceBits.get());

	const int nNewStrechMode = bIsMonochrome ? BLACKONWHITE : COLORONCOLOR;
	const int nOldStrechMode = ::SetStretchBltMode(pDC->m_hDC, nNewStrechMode);

	pSourceBitmapInfo->bmiHeader.biHeight =
		-pSourceBitmapInfo->bmiHeader.biHeight;

	::StretchDIBits
	(
		pDC->m_hDC,
		destRect.GetLeft(), destRect.GetTop(),
		destRect.Width, destRect.Height,
		rectSrc.left, rectSrc.top,
		rectSrc.Width(), rectSrc.Height(),
		lpSourceBits.get(),
		pSourceBitmapInfo.get(),
		DIB_RGB_COLORS,
		SRCCOPY
	);

	::SetStretchBltMode(pDC->m_hDC, nOldStrechMode);

	pBitmap->DeleteObject();
} // Draw

/////////////////////////////////////////////////////////////////////////////

