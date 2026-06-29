/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <map>
#include <gdiplus.h>
using namespace Gdiplus;

#define MAX_EXT_LEN 20 

/////////////////////////////////////////////////////////////////////////////
// ImageFormat_ID_Ext
//
// Static lookup table mapping IMAGE_FORMAT_ID → file extension.
// Used for extension parsing and encoder lookup.
/////////////////////////////////////////////////////////////////////////////
typedef struct tagImageFormatIDExt
{
	UINT m_nFormatID;
	WCHAR m_szExt[ MAX_EXT_LEN ];

} IMAGEFORMAT_ID_EXT;

/////////////////////////////////////////////////////////////////////////////
// IMAGE_FORMAT_ID
//
// Enumeration of all standard GDI+ image format identifiers.
// These correspond to common file extensions and encoder types.
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
// IMAGE_COMPARE_RESULT
//
// Result codes for CompareTwoImages():
//   EQUAL                → images match exactly
//   DIFFERENT_SIZE       → dimensions differ
//   DIFFERENT_CONTENT    → pixel data differs
//   MEMORY_ALLOCATION_ERROR
//   UNKNOWN_ERROR
/////////////////////////////////////////////////////////////////////////////
enum IMAGE_COMPARE_RESULT
{
	EQUAL = 0,
	DIFFERENT_SIZE,
	DIFFERENT_CONTENT,
	MEMORY_ALLOCATION_ERROR,
	UNKNOWN_ERROR
};

/////////////////////////////////////////////////////////////////////////////
// IMAGEFORMAT_ID_EXT
//
// Maps a numeric IMAGE_FORMAT_ID to a file extension or encoder MIME type.
// Used for:
//   • Determining encoder CLSID
//   • Validating supported formats
//   • Mapping between extension → format ID → encoder string
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
// ImageFormat_ID_Ext
//
// Static lookup table mapping IMAGE_FORMAT_ID → file extension.
// Used for extension parsing and encoder lookup.
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
// IMAGE_FORMAT_ID
//
// Enumeration of all standard GDI+ image format identifiers.
// These correspond to common file extensions and encoder types.
/////////////////////////////////////////////////////////////////////////////
// 
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

/////////////////////////////////////////////////////////////////////////////
// CPlusGDI
//
// High‑level wrapper around GDI+ providing safe, convenient access to
// image loading, saving, format identification, multi‑frame handling,
// and device‑context drawing.
//
// Purpose:
//   • Abstract away raw GDI+ Image/Bitmap usage
//   • Provide unified open/save operations (file, stream, metafile)
//   • Support multi‑frame formats (TIFF, GIF, multi‑page EMF)
//   • Provide encoder lookup by extension or MIME type
//   • Provide conversion helpers (monochrome, scaling, padding)
//   • Provide device‑context drawing helpers (Draw, DIB brushes)
//   • Provide image comparison utilities
//
// Key Features:
//   • Open images from file or IStream
//   • Save images in any GDI+ encoder format
//   • Append frames to multi‑frame images (TIFF, GIF)
//   • Retrieve frame count and dimension IDs
//   • Convert images to monochrome
//   • Create DIB pattern brushes
//   • Extract HBITMAP, HICON, HENHMETAFILE
//   • Compare two images pixel‑by‑pixel
//
// Format Support:
//   • Uses IMAGE_FORMAT_ID and IMAGEFORMAT_ID_EXT tables to map
//     extensions → format IDs → encoder MIME types
//   • Supports both openable formats and savable formats
//
// This class is used throughout your imaging tools (PhotoPrinter,
// PhotoExplorer, PlotStudio) to provide a consistent, safe interface
// to GDI+ image manipulation.
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
	/////////////////////////////////////////////////////////////////////////////
	// IsOpen
	//
	// Returns true if an image is currently loaded (m_pImage != nullptr).
	/////////////////////////////////////////////////////////////////////////////
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
	/////////////////////////////////////////////////////////////////////////////
	// GetImage / operator Image*
	//
	// Provides direct access to the underlying GDI+ Image pointer.
	// Useful for advanced operations not wrapped by CPlusGDI.
	/////////////////////////////////////////////////////////////////////////////
	inline Gdiplus::Image* GetImage()
	{
		return m_pImage;
	}

	// cast to image pointer
	inline operator Gdiplus::Image*( )
	{
		return GetImage();
	}

	/////////////////////////////////////////////////////////////////////////////
	// IsMonochrome
	//
	// Returns true if the image is 1‑bit per pixel.
	/////////////////////////////////////////////////////////////////////////////
	inline bool IsMonochrome()
	{
		return GetBitsPerPixel() == 1;
	}

	/////////////////////////////////////////////////////////////////////////////
	// ImagePathname
	//
	// Gets or sets the filename associated with the image.
	// Used for metadata, logging, and save operations.
	/////////////////////////////////////////////////////////////////////////////
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
	/////////////////////////////////////////////////////////////////////////////
	// RotateFlip
	//
	// Applies GDI+ rotation or flip operations to the image.
	/////////////////////////////////////////////////////////////////////////////
	inline Status RotateFlip( Gdiplus::RotateFlipType rotateFlipType )
	{
		return m_pImage->RotateFlip( rotateFlipType );
	}

