/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoExplorer.h"
#include "AboutVersions.h"
#include "afxdialogex.h"
#include "CHelper.h"
#include <atlimage.h>

using namespace Gdiplus;

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC( CAboutVersions, CDialogEx )

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP( CAboutVersions, CDialogEx )
	ON_WM_LBUTTONDOWN()
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
void CAboutVersions::OnLButtonDown( UINT nFlags, CPoint point )
{
	OnOK();

} // OnLButtonDown

  /////////////////////////////////////////////////////////////////////////////
CAboutVersions::CAboutVersions( CWnd* pParent /*=NULL*/ )
	: CDialogEx( CAboutVersions::IDD, pParent )
{

}

/////////////////////////////////////////////////////////////////////////////
CAboutVersions::~CAboutVersions()
{
	ReleaseCapture();

} // ~CAboutVersions

  /////////////////////////////////////////////////////////////////////////////
void CAboutVersions::DoDataExchange( CDataExchange* pDX )
{
	CDialogEx::DoDataExchange( pDX );
}

/////////////////////////////////////////////////////////////////////////////
// get the module name (executable without path or extension)
CString CAboutVersions::GetModuleName()
{
	TCHAR szPath[ _MAX_PATH + 1 ];
	::GetModuleFileName( NULL, szPath, _MAX_PATH );
	const CString value = CHelper::GetFileName( szPath );
	return value;
} // GetModuleName

/////////////////////////////////////////////////////////////////////////////
BOOL CAboutVersions::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();

	SetCapture();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
void CAboutVersions::DrawImage( CPaintDC& dc )
{
	Graphics graphics( dc );

	// load the background image from the resources
	CBitmap bm;
	BOOL bBM = bm.LoadBitmap( IDB_LOGO );
	if ( bBM == FALSE )
	{
		return;
	}

	HBITMAP hBM = bm.operator HBITMAP();
	Bitmap bitmap( hBM, NULL );

	if ( bitmap.GetLastStatus() == Ok )
	{
		// Get the dimensions of the dialog
		CRect rect;
		GetClientRect( &rect );

		// Calculate the aspect ratio of the image
		int imgWidth = bitmap.GetWidth();
		int imgHeight = bitmap.GetHeight();
		float imgAspect = 
			static_cast<float>( imgWidth ) / 
			static_cast<float>( imgHeight );

		// Calculate the aspect ratio of the dialog
		int dlgWidth = rect.Width();
		int dlgHeight = rect.Height();
		float dlgAspect = 
			static_cast<float>( dlgWidth ) / 
			static_cast<float>( dlgHeight );

		// Determine the dimensions to draw the image
		int drawWidth, drawHeight;
		if ( dlgAspect > imgAspect )
		{
			// Dialog is wider than the image, fit to height
			drawHeight = dlgHeight;
			drawWidth = static_cast<int>( dlgHeight * imgAspect );

		} else
		{
			// Dialog is taller than the image, fit to width
			drawWidth = dlgWidth;
			drawHeight = static_cast<int>( dlgWidth / imgAspect );
		}

		// Draw the image, centered
		graphics.DrawImage
		( 
			&bitmap, ( dlgWidth - drawWidth ) / 2,
			( dlgHeight - drawHeight ) / 2, drawWidth, 
			drawHeight 
		);

	} else
	{
		AfxMessageBox( _T( "Failed to load image." ) );
	}
} // DrawImage

///////////////////////////////////////////////////////////////////////////
void CAboutVersions::DrawLabels
( 
	Graphics& graphics, PointF pointText, 
	Gdiplus::Font& largeFont, Gdiplus::Font& smallFont,
	int nLargeHeight, int nSmallHeight,
	SolidBrush& brush, StringFormat& format
)
{
	// Draw large text
	graphics.DrawString
	(
		m_version.ProductName, -1,
		&largeFont, pointText, &format, &brush
	);

	pointText.Y += nLargeHeight;

	// Draw small text
	CString csVersion;
	csVersion.Format( L"Version: %s", m_version.FileVersion );
	graphics.DrawString
	(
		csVersion, -1,
		&smallFont, pointText, &format, &brush
	);

	pointText.Y += nSmallHeight;

	graphics.DrawString
	(
		m_version.LegalCopyright, -1,
		&smallFont, pointText, &format, &brush
	);

	pointText.Y += nSmallHeight;

	graphics.DrawString
	(
		m_version.CompanyName, -1,
		&smallFont, pointText, &format, &brush
	);

	pointText.Y += nSmallHeight;

	int nStart = 0;
	CString csDesc = m_version.FileDescription;
	CString csFirst = csDesc.Tokenize( L",", nStart );
	CString csSecond = csDesc.Tokenize( L",", nStart );

	graphics.DrawString
	(
		csFirst + L",", -1,
		&smallFont, pointText, &format, &brush
	);

	pointText.Y += nSmallHeight;

	graphics.DrawString
	(
		csSecond, -1,
		&smallFont, pointText, &format, &brush
	);
} // DrawLabels

///////////////////////////////////////////////////////////////////////////
void CAboutVersions::DrawText( CPaintDC& dc )
{
	Graphics graphics( dc );

	// Get the dimensions of the dialog
	CRect rect;
	GetClientRect( &rect );

	// Calculate the aspect ratio of the dialog
	int dlgWidth = rect.Width();
	int dlgHeight = rect.Height();
	int nLargeHeight = 72;
	int nSmallHeight = 30;

	// Define text positions
	PointF pointText
	( 
		static_cast<float>( dlgWidth ) / 2, 
		static_cast<float>( dlgHeight ) / 2
	);

	// Set text alignment to center
	StringFormat format;
	format.SetAlignment( StringAlignmentCenter );
	format.SetLineAlignment( StringAlignmentCenter );

	// Draw text with different font sizes
	Gdiplus::FontFamily fontFamily( L"Comic Sans MS" );
	Gdiplus::Font largeFont
	( 
		&fontFamily, REAL( nLargeHeight ),
		Gdiplus::FontStyleBoldItalic, Gdiplus::UnitPixel 
	);
	Gdiplus::Font smallFont
	( 
		&fontFamily, REAL( nSmallHeight ),
		Gdiplus::FontStyleBold, Gdiplus::UnitPixel 
	);
	SolidBrush brushTop( Color::OrangeRed );
	SolidBrush brushBottom( Color::White );

	DrawLabels
	( 
		graphics, pointText, largeFont, smallFont, 
		nLargeHeight, nSmallHeight, brushBottom, format 
	);

	pointText.X += 3;
	pointText.Y += 3;

	DrawLabels
	( 
		graphics, pointText, largeFont, smallFont, 
		nLargeHeight, nSmallHeight, brushTop, format 
	);

} // DrawText

///////////////////////////////////////////////////////////////////////////
void CAboutVersions::OnPaint()
{
	CPaintDC dc( this ); // Device context for painting
	CDialogEx::OnPaint();

	DrawImage( dc );
	DrawText( dc );

} // OnPaint

  /////////////////////////////////////////////////////////////////////////////
