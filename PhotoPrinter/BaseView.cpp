/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include "BaseDoc.h"
#include "BaseView.h"
#include "CHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define _DOUBLE_BUFFER

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CBaseView, CScrollView)

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CBaseView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CBaseView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CBaseView::CBaseView()
{
	TopOfView = 0;
	LeftOfView = 0;
	Horizontal = false;
	FitToWindow = true;
	Printing = false;
	Exporting = false;
	PDF = false;
	Scale = 1;
}

/////////////////////////////////////////////////////////////////////////////
CBaseView::~CBaseView()
{
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBaseView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// render the page or view
void CBaseView::render
(
	CDC* pDC,
	double dLeftOfView,
	double dTopOfView,
	double dRightOfView,
	double dBottomOfView
)
{
	// logical coordinates allow the drawing to be device independent
	// i.e. rendering works on the screen as well as printing and print
	// preview
	const int nTopOffset = InchesToLogical( dTopOfView );

	const int nLeftOffset = InchesToLogical( dLeftOfView );

	// account for the shift of the view due to scrolling or printed pages
	pDC->SetWindowOrg( nLeftOffset, nTopOffset );

} // render

/////////////////////////////////////////////////////////////////////////////
void CBaseView::OnDraw( CDC* pDC )
{
	CBaseDoc* pDoc = Document;
	ASSERT_VALID( pDoc );
	if ( !pDoc )
		return;

	CDC* pTargetDC = pDC;

#ifdef _DOUBLE_BUFFER
	// double buffer output by creating a memory bitmap and drawing
	// directly to it and then copy the bitmap to the screen to reduce 
	// screen flicker
	CRect rectClient;
	GetClientRect( &rectClient );
	const int nRectWidth = rectClient.Width();
	const int nRectHeight = rectClient.Height();

	CBitmap bm;
	bm.CreateCompatibleBitmap( pDC, nRectWidth, nRectHeight );

	CDC dcMem;
	dcMem.CreateCompatibleDC( pDC );
	CBitmap* pBmOld = dcMem.SelectObject( &bm );
	dcMem.PatBlt( 0, 0, nRectWidth, nRectHeight, WHITENESS );
	pTargetDC = &dcMem;
	int nDcOrg = pTargetDC->SaveDC();
#endif

	const int nLogicalWidth = SetDrawDC( pTargetDC );
	const double dTopOfView = TopOfView;
	const double dLeftOfView = LeftOfView;
	const double dBottomOfView = BottomOfView;
	const double dRightOfView = RightOfView;

	render
	( 
		pTargetDC, dLeftOfView, dTopOfView, dRightOfView, dBottomOfView
	);

#ifdef _DOUBLE_BUFFER
	pTargetDC->RestoreDC( nDcOrg );

	// output the drawing to the screen in a single bitblit
	pDC->BitBlt
	(
		0, 0, nRectWidth, nRectHeight, &dcMem, 0, 0, SRCCOPY
	);

	dcMem.SelectObject( pBmOld );
#endif

} // OnDraw

/////////////////////////////////////////////////////////////////////////////
void CBaseView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	OnVScroll( SB_TOP, 0, nullptr );

	Invalidate();

	m_bInitialUpdate = true;
}

/////////////////////////////////////////////////////////////////////////////
void CBaseView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CBaseView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// turning off multiple copies and collation 
	pInfo->m_pPD->m_pd.Flags &= ~PD_USEDEVMODECOPIES;

	return DoPreparePrinting(pInfo);
}

/////////////////////////////////////////////////////////////////////////////
void CBaseView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// setup the device context for our printer
	SetPrintDC
	(
		pDC,
		m_nPhysicalPageWidth,
		m_nPhysicalPageHeight,
		m_nLogicalPageWidth,
		m_nLogicalPageHeight
	);

	Printing = true;

	// height of the document in inches
	const double dDocumentHeight = DocumentHeight;

	// height of a page in inches
	const double dPageHeight = LogicalToInches( m_nLogicalPageHeight );

	// number of printer pages
	double dPages = dDocumentHeight / dPageHeight;

	// add a page if there is a fraction of a page
	m_nNumPages = (int)dPages;
	if ( !CHelper::NearlyEqual( double( m_nNumPages ), dPages, 0.05 ) )
	{
		m_nNumPages++; // account for fractional page
	}

	// let the print dialog know
	pInfo->SetMinPage( 1 );
	pInfo->SetMaxPage( m_nNumPages );
}

