/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "CHelper.h"
#include "PlusGDI.h"

using namespace Gdiplus;

/////////////////////////////////////////////////////////////////////////////
CPlusGDI::CPlusGDI()
{
	Initialize();
}

/////////////////////////////////////////////////////////////////////////////
CPlusGDI::CPlusGDI(CBitmap* pBitmap)
{
	Initialize();
	m_pImage = (Image*)Bitmap::FromHBITMAP( *pBitmap, NULL );
}

/////////////////////////////////////////////////////////////////////////////
CPlusGDI::CPlusGDI(Bitmap* pBitmap)
{
	Initialize();
	m_pImage = (Image*)pBitmap;
}

/////////////////////////////////////////////////////////////////////////////
CPlusGDI::~CPlusGDI()
{
	CleanUp();

} // ~CPlusGDI()

/////////////////////////////////////////////////////////////////////////////
void CPlusGDI::Initialize()
{
	m_pImage = NULL;
	m_pDimensionIDs = NULL;
	m_nDimensionsCount = 0;
	m_fDefaultWidth = 0.0f;
	m_nFrameCount = 0;

} // Initialize

/////////////////////////////////////////////////////////////////////////////
void CPlusGDI::CleanUp()
{
	CHelper::SAFE_DELETE_ARRAY( m_pDimensionIDs );
	CHelper::SAFE_DELETE( m_pImage );

} // CleanUp

/////////////////////////////////////////////////////////////////////////////
// determines if the given file extension can be opened
bool CPlusGDI::IsImageFormatOpenSupported
(
	CString strImageFormat, 
	bool bShowWarning/* = false*/
)
{
	USES_CONVERSION;

	bool value = false;
	
	CString strExt = strImageFormat;

	// If there is a '.', remove it
	strExt.Remove( '.' );

	// get the image format id
	UINT nImageFormatID;
	if ( !GetFormatIDByExt( T2CW( strExt ), nImageFormatID ))
	{
		return value;
	}

	const int iCount = 
		sizeof( ImageFormatOpenSupported_ID ) / 
		sizeof( IMAGE_FORMAT_ID );

	for ( int iIndex = 0; iIndex < iCount; ++iIndex )
	{
		if ( nImageFormatID == ImageFormatOpenSupported_ID[ iIndex ] )
		{
			return true;
		}
	}

	if ( bShowWarning )
	{
		CString strWarningMessage;
		strWarningMessage.Format
		(
			_T( "Can not save %s image format!" ), strImageFormat
		);
		AfxMessageBox( strWarningMessage );
	}

	return value;
} // IsImageFormatOpenSupported

/////////////////////////////////////////////////////////////////////////////
// determines if the given file extension can be saved
bool CPlusGDI::IsImageFormatSaveSupported
(
	CString strImageFormat, 
	bool bShowWarning/* = false*/
)
{
	USES_CONVERSION;

	bool value = false;
	
	CString strExt = strImageFormat;
	
	// If there is a '.', remove it
	strExt.Remove( '.' );

	UINT nImageFormatID;
	// get the image format id
	if ( !GetFormatIDByExt( T2CW( strExt ), nImageFormatID ) )
	{
		return value;
	}

	const int iCount = 
		sizeof( ImageFormatSaveSupported_ID ) / 
		sizeof( IMAGE_FORMAT_ID );

	for ( int iIndex = 0; iIndex < iCount; ++iIndex)
	{
		if ( nImageFormatID == ImageFormatSaveSupported_ID[ iIndex ] )
		{
			return true;
		}
	}

	if ( bShowWarning)
	{
		CString strWarningMessage;
		strWarningMessage.Format
		(
			_T( "Can not save %s image format!" ), strImageFormat
		);
		AfxMessageBox( strWarningMessage );
	}

	return value;
}// IsImageFormatSaveSupported

/////////////////////////////////////////////////////////////////////////////
// Pass the image extension, like bmp or jpg, it will return true if this
// kind of image can be saved as multi-frames; otherwise, it will return false
bool CPlusGDI::IsImageFormatSupportMultiFrame( CString strImageFormat )
{
	USES_CONVERSION;

	bool value = false;
	
	CString strExt = strImageFormat;

	// If there is a '.', remove it
	strExt.Remove( '.' );

	// get the image format id
	UINT nImageFormatID;
	if ( !GetFormatIDByExt( T2CW( strExt ), nImageFormatID ))
	{
		return value;
	}

	// Currently, only tif format are able to be saved as multi-frame file
	value = nImageFormatID == TIF_ID || nImageFormatID == TIFF_ID;

	return value;
} // IsImageFormatSupportMultiFrame

/////////////////////////////////////////////////////////////////////////////
// open the image from an IStream interface pointer.  the optional
// key string is a string used to identify the image and is stored in the 
// image's pathname.
bool CPlusGDI::Open( IStream* pStream, LPCTSTR key )
{	// this seek needs to be here because of a quirk of GDI+ 
	LARGE_INTEGER li;
	li.QuadPart = 0;
	pStream->Seek( li, tagSTREAM_SEEK::STREAM_SEEK_SET, 0 );

	if ( m_pImage != 0 )
	{
		CleanUp();
		Initialize();
	}
	if ( key )
	{
		ImagePathname( key );
	}
	else
	{
		ImagePathname( _T( "" ) );
	}

	m_pImage = Image::FromStream( pStream );
	if ( m_pImage == 0 )
	{
		return false;
	}

	Status status = m_pImage->GetLastStatus();
	if ( status != Ok )
	{
		// Try again with use embedded color
		m_pImage = Image::FromStream( pStream, TRUE );
		if ( m_pImage == 0 )
		{
			return false;
		}

		status = m_pImage->GetLastStatus();
		if ( status != Ok )
		{
			// If it still fail, then return false
			return false;
		}
	}

	m_nDimensionsCount = m_pImage->GetFrameDimensionsCount();
	m_pDimensionIDs = new GUID[ m_nDimensionsCount ];

	// Get the list of frame dimensions from the Image (Image) object.
	m_pImage->GetFrameDimensionsList( m_pDimensionIDs, m_nDimensionsCount );

	// Get the number of frames in the first dimension.
	m_nFrameCount = m_pImage->GetFrameCount( &m_pDimensionIDs[ 0 ] );
	
	return m_pImage != 0;
} // Open

