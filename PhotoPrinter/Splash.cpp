/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "CHelper.h"
#include "PhotoPrinter.h"
#include "Splash.h"
#include "afxdialogex.h"
#include <atlimage.h>

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CSplash, CDialogEx)

/////////////////////////////////////////////////////////////////////////////
CSplash::CSplash(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSplash::IDD, pParent)
{
}

/////////////////////////////////////////////////////////////////////////////
CSplash::~CSplash()
{
	ReleaseCapture();
}

/////////////////////////////////////////////////////////////////////////////
void CSplash::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
} // DoDataExchange

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CSplash, CDialogEx)
	ON_WM_PAINT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// get the module name (executable without path or extension)
CString CSplash::GetModuleName()
{
	TCHAR szPath[ _MAX_PATH + 1 ];
	::GetModuleFileName( NULL, szPath, _MAX_PATH );
	const CString value = CHelper::GetFileName( szPath );
	return value;
} // GetModuleName

/////////////////////////////////////////////////////////////////////////////
void CSplash::CreateDlg()
{
	CWnd* pWnd = AfxGetMainWnd();
	if ( pWnd == 0 )
	{
		pWnd = m_pParentWnd;
		if ( pWnd == 0 )
		{
			return;
		}
	}

	CRect rMain;
	pWnd->GetWindowRect( &rMain );

	CWnd* effectiveParent = m_pParentWnd;
	if ( !effectiveParent )
	{
		effectiveParent = pWnd;
	}
	Create( IDD, effectiveParent );

	CRect rDlg;
	GetWindowRect( &rDlg );

	const int nXDiff = 
		(( rMain.left + rMain.right ) - ( rDlg.left + rDlg.right )) / 2;
	const int nYDiff = 
		(( rMain.top + rMain.bottom ) - ( rDlg.top + rDlg.bottom )) / 2;

	rDlg.left += nXDiff - rMain.left;
	rDlg.right += nXDiff - rMain.left;
	rDlg.top += nYDiff - rMain.top;
	rDlg.bottom += nYDiff - rMain.top;

	//// make room from progress dialog to run below
	//rDlg.OffsetRect( 0, int( -rDlg.Height() * 0.7 ));

	MoveWindow( &rDlg );
	
	BringWindowToTop();

	ShowWindow( SW_SHOW );
} // CreateDlg

/////////////////////////////////////////////////////////////////////////////
BOOL CSplash::OnInitDialog()
{
	CDialog::OnInitDialog();
	CenterWindow();

	SetCapture();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
} // OnInitDialog

/////////////////////////////////////////////////////////////////////////////
void CSplash::DrawImage( CPaintDC& dc )
{
	Graphics graphics( dc );

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
void CSplash::DrawLabels
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
	csVersion.Format( L"Version: %s", (LPCTSTR)m_version.FileVersion );
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
void CSplash::DrawText( CPaintDC& dc )
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
		static_cast<float>(dlgWidth - 50),
		static_cast<float>(50)
	);

	// Set text alignment to center
	StringFormat format;
	format.SetAlignment(StringAlignmentFar);
	format.SetLineAlignment(StringAlignmentNear);

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

	// draw the shadow in white
	DrawLabels
	(
		graphics, pointText, largeFont, smallFont,
		nLargeHeight, nSmallHeight, brushBottom, format
	);

	pointText.X += 3;
	pointText.Y += 3;

	// draw the text in Orange Red
	DrawLabels
	(
		graphics, pointText, largeFont, smallFont,
		nLargeHeight, nSmallHeight, brushTop, format
	);
} // DrawText

///////////////////////////////////////////////////////////////////////////
void CSplash::OnPaint()
{
	CPaintDC dc( this ); // Device context for painting
	CDialogEx::OnPaint();

	DrawImage( dc );
	DrawText( dc );

} // OnPaint

/////////////////////////////////////////////////////////////////////////////