/////////////////////////////////////////////////////////////////////////////
void CBaseView::OnPrint( CDC* pDC, CPrintInfo* pInfo )
{
	const double dPageHeight = LogicalToInches( m_nLogicalPageHeight );
	const double dPageWidth = LogicalToInches(m_nLogicalPageWidth);
	const int nPage = (int)pInfo->m_nCurPage;
	const double dTopOfPage = ( (float)( nPage - 1 ) ) * dPageHeight;
	const double dBottomOfPage = dTopOfPage + dPageHeight;

	// the same render method used to draw on the screen
	render
	( 
		pDC, 0.0, dTopOfPage, dPageWidth, dBottomOfPage
	);

} // OnPrint

/////////////////////////////////////////////////////////////////////////////
void CBaseView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	Printing = false;
}

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void CBaseView::AssertValid() const
{
	CScrollView::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////
void CBaseView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

/////////////////////////////////////////////////////////////////////////////
CBaseDoc* CBaseView::GetDocument() const // non-debug version is inline
{
	if (this == nullptr)
	{
		return nullptr;
	}
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBaseDoc)));
	return (CBaseDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// generate font characteristics from given properties
void CBaseView::BuildFont
(
	CString csFace, // name of the font face
	bool bBold, // bold font if true
	bool bItalic, // italic font if true
	int nTextHeight, // text height in pixels
	bool bVertical, // vertical orientation
	CFont& font, // generated font
	double dAngle /*= 0*/, // angle in degrees
	BYTE nCharSet/* = ANSI_CHARSET*/, // current character set
	bool bFlipX/* = false*/, // flip horizontally
	bool bFlipY/* = false*/, // flip vertically
	short nUp/* = -1*/, // moving up is a negative value
	int nTextWidth/* = 0*/ // default width
)
{
	LOGFONT lf;
	// Populate logical font with defaults
	::GetObject( GetStockObject( SYSTEM_FONT ), sizeof( LOGFONT ), &lf );
	int nAngle = int( dAngle * nUp * 10 );
	nAngle += bVertical ? nUp * 900 : 0;

	// rotate 180 degrees (happens when printing up-side-down)
	if ( bFlipX && bFlipY )
	{
		nAngle += ( nUp * 1800 );
		nAngle = nAngle % 3600;
	}

	// customize our font
	if ( nTextHeight != 0 )
	{
		lf.lfHeight = nTextHeight;
		if ( nTextWidth != 0 )
		{
			lf.lfWidth = nTextWidth;
		}
		else
		{
			lf.lfWidth = lf.lfHeight * 2 / 5;
		}
	}

	lf.lfEscapement = nAngle;
	lf.lfOrientation = nAngle;

	lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;
	lf.lfItalic = bItalic == true;
	lf.lfCharSet = nCharSet;
	_tcscpy( lf.lfFaceName, csFace );
	font.CreateFontIndirect( &lf );
} // BuildFont

/////////////////////////////////////////////////////////////////////////////
void CBaseView::OnPrepareDC( CDC * pDC, CPrintInfo * pInfo )
{
	if ( pInfo != nullptr ) // printing
	{
		SetPrintDC
		(
			pDC,
			m_nPhysicalPageWidth, m_nPhysicalPageHeight,
			m_nLogicalPageWidth, m_nLogicalPageHeight
		);
	}

} // OnPrepareDC

/////////////////////////////////////////////////////////////////////////////
// prepare the device context for drawing and
// return the logical width
int CBaseView::SetDrawDC
(
	CDC* pDC
)
{
	// client dimensions are affected by the horizontal layout property
	const int nClientWidth = PhysicalViewWidth;
	const int nClientHeight = PhysicalViewHeight;
	int nLogicalWidth = LogicalPageWidth;
	const int nLogicalHeight = LogicalPageHeight;
	const int nLogicalPageHeight = 11000;
	const double dScale = Scale;

	if ( nLogicalWidth != 0 )
	{
		//const double dAspect = double( nClientWidth ) / nClientHeight;
		//const double dDocWidth = DocumentWidth;

		//// account for zooming in and out
		//nLogicalWidth = InchesToLogical( dDocWidth / dScale );
		//const int nLogicalHeight = int( double( nLogicalWidth ) / dAspect );
		//CSize size( nLogicalWidth, nLogicalHeight );
		//LogicalSizeClient = size;

		// isotropic means the values are the same in the X and Y directions
		pDC->SetMapMode( MM_ISOTROPIC );

		// using the width for X and Y so that the image will always fit
		// horizontally in the window and the vertical dimension will 
		// adjust proportionally
		//pDC->SetWindowExt(nLogicalWidth, nLogicalWidth);
		//pDC->SetViewportExt(nClientWidth, nClientWidth);
		pDC->SetWindowExt(nLogicalWidth, nLogicalHeight);
		pDC->SetViewportExt(nClientWidth, nClientHeight);

		// remember dimensions
		CSize sizeLogical = CSize( nLogicalWidth, nLogicalHeight);
		LogicalSizeClient = sizeLogical;
	}

	return nLogicalWidth;
} // SetDrawDC

/////////////////////////////////////////////////////////////////////////////
// prepare the device context for printing
void CBaseView::SetPrintDC
(
	CDC* pDC,
	int& nPhysicalWidth, // in pixels
	int& nPhysicalHeight, // in pixels
	int& nLogicalWidth, // in inches * Map
	int& nLogicalHeight // in inches * Map
)
{
	nPhysicalWidth = pDC->GetDeviceCaps( HORZRES );
	nPhysicalHeight = pDC->GetDeviceCaps( VERTRES );
	const int nPixelsPerInchX = pDC->GetDeviceCaps( LOGPIXELSX );
	const int nPixelsPerInchY = pDC->GetDeviceCaps( LOGPIXELSY );

	CBaseDoc* pDoc = Document;
	const int nMap = pDoc->Map;
	const double dWidth = 
		(double)nPhysicalWidth * nMap / (double)nPixelsPerInchX;
	nLogicalWidth = (int)CHelper::RoundToNearest( dWidth );
	double dHeight = 
		(double)nPhysicalHeight * nMap / (double)nPixelsPerInchY;
	nLogicalHeight = (int)CHelper::RoundToNearest(dHeight);

	// create custom MM_HIENGLISH mapping mode:
	// 	1. maintain constant aspect ratio
	//	2. accept coordinates in 1/Map inches
	//	3. keep entire page width visible at all times
	//	4. vertical dimensions increase from top to bottom
	pDC->SetMapMode( MM_ISOTROPIC );
	//	base horizontal AND VERTICAL extents on page WIDTH
	//                  ============                 =====
	pDC->SetWindowExt( nLogicalWidth, nLogicalHeight );
	//	client WIDTH
	//		   =====
	pDC->SetViewportExt( nPhysicalWidth, nPhysicalHeight );
} // SetPrintDC

/////////////////////////////////////////////////////////////////////////////
// prepare the device context for printing
void CBaseView::SetImageDC
(
	CDC* pDC,
	int nDPI,
	int nPhysicalWidth, // in pixels
	int nPhysicalHeight, // in pixels
	int& nLogicalWidth, // in inches * Map
	int& nLogicalHeight // in inches * Map
)
{
	const int nPixelsPerInchX = nDPI;
	const int nPixelsPerInchY = nDPI;

	CBaseDoc* pDoc = Document;
	const int nMap = pDoc->Map;
	const double dWidth = 
		(double)nPhysicalWidth * nMap / (double)nPixelsPerInchX;
	nLogicalWidth = (int)CHelper::RoundToNearest( dWidth );
	double dHeight = 
		(double)nPhysicalHeight * nMap / (double)nPixelsPerInchY;
	nLogicalHeight = (int)CHelper::RoundToNearest(dHeight);

	// create custom MM_HIENGLISH mapping mode:
	// 	1. maintain constant aspect ratio
	//	2. accept coordinates in 1/Map inches
	//	3. keep entire page width visible at all times
	//	4. vertical dimensions increase from top to bottom
	pDC->SetMapMode( MM_ISOTROPIC );
	//	base horizontal AND VERTICAL extents on page WIDTH
	//                  ============                 =====
	pDC->SetWindowExt( nLogicalWidth, nLogicalHeight );
	//	client WIDTH
	//		   =====
	pDC->SetViewportExt( nPhysicalWidth, nPhysicalHeight );
} // SetImageDC

/////////////////////////////////////////////////////////////////////////////
// record the client rectangle every time the windows is resized
void CBaseView::OnSize( UINT nType, int cx, int cy )
{
	CView::OnSize( nType, cx, cy );

	m_sizeClient.cx = cx;
	m_sizeClient.cy = cy;

	SetupScrollBars();
} // OnSize

/////////////////////////////////////////////////////////////////////////////
BOOL CBaseView::OnEraseBkgnd( CDC* pDC )
{
#ifdef _DOUBLE_BUFFER
	// double buffer output by creating a memory bitmap and drawing
	// directly to it and then copy the bitmap to the screen to reduce flicker
	// need to prevent the default erase background behavior
	return TRUE;
#else
	return CView::OnEraseBkgnd( pDC );
#endif
} // OnEraseBkgnd

/////////////////////////////////////////////////////////////////////////////
// adjust width scroll bar
void CBaseView::AdjustWidthScrollBar()
{
	// horizontal or vertical layout (currently always vertical)
	const bool bHorizontal = Horizontal;

	// The scroll bar depends on the orientation
	const int nBar = bHorizontal ? SB_VERT : SB_HORZ;

	SCROLLINFO si;
	ZeroMemory( &si, sizeof( SCROLLINFO ) );
	si.cbSize = sizeof( SCROLLINFO );
	si.fMask = SIF_ALL;
	GetScrollInfo( nBar, &si );

	const double dLeft = LeftOfView;
	const double dLast = LastScrollWidth;

	double dOffset = dLeft;
	if ( dOffset > dLast )
	{
		dOffset = dLast;
		LeftOfView = dOffset;
	}
	else if ( dOffset < 0 )
	{
		dOffset = 0;
		LeftOfView = dOffset;
	}

	// left column on the visible page
	int nCol = LeftColumn;

	// number of columns in the entire document
	int nCols = ScrollColumns;

	// number of columns visible on the screen
	int nView = PageColumns;

	// maximum range in columns
	const int nMaxCols = nCols - nView;

	if ( nView >= nCols )
	{
		nCols = 0;
		nView = 0;
	}

	nCol = min( nCol, nCols );

	si.nMin = 0;
	si.nMax = nCols;
	si.nPage = nView;
	si.nPos = nCol;

	SetScrollInfo( nBar, &si );

} // AdjustTrackScrollBar 

/////////////////////////////////////////////////////////////////////////////
// adjust depth scroll bar
void CBaseView::AdjustHeightScrollBar()
{
	// horizontal or vertical layout (currently always vertical)
	const bool bHorizontal = Horizontal;

	// The scroll bar depends on the orientation
	const int nBar = bHorizontal ? SB_HORZ : SB_VERT;

	SCROLLINFO si;
	ZeroMemory( &si, sizeof( SCROLLINFO ) );
	si.cbSize = sizeof( SCROLLINFO );
	si.fMask = SIF_ALL;
	GetScrollInfo( nBar, &si );

	const double dTop = TopOfView;
	const double dLast = LastScrollHeight;
	const double dBottom = DocumentHeight;
	const double dViewHeight = ViewHeight;

	double dOffset = dTop;
	if ( dOffset > dLast )
	{
		dOffset = dLast;
		TopOfView = dOffset;
	}
	else if ( dOffset < 0 )
	{
		dOffset = 0;
		TopOfView = dOffset;
	}

	// top line on the visible page
	int nLine = TopLine;

	// number of lines in the entire document
	int nLines = ScrollLines;

	// number of lines visible on the screen
	int nView = PageLines;

	// maximum range in lines
	const int nMaxLines = nLines - nView;

	if ( nView > nLines )
	{
		nLines = 0;
		nView = 0;
	}

	nLine = min( nLine, nLines );

	si.nMin = 0;
	si.nMax = nLines;
	si.nPage = nView;
	si.nPos = nLine;

	SetScrollInfo( nBar, &si );

} // AdjustHeightScrollBar 

 /////////////////////////////////////////////////////////////////////////////
// set the scroll bar position and range
void CBaseView::SetupScrollBars()
{
	AdjustHeightScrollBar();
	AdjustWidthScrollBar();

} // SetupScrollBars

/////////////////////////////////////////////////////////////////////////////
// handler for the windows WM_VSCROLL message
void CBaseView::OnVScroll
( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar
)
{
	if ( Horizontal )
	{
		WidthScroll( nSBCode, nPos, pScrollBar );

	}
	else
	{
		HeightScroll( nSBCode, nPos, pScrollBar );
	}

} // OnVScroll

/////////////////////////////////////////////////////////////////////////////
void CBaseView::HeightScroll
(
	UINT nSBCode, UINT nPos, CScrollBar* pScrollBar
)
{
	// horizontal or vertical layout (currently always vertical)
	const bool bHorizontal = Horizontal;

	// scroll bar
	const int nBar = bHorizontal ? SB_HORZ : SB_VERT;

	SCROLLINFO si;
	ZeroMemory( &si, sizeof( SCROLLINFO ) );
	si.cbSize = sizeof( SCROLLINFO );
	si.fMask = SIF_ALL;
	GetScrollInfo( nBar, &si );

	double dTop = TopOfView;
	double dLast = LastScrollHeight;
	const double dLine = LineHeight;
	const double dPage = PagingHeight;

	switch ( nSBCode )
	{
		case SB_PAGEDOWN: // Scroll one page down.
			if ( CHelper::GreaterThanOrEqual( dTop, dLast ) )
			{
				dTop = dLast;

			}
			else
			{
				dTop += dPage;
			}
			break;
		case SB_LINEDOWN: // Scroll one line down.
			if ( CHelper::GreaterThanOrEqual( dTop, dLast ) )
			{
				dTop = dLast;

			}
			else
			{
				dTop += dLine;
			}
			break;
		case SB_PAGEUP: // Scroll one page up.
			if ( CHelper::LessThanOrEqual( dTop, 0.0 ) )
			{
				dTop = 0.0;

			}
			else
			{
				dTop -= dPage;
			}
			break;
		case SB_LINEUP: // Scroll one line up.
			if ( CHelper::LessThanOrEqual( dTop, 0.0 ) )
			{
				dTop = 0.0;

			}
			else
			{
				dTop -= dLine;
			}
			break;
			// Drag scroll box to specified position. 
			// The current position is provided in nPos.
		case SB_THUMBTRACK:
			nPos = si.nTrackPos;
			dTop = nPos * LineHeight;
			break;
		case SB_THUMBPOSITION: // Scroll to the absolute position. 
								// The current position is provided in nPos.

			dTop = nPos * GetLineHeight();
			break;
		case SB_TOP: // Scroll to top.
			dTop = 0;
			break;
		case SB_BOTTOM: // Scroll to bottom.
			dTop = dLast;
			break;
		case SB_ENDSCROLL: // End scroll.
			return;
	} // switch 

	if ( dTop < 0 )
	{
		dTop = 0;

	}
	else if ( dTop > dLast )
	{
		dTop = dLast;
	}

	TopOfView = dTop;

	SetupScrollBars();
	Invalidate();

} // HeightScroll

/////////////////////////////////////////////////////////////////////////////
// handler for the windows WM_HSCROLL message
void CBaseView::OnHScroll
(
	UINT nSBCode, UINT nPos, CScrollBar* pScrollBar
)
{
	if ( Horizontal )
	{
		HeightScroll( nSBCode, nPos, pScrollBar );

	}
	else
	{
		WidthScroll( nSBCode, nPos, pScrollBar );
	}

} // OnHScroll

/////////////////////////////////////////////////////////////////////////////
// width scroll is horizontal scrolling when using horizontal layout and is
// vertical scrolling when using vertical layout
void CBaseView::WidthScroll
(
	UINT nSBCode, UINT nPos, CScrollBar* pScrollBar
)
{
	// horizontal or vertical layout 
	const bool bHorizontal = Horizontal;

	// scroll bar
	const int nBar = bHorizontal ? SB_VERT : SB_HORZ;

	SCROLLINFO si;
	ZeroMemory( &si, sizeof( SCROLLINFO ) );
	si.cbSize = sizeof( SCROLLINFO );
	si.fMask = SIF_ALL;
	GetScrollInfo( nBar, &si );

	double dLeft = LeftOfView;
	double dLast = LastScrollWidth;
	const double dPage = PagingWidth;
	const double dCol = ColumnWidth;

	// verticals are inverted when horizontal
	if ( Horizontal )
	{
		switch ( nSBCode )
		{
			case SB_PAGERIGHT: // Scroll one page right. 
				nSBCode = SB_PAGELEFT;
				break;
			case SB_PAGELEFT: // Scroll one page left.
				nSBCode = SB_PAGERIGHT;
				break;
			case SB_LINERIGHT: // Scroll one column right.
				nSBCode = SB_LINELEFT;
				break;
			case SB_LINELEFT: // Scroll one column left.
				nSBCode = SB_LINERIGHT;
				break;
			case SB_TOP: // Scroll to top
				nSBCode = SB_BOTTOM;
				break;
			case SB_BOTTOM: // Scroll to bottom
				nSBCode = SB_TOP;
				break;
		}
	}

	switch ( nSBCode )
	{
		case SB_PAGERIGHT: // Scroll one page right.
			if ( CHelper::NearlyEqual( dLeft, dLast ) ) return;
			dLeft += dPage;
			break;
		case SB_LINERIGHT: // Scroll one column right.
			if ( CHelper::NearlyEqual( dLeft, dLast ) ) return;
			dLeft += dCol;
			break;
		case SB_PAGELEFT: // Scroll one page left.
			if ( CHelper::NearlyEqual( dLeft, 0.0 ) ) return;
			dLeft -= dPage;
			break;
		case SB_LINELEFT: // Scroll one column left.
			if ( CHelper::NearlyEqual( dLeft, 0.0 ) ) return;
			dLeft -= dCol;
			break;
		case SB_THUMBTRACK: 	// Drag scroll box to specified position. 
								// The current position is provided in nPos.
			nPos = si.nTrackPos;
			dLeft = nPos * dCol;
			if ( bHorizontal )
			{
				dLeft = dLast - dLeft;
			}
			break;
		case SB_THUMBPOSITION: // Scroll to the absolute position. 
								// The current position is provided in nPos.

			dLeft = nPos * ColumnWidth;
			if ( Horizontal )
			{
				dLeft = dLast - dLeft;
			}
			break;
		case SB_TOP: // Scroll to top.
			dLeft = 0;
			break;
		case SB_BOTTOM: // Scroll to bottom.
			dLeft = dLast;
			break;
		case SB_ENDSCROLL: // End scroll.
			return;
	} // switch 

	if ( dLeft < 0 )
	{
		dLeft = 0;

	}
	else if ( dLeft > dLast )
	{
		dLeft = dLast;
	}

	LeftOfView = dLeft;

	SetupScrollBars();
	Invalidate();

} // WidthScroll

/////////////////////////////////////////////////////////////////////////////
// handler for the WM_KEYDOWN windows message
void CBaseView::OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	switch ( nChar )
	{
		case VK_PRIOR:
			OnVScroll( SB_PAGEUP, 0, 0 );
			return;
		case VK_NEXT:
			OnVScroll( SB_PAGEDOWN, 0, 0 );
			return;
		case VK_HOME:
			OnVScroll( SB_TOP, 0, 0 );
			return;
		case VK_END:
			OnVScroll( SB_BOTTOM, 0, 0 );
			return;
		case VK_UP:
			OnVScroll( SB_LINEUP, 0, 0 );
			return;
		case VK_DOWN:
			OnVScroll( SB_LINEDOWN, 0, 0 );
			return;
		case VK_LEFT:
			OnHScroll( SB_LINELEFT, 0, NULL );
			return;
		case VK_RIGHT:
			OnHScroll( SB_LINERIGHT, 0, NULL );
			return;
		case VK_DIVIDE:
		case VK_MULTIPLY:
		{
			double dScale = Scale;
			if ( nChar == VK_DIVIDE )
			{
				dScale -= 0.1f;
			}
			else
			{
				dScale += 0.1f;
			}

			if ( dScale < 1 )
			{
				dScale = 1;
			}

			Scale = dScale;
			if ( !CHelper::NearlyEqual( dScale, 1.0 ) )
			{
				FitToWindow = true;
			}
			else
			{
				Scale = 1;
			}

			CClientDC dc( this );
			SetDrawDC( &dc );
			AdjustWidthScrollBar();
			AdjustHeightScrollBar();
			Invalidate();
			return;
		}
	}

	CScrollView::OnKeyDown( nChar, nRepCnt, nFlags );
} // OnKeyDown