/////////////////////////////////////////////////////////////////////////////
// Open the given image path
bool CPlusGDI::Open( LPCTSTR lpszPathName )
{
	USES_CONVERSION;

	bool value = false;
	
	// close if already open
	if ( m_pImage != NULL )
	{
		CleanUp();
		Initialize();
	}

	ImagePathname( lpszPathName );

	m_pImage = Image::FromFile( T2CW( lpszPathName ), false );
	if ( m_pImage == NULL )
	{
		return value;
	}

	Status status = m_pImage->GetLastStatus();
	if ( status != Ok )
	{
		return value;
	}

	m_nDimensionsCount = m_pImage->GetFrameDimensionsCount();
	m_pDimensionIDs = new GUID[ m_nDimensionsCount ];

	// Get the list of frame dimensions from the Image (Image) object.
	m_pImage->GetFrameDimensionsList( m_pDimensionIDs, m_nDimensionsCount );

	// Get the number of frames in the first dimension.
	m_nFrameCount = m_pImage->GetFrameCount( &m_pDimensionIDs[ 0 ] );

	value = m_pImage != NULL;

	return value;
} // Open

/////////////////////////////////////////////////////////////////////////////
// Save the data inside pImage as file name lpszPathName
bool CPlusGDI::Save( LPCTSTR lpszPathName )
{
	USES_CONVERSION;

	bool value = false;
	
	CString csExt = CHelper::GetExtension( lpszPathName );

	// save and overwrite the selected image file with current page
	int iValue = 
		EncoderValue::EncoderValueVersionGif89 | 
		EncoderValue::EncoderValueCompressionLZW | 
		EncoderValue::EncoderValueFlush;

	EncoderParameters param;
	param.Count = 1;
	param.Parameter[ 0 ].Guid = EncoderSaveFlag;
	param.Parameter[ 0 ].Value = &iValue;
	param.Parameter[ 0 ].Type = EncoderParameterValueTypeLong;
	param.Parameter[ 0 ].NumberOfValues = 1;

	UINT nImageFormatID;
	// get the image format id
	if ( !GetFormatIDByExt( T2CW( csExt ), nImageFormatID ))
	{
		return value;
	}

	// Get the Encoder extension
	WCHAR EncoderExt[ MAX_EXT_LEN ];
	if ( !GetEncoderExtByID( nImageFormatID, EncoderExt ))
	{
		return value;
	}

	CLSID clsid;
	GetEncoderClsid( EncoderExt, &clsid );

	Status status = m_pImage->Save( T2CW( lpszPathName ), &clsid, &param );

	value = status == Ok;

	return value;
} // Save

/////////////////////////////////////////////////////////////////////////////
// Save the image data to stream as the specified image format
bool CPlusGDI::Save
(
	IStream* pIStream, 
	UINT nImageFormatID, 
	Image* pInputImage /*=NULL*/ 
)
{
	bool value = false;

	// Get the Encoder extension
	WCHAR EncoderExt[ MAX_EXT_LEN ];
	if ( !GetEncoderExtByID( nImageFormatID, EncoderExt ) )
	{
		return value;
	}

	int iValue = 
		EncoderValue::EncoderValueCompressionLZW | 
		EncoderValue::EncoderValueFlush;

	EncoderParameters param;
	param.Count = 1;
	param.Parameter[ 0 ].Guid = EncoderSaveFlag;
	param.Parameter[ 0 ].Value = &iValue;
	param.Parameter[ 0 ].Type = EncoderParameterValueTypeLong;
	param.Parameter[ 0 ].NumberOfValues = 1;
	
	// Get the encoder CLSID
	CLSID clsid;
	GetEncoderClsid( EncoderExt, &clsid );

	// If pInputImage is not null, the save its data; 
	// otherwise, save the current image's data
	Image* pSavedImage = pInputImage ? pInputImage : m_pImage;
	
	// Save the image to the stream
	Status SaveStatus = pSavedImage->Save( pIStream, &clsid, &param );

	value = SaveStatus == Ok;

	return value;
} // Save

/////////////////////////////////////////////////////////////////////////////
// Save the data inside the Enhanced Meta-file HMF as file name lpszPathName
// If bAppend is false, then overwrite the old data if the lpszPathName file
// already existed; otherwise, append the image to the tail of the image file
bool CPlusGDI::Save
(
	HENHMETAFILE& hmf, 
	Size& szImageSizeInPixel,
	Point& ptResolution, 
	LPCTSTR lpszPathName,
	bool bAppend/* = false*/, 
	bool bCloseFile/* = true */
)
{
	bool value = false;

	ImagePathname( lpszPathName );

	Metafile metafile( hmf );

	Bitmap image( szImageSizeInPixel.Width, szImageSizeInPixel.Height );
	image.SetResolution( (REAL)ptResolution.X, (REAL)ptResolution.Y );
	Graphics gp( &image );

	// Fill the background color with white
	Status status = gp.Clear( Color( 255, 255, 255, 255 ) );

	// Do we need to use an extra pixel for boundary gap?
	Rect rect( 0, 0, szImageSizeInPixel.Width, szImageSizeInPixel.Height );

	// draw the image, 
	status = gp.DrawImage
	(
		&metafile, rect, 0, 0,
		metafile.GetWidth(), metafile.GetHeight(),
		UnitPixel
	);

	if ( status != Ok )
	{
		return value;
	}

	value = SaveAdd
	(
		&image, szImageSizeInPixel, lpszPathName, bAppend, bCloseFile
	);

	return value;
} // Save

/////////////////////////////////////////////////////////////////////////////
bool CPlusGDI::SaveAdd
(
	Image* pImage, 
	Size& szImageSizeInPixel,
	LPCTSTR lpszPathName, 
	bool bAppend/* = false*/, 
	bool bCloseFile/* = true */
)
{
	bool value = false;

	CString csExt = CHelper::GetExtension(lpszPathName);

	if ( IsImageFormatSupportMultiFrame( csExt ))
	{
		value = SaveMultiFrameImage
		(
			pImage, lpszPathName, bAppend, bCloseFile 
		);
	} else
	{
		value = SaveSingleFrameImage
		(
			pImage, szImageSizeInPixel, lpszPathName, bAppend, bCloseFile 
		);
	}

	return value;
} // SaveAdd

