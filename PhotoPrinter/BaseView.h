/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "CHelper.h"
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CBaseView
//
// Base class for all multipage, device‑independent views in PhotoPrinter.
//
// Purpose:
//   • Provide a unified rendering pipeline for screen, print preview,
//     printer output, and image/PDF export.
//   • Abstract away all device‑dependent details (mapping modes,
//     scrollbars, zooming, page layout, DPI differences).
//   • Provide consistent logical coordinate space measured in inches,
//     independent of device resolution.
//   • Handle scrolling, paging, zooming, and orientation (horizontal/vertical).
//   • Provide reusable font creation utilities.
//   • Provide double‑buffered drawing to eliminate flicker.
//
// Why this class exists:
//   PhotoPrinter builds entire books — multi‑page documents with precise
//   layout requirements. MFC’s default CScrollView is not sufficient for
//   device‑independent rendering. CBaseView introduces a custom mapping
//   mode (MM_ISOTROPIC) and a logical coordinate system based on inches,
//   ensuring identical layout across:
//
//       • Screen display
//       • Print preview
//       • Printer output
//       • PDF export
//       • Image export
//
// Responsibilities:
//   • Maintain logical view origin (TopOfView / LeftOfView)
//   • Maintain zoom scale (Scale)
//   • Maintain orientation (Horizontal)
//   • Convert between logical units and inches
//   • Manage scrollbars based on logical document size
//   • Handle keyboard and mousewheel navigation
//   • Handle zooming with Ctrl+MouseWheel
//   • Keep mouse focus stable during zoom (TranslatePresentation)
//   • Provide double‑buffered drawing (optional)
//   • Provide unified rendering entry point: render()
//
// This class is intentionally generic. All document‑specific drawing is
// performed by derived classes (e.g., PhotoPrinterView).
/////////////////////////////////////////////////////////////////////////////
class CBaseView : public CScrollView
{
protected: // create from serialization only
	CBaseView();
	DECLARE_DYNCREATE(CBaseView)

	// physical size of client area
	CSize m_sizeClient;	
	
	// logical size of client area
	CSize m_LogicalSizeClient;	
	
	// top of view in inches
	double m_dTopOfView;

	// left of view in inches
	double m_dLeftOfView;

	// horizontal layout
	bool m_bHorizontal;

	// fit to window
	bool m_bFitToWindow;

	// zooming scale factor
	double m_dScale;

	int m_nPhysicalPageWidth;
	int m_nPhysicalPageHeight;
	int m_nLogicalPageWidth;
	int m_nLogicalPageHeight;
	bool m_bInitialUpdate;
	int m_nNumPages;
	bool m_bPrinting;
	bool m_bExporting;
	bool m_bPDF;

	// properties
public:
	// size of the hosting client window in physical pixels
	inline CSize GetSizeClient()
	{
		return m_sizeClient;
	}
	// size of the hosting client window in physical pixels
	inline void SetSizeClient( CSize value )
	{
		m_sizeClient = value;
	}
	// size of the hosting client window in physical pixels
	__declspec( property( get = GetSizeClient, put = SetSizeClient ) )
		CSize SizeClient;

	// size of the hosting client window in physical pixels
	inline CSize GetLogicalSizeClient()
	{
		return m_LogicalSizeClient;
	}
	// size of the hosting client window in physical pixels
	inline void SetLogicalSizeClient( CSize value )
	{
		m_LogicalSizeClient = value;
	}
	// size of the hosting client window in physical pixels
	__declspec( property( get = GetLogicalSizeClient, put = SetLogicalSizeClient ) )
		CSize LogicalSizeClient;

	// height of the client in logical pixels
	int GetLogicalClientHeight()
	{
		const int value = LogicalSizeClient.cy;
		return value;
	}
	// height of the client in logical pixels
	__declspec( property( get = GetLogicalClientHeight ) )
		int LogicalClientHeight;

	// width of the client in logical pixels
	int GetLogicalClientWidth()
	{
		const int value = LogicalSizeClient.cx;
		return value;
	}
	// width of the client in logical pixels
	__declspec( property( get = GetLogicalClientWidth ) )
		int LogicalClientWidth;

	// pointer to the document class
	CBaseDoc* GetDocument() const;
	// pointer to the document class
	__declspec( property( get = GetDocument ) )
		CBaseDoc* Document;

	// logical pixels per inch
	int GetMap()
	{
		CBaseDoc* pDoc = Document;
		const int value = pDoc->Map;
	}
	// logical pixels per inch
	__declspec( property( get = GetMap ) )
		int Map;

