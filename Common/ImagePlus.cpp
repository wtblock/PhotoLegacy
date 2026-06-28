/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ImagePlus.h"
#include "CHelper.h"

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
// Get the CLSID for the specified image format
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
// Get the image format id corresponding to the image extension
bool CImagePlus::GetFormatIDByExt(LPCWSTR pExt, UINT& nImageFormatID)
{
	bool value = false;

	CString strExt(pExt);

	//If there is a '.', remove it
	strExt.Remove('.');
	const int iCount =
		sizeof(ImageFormat_ID_Ext) /
		sizeof(IMAGEFORMAT_ID_EXT);

	for (int iIndex = 0; iIndex < iCount; ++iIndex)
	{
		if
		(
			strExt.CompareNoCase
			(
				CString(ImageFormat_ID_Ext[iIndex].m_szExt)
			) == 0
		)
		{
			nImageFormatID = ImageFormat_ID_Ext[iIndex].m_nFormatID;
			return true;
		}
	}

	return value;
} // GetFormatIDByExt

/////////////////////////////////////////////////////////////////////////////
// Save the data inside pImage as file name lpszPathName
bool CImagePlus::Save(LPCTSTR lpszPathName)
{
	USES_CONVERSION;

	bool value = false;

	CString csExt = CHelper::GetExtension(lpszPathName);

	// save and overwrite the selected image file with current page
	int iValue =
		EncoderValue::EncoderValueVersionGif89 |
		EncoderValue::EncoderValueCompressionLZW |
		EncoderValue::EncoderValueFlush;

	EncoderParameters param;
	param.Count = 1;
	param.Parameter[0].Guid = EncoderSaveFlag;
	param.Parameter[0].Value = &iValue;
	param.Parameter[0].Type = EncoderParameterValueTypeLong;
	param.Parameter[0].NumberOfValues = 1;

	UINT nImageFormatID;
	// get the image format id
	if (!GetFormatIDByExt(T2CW(csExt), nImageFormatID))
	{
		return value;
	}

	// Get the Encoder extension
	WCHAR EncoderExt[MAX_EXT_LEN];
	if (!GetEncoderExtByID(nImageFormatID, EncoderExt))
	{
		return value;
	}

	CLSID clsid;
	GetEncoderClsid(EncoderExt, &clsid);

	Status status = m_pImage->Save(T2CW(lpszPathName), &clsid, &param);

	value = status == Ok;

	return value;
} // Save