// public methods
public:
	/////////////////////////////////////////////////////////////////////////////
	// Open(IStream*, key)
	//
	// Loads an image from an IStream. Optional key is stored as pathname.
	// Supports memory streams, embedded resources, and custom loaders.
	/////////////////////////////////////////////////////////////////////////////
	bool Open( IStream* pStream, LPCTSTR key = 0 );

	/////////////////////////////////////////////////////////////////////////////
	// Open(path)
	//
	// Loads an image from a file path using GDI+ Image::FromFile().
	/////////////////////////////////////////////////////////////////////////////
	bool Open( LPCTSTR lpszPathName );

	/////////////////////////////////////////////////////////////////////////////
	// GetFrameCount
	//
	// Returns number of frames/pages in multi‑frame images (TIFF, GIF).
	/////////////////////////////////////////////////////////////////////////////
	int GetFrameCount()
	{
		return m_nFrameCount;
	}

	/////////////////////////////////////////////////////////////////////////////
	// GetWidth / GetHeight
	//
	// Returns physical pixel dimensions of the image.
	/////////////////////////////////////////////////////////////////////////////
	int GetWidth();
	
	// height in physical pixels
	int GetHeight();

	/////////////////////////////////////////////////////////////////////////////
	// GetWidthInches / GetHeightInches
	//
	// Converts pixel dimensions to physical inches using DPI.
	/////////////////////////////////////////////////////////////////////////////
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

	/////////////////////////////////////////////////////////////////////////////
	// GetBitsPerPixel
	//
	// Returns the image’s color depth (1, 8, 24, 32, etc.).
	/////////////////////////////////////////////////////////////////////////////
	long GetBitsPerPixel();

	/////////////////////////////////////////////////////////////////////////////
	// GetEnhMetafileHandle
	//
	// Extracts an enhanced metafile handle for vector formats (EMF).
	/////////////////////////////////////////////////////////////////////////////
	bool GetEnhMetafileHandle
	( 
		CDC& dcRef, HENHMETAFILE& hENH, int iFrameIndex = 0 
	);

	/////////////////////////////////////////////////////////////////////////////
	// GetDIBHandleForBrush / GetHBITMAP / GetHICON
	//
	// Converts the image into various Win32 handles for GDI operations.
	/////////////////////////////////////////////////////////////////////////////
	bool GetDIBHandleForBrush( CDC* pDC, HBITMAP& hBitmap );

	bool GetHBITMAP( HBITMAP& hBitmap );

	bool GetHICON( HICON& hIcon );

	/////////////////////////////////////////////////////////////////////////////
	// Draw
	//
	// Draws the entire image or a sub‑rectangle into a destination DC.
	// Supports scaling and cropping.
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
		CRect rectSrc( 0, 0, GetWidth(), GetHeight() );
		Draw( pDC, rectDest, rectSrc );
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// CreateDIBBitmapInfo / CreateDIBPatternBrush
	//
	// Creates DIB headers and pattern brushes for monochrome or color images.
	// Used for printing, previewing, and brush creation.
	/////////////////////////////////////////////////////////////////////////////
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

	/////////////////////////////////////////////////////////////////////////////
	// ConverToMonochrome
	//
	// Converts the image to 1‑bit monochrome using foreground/background colors.
	/////////////////////////////////////////////////////////////////////////////
	void ConverToMonochrome( COLORREF& rgbFG, COLORREF& rgbBG );

	/////////////////////////////////////////////////////////////////////////////
	// ScaleImage
	//
	// Scales a CBitmap to a new size using GDI+ interpolation.
	// Used for preview thumbnails and DPI adjustments.
	/////////////////////////////////////////////////////////////////////////////
	static CBitmap* ScaleImage( CDC* pDC, CBitmap*& pBitmap, bool bPreview = false );

