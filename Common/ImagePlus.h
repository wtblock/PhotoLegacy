/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gdiplus.h>
#include <memory>

using namespace Gdiplus;
using namespace std;

#define MAX_EXT_LEN 20 

/////////////////////////////////////////////////////////////////////////////
// IMAGEFORMAT_ID_EXT
//
// Maps IMAGE_FORMAT_ID → file extension or encoder MIME type.
// Used for extension parsing and encoder lookup.
/////////////////////////////////////////////////////////////////////////////
typedef struct tagImageFormatIDExt
{
	UINT m_nFormatID;
	WCHAR m_szExt[MAX_EXT_LEN];

} IMAGEFORMAT_ID_EXT;

/////////////////////////////////////////////////////////////////////////////
// IMAGE_FORMAT_ID
//
// Enumeration of standard GDI+ image format identifiers.
// Used for open/save support and encoder lookup.
/////////////////////////////////////////////////////////////////////////////
typedef enum tagImageFormatId
{
	BMP_ID = 0,
	DIB_ID,
	EMF_ID,
	EXIF_ID,
	GIF_ID,
	GUID_ID,
	ICON_ID,
	JFIF_ID,
	JPE_ID,
	JPEG_ID,
	JPG_ID,
	MEMORYBMP_ID,
	PNG_ID,
	RLE_ID,
	TIF_ID,
	TIFF_ID,
	WMF_ID
} IMAGE_FORMAT_ID;

/////////////////////////////////////////////////////////////////////////////
// ImageFormat_ID_Ext
//
// Static lookup table mapping format IDs → file extensions.
// Used by GetFormatIDByExt().
/////////////////////////////////////////////////////////////////////////////
static IMAGEFORMAT_ID_EXT ImageFormat_ID_Ext[] =
{
	{ BMP_ID, L"bmp" },
	{ DIB_ID, L"dib" },
	{ EMF_ID, L"emf" },
	{ EXIF_ID, L"exif" },
	{ GIF_ID, L"gif" },
	{ GUID_ID, L"guid" },
	{ ICON_ID, L"ico" },
	{ JFIF_ID, L"jfif" },
	{ JPE_ID, L"jpe" },
	{ JPEG_ID, L"jpeg" },
	{ JPG_ID, L"jpg" },
	{ MEMORYBMP_ID, L"MemoryBmp" },
	{ PNG_ID, L"png" },
	{ RLE_ID, L"rle" },
	{ TIF_ID, L"tif" },
	{ TIFF_ID, L"tiff" },
	{ WMF_ID, L"wmf" }
};

/////////////////////////////////////////////////////////////////////////////
// ImageFormatSaveSupported_ID
//
// List of formats that GDI+ can save.
// Used by Save() to validate output formats.
/////////////////////////////////////////////////////////////////////////////
static IMAGE_FORMAT_ID ImageFormatSaveSupported_ID[] =
{
	{ BMP_ID },
	{ DIB_ID },
	{ RLE_ID },
	{ GIF_ID },
	{ JPEG_ID },
	{ JPG_ID },
	{ JPE_ID },
	{ JFIF_ID },
	{ PNG_ID },
	{ TIF_ID },
	{ TIFF_ID }
};

/////////////////////////////////////////////////////////////////////////////
// ImageFormat_ID_EncoderExt
//
// Maps format IDs → encoder MIME types (e.g., "image/jpeg").
// Used by GetEncoderClsid().
/////////////////////////////////////////////////////////////////////////////
static IMAGEFORMAT_ID_EXT ImageFormat_ID_EncoderExt[] =
{
	{ BMP_ID, L"image/bmp" },
	{ DIB_ID, L"image/bmp" },
	{ RLE_ID, L"image/bmp" },
	{ GIF_ID, L"image/gif" },
	{ JPEG_ID, L"image/jpeg" },
	{ JPG_ID, L"image/jpeg" },
	{ JPE_ID, L"image/jpeg" },
	{ JFIF_ID, L"image/jpeg" },
	{ PNG_ID, L"image/png" },
	{ TIF_ID, L"image/tiff" },
	{ TIFF_ID, L"image/tiff" }
};