/////////////////////////////////////////////////////////////////////////////
// handler for the WM_MOUSEWHEEL windows message
BOOL CBaseView::OnMouseWheel
(
	UINT nFlags, short zDelta, CPoint point
)
{
	ScreenToClient( &point );

	short nKeyState = ::GetKeyState( VK_CONTROL );
	const bool bControlDown = nKeyState < 0;
	nKeyState = ::GetKeyState( VK_SHIFT );
	const bool bShiftDown = nKeyState < 0;

	// control mouse scrolling is standard method of zooming in and out
	if ( bControlDown )
	{
		CClientDC dc( this );
		SetDrawDC( &dc );
		const int nLogicalWidth = LogicalClientWidth;
		CPoint pt( point ); // logical point
		dc.DPtoLP( &pt );
		if ( zDelta > 0 )
		{
			OnKeyDown( VK_MULTIPLY, 1, 0 );

		}
		else
		{
			OnKeyDown( VK_DIVIDE, 1, 0 );
		}

		// translate the presentation to attempt to keep the focus
		// of the mouse arrow on the same spot
		TranslatePresentation( &dc, point, pt, nLogicalWidth );
		SetupScrollBars();
		Invalidate();
	}
	else if ( bShiftDown )
	{
		if ( zDelta > 0 )
		{
			OnHScroll( SB_LINELEFT, 0, NULL );

		}
		else
		{
			OnHScroll( SB_LINERIGHT, 0, NULL );
		}

	}
	else // no control key is used for vertical scrolling
	{
		if ( zDelta > 0 )
		{
			OnVScroll( SB_LINEUP, 0, NULL );

		}
		else
		{
			OnVScroll( SB_LINEDOWN, 0, NULL );
		}
	}

	return TRUE;
} // OnMouseWheel