	// height of page in inches
	double GetHeightOfPage()
	{
		CBaseDoc* pDoc = Document;
		return pDoc->HeightOfPage;

	}
	// height of page in inches
	__declspec(property(get = GetHeightOfPage))
		double HeightOfPage;

	// width of page in inches
	double GetWidthOfPage()
	{
		CBaseDoc* pDoc = Document;
		return pDoc->WidthOfPage;
	}
	// width of page in inches
	__declspec(property(get = GetWidthOfPage))
		double WidthOfPage;

	// height of the page in logical pixels
	int GetLogicalPageHeight()
	{
		const int value = InchesToLogical(HeightOfPage);
		return value;
	}
	// height of the page in logical pixels
	__declspec(property(get = GetLogicalPageHeight))
		int LogicalPageHeight;

	// width of the page in logical pixels
	int GetLogicalPageWidth()
	{
		const int value = InchesToLogical(WidthOfPage);
		return value;
	}
	// width of the page in logical pixels
	__declspec(property(get = GetLogicalPageWidth))
		int LogicalPageWidth;

	// height of the document in inches
	double GetDocumentHeight()
	{
		CBaseDoc* pDoc = Document;
		const double value = pDoc->Height;
		return value;
	}
	// height of document in inches
	__declspec( property( get = GetDocumentHeight ) )
		double DocumentHeight;

	// width of the document in inches
	double GetDocumentWidth()
	{
		CBaseDoc* pDoc = Document;
		const double value = pDoc->Width;
		return value;
	}
	// width of the document in inches
	__declspec( property( get = GetDocumentWidth ) )
		double DocumentWidth;

	// margin of the document in inches
	double GetDocumentMargin()
	{
		CBaseDoc* pDoc = Document;
		const double value = pDoc->Margin;
		return value;
	}
	// margin of the document in inches
	__declspec( property( get = GetDocumentMargin ) )
		double DocumentMargin;

	// height of the document in logical pixels
	int GetLogicalDocumentHeight()
	{
		const int value = InchesToLogical( DocumentHeight );
		return value;
	}
	// height of the document in logical pixels
	__declspec( property( get = GetLogicalDocumentHeight ) )
		int LogicalDocumentHeight;

	// width of the document in logical pixels
	int GetLogicalDocumentWidth()
	{
		const int value = InchesToLogical( DocumentWidth );
		return value;
	}
	// width of the document in logical pixels
	__declspec( property( get = GetLogicalDocumentWidth ) )
		int LogicalDocumentWidth;

	// margin of the document in logical pixels
	int GetLogicalDocumentMargin()
	{
		const int value = InchesToLogical( DocumentMargin );
		return value;
	}
	// margin of the document in logical pixels
	__declspec( property( get = GetLogicalDocumentMargin ) )
		int LogicalDocumentMargin;

	// get height of the view in inches that are visible
	double GetVisibleHeight()
	{
		CClientDC dc( this );
		SetDrawDC( &dc );
		CSize sizeLogical = m_sizeClient;
		dc.DPtoLP( &sizeLogical );
		const double value = LogicalToInches( sizeLogical.cy );
		return value;
	}
	// get height of the view in inches that are visible
	__declspec( property( get = GetVisibleHeight ) )
		double VisibleHeight;

	// get width of the view in inches that are visible
	double GetVisibleWidth()
	{
		CClientDC dc( this );
		SetDrawDC( &dc );
		CSize sizeLogical = m_sizeClient;
		dc.DPtoLP( &sizeLogical );
		const double value = LogicalToInches( sizeLogical.cx );
		return value;
	}
	// get width of the view in inches that are visible
	__declspec( property( get = GetVisibleWidth ) )
		double VisibleWidth;

	// get line height in inches
	double GetLineHeight()
	{
		const double value = 0.25 / Scale;
		return value;
	}
	// get line height in inches
	__declspec( property( get = GetLineHeight ) )
		double LineHeight;

	// get column width in inches
	inline double GetColumnWidth()
	{
		const double value = 0.25 / Scale;
		return value;
	}
	// get column width in inches
	__declspec( property( get = GetColumnWidth ) )
		double ColumnWidth;

	// last viewable position in inches accounting for view height
	inline double GetLastScrollHeight()
	{
		const double dDocHeight = DocumentHeight;
		const double dViewHeight = ViewHeight;
		const double dLast = dDocHeight - dViewHeight;
		if ( dLast < 0 )
		{
			return 0;
		}
		return dLast;
	}
	// last viewable position in inches accounting for view height
	__declspec( property( get = GetLastScrollHeight ) )
		double LastScrollHeight;