/////////////////////////////////////////////////////////////////////////////
// Save the data inside pImage as file name lpszPathName
// If bAppend is false, then overwrite the old data if the lpszPathName file
// already existed; otherwise, append the image to the tail of the image file
bool CPlusGDI::SaveSingleFrameImage
(
	Image* pImage,
	Size& szImageSizeInPixel,
	LPCTSTR lpszPathName,
	bool bAppend/* = false*/, 
	bool bCloseFile/* = true */
)
{
	USES_CONVERSION;

	bool value = false;
	
	// The File named lpszPathName exists, and we just want to append the 
	// new image to the file so we need to open the image from the existing
	// file
	if ( m_pImage == NULL && bAppend )
	{
		if ( !Open( lpszPathName ))
		{
			return false;
		}
	}

	int iWidthNewImage;
	int iHeightNewImage;
	int iWidthOriginalImage = GetWidth();
	int iHeightOriginalImage = GetHeight();
	if ( m_pImage == NULL )
	{
		iWidthNewImage = szImageSizeInPixel.Width;
		iHeightNewImage = szImageSizeInPixel.Height;
	} 
	else
	{
		iWidthNewImage = max(iWidthOriginalImage, szImageSizeInPixel.Width);
		iHeightNewImage = iHeightOriginalImage + szImageSizeInPixel.Height;
	}

	Status status;
	if ( m_pImage != NULL )
	{	Bitmap image( iWidthNewImage, iHeightNewImage );
		image.SetResolution
		(
			(REAL)GetHorizontalResolution(), 
			(REAL)GetVerticalResolution()
		);

		Graphics gp( &image );
		Status status = gp.Clear( Color( 255, 255, 255, 255 ));

		// Do we need to use an extra pixel for boundary gap?
		// Draw the original image first
		Rect rect( 0, 0, iWidthOriginalImage, iHeightOriginalImage + 1 );  

		// draw the image, 
		status = gp.DrawImage
		(
			m_pImage, rect, 0, 0, iWidthOriginalImage, iHeightOriginalImage, 
			UnitPixel 
		);

		// Second append the new image to the tail
		rect = Rect
		(
			0, iHeightOriginalImage, 
			szImageSizeInPixel.Width, 
			szImageSizeInPixel.Height
		);

		// draw the image, 
		status = gp.DrawImage
		(
			pImage, rect, 0, 0, 
			pImage->GetWidth(), pImage->GetHeight(), 
			UnitPixel 
		);

		CHelper::SAFE_DELETE( m_pImage );
		m_pImage = ((Image&)image).Clone();
		if ( status != Ok )
		{
			return value;
		}
	} else
	{
		m_pImage = pImage->Clone();
	}
	
	if ( !bCloseFile )
	{
		return true;
	}

	// save and overwrite the selected image file with current page
	int iValue = 
		EncoderValue::EncoderValueFlush | 
		EncoderValue::EncoderValueCompressionLZW;

	EncoderParameters param;
	param.Count = 1;
	param.Parameter[ 0 ].Guid = EncoderSaveFlag;
	param.Parameter[ 0 ].Value = &iValue;
	param.Parameter[ 0 ].Type = EncoderParameterValueTypeLong;
	param.Parameter[ 0 ].NumberOfValues = 1;

	CString csExt = CHelper::GetExtension( lpszPathName );
	UINT nImageFormatID;
	// get the image format id
	if ( !GetFormatIDByExt( T2CW( csExt ), nImageFormatID ))
	{
		return value;
	}

	// Get the Encoder extension
	WCHAR EncoderExt[ MAX_EXT_LEN ];
	if ( !GetEncoderExtByID( nImageFormatID, EncoderExt ))
	{
		return value;
	}
	
	CLSID clsid;
	GetEncoderClsid( EncoderExt, &clsid );

	// Save the image
	status = m_pImage->Save( T2CW( lpszPathName ), &clsid, &param );

	value = status == Ok;

	return value;
} // SaveSingleFrameImage

/////////////////////////////////////////////////////////////////////////////
bool CPlusGDI::SaveMultiFrameImage
(
	Image* pImage,
	LPCTSTR lpszPathName,
	bool bAppend /* = false*/,
	bool bCloseFile /* = true */ 
)
{
	USES_CONVERSION;

	bool value = false;
	
	// The File named lpszPathName exists, and we just want to 
	// append the new image to the file so we need to open the 
	// image from the existing file
	Status status;
	if ( m_pImage == NULL && bAppend )
	{
		if ( !Open( lpszPathName ))
		{
			return value;
		}
	}
	
	// save the current page to the tail of selected image file
	if ( bAppend && m_pImage != NULL )
	{
		int iValue = 
			EncoderValue::EncoderValueMultiFrame | 
			EncoderValue::EncoderValueFrameDimensionPage | 
			EncoderValue::EncoderValueCompressionLZW;

		if ( bCloseFile)
		{
			iValue |= EncoderValue::EncoderValueFlush;
		}

		EncoderParameters param;
		param.Count = 1;
		param.Parameter[ 0 ].Guid = EncoderSaveFlag;
		param.Parameter[ 0 ].Value = &iValue;
		param.Parameter[ 0 ].Type = EncoderParameterValueTypeLong;
		param.Parameter[ 0 ].NumberOfValues = 1;

		// Append the passed-in image to the original one
		status = m_pImage->SaveAdd(pImage, &param);

	} else // save and overwrite the selected image file with current page
	{
		int iValue = 
			EncoderValue::EncoderValueMultiFrame | 
			EncoderValue::EncoderValueCompressionLZW;

		if ( bCloseFile )
		{
			iValue |= EncoderValue::EncoderValueFlush;
		}

		EncoderParameters param;
		param.Count = 1;
		param.Parameter[ 0 ].Guid = EncoderSaveFlag;
		param.Parameter[ 0 ].Value = &iValue;
		param.Parameter[ 0 ].Type = EncoderParameterValueTypeLong;
		param.Parameter[ 0 ].NumberOfValues = 1;

		CString csExt = CHelper::GetExtension( lpszPathName );

		// get the image format id
		UINT nImageFormatID;
		if ( !GetFormatIDByExt( T2CW( csExt ), nImageFormatID ))
		{
			return value;
		}

		// Get the Encoder extension
		WCHAR EncoderExt[ MAX_EXT_LEN ];
		if ( !GetEncoderExtByID( nImageFormatID, EncoderExt ))
		{
			return value;
		}

		CLSID clsid;
		GetEncoderClsid( EncoderExt, &clsid );

		// If there is no instance for m_pImage, we need to clone one 
		// from the passed-in image
		if ( m_pImage == NULL )
		{
			m_pImage = pImage->Clone();
		}

		// Save the image
		if ( lpszPathName )
		{
			status = m_pImage->Save( T2CW( lpszPathName ), &clsid, &param );
			for ( UINT nFrame = 1; nFrame < m_nFrameCount; nFrame++ )
			{
				m_pImage->SelectActiveFrame( &m_pDimensionIDs[ 0 ], nFrame );
				SaveMultiFrameImage( m_pImage, NULL, true, false );
			}
		}
	}

	value = status == Ok;

	return value;
} // SaveMultiFrameImage