/////////////////////////////////////////////////////////////////////////////
// CImagePlus
//
// Lightweight GDI+ image wrapper designed specifically for Photo Printer.
//
// Purpose:
//   • Provide a simple, predictable interface for drawing images in any
//     GDI mapping mode (MM_TEXT, MM_LOMETRIC, MM_HIMETRIC, MM_TWIPS, etc.).
//   • Work around a GDI+ limitation: GDI+ ignores the device’s mapping mode,
//     which causes images to appear at incorrect physical sizes when drawn
//     to printers, print preview windows, or high‑DPI screens.
//
// Why this class exists:
//   • Photo Printer needs images to render identically across:
//         - Screen display
//         - Print preview
//         - Actual printed output
//     regardless of DPI or mapping mode.
//   • Raw GDI+ drawing does NOT honor mapping mode scaling.
//   • This wrapper converts the image into a DIB and uses StretchDIBits,
//     which *does* honor mapping mode and device resolution.
//
// Key Features:
//   • Wraps a GDI+ Bitmap in a shared_ptr for safe ownership.
//   • Provides pixel dimensions, DPI, and bits‑per‑pixel.
//   • Computes monochrome padding width (32‑bit alignment).
//   • Creates DIB headers for device‑independent drawing.
//   • Draws images using StretchDIBits so mapping mode is honored.
//   • Supports saving images using GDI+ encoders.
//   • Supports HBITMAP extraction for printing and brush creation.
//
// This class is intentionally smaller than CPlusGDI — it focuses only on
// the operations required for Photo Printer’s rendering pipeline.
/////////////////////////////////////////////////////////////////////////////
class CImagePlus
{
// protected data
protected:
	/////////////////////////////////////////////////////////////////////////////
	// m_pImage
	//
	// Shared pointer to a GDI+ Bitmap.
	// Using shared_ptr ensures automatic cleanup and safe ownership.
	// This class never exposes raw Bitmap* except through GetHBITMAP().
	/////////////////////////////////////////////////////////////////////////////
	shared_ptr<Bitmap> m_pImage;

// public properties
public:
	/////////////////////////////////////////////////////////////////////////////
	// Height / Width
	//
	// Returns pixel dimensions of the image.
	// If no image is loaded, returns 0.
	/////////////////////////////////////////////////////////////////////////////
	int GetHeight()
	{
		int value = 0;
		if (!m_pImage)
		{
			return value;
		}

		value = m_pImage->GetHeight();

		return value;
	}
	// height of image in pixels
	__declspec( property( get = GetHeight ) )
		int Height;

	// width of image in pixels
	int GetWidth()
	{
		int value = 0;
		if (!m_pImage)
		{
			return value;
		}

		value = m_pImage->GetWidth();

		return value;
	}
	// width of image in pixels
	__declspec(property(get = GetWidth))
		int Width;

	/////////////////////////////////////////////////////////////////////////////
	// HorizontalResolution / VerticalResolution
	//
	// Returns DPI values stored in the image metadata.
	// Used for physical size calculations and scaling.
	/////////////////////////////////////////////////////////////////////////////
	double GetHorizontalResolution()
	{
		double value = 0;
		if (!m_pImage)
		{
			return value;
		}

		value = m_pImage->GetHorizontalResolution();
		return value;
	}
	// the horizontal resolution of the image
	__declspec(property(get = GetHorizontalResolution))
		double HorizontalResolution;

	// the vertical resolution of the image
	double GetVerticalResolution() 
	{
		double value = 0;
		if (!m_pImage)
		{
			return value;
		}

		value = m_pImage->GetVerticalResolution();
		return value;
	}
	// the vertical resolution of the image
	__declspec(property(get = GetVerticalResolution))
		double VerticalResolution;

	/////////////////////////////////////////////////////////////////////////////
	// BitsPerPixel
	//
	// Returns the color depth based on PixelFormat.
	// Supports monochrome detection (1 bpp).
	/////////////////////////////////////////////////////////////////////////////
	long GetBitsPerPixel();
	// return the number of bits used for each pixel
	__declspec(property(get = GetBitsPerPixel))
		long BitsPerPixel;

	/////////////////////////////////////////////////////////////////////////////
	// IsMonochrome
	//
	// Returns true if BitsPerPixel == 1.
	// Monochrome images require special padding and DIB handling.
	/////////////////////////////////////////////////////////////////////////////
	bool GetIsMonochrome()
	{
		return BitsPerPixel == 1;
	}
	// monochrome flag
	__declspec(property(get = GetIsMonochrome))
		bool IsMonochrome;

	/////////////////////////////////////////////////////////////////////////////
	// SizeDIBBitmapInfo
	//
	// Returns the size of a BITMAPINFO structure required to describe the image.
	// Monochrome images require two RGBQUAD entries (black/white).
	/////////////////////////////////////////////////////////////////////////////
	int GetSizeDIBBitmapInfo()
	{
		const int nHeader = sizeof( BITMAPINFOHEADER );
		const bool bMono = IsMonochrome;
		const int nQuad = sizeof( RGBQUAD );
		const int value = nHeader + ( bMono ? 2 : 1 ) * nQuad;
		return value;
	}
	// return the size of the bitmap information header
	__declspec(property(get = GetSizeDIBBitmapInfo))
		int SizeDIBBitmapInfo;