	// last viewable position in inches accounting for view Width
	inline double GetLastScrollWidth()
	{
		const double dDocWidth = DocumentWidth;
		const double dViewWidth = ViewWidth;
		const double dLast = dDocWidth - dViewWidth;
		if ( dLast < 0 )
		{
			return 0;
		}
		return dLast;
	}
	// last viewable position in inches accounting for view width
	__declspec( property( get = GetLastScrollWidth ) )
		double LastScrollWidth;

	// the amount the screen pages up or down in inches
	double GetPagingHeight()
	{
		const double value = VisibleHeight;
		return value;
	}
	// the amount the screen pages up or down in inches
	__declspec( property( get = GetPagingHeight) )
		double PagingHeight;

	// the amount the screen pages left or right in inches
	inline double GetPagingWidth()
	{
		const double value = VisibleWidth / 2;
		return value;
	}
	// the amount the screen pages left or right in inches
	__declspec( property( get = GetPagingWidth) )
		double PagingWidth;

	// number of scrolling lines
	int GetScrollLines()
	{
		const double dLines = DocumentHeight / LineHeight;
		int value = int( dLines );
		if ( !CHelper::NearlyEqual( double( value ), dLines ) )
		{
			value++;
		}
		return value;
	}
	// number of scrolling lines
	__declspec( property( get = GetScrollLines ) )
		int ScrollLines;

	// number of scrolling columns
	inline int GetScrollColumns()
	{
		const double dLines = DocumentWidth / ColumnWidth;
		int nLines = int( dLines );
		if ( nLines != dLines )
		{
			nLines++;
		}
		return nLines;
	}
	// number of scrolling columns
	__declspec( property( get = GetScrollColumns ) )
		int ScrollColumns;

	// number of lines on the page
	inline int GetPageLines()
	{
		const double dLines = ViewHeight / LineHeight;
		int nLines = int( dLines );
		if ( nLines != dLines )
		{
			nLines++;
		}
		return nLines;
	}
	// number of lines on the page
	__declspec( property( get = GetPageLines ) )
		int PageLines;

	// number of columns on the page
	inline int GetPageColumns()
	{
		const double dColumns = ViewWidth / ColumnWidth;
		int nColumns = int( dColumns );
		if ( nColumns != dColumns )
		{
			nColumns++;
		}
		return nColumns;
	}
	// number of Columns on the page
	__declspec( property( get = GetPageColumns ) )
		int PageColumns;

	// first line number visible on the screen
	inline int GetTopLine()
	{
		const double dLine = TopOfView / LineHeight;
		int nLine = int( dLine );
		if ( nLine != dLine )
		{
			nLine++;
		}
		return nLine;
	}
	// first line number visible on the screen
	__declspec( property( get = GetTopLine ) )
		int TopLine;

	// first Column number visible on the screen
	inline int GetLeftColumn()
	{
		const double dColumn = LeftOfView / ColumnWidth;
		int nColumn = int( dColumn );
		if ( nColumn != dColumn )
		{
			nColumn++;
		}
		return nColumn;
	}
	// first Column number visible on the screen
	__declspec( property( get = GetLeftColumn ) )
		int LeftColumn;

	// height of view depends on the orientation of the view
	inline LONG GetPhysicalViewHeight()
	{
		CSize size = SizeClient;
		return Horizontal ? size.cx : size.cy;
	}
	// height of view depends on the orientation of the view
	__declspec( property( get = GetPhysicalViewHeight ) )
		LONG PhysicalViewHeight;

	// width of view depends on the orientation of the view
	inline LONG GetPhysicalViewWidth()
	{
		CSize size = SizeClient;
		return Horizontal ? size.cy : size.cx;
	}
	// width of view depends on the orientation of the view
	__declspec( property( get = GetPhysicalViewWidth ) )
		LONG PhysicalViewWidth;

	// get height of the view in logical units depends on the 
	// orientation of the view
	LONG GetLogicalViewHeight()
	{
		CSize size( LogicalSizeClient );
		const bool bHorizontal = Horizontal;
		const int value = bHorizontal ? size.cx : size.cy;
		return value;
	}
	// get height of the view in logical units depends on the 
	// orientation of the view
	__declspec( property( get = GetLogicalViewHeight ) )
		LONG LogicalViewHeight;