public:
	// Initialize all members	
	void Initialize();

	// Free all resources
	void CleanUp();

	/////////////////////////////////////////////////////////////////////////////
	// Save(HENHMETAFILE, Size, Point, path, append, close)
	//
	// Saves an enhanced metafile to disk, optionally appending frames.
	/////////////////////////////////////////////////////////////////////////////
	bool Save
	( 
		HENHMETAFILE& hmf, // handle to enhanced metafile
		Size& szImageSizeInPixel, 
		Point& ptResolution, 
		LPCTSTR lpszPathName, 
		bool bAppend = false, // overwrite if false
		bool bCloseFile = true // close file on last frame
	);

	/////////////////////////////////////////////////////////////////////////////
	// Save(path)
	//
	// Saves the current image using its detected encoder.
	/////////////////////////////////////////////////////////////////////////////
	bool Save( LPCTSTR lpszPathName );

	/////////////////////////////////////////////////////////////////////////////
	// Save(IStream*, formatID, inputImage)
	//
	// Saves an image to an IStream using a specific encoder format.
	/////////////////////////////////////////////////////////////////////////////
	bool Save
	( 
		IStream* pIStream, 
		UINT nImageFormatID, 
		Image* pInputImage = NULL 
	);

	/////////////////////////////////////////////////////////////////////////////
	// SaveAdd / SaveSingleFrameImage / SaveMultiFrameImage
	//
	// Handles multi‑frame save operations (TIFF, GIF).
	// Supports append mode and frame‑by‑frame writing.
	/////////////////////////////////////////////////////////////////////////////
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
	/////////////////////////////////////////////////////////////////////////////
	// IsImageFormatOpenSupported / IsImageFormatSaveSupported
	//
	// Returns true if the given extension is supported for open/save.
	/////////////////////////////////////////////////////////////////////////////
	static bool IsImageFormatOpenSupported
	( 
		CString strImageFormat, bool bShowWarning = false 
	);

	static bool IsImageFormatSaveSupported
	( 
		CString strImageFormat, bool bShowWarning = false 
	);

	/////////////////////////////////////////////////////////////////////////////
	// IsImageFormatSupportMultiFrame
	//
	// Returns true if the format supports multiple frames/pages.
	/////////////////////////////////////////////////////////////////////////////
	static bool IsImageFormatSupportMultiFrame( CString strImageFormat );

	/////////////////////////////////////////////////////////////////////////////
	// GetEncoderClsid(format, pClsid)
	//
	// Retrieves the CLSID for the encoder matching the MIME type.
	// Required for GDI+ Image::Save().
	/////////////////////////////////////////////////////////////////////////////
	static int GetEncoderClsid( LPCWSTR format, CLSID* pClsid );
	
	/////////////////////////////////////////////////////////////////////////////
	// GetEncoderClsid(extension, pClsid)
	//
	// Convenience overload: determines encoder from file extension.
	// Uses extension → format ID → encoder MIME type → CLSID.
	/////////////////////////////////////////////////////////////////////////////
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

	/////////////////////////////////////////////////////////////////////////////
	// GetFormatIDByExt
	//
	// Maps a file extension (".jpg", "png") to an IMAGE_FORMAT_ID.
	/////////////////////////////////////////////////////////////////////////////
	static bool GetFormatIDByExt( LPCWSTR pExt, UINT& nImageFormatID );
	
	// WTB: same as above but uses the enumeration
	inline bool GetFormatIDByExt( LPCWSTR pExt, IMAGE_FORMAT_ID& eImageFormatID )
	{
		UINT nImageFormatID = BMP_ID;
		const bool bOK = GetFormatIDByExt( pExt, nImageFormatID );
		eImageFormatID = (IMAGE_FORMAT_ID)nImageFormatID;
		return bOK;
	}

	/////////////////////////////////////////////////////////////////////////////
	// GetEncoderExtByID
	//
	// Maps IMAGE_FORMAT_ID → encoder MIME type (e.g., "image/jpeg").
	/////////////////////////////////////////////////////////////////////////////
	static bool GetEncoderExtByID( UINT nImageFormatID, LPWSTR pEncoderExt );
	
	// WTB: same as above but uses the enumeration
	static bool GetEncoderExtByID( IMAGE_FORMAT_ID eImageFormatID, LPWSTR pEncoderExt )
	{
		const UINT uID = (UINT)eImageFormatID;
		return GetEncoderExtByID( uID, pEncoderExt );
	}

	/////////////////////////////////////////////////////////////////////////////
	// CompareTwoImages
	//
	// Compares two images pixel‑by‑pixel.
	//
	// Returns:
	//   EQUAL
	//   DIFFERENT_SIZE
	//   DIFFERENT_CONTENT
	//   MEMORY_ALLOCATION_ERROR
	//   UNKNOWN_ERROR
	//
	// Optionally writes a difference image to pImageDiff.
	/////////////////////////////////////////////////////////////////////////////
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
	/////////////////////////////////////////////////////////////////////////////
	// m_fDefaultWidth
	//
	// Legacy field used for metafile scaling and default DPI assumptions.
	/////////////////////////////////////////////////////////////////////////////
	float m_fDefaultWidth; // Do not why has a default width

	/////////////////////////////////////////////////////////////////////////////
	// m_csPathname
	//
	// Stores the filename or key associated with the image.
	/////////////////////////////////////////////////////////////////////////////
	CString m_csPathname; // Image file name

	/////////////////////////////////////////////////////////////////////////////
	// m_nDimensionsCount / m_nFrameCount / m_pDimensionIDs
	//
	// Used for multi‑frame image handling (TIFF/GIF).
	/////////////////////////////////////////////////////////////////////////////
	UINT m_nDimensionsCount; // Image dimensions count, used for multi-page image
	UINT m_nFrameCount; // Image page size
	GUID* m_pDimensionIDs; // image dimension's id, used for multi-page image

	/////////////////////////////////////////////////////////////////////////////
	// m_pImage
	//
	// Underlying GDI+ Image pointer. All operations are performed on this object.
	/////////////////////////////////////////////////////////////////////////////
	Gdiplus::Image* m_pImage;
};