/////////////////////////////////////////////////////////////////////////////
// return the number of bits used for each pixel
long CImagePlus::GetBitsPerPixel()
{
	long value = -1;
	if (!m_pImage)
	{
		return value;
	}

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
shared_ptr<BITMAPINFO> CImagePlus::CreateDIBBitmapInfo
(
	CDC* pDC, Size& szSizeInPixel, COLORREF& rgbFG, COLORREF& rgbBG
)
{
	shared_ptr<BITMAPINFO> value;

	// if the original bitmap is monochrome, leave it that way
	// otherwise convert to true color
	bool bMonochrome = IsMonochrome;
	int nBitsPerPixel = bMonochrome ? 1 : 32;
	int nSizeBitmapInfo = SizeDIBBitmapInfo;

	// Allocate memory for info header
	BITMAPINFO* pBitmapInfo = (BITMAPINFO*)new byte[nSizeBitmapInfo];
	memset(pBitmapInfo, 0, nSizeBitmapInfo);
	value = shared_ptr<BITMAPINFO>(pBitmapInfo);

	const float fMillimetersPerInch = 25.4f;
	const float fPixelsPerInchX = (float)pDC->GetDeviceCaps(LOGPIXELSX);
	const float fPixelsPerInchY = (float)pDC->GetDeviceCaps(LOGPIXELSY);

	// Create true color header
	value->bmiHeader.biSize = sizeof( BITMAPINFOHEADER );
	value->bmiHeader.biWidth = szSizeInPixel.Width;
	value->bmiHeader.biHeight = szSizeInPixel.Height;
	value->bmiHeader.biPlanes = 1;
	value->bmiHeader.biBitCount = nBitsPerPixel;
	value->bmiHeader.biCompression = BI_RGB;
	value->bmiHeader.biSizeImage = 0;
	value->bmiHeader.biXPelsPerMeter =
		(int)( 1000.0f * fPixelsPerInchX / fMillimetersPerInch );
	value->bmiHeader.biYPelsPerMeter =
		(int)( 1000.0f * fPixelsPerInchY / fMillimetersPerInch);
	value->bmiHeader.biClrUsed = bMonochrome ? 2 : 0;
	value->bmiHeader.biClrImportant = bMonochrome ? 2 : 0;
	if ( bMonochrome )
	{
		//If it is monchrome, set the foreground and background color
		value->bmiColors[ 0 ].rgbBlue = GetBValue( rgbFG );
		value->bmiColors[ 0 ].rgbGreen = GetGValue( rgbFG );
		value->bmiColors[ 0 ].rgbRed = GetRValue( rgbFG );
		value->bmiColors[ 1 ].rgbBlue = GetBValue( rgbBG );
		value->bmiColors[ 1 ].rgbGreen = GetGValue( rgbBG );
		value->bmiColors[ 1 ].rgbRed = GetRValue( rgbBG );
	}

	return value;
} // CreateDIBBitmapInfo

/////////////////////////////////////////////////////////////////////////////
// Get bitmap handle for current image
bool CImagePlus::GetHBITMAP(HBITMAP& hBitmap)
{
	bool value = false;
	if ( m_pImage == NULL )
	{
		return false;
	}

	Bitmap image( Width, Height );
	const double dHRes = HorizontalResolution;
	const double dVRes = VerticalResolution;
	image.SetResolution((REAL)dHRes, (REAL)dVRes);

	Graphics gp( &image );

	// Fill the background color with white
	Status status = gp.Clear( Color( 255, 255, 255, 255 ) );
	
	// draw the image, 
	status = gp.DrawImage( m_pImage.get(), 0, 0 );
	Color backGround( 255, 255, 255, 255 );
	status = image.GetHBITMAP( backGround, &hBitmap );

	value = status == Ok;

	return value;
} // GetHBITMAP

/////////////////////////////////////////////////////////////////////////////
void CImagePlus::Draw( CDC* pDC, CRect& rectDest, CRect& rectSrc )
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
	const int nWidth = Width;
	const int nHeight = Height;
	const bool bIsMonochrome = IsMonochrome;
	const int nImagePaddingWidth = ImagePaddingWidth[nWidth];

	// Create the bitmap info
	Size size = Size(nWidth, nHeight);
	shared_ptr<BITMAPINFO> pSourceBitmapInfo = CreateDIBBitmapInfo
	( 
		pDC, size, rgbFG, rgbBG 
	);

	// Get the image byte size
	const int iSizeData = bIsMonochrome ?
		(int)ceil
		( 
			(double)nImagePaddingWidth * (double)nHeight / 8.0
		) :
		nWidth * nHeight * 4;

	shared_ptr<byte> lpSourceBits = shared_ptr<byte>( new byte[ iSizeData ] );

	// Retrieve the image data
	pBitmap->GetBitmapBits( iSizeData, lpSourceBits.get() );

	const int nNewStrechMode = bIsMonochrome ? BLACKONWHITE : COLORONCOLOR;

	// Set the stretchmode
	const int nOldStrechMode = 
		::SetStretchBltMode( pDC->m_hDC, nNewStrechMode );

	// Since the image is stored upside down, we need to reverse it back
	pSourceBitmapInfo->bmiHeader.biHeight = 
		-pSourceBitmapInfo->bmiHeader.biHeight;

	const int nDestLeft = destRect.GetLeft();
	const int nDestTop = destRect.GetTop();
	const int nDestWidth = destRect.Width;
	const int nDestHeight = destRect.Height;

	const int nSrcLeft =   rectSrc.left;
	const int nSrcTop =    rectSrc.top;
	const int nSrcWidth =  rectSrc.Width();
	const int nSrcHeight = rectSrc.Height();

	// Stretch and copy the image to the destination rectangle
	::StretchDIBits
	( 
		pDC->m_hDC, nDestLeft, nDestTop, nDestWidth, nDestHeight,
		nSrcLeft, nSrcTop, nSrcWidth, nSrcHeight,
		lpSourceBits.get(), pSourceBitmapInfo.get(), 
		DIB_RGB_COLORS, SRCCOPY
	);

	// restore the device contents to its starting value
	::SetStretchBltMode( pDC->m_hDC, nOldStrechMode );

	// GDI stores data separately from the pointer contents, so it needs
	// be cleaned up prior to deleting the pointer
	pBitmap->DeleteObject();

} // Draw

/////////////////////////////////////////////////////////////////////////////