	// get width of the view in logical units depends on the 
	// orientation of the view
	LONG GetLogicalViewWidth()
	{
		CSize size( LogicalSizeClient );
		const bool bHorizontal = Horizontal;
		const int value = bHorizontal ? size.cy : size.cx;
		return value;
	}
	// get width of the view in logical units depends on the 
	// orientation of the view
	__declspec( property( get = GetLogicalViewWidth ) )
		LONG LogicalViewWidth;

	// get view height in inches
	inline double GetViewHeight()
	{
		return LogicalToInches( LogicalViewHeight );
	}
	// get view height in inches
	__declspec( property( get = GetViewHeight ) )
		double ViewHeight;

	// get view width in inches
	inline double GetViewWidth()
	{
		return LogicalToInches( LogicalViewWidth );
	}
	// get view width in inches
	__declspec( property( get = GetViewWidth ) )
		double ViewWidth;

	// top of view in inches
	double GetTopOfView()
	{
		return m_dTopOfView;
	}
	// top of view in inches
	void SetTopOfView( double value )
	{
		m_dTopOfView = value;
	}
	// top of view in inches
	__declspec( property( get = GetTopOfView, put = SetTopOfView ) )
		double TopOfView;

	// left of view in inches
	double GetLeftOfView()
	{
		return m_dLeftOfView;
	}
	// left of view in inches
	void SetLeftOfView( double value )
	{
		m_dLeftOfView = value;
	}
	// left of view in inches
	__declspec( property( get = GetLeftOfView, put = SetLeftOfView ) )
		double LeftOfView;

	// get bottom of view in inches
	double GetBottomOfView()
	{
		const double dTop = TopOfView;
		const double dHeight = VisibleHeight;
		const double value = dTop + dHeight;
		return value;
	}
	// get bottom of view in inches
	__declspec( property( get = GetBottomOfView ) )
		double BottomOfView;

	// get right of view in inches
	double GetRightOfView()
	{
		const double dLeft = LeftOfView;
		const double dWidth = VisibleWidth;
		const double value = dLeft + dWidth;
		return value;
	}
	// get right of view in inches
	__declspec( property( get = GetRightOfView ) )
		double RightOfView;

	// horizontal layout
	inline bool GetHorizontal()
	{
		return m_bHorizontal;
	}
	// horizontal layout
	inline void SetHorizontal( bool value )
	{
		m_bHorizontal = value;
	}
	// horizontal layout
	__declspec( property( get = GetHorizontal, put = SetHorizontal ) )
		bool Horizontal;

	// fit to window
	inline bool GetFitToWindow()
	{
		return m_bFitToWindow;
	}
	// fit to window
	inline void SetFitToWindow( bool value )
	{
		m_bFitToWindow = value;
	}
	// fit to window
	__declspec( property( get = GetFitToWindow, put = SetFitToWindow ) )
		bool FitToWindow;

	// zooming scale factor
	double GetScale()
	{
		return m_dScale;
	}
	// zooming scale factor
	void SetScale( double value )
	{
		m_dScale = value;
	}
	// zooming scale factor
	__declspec( property( get = GetScale, put = SetScale ) )
		double Scale;

	// true when printing
	inline bool GetPrinting()
	{
		return m_bPrinting;
	}
	// true when printing
	inline void SetPrinting( bool value )
	{
		m_bPrinting = value;
	}
	// true when printing
	__declspec( property( get = GetPrinting, put = SetPrinting ) )
		bool Printing;

	// true when exporting images
	inline bool GetExporting()
	{
		return m_bExporting;
	}
	// true when exporting images
	inline void SetExporting( bool value )
	{
		m_bExporting = value;
	}
	// true when exporting images
	__declspec( property( get = GetExporting, put = SetExporting ) )
		bool Exporting;

	// true when exporting a PDF
	inline bool GetPDF()
	{
		return m_bPDF;
	}
	// true when exporting a PDF
	inline void SetPDF( bool value )
	{
		m_bPDF = value;
	}
	// true when exporting a PDF
	__declspec( property( get = GetPDF, put = SetPDF ) )
		bool PDF;

// public methods
public:
	// convert logical co-ordinate value to inches
	double LogicalToInches( int nValue )
	{
		CBaseDoc* pDoc = Document;
		const int nMap = pDoc->Map;
		return ( double( nValue ) / nMap );
	}

	// convert inches to logical co-ordinate value
	int InchesToLogical( double dValue )
	{
		CBaseDoc* pDoc = Document;
		const int nMap = pDoc->Map;
		return int( dValue * nMap );
	}

