/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <map>
#include <gdiplus.h>
using namespace Gdiplus;

#define MAX_EXT_LEN 20 

typedef struct tagImageFormatIDExt
{
	UINT m_nFormatID;
	WCHAR m_szExt[ MAX_EXT_LEN ];

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

enum IMAGE_COMPARE_RESULT
{
	EQUAL = 0,
	DIFFERENT_SIZE,
	DIFFERENT_CONTENT,
	MEMORY_ALLOCATION_ERROR,
	UNKNOWN_ERROR
};

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

// The list of image formats that are able to be opened
static IMAGE_FORMAT_ID ImageFormatOpenSupported_ID[] =
{
	{ BMP_ID },
	{ EMF_ID },
	{ EXIF_ID },
	{ GIF_ID },
	{ GUID_ID },
	{ ICON_ID },
	{ JPEG_ID },
	{ JPG_ID },
	{ MEMORYBMP_ID },
	{ PNG_ID },
	{ TIF_ID },
	{ TIFF_ID },
	{ WMF_ID }
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
class CPlusGDI
{
public:
	CPlusGDI();

	CPlusGDI( CBitmap* pBitmap );

	CPlusGDI( Bitmap* pBitmap );

	CPlusGDI( Image* pImage )
	{
		Initialize();
		m_pImage = pImage;
	}

	~CPlusGDI();

// public properties
public:
	// is the image currently open
	inline bool GetIsOpen()
	{
		const bool value = m_pImage != nullptr;
		return value;
	}
	// is the image currently open
	__declspec( property( get = GetIsOpen ) )
		bool IsOpen;


// public attributes
public:
	// access to the underlying image
	inline Gdiplus::Image* GetImage()
	{
		return m_pImage;
	}
	// cast to image pointer
	inline operator Gdiplus::Image*( )
	{
		return GetImage();
	}
	// monochrome flag
	inline bool IsMonochrome()
	{
		return GetBitsPerPixel() == 1;
	}

	// image filename
	inline CString ImagePathname()
	{
		return m_pImage != 0 ? m_csPathname : _T( "" );
	}
	// image filename
	inline void ImagePathname( LPCTSTR value ){
		m_csPathname = value;
	}

// public inline methods
public:
	// rotate / flip the image
	inline Status RotateFlip( Gdiplus::RotateFlipType rotateFlipType )
	{
		return m_pImage->RotateFlip( rotateFlipType );
	}

// public methods
public:
	// open the image from an IStream interface pointer.  the optional
	// key string is a string used to identify the image and is stored in the 
	// image's pathname.
	bool Open( IStream* pStream, LPCTSTR key = 0 );

	// Open the image which is stored in lpszPathName
	bool Open( LPCTSTR lpszPathName );

	// returns the number of frames in an image
	int GetFrameCount() 
	{
		return m_nFrameCount;
	}

	// width in physical pixels
	int GetWidth();
	
	// height in physical pixels
	int GetHeight();

	double GetWidthInches();

	double GetHeightInches();

	double GetHorizontalResolution() 
	{
		return m_pImage->GetHorizontalResolution();
	}

	double GetVerticalResolution() 
	{
		return m_pImage->GetVerticalResolution();
	}

	void SetDefaultWidth( float fDepth ) 
	{
		m_fDefaultWidth = fDepth;
	}

	long GetBitsPerPixel();

	bool GetEnhMetafileHandle
	( 
		CDC& dcRef, HENHMETAFILE& hENH, int iFrameIndex = 0 
	);

	bool GetDIBHandleForBrush( CDC* pDC, HBITMAP& hBitmap );

	bool GetHBITMAP( HBITMAP& hBitmap );

	bool GetHICON( HICON& hIcon );

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
		CRect rectSrc( 0, 0, GetWidth(), GetHeight() );
		Draw( pDC, rectDest, rectSrc );
	}
	
	BITMAPINFO* CreateDIBBitmapInfo
	( 
		CDC* pDC, Size& szSizeInPixel, COLORREF& rgbFG, COLORREF& rgbBG 
	);

	int GetSizeDIBBitmapInfo() 
	{
		const int nHeader = sizeof( BITMAPINFOHEADER );
		const bool bMono = IsMonochrome();
		const int nQuad = sizeof( RGBQUAD );
		const int value = nHeader + ( bMono ? 2 : 1 ) * nQuad;
		return value;
	}

	static bool CreateDIBPatternBrush
	( 
		CDC* pDC, CBitmap*& pBitmap, 
		COLORREF& rgbFG, COLORREF& rgbBG, 
		HBRUSH& hBrush 
	);

	HBRUSH CreateDIBPatternBrushFromBuffer
	( 
		BITMAPINFO*& pbmpinfo, LPVOID& pBuffer, Size& szImage 
	);

	int GetImagePaddingWidth( int iWidth );

	void ConverToMonochrome( COLORREF& rgbFG, COLORREF& rgbBG );

	static CBitmap* ScaleImage( CDC* pDC, CBitmap*& pBitmap, bool bPreview = false );

public:
	// Initialize all members	
	void Initialize();

	// Free all resources
	void CleanUp();

	bool Save
	( 
		HENHMETAFILE& hmf, // handle to enhanced metafile
		Size& szImageSizeInPixel, 
		Point& ptResolution, 
		LPCTSTR lpszPathName, 
		bool bAppend = false, // overwrite if false
		bool bCloseFile = true // close file on last frame
	);

	bool Save( LPCTSTR lpszPathName );

	bool Save
	( 
		IStream* pIStream, 
		UINT nImageFormatID, 
		Image* pInputImage = NULL 
	);

	bool SaveAdd
	( 
		Image* pImage, 
		Size& szImageSizeInPixel, 
		LPCTSTR lpszPathName, 
		bool bAppend = false, // overwrite if false
		bool bCloseFile = true // close file on last frame
	);

	bool SaveSingleFrameImage
	( 
		Image* pImage, 
		Size& szImageSizeInPixel, 
		LPCTSTR lpszPathName, 
		bool bAppend = false, // overwrite if false
		bool bCloseFile = true // close file on last frame
	);

	bool SaveMultiFrameImage
	( 
		Image* pImage, 
		LPCTSTR lpszPathName, 
		bool bAppend = false, // overwrite if false
		bool bCloseFile = true // close file on last frame
	);

public:
	static bool IsImageFormatOpenSupported
	( 
		CString strImageFormat, bool bShowWarning = false 
	);

	static bool IsImageFormatSaveSupported
	( 
		CString strImageFormat, bool bShowWarning = false 
	);

	static bool IsImageFormatSupportMultiFrame( CString strImageFormat );

	static int GetEncoderClsid( LPCWSTR format, CLSID* pClsid );
	
	// WTB: more useful equivalent of the above for an outside application
	// since outside applications are probably not using wide character strings
	// and the extension is more likely to be known than the format string
	inline bool GetEncoderClsid( CString csExtention, CLSID* pClsid )
	{
		USES_CONVERSION;
		IMAGE_FORMAT_ID eImageFormatID;
		// get the image format id
		if ( !GetFormatIDByExt( T2CW( csExtention ), eImageFormatID ) )
		{
			return false;
		}
		WCHAR EncoderExt[ MAX_EXT_LEN ];
		// get the Encoder extension
		if ( !GetEncoderExtByID( eImageFormatID, EncoderExt ) )
		{
			return false;
		}
		const bool bOK = -1 != GetEncoderClsid( EncoderExt, pClsid );
		return bOK;
	}

	static bool GetFormatIDByExt( LPCWSTR pExt, UINT& nImageFormatID );
	
	// WTB: same as above but uses the enumeration
	inline bool GetFormatIDByExt( LPCWSTR pExt, IMAGE_FORMAT_ID& eImageFormatID )
	{
		UINT nImageFormatID = BMP_ID;
		const bool bOK = GetFormatIDByExt( pExt, nImageFormatID );
		eImageFormatID = (IMAGE_FORMAT_ID)nImageFormatID;
		return bOK;
	}

	static bool GetEncoderExtByID( UINT nImageFormatID, LPWSTR pEncoderExt );
	
	// WTB: same as above but uses the enumeration
	static bool GetEncoderExtByID( IMAGE_FORMAT_ID eImageFormatID, LPWSTR pEncoderExt )
	{
		const UINT uID = (UINT)eImageFormatID;
		return GetEncoderExtByID( uID, pEncoderExt );
	}

	// compare two images
	static IMAGE_COMPARE_RESULT CompareTwoImages
	(
		Bitmap* pImage1,
		Bitmap* pImage2,
		bool bAlwaysReturnDiff,
		Bitmap* pImageDiff = NULL 
	);

// private members
private:

// Attributes
private:
	float m_fDefaultWidth; // Do not why has a default width
	CString m_csPathname; // Image file name
	UINT m_nDimensionsCount; // Image dimensions count, used for multi-page image
	UINT m_nFrameCount; // Image page size
	GUID* m_pDimensionIDs; // image dimension's id, used for multi-page image
	Gdiplus::Image* m_pImage;
};