/////////////////////////////////////////////////////////////////////////////
// The given points represent the physical and logical co-ordinates
// of the mouse pointer before a zoom operation. This routine will 
// translate the presentation to keep the displayed data centered 
// on the given co-ordinates after the zoom operation completes
void CBaseView::TranslatePresentation
(
	CDC* pDC, CPoint ptPhysical, CPoint ptLogical, int nLogicalWidth
)
{
	// horizontal layout of the document
	const bool bHorizontal = Horizontal;

	// logical width following the zoom operation
	const int nNewLogicalWidth = SetDrawDC( pDC );

	// logical point following the zoom operation
	CPoint pt2( ptPhysical );
	pDC->DPtoLP( &pt2 );

	// because of the rotation, y co-ordinates are inverse x co-ordinates
	// so for horizontal presentation, the y co-ordinates need to be 
	// inverted
	if ( bHorizontal )
	{
		ptLogical.y = nLogicalWidth - ptLogical.y;
		pt2.y = nNewLogicalWidth - pt2.y;
	}

	// logical difference
	CPoint ptDiff = ptLogical - pt2;

	// horizontal scroll position in inches
	const double dScroll = LeftOfView;

	// x offset in inches
	double dxOffset = LogicalToInches( ptDiff.x );

	// y offset in inches
	double dyOffset = LogicalToInches( ptDiff.y );

	// flip the x and y co-ordinates for horizontal display
	if ( bHorizontal )
	{
		swap( dxOffset, dyOffset );
	}

	// new horizontal scroll position in inches
	double dNew = max( 0, dScroll + dxOffset );
	LeftOfView = dNew;

	// new top depth
	const double dTop = TopOfView;

	// new vertical scroll position in inches
	dNew = max( 0.0, dTop + dyOffset );
	TopOfView = dNew;

} // TranslatePresentation

/////////////////////////////////////////////////////////////////////////////