	// prepare the device context for printing
	void SetPrintDC
	(
		CDC* pDC,
		int& nPhysicalWidth, // in pixels
		int& nPhysicalHeight, // in pixels
		int& nLogicalWidth, // in inches * Map
		int& nLogicalHeight // in inches * Map
	);

	// prepare the device context for exporting
	void SetImageDC
	(
		CDC* pDC,
		int nDPI,
		int nPhysicalWidth, // in pixels
		int nPhysicalHeight, // in pixels
		int& nLogicalWidth, // in inches * Map
		int& nLogicalHeight // in inches * Map
	);

	// prepare the device context for drawing and
	// return the logical width
	int SetDrawDC( CDC* pDC );

	// set the scroll bar position and range
	void SetupScrollBars();

// protected methods
protected:
	// height scroll is vertical scrolling when using vertical layout and is
	// horizontal scrolling when using horizontal layout
	afx_msg void HeightScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );

	// width scroll is horizontal scrolling when using horizontal layout and is
	// vertical scrolling when using vertical layout
	afx_msg void WidthScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );

	// set the position of the height scroll bar
	void SetHeightScollPos( int nPos, BOOL bRedraw = FALSE )
	{
		if ( Horizontal )
		{
			SetScrollPos( SB_HORZ, nPos, bRedraw );
		}
		else
		{
			SetScrollPos( SB_VERT, nPos, bRedraw );
		}
	}

	// set the position of the width scroll bar
	void SetTrackScollPos( int nPos, BOOL bRedraw = FALSE )
	{
		if ( Horizontal )
		{
			int nMin, nMax;
			GetScrollRange( SB_VERT, &nMin, &nMax );
			nPos = nMax - nPos; // invert the position given
			SetScrollPos( SB_VERT, nPos, bRedraw );
		}
		else
		{
			SetScrollPos( SB_HORZ, nPos, bRedraw );
		}
	}

	// set the scroll range of the height scroll bar
	void SetDepthScrollRange( int nMinPos, int nMaxPos, BOOL bRedraw )
	{
		if ( Horizontal )
		{
			SetScrollRange( SB_HORZ, nMinPos, nMaxPos, bRedraw );
		}
		else
		{
			SetScrollRange( SB_VERT, nMinPos, nMaxPos, bRedraw );
		}
	}

	// set the scroll range of the width scroll bar
	void SetTrackScrollRange( int nMinPos, int nMaxPos, BOOL bRedraw )
	{
		if ( Horizontal )
		{
			SetScrollRange( SB_VERT, nMinPos, nMaxPos, bRedraw );
		}
		else
		{
			SetScrollRange( SB_HORZ, nMinPos, nMaxPos, bRedraw );
		}
	}

	// make corrections after a scale change
	void AdjustHeightScrollBar();

	// make corrections after a scale change
	void AdjustWidthScrollBar();

	// The given points represent the physical and logical co-ordinates
	// of the mouse pointer before a zoom operation. This routine will 
	// translate the presentation to keep the displayed data centered 
	// on the given co-ordinates after the zoom operation completes
	void TranslatePresentation
	( 
		CDC* pDC, CPoint ptPhysical, CPoint ptLogical, int nLogicalWidth
	);

	/////////////////////////////////////////////////////////////////////////////
	// generate font characteristics from given font enumeration, where
	// the enumeration is based on Atlas PDF definition
	static void BuildFont
	(
		CString csFace, // name of the font face
		bool bBold, // bold font if true
		bool bItalic, // italic font if true
		int nTextHeight, // text height in pixels
		bool bVertical, // vertical orientation
		CFont& font, // generated font
		double dAngle = 0, // angle in degrees
		BYTE nCharSet = ANSI_CHARSET, // current character set
		bool bFlipX = false, // flip horizontally
		bool bFlipY = false, // flip vertically
		short nUp = -1, // moving up is a negative value
		int nTextWidth = 0 // default width
	);

// Overrides
public:
	// render the page or view
	virtual void render
	(
		CDC* pDC, 
		double dLeftOfView, 
		double dTopOfView, 
		double dRightOfView,
		double dBottomOfView
	);

	virtual void OnPrepareDC( CDC* pDC, CPrintInfo* pInfo = NULL );
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrint( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CBaseView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg BOOL OnMouseWheel( UINT nFlags, short zDelta, CPoint pt );
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	DECLARE_MESSAGE_MAP()
public:

};

#ifndef _DEBUG  // debug version in BaseView.cpp
inline CBaseDoc* CBaseView::GetDocument() const
   { return reinterpret_cast<CBaseDoc*>(m_pDocument); }
#endif