	/////////////////////////////////////////////////////////////////////////////
	// ImagePaddingWidth
	//
	// Returns padded width for monochrome bitmaps.
	// GDI requires monochrome scanlines to be 32‑bit aligned.
	// Color images return the original width.
	/////////////////////////////////////////////////////////////////////////////
	int GetImagePaddingWidth(int nWidth)
	{
		int value = nWidth;
		if (IsMonochrome)
		{
			value = 32 * (int)ceil(((double)nWidth) / 32.0);
		}
		return value;
	}
	// padding required for monochrome bitmaps
	__declspec(property(get = GetImagePaddingWidth))
		int ImagePaddingWidth[];

// protected methods
protected:

// public methods
public:
	/////////////////////////////////////////////////////////////////////////////
	// Save(path)
	//
	// Saves the image using the encoder determined by file extension.
	// Uses GDI+ Image::Save() with the correct CLSID.
	//
	// Returns true on success.
	/////////////////////////////////////////////////////////////////////////////
	bool Save(LPCTSTR lpszPathName);

	/////////////////////////////////////////////////////////////////////////////
	// GetFormatIDByExt
	//
	// Maps a file extension (".jpg", "png") to an IMAGE_FORMAT_ID.
	// Removes leading '.' if present.
	/////////////////////////////////////////////////////////////////////////////
	static bool GetFormatIDByExt(LPCWSTR pExt, UINT& nImageFormatID);

	/////////////////////////////////////////////////////////////////////////////
	// GetEncoderExtByID
	//
	// Maps IMAGE_FORMAT_ID → encoder MIME type (e.g., "image/jpeg").
	// Used by GetEncoderClsid().
	/////////////////////////////////////////////////////////////////////////////
	static bool GetEncoderExtByID(UINT nImageFormatID, LPWSTR pEncoderExt);

	// WTB: same as above but uses the enumeration
	static bool GetEncoderExtByID(IMAGE_FORMAT_ID eImageFormatID, LPWSTR pEncoderExt)
	{
		const UINT uID = (UINT)eImageFormatID;
		return GetEncoderExtByID(uID, pEncoderExt);
	}

	/////////////////////////////////////////////////////////////////////////////
	// GetEncoderClsid(format, pClsid)
	//
	// Retrieves the CLSID for the encoder matching the MIME type.
	// Required for GDI+ Image::Save().
	/////////////////////////////////////////////////////////////////////////////
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	/////////////////////////////////////////////////////////////////////////////
	// GetHBITMAP
	//
	// Renders the image into a 32‑bit bitmap and returns an HBITMAP.
	// Background is forced to white.
	//
	// Used for printing, brush creation, and device‑independent rendering.
	/////////////////////////////////////////////////////////////////////////////
	bool GetHBITMAP( HBITMAP& hBitmap );

	/////////////////////////////////////////////////////////////////////////////
	// CreateDIBBitmapInfo
	//
	// Creates a BITMAPINFO structure describing the image for DIB operations.
	// Supports monochrome and color images.
	// Used by Draw() and GetHBITMAP().
	/////////////////////////////////////////////////////////////////////////////
	shared_ptr<BITMAPINFO> CreateDIBBitmapInfo
	( 
		CDC* pDC, Size& szSizeInPixel, COLORREF& rgbFG, COLORREF& rgbBG 
	);

	/////////////////////////////////////////////////////////////////////////////
	// Draw(pDC, rectDest, rectSrc)
	//
	// Draws a portion of the image into the destination rectangle using
	// StretchDIBits — NOT GDI+.
	//
	// Why this matters:
	//   • StretchDIBits honors the device’s mapping mode.
	//   • GDI+ Graphics::DrawImage does NOT honor mapping mode.
	//
	// This ensures consistent rendering across:
	//   • Screen
	//   • Print preview
	//   • Printer output
	//
	// Supports monochrome vs color stretch modes automatically.
	/////////////////////////////////////////////////////////////////////////////
	void Draw( CDC* pDC, CRect& rectDest, CRect& rectSrc );

	// draw a portion of the image into the destination rectangle
	inline void Draw( HDC hDC, CRect& rectDest, CRect& rectSrc )
	{
		CDC* pDC = CDC::FromHandle( hDC );
		Draw( pDC, rectDest, rectSrc );
	}

	// draw the entire image into the given destination rectangle
	inline void Draw( CDC* pDC, CRect& rectDest )
	{
		// default the source rectangle to the entire image
		CRect rectSrc( 0, 0, Width, Height );
		Draw( pDC, rectDest, rectSrc );
	}
	

// protected overrides
protected:

// public overrides
public:

// public constructor/destructor
public:
	/////////////////////////////////////////////////////////////////////////////
	// CImagePlus(shared_ptr<Bitmap>)
	//
	// Wraps an existing GDI+ Bitmap in a shared_ptr.
	// Used when Photo Printer already owns a Bitmap and wants to draw it
	// using mapping‑mode‑aware rendering.
	/////////////////////////////////////////////////////////////////////////////
	CImagePlus(shared_ptr<Bitmap> value)
	{
		m_pImage = value;
	}

};

/////////////////////////////////////////////////////////////////////////////
