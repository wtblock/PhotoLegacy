/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gdiplus.h>
#include <memory>

using namespace Gdiplus;
using namespace std;

#define MAX_EXT_LEN 20 

typedef struct tagImageFormatIDExt
{
	UINT m_nFormatID;
	WCHAR m_szExt[MAX_EXT_LEN];

} IMAGEFORMAT_ID_EXT;

// Define the list of all standard image format ids
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

// The list of image format ids paired with standard image extension
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

// The list of image formats that are able to be saved
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

// The list of image format ids paired with encoder extension
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
// GDI+ does not correctly honor the GDI mapping mode so the
// CImagePlus class is my work around for that. By honoring
// the GDI mapping mode, the same code can display images
// on the screen, the print preview, and printed output without
// being concerned about the resolution of these devices.
class CImagePlus
{
// protected data
protected:
	shared_ptr<Bitmap> m_pImage;

// public properties
public:
	// height of image in pixels
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

	// the horizontal resolution of the image
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

	// return the number of bits used for each pixel
	long GetBitsPerPixel();
	// return the number of bits used for each pixel
	__declspec(property(get = GetBitsPerPixel))
		long BitsPerPixel;

	// monochrome flag
	bool GetIsMonochrome()
	{
		return BitsPerPixel == 1;
	}
	// monochrome flag
	__declspec(property(get = GetIsMonochrome))
		bool IsMonochrome;

	// return the size of the bitmap information header
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

	// padding required for monochrome bitmaps
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
	// Save the data inside pImage as file name lpszPathName
	bool Save(LPCTSTR lpszPathName);

	static bool GetFormatIDByExt(LPCWSTR pExt, UINT& nImageFormatID);

	// Get the encoder extension corresponding to the image format id
	static bool GetEncoderExtByID(UINT nImageFormatID, LPWSTR pEncoderExt);

	// WTB: same as above but uses the enumeration
	static bool GetEncoderExtByID(IMAGE_FORMAT_ID eImageFormatID, LPWSTR pEncoderExt)
	{
		const UINT uID = (UINT)eImageFormatID;
		return GetEncoderExtByID(uID, pEncoderExt);
	}

	// Get the CLSID for the specified image format
	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

	// get a handle to the bitmap
	bool GetHBITMAP( HBITMAP& hBitmap );

	shared_ptr<BITMAPINFO> CreateDIBBitmapInfo
	( 
		CDC* pDC, Size& szSizeInPixel, COLORREF& rgbFG, COLORREF& rgbBG 
	);

	// draw a portion of the image into the destination rectangle
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
	CImagePlus(shared_ptr<Bitmap> value)
	{
		m_pImage = value;
	}

};

/////////////////////////////////////////////////////////////////////////////