/////////////////////////////////////////////////////////////////////////////
// Get the encoder extension corresponding to the image format id
bool CPlusGDI::GetEncoderExtByID( UINT nImageFormatID, LPWSTR pEncoderExt )
{
	bool value = false;

	const int iCount = 
		sizeof( ImageFormat_ID_EncoderExt ) / 
		sizeof( IMAGEFORMAT_ID_EXT );

	for ( int iIndex = 0; iIndex < iCount; ++iIndex )
	{
		if ( nImageFormatID == ImageFormat_ID_EncoderExt[ iIndex ].m_nFormatID)
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
// Get the image format id corresponding to the image extension
bool CPlusGDI::GetFormatIDByExt( LPCWSTR pExt, UINT& nImageFormatID)
{
	bool value = false;

	CString strExt( pExt );

	//If there is a '.', remove it
	strExt.Remove( '.' );
	const int iCount =
		sizeof( ImageFormat_ID_Ext ) / 
		sizeof( IMAGEFORMAT_ID_EXT );

	for ( int iIndex = 0; iIndex < iCount; ++iIndex )
	{
		if
		( 
			strExt.CompareNoCase
			( 
				CString( ImageFormat_ID_Ext[ iIndex ].m_szExt )
			) == 0
		)
		{
			nImageFormatID = ImageFormat_ID_Ext[ iIndex ].m_nFormatID;
			return true;
		}
	}

	return value;
} // GetFormatIDByExt

/////////////////////////////////////////////////////////////////////////////
// Get image width in pixels
int CPlusGDI::GetWidth()
{
	int value = 0;
	if ( !m_pImage )
	{
		return value;
	}

	value = m_pImage->GetWidth();

	return value;
} // GetWidth

/////////////////////////////////////////////////////////////////////////////
// Get image height in pixels
int CPlusGDI::GetHeight()
{
	int value = 0;
	if ( !m_pImage )
	{
		return value;
	}

	value = m_pImage->GetHeight();

	return value;
} // GetHeight

/////////////////////////////////////////////////////////////////////////////
// Get image width in inches
double CPlusGDI::GetWidthInches()
{
	double value = 0.0;

	if ( !m_pImage )
	{
		return value;
	}

	const int nWidth = m_pImage->GetWidth();
	const double xPelsPerInch = m_pImage->GetHorizontalResolution();
	value = ( (double)nWidth ) / xPelsPerInch;

	return value;
} // GetWidthInches

/////////////////////////////////////////////////////////////////////////////
// Get image height in inches	
double CPlusGDI::GetHeightInches()
{
	double value = 0.0;

	if ( !m_pImage )
	{
		return value;
	}

	const int nHeight = m_pImage->GetHeight(); 
	const double yPelsPerInch = m_pImage->GetVerticalResolution();
	value = ((double)nHeight) / yPelsPerInch;

	return value;
} // GetHeightInches

/////////////////////////////////////////////////////////////////////////////
// get the metafile handle for the iFrameIndex-th page. The default
// is the first page
bool CPlusGDI::GetEnhMetafileHandle 
(
	CDC& dcRef, 
	HENHMETAFILE& hENH,
	int iFrameIndex/*=0*/ 
)
{
	bool value = false;
	if ( !m_pImage )
	{
		return value;
	}

	// Select the page 
	m_pImage->SelectActiveFrame( &m_pDimensionIDs[ 0 ], iFrameIndex );
	
	// create a new bitmap adjusted in size based on device resolution
	const int nWidth = m_pImage->GetWidth();
	const int nHeight = m_pImage->GetHeight();
	if ( nWidth <= 0 || nHeight <= 0 )
	{
		return value;
	}

	const double dWidthInInches = GetWidthInches();
	const double dPageWidth = 
		( m_fDefaultWidth == 0 ) ? dWidthInInches : m_fDefaultWidth;
	const double dPageHeight = GetHeightInches();	
	
	// Create a metafile device context, the image will be drawn on it
	CMetaFileDC dc;
	if ( !dc.CreateEnhanced( &dcRef, 0, NULL, ImagePathname()))
	{
		return value;
	}

	// select dc into graphics
	Graphics gp( dc.GetSafeHdc());

	// draw the image, so the image will be drawn on dc
	Status status = gp.DrawImage
	(
		m_pImage, 0, 0, 0, 0, nWidth, nHeight, UnitPixel 
	);

	// retrieve the metafile handle
	value = status == Ok;
	ASSERT( value == true );

	hENH = dc.CloseEnhanced();

	return value;
} // GetEnhMetafileHandle

/////////////////////////////////////////////////////////////////////////////
// Get bitmap handle for creating a brush
bool CPlusGDI::GetDIBHandleForBrush( CDC* pDC, HBITMAP& hBitmap )
{
	bool value = false;
	if ( !m_pImage )
	{
		return value;
	}

	int iWidth = GetImagePaddingWidth( m_pImage->GetWidth() );
	int iHeight = m_pImage->GetHeight();

	// Create true color header
	COLORREF rgbFG = RGB( 0, 0, 0 );
	COLORREF rgbBG = RGB( 255, 255, 255 );
	Size size = Size(iWidth, iHeight);
	BITMAPINFO* pbmpinfo =
		CreateDIBBitmapInfo( pDC, size, rgbFG, rgbBG );

	// Create memory device context
	CDC dcMemTarget;
	dcMemTarget.CreateCompatibleDC( pDC );
	dcMemTarget.SetMapMode( MM_TEXT );

	// Create a DIB bitmap handle
	LPVOID pBuffer;
	hBitmap = ::CreateDIBSection
	(
		dcMemTarget.GetSafeHdc(),
		pbmpinfo,
		DIB_RGB_COLORS,
		(void**)&pBuffer,
		NULL,
		0 
	);

	// select the created bitmap into dcMem
	HBITMAP hOldBitmap = (HBITMAP)::SelectObject
	( 
		dcMemTarget.GetSafeHdc(), hBitmap 
	);

	//select dc into graphics
	Graphics gp( dcMemTarget.GetSafeHdc() );
	gp.SetPageUnit( Gdiplus::Unit::UnitPixel );

	// draw the image, so the image will be drawn on dc
	RectF rect( (REAL)0, (REAL)0, (REAL)iWidth, (REAL)iHeight );
	gp.DrawImage
	( 
		m_pImage, rect, (REAL)0, (REAL)0,
		(REAL)m_pImage->GetWidth(), (REAL)m_pImage->GetHeight(),
		UnitPixel
	);


	::SelectObject( dcMemTarget.GetSafeHdc(), hOldBitmap );
	CHelper::SAFE_DELETE( pbmpinfo );

	return true;
} // GetDIBHandle

/////////////////////////////////////////////////////////////////////////////
// Get bitmap handle for current image
bool CPlusGDI::GetHBITMAP(HBITMAP& hBitmap)
{
	bool value = false;
	if ( m_pImage == NULL )
	{
		return false;
	}

	Bitmap image( GetWidth(), GetHeight() );
	image.SetResolution
	(
		(REAL)GetHorizontalResolution(), (REAL)GetVerticalResolution()
	);

	Graphics gp( &image );

	// Fill the background color with white
	Status status = gp.Clear( Color( 255, 255, 255, 255 ) );
	
	// draw the image, 
	status = gp.DrawImage( m_pImage, 0, 0 );
	Color backGround( 255, 255, 255, 255 );
	status = image.GetHBITMAP( backGround, &hBitmap );

	value = status == Ok;

	return value;
} // GetHBITMAP

/////////////////////////////////////////////////////////////////////////////
// Get icon handle for current image
bool CPlusGDI::GetHICON(HICON& hIcon)
{
	bool value = false;
	if ( m_pImage == NULL )
	{
		return false;
	}

	Bitmap image( GetWidth(), GetHeight() );
	image.SetResolution
	(
		(REAL)GetHorizontalResolution(), (REAL)GetVerticalResolution()
	);

	Graphics gp( &image );

	// Fill the background color with white
	Status status = gp.Clear( Color( 255, 255, 255, 255 ) );

	// draw the image, 
	status = gp.DrawImage( m_pImage, 0, 0 );
	Color backGround( 255, 255, 255, 255 );
	status = image.GetHICON( &hIcon );

	value = status == Ok;

	return value;
} // GetHICON

/////////////////////////////////////////////////////////////////////////////
// return the number of bits used for each pixel
long CPlusGDI::GetBitsPerPixel()
{
	long value = -1;
	PixelFormat pixelFormat = m_pImage->GetPixelFormat();
	switch ( pixelFormat )
	{
		case PixelFormat1bppIndexed:
			value = 1;
			break;
		case PixelFormat4bppIndexed:
			value = 4;
			break;
		case PixelFormat8bppIndexed:
			value = 8;
			break;
		case PixelFormat16bppARGB1555:
		case PixelFormat16bppGrayScale:
		case PixelFormat16bppRGB555:
		case PixelFormat16bppRGB565:
			value = 16;
			break;
		case PixelFormat24bppRGB:
			value = 24;
			break;
		case PixelFormat32bppARGB:
		case PixelFormat32bppPARGB:
		case PixelFormat32bppRGB:
			value = 32;
			break;
		case PixelFormat48bppRGB:
			value = 48;
			break;
		case PixelFormat64bppARGB:
		case PixelFormat64bppPARGB:
			value = 64;
			break;
		default:
			ASSERT( 0 );
			break;
	}

	return value;
} // GetBitsPerPixel

/////////////////////////////////////////////////////////////////////////////
// Get the CLSID for the specified image format
int CPlusGDI::GetEncoderClsid( LPCWSTR format, CLSID* pClsid )
{
	int value = -1;
	// Get the string length
	size_t unLength = wcslen( format );

	UINT num = 0;
	UINT size = 0;
	GetImageEncodersSize( &num, &size );
	if ( size == 0 )
	{
		return value;
	}

	ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)malloc( size );
	if ( pImageCodecInfo == NULL )
	{
		return value;
	}

	GetImageEncoders( num, size, pImageCodecInfo );
	for ( UINT nIndex = 0; nIndex < num; ++nIndex )
	{
		if ( wcsncmp( pImageCodecInfo[ nIndex ].MimeType, format, unLength ) == 0 )
		{
			*pClsid = pImageCodecInfo[ nIndex ].Clsid;
			free( pImageCodecInfo );
			return nIndex;
		}
	}

	free( pImageCodecInfo );

	return value;
} // GetEncoderClsid

/////////////////////////////////////////////////////////////////////////////
void CPlusGDI::Draw( CDC* pDC, CRect& rectDest, CRect& rectSrc )
{
	Rect destRect
	(
		rectDest.left, rectDest.top, rectDest.Width(), rectDest.Height()
	);

	HBITMAP hBitmap = NULL;
	GetHBITMAP( hBitmap );
	CBitmap* pBitmap = CBitmap::FromHandle( hBitmap );

	COLORREF rgbFG = RGB( 0, 0, 0 );
	COLORREF rgbBG = RGB( 255, 255, 255 );

	// Create the bitmap info
	Size size = Size(GetWidth(), GetHeight());
	BITMAPINFO* pSourceBmpinfo = CreateDIBBitmapInfo
	( 
		pDC, size, rgbFG, rgbBG 
	);

	// Get the image byte size
	int iSizeData = IsMonochrome() ?
		(int)ceil
		( 
			(double)GetImagePaddingWidth( GetWidth() ) *
			(double)GetHeight() / 8.0
		) :
		GetWidth() * GetHeight() * 4;

	LPVOID lpSourceBits = new byte[ iSizeData ];

	// Retrieve the image data
	pBitmap->GetBitmapBits( iSizeData, lpSourceBits );

	int iNewStrechMode = IsMonochrome() ? BLACKONWHITE : COLORONCOLOR;

	// Set the stretchmode
	int iOldStrechMode = ::SetStretchBltMode( pDC->m_hDC, iNewStrechMode );

	// Since the image is store upside down, so we need to reverse it back
	pSourceBmpinfo->bmiHeader.biHeight = -pSourceBmpinfo->bmiHeader.biHeight;

	// Stretch and copy the image to the destination place
	::StretchDIBits
	( 
		pDC->m_hDC, destRect.GetLeft(), destRect.GetTop(),
		destRect.Width, destRect.Height,
		rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(),
		lpSourceBits, pSourceBmpinfo, DIB_RGB_COLORS, SRCCOPY
	);

	::SetStretchBltMode( pDC->m_hDC, iOldStrechMode );

	// clean up
	CHelper::SAFE_DELETE_ARRAY( lpSourceBits );
	CHelper::SAFE_DELETE_ARRAY( pSourceBmpinfo );
} // Draw

/////////////////////////////////////////////////////////////////////////////
bool CPlusGDI::CreateDIBPatternBrush
(
	CDC* pDC, CBitmap*& pBitmap, 
	COLORREF& rgbFG, COLORREF& rgbBG, HBRUSH& hBrush
)
{
	CBitmap* pBitmapScaled = CPlusGDI::ScaleImage( pDC, pBitmap );

	CPlusGDI image( pBitmapScaled );
	if ( image.IsMonochrome() )
	{
		image.ConverToMonochrome( rgbFG, rgbBG );
	}

	int iWidth = image.GetWidth();
	int iHeight = image.GetHeight();
	
	// brush requires a square bitmap
	iWidth = iWidth >= iHeight ? iWidth : iHeight;

	COLORREF rgbFG1 = RGB( 0, 0, 0 );
	COLORREF rgbBG1 = RGB( 255, 255, 255 );
	Size size = Size(image.GetWidth(), image.GetHeight());
	BITMAPINFO* pSourceBmpinfo = image.CreateDIBBitmapInfo
	(
		pDC, size, rgbFG1, rgbBG1
	);

	Size size2 = Size(iWidth, iHeight);
	BITMAPINFO* pbmpinfo = image.CreateDIBBitmapInfo
	( 
		pDC, size2, rgbFG, rgbBG 
	);

	int iSizeData = 
		image.IsMonochrome() ? 
		(int)ceil
		(
			(double)image.GetImagePaddingWidth( image.GetWidth()) * 
			(double)image.GetHeight() / 8.0
		) : 
		image.GetWidth() * image.GetHeight() * 4;

	LPVOID lpSourceBits = new byte[ iSizeData ];
	pBitmapScaled->GetBitmapBits( iSizeData, lpSourceBits );

	// Create memory device context
	CDC dcMem;
	dcMem.CreateCompatibleDC( pDC );

	// Create a DIB bitmap handle
	LPVOID pBuffer;
	HBITMAP hBitmap = ::CreateDIBSection
	(
		dcMem.GetSafeHdc(),
		pbmpinfo,
		DIB_RGB_COLORS,
		(void**)&pBuffer,
		NULL,
		0
	);

	// select the created bitmap into dcMem
	HBITMAP hOldBitmap = ( HBITMAP )::SelectObject( dcMem.GetSafeHdc(), hBitmap );

	int iNewStrechMode = image.IsMonochrome() ? BLACKONWHITE : COLORONCOLOR;
	int iOldStrechMode = ::SetStretchBltMode( dcMem.GetSafeHdc(), iNewStrechMode );

	::StretchDIBits
	(
		dcMem.GetSafeHdc(), 0, 0, iWidth, iHeight,
		0, 0, image.GetWidth(), image.GetHeight(),
		lpSourceBits, pSourceBmpinfo, DIB_RGB_COLORS, SRCCOPY
	);

	::SetStretchBltMode( dcMem.GetSafeHdc(), iOldStrechMode );

	::SelectObject( dcMem.GetSafeHdc(), hOldBitmap );

	Size szImage = Size( iWidth, iHeight );
	hBrush = image.CreateDIBPatternBrushFromBuffer( pbmpinfo, pBuffer, szImage );

	// clean up
	CHelper::SAFE_DELETE_ARRAY( pbmpinfo );
	DeleteObject( hBitmap );
	pBitmapScaled->DeleteObject();
	CHelper::SAFE_DELETE_ARRAY( lpSourceBits );
	CHelper::SAFE_DELETE_ARRAY( pSourceBmpinfo );

	return true;
} // CreateDIBPatternBrush

/////////////////////////////////////////////////////////////////////////////
BITMAPINFO* CPlusGDI::CreateDIBBitmapInfo
(
	CDC* pDC, Size& szSizeInPixel, COLORREF& rgbFG, COLORREF& rgbBG
)
{	// if the original bitmap is monochrome, leave it that way
	// otherwise convert to true color
	bool bMonochrome = IsMonochrome();
	int iBitsPerPixel = bMonochrome ? 1 : 32;
	int iSizeBmpInfo = GetSizeDIBBitmapInfo();

	// Allocate memory for info header
	BITMAPINFO* pbmpinfo = ( BITMAPINFO* )new byte[ iSizeBmpInfo ];
	memset( pbmpinfo, 0, iSizeBmpInfo );

	// Create true color header
	pbmpinfo->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	pbmpinfo->bmiHeader.biWidth = szSizeInPixel.Width;
	pbmpinfo->bmiHeader.biHeight = szSizeInPixel.Height;
	pbmpinfo->bmiHeader.biPlanes = 1;
	pbmpinfo->bmiHeader.biBitCount = iBitsPerPixel;
	pbmpinfo->bmiHeader.biCompression = BI_RGB;
	pbmpinfo->bmiHeader.biSizeImage = 0;
	pbmpinfo->bmiHeader.biXPelsPerMeter =
		(int)( 1000.0f * (float)pDC->GetDeviceCaps( LOGPIXELSX ) / 25.4f );
	pbmpinfo->bmiHeader.biYPelsPerMeter =
		(int)( 1000.0f * (float)pDC->GetDeviceCaps( LOGPIXELSX ) / 25.4f );
	pbmpinfo->bmiHeader.biClrUsed = bMonochrome ? 2 : 0;
	pbmpinfo->bmiHeader.biClrImportant = bMonochrome ? 2 : 0;
	if ( bMonochrome )
	{
		//If it is monchrome, set the foreground and background color
		pbmpinfo->bmiColors[ 0 ].rgbBlue = GetBValue( rgbFG );
		pbmpinfo->bmiColors[ 0 ].rgbGreen = GetGValue( rgbFG );
		pbmpinfo->bmiColors[ 0 ].rgbRed = GetRValue( rgbFG );
		pbmpinfo->bmiColors[ 1 ].rgbBlue = GetBValue( rgbBG );
		pbmpinfo->bmiColors[ 1 ].rgbGreen = GetGValue( rgbBG );
		pbmpinfo->bmiColors[ 1 ].rgbRed = GetRValue( rgbBG );
	}

	return pbmpinfo;
} // CreateDIBBitmapInfo

/////////////////////////////////////////////////////////////////////////////
HBRUSH CPlusGDI::CreateDIBPatternBrushFromBuffer
(
	BITMAPINFO*& pbmpinfo, LPVOID& pBuffer, Size& szImage
)
{
	bool bMonochrome = IsMonochrome();
	int iSizeBmpInfo = GetSizeDIBBitmapInfo();
	int iWidth = GetImagePaddingWidth( szImage.Width );
	int iSizeImage = bMonochrome ? 
		(int)ceil
		(
			(double)iWidth * (double)szImage.Height / 8.0
		) : 
		iWidth * szImage.Height * 4;

	byte* pData = new byte[ iSizeBmpInfo + iSizeImage ];
	memset( pData, 0, iSizeBmpInfo + iSizeImage );
	memcpy( pData, pbmpinfo, iSizeBmpInfo );
	pData += iSizeBmpInfo;
	memcpy( pData, pBuffer, iSizeImage );
	pData -= iSizeBmpInfo;

	HBRUSH value = ::CreateDIBPatternBrushPt( pData, DIB_RGB_COLORS );

	// Free the data
	CHelper::SAFE_DELETE_ARRAY( pData );

	return value;
} // CreateDIBPatternBrushFromBuffer

/////////////////////////////////////////////////////////////////////////////
int CPlusGDI::GetImagePaddingWidth( int iWidth )
{
	if ( IsMonochrome() )
		return 32 * (int)ceil( ( (double)iWidth ) / 32.0 );
	else
		return iWidth;
} // GetImagePaddingWidth

/////////////////////////////////////////////////////////////////////////////
void CPlusGDI::ConverToMonochrome( COLORREF& rgbFG, COLORREF& rgbBG )
{
	ColorPalette* pPalette =
		( ColorPalette* )new byte[ sizeof( ColorPalette ) + 2 * sizeof( ARGB ) ];
	pPalette->Count = 2;
	pPalette->Flags = PaletteFlagsGrayScale;
	pPalette->Entries[ 0 ] = MAKELONG
	( 
		MAKEWORD( GetBValue( rgbFG ), GetGValue( rgbFG ) ),
		MAKEWORD( GetRValue( rgbFG ), 255 )
	);

	pPalette->Entries[ 1 ] = MAKELONG
	( 
		MAKEWORD( GetBValue( rgbBG ), GetGValue( rgbBG ) ),
		MAKEWORD( GetRValue( rgbBG ), 255 )
	);

	m_pImage->SetPalette( pPalette );
	CHelper::SAFE_DELETE_ARRAY( pPalette );
} // ConverToMonochrome

/////////////////////////////////////////////////////////////////////////////
CBitmap* CPlusGDI::ScaleImage
(
	CDC* pDC, 
	CBitmap*& pBitmap, 
	bool bPreview/*=false*/
)
{
	CPlusGDI image( pBitmap );
	int iWidth = image.GetWidth();
	int iHeight = image.GetHeight();

	//Get the resolution
	int iPixelPerInchX = ::GetDeviceCaps
	( pDC->m_hAttribDC ? pDC->m_hAttribDC : pDC->m_hDC, LOGPIXELSX
	);
	int iPixelPerInchY = ::GetDeviceCaps
	( pDC->m_hAttribDC ? pDC->m_hAttribDC : pDC->m_hDC, LOGPIXELSY
	);

	if ( bPreview )
	{
		//if it is preview, we use monitor's resolution
		CClientDC dc( NULL );
		iPixelPerInchX = ::GetDeviceCaps
		( dc.m_hAttribDC ? dc.m_hAttribDC : dc.m_hDC, LOGPIXELSX
		);
		iPixelPerInchY = ::GetDeviceCaps
		( dc.m_hAttribDC ? dc.m_hAttribDC : dc.m_hDC, LOGPIXELSY
		);
	}

	// We scale the image, 256 pixel is one inch. 
	// For the smaller than 32 pixel, we use 1/6 inch
	int iScaledWidth = iWidth <= 32 ?
		iPixelPerInchX / 6 :
		(int)min( (float)iPixelPerInchX * ( (float)iWidth ) / 256.0f, 1024 );
	int iScaledHeight = iHeight <= 32 ?
		iPixelPerInchY / 6 :
		(int)min( (float)iPixelPerInchY * ( (float)iHeight ) / 256.0f, 1024 );
	iScaledWidth = image.GetImagePaddingWidth( iScaledWidth );

	COLORREF rgbFG = RGB( 0, 0, 0 );
	COLORREF rgbBG = RGB( 255, 255, 255 );
	Size size = Size(iWidth, iHeight);
	BITMAPINFO* pbmpinfoSource = image.CreateDIBBitmapInfo
	( 
		pDC, size, rgbFG, rgbBG 
	);

	Size size2 = Size(iScaledWidth, iScaledHeight);
	BITMAPINFO* pbmpinfo = image.CreateDIBBitmapInfo
	( 
		pDC, size2, rgbFG, rgbBG
	);

	int iSizeData = image.IsMonochrome() ?
		(int)ceil
		( 
			(double)image.GetImagePaddingWidth( iWidth ) *
			(double)iHeight / 8.0
		) :
		iWidth * iHeight * 4;

	LPVOID lpSourceBits = new byte[ iSizeData ];
	pBitmap->GetBitmapBits( iSizeData, lpSourceBits );

	// Create memory device context
	CDC dcMemTarget;
	dcMemTarget.CreateCompatibleDC( pDC );
	dcMemTarget.SetMapMode( MM_TEXT );

	// Create a DIB bitmap handle
	LPVOID pBuffer;
	HBITMAP hScaledBitmap = ::CreateDIBSection
	(
		dcMemTarget.GetSafeHdc(),
		pbmpinfo,
		DIB_RGB_COLORS,
		(void**)&pBuffer,
		NULL,
		0 
	);

	// select the created bitmap into dcMem
	HBITMAP hOldTarget = ( HBITMAP )::SelectObject
	( 
		dcMemTarget.GetSafeHdc(), hScaledBitmap
	);

	int iNewStrechMode =
		image.IsMonochrome() ? BLACKONWHITE : COLORONCOLOR;
	int iOldStrechMode =
		::SetStretchBltMode( dcMemTarget.GetSafeHdc(), iNewStrechMode );

	::StretchDIBits
	( 
		dcMemTarget.GetSafeHdc(), 0, 0, iScaledWidth, iScaledHeight,
		0, 0, iWidth, iHeight, lpSourceBits, pbmpinfoSource,
		DIB_RGB_COLORS, SRCCOPY
	);

	::SetStretchBltMode( dcMemTarget.GetSafeHdc(), iOldStrechMode );

	::SelectObject( dcMemTarget.GetSafeHdc(), hOldTarget );
	CHelper::SAFE_DELETE( pbmpinfo );
	CHelper::SAFE_DELETE( pbmpinfoSource );
	CHelper::SAFE_DELETE_ARRAY( lpSourceBits );
	return CBitmap::FromHandle( hScaledBitmap );
} // ScaleImage

/////////////////////////////////////////////////////////////////////////////
// compare two images
IMAGE_COMPARE_RESULT CPlusGDI::CompareTwoImages
(	Bitmap* pImage1,
	Bitmap* pImage2,
	bool bAlwaysReturnDiff, 
	Bitmap* pImageDiff/*=NULL*/
)
{
	if ( pImage1 == NULL || pImage2 == NULL )
	{
		return UNKNOWN_ERROR;
	}

	SizeF szImageSize1;
	pImage1->GetPhysicalDimension( &szImageSize1 );
	SizeF szImageSize2;
	pImage2->GetPhysicalDimension( &szImageSize2 );

	if 
	(
		szImageSize1.Width != szImageSize2.Width || 
		szImageSize1.Height != szImageSize2.Height 
	)
	{
		return DIFFERENT_SIZE;
	}

	// Retrieve the data from first image
	BitmapData* pBitmapData1 = new BitmapData;
	Rect rect( 0, 0, (int)szImageSize1.Width, (int)szImageSize1.Height );
	pImage1->LockBits
	(
		&rect, ImageLockModeRead, PixelFormat32bppARGB, pBitmapData1 
	);

	//Retrieve the data from first image
	BitmapData* pBitmapData2 = new BitmapData;
	pImage2->LockBits
	(
		&rect, ImageLockModeRead, PixelFormat32bppARGB, pBitmapData2
	);

	int iBytesCount = pBitmapData1->Stride * pBitmapData1->Height;
	bool bCompare = false;
	if ( memcmp( pBitmapData1->Scan0, pBitmapData2->Scan0, iBytesCount ) == 0 )
	{
		if ( !bAlwaysReturnDiff )
		{
			return EQUAL;
		}
		bCompare = true;
	}

	if ( pImageDiff )
	{
		BitmapData* pBitmapDataDiff = new BitmapData;
		pImageDiff->LockBits
		(
			&rect, ImageLockModeWrite, PixelFormat32bppARGB, pBitmapDataDiff 
		);

		UINT* pBytesImageDiff = (UINT*)pBitmapDataDiff->Scan0;
		UINT* pBytesImage1 = (UINT*)pBitmapData1->Scan0;
		UINT* pBytesImage2 = (UINT*)pBitmapData2->Scan0;
		for ( int iRowIndex = 0; iRowIndex < szImageSize1.Height; ++iRowIndex )
		{
			int iBytesRead = iRowIndex * pBitmapData1->Stride / 4;
			for ( int iColIndex = 0; iColIndex < szImageSize1.Width; ++iColIndex )
			{
				const int iCurrentIndex = iBytesRead + iColIndex;
				pBytesImageDiff[ iCurrentIndex ] = 
					(	pBytesImage1[ iCurrentIndex ] ^ 
						pBytesImage2[ iCurrentIndex ] 
					) ^ 0xFFFFFFFF;
			}
		}

		pImageDiff->UnlockBits( pBitmapDataDiff );
		CHelper::SAFE_DELETE( pBitmapDataDiff );
	}

	// Free memory
	pImage1->UnlockBits( pBitmapData1 );
	pImage2->UnlockBits( pBitmapData2 );
	CHelper::SAFE_DELETE( pBitmapData1 );
	CHelper::SAFE_DELETE( pBitmapData2 );

	return bCompare ? EQUAL : DIFFERENT_CONTENT;
} // CompareTwoImages

/////////////////////////////////////////////////////////////////////////////
