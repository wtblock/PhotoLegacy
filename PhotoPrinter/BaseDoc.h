/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CBaseDoc
//
// Base document class for all multi‑page, device‑independent documents in
// the Legacy applications (PhotoPrinter, PhotoExplorer, etc.).
//
// Purpose:
//   • Provide a unified definition of document geometry (page size,
//     margins, logical DPI) so that all derived document classes render
//     consistently across screen, print preview, printer output, and
//     high‑DPI export.
//   • Supply common unit‑conversion helpers (inches ↔ logical units).
//   • Provide default implementations for page count, page size, and
//     margin size.
//   • Provide access to the associated view (CBaseView or derived).
//
// Why this class exists:
//   MFC’s CDocument does not define any notion of page geometry or logical
//   coordinate space. PhotoPrinter and other Legacy applications require a
//   device‑independent layout system based on inches, not pixels. CBaseDoc
//   introduces a fixed logical DPI (Map = 1000 logical units per inch) and
//   exposes all document dimensions in inches, allowing CBaseView to render
//   identically on any device.
//
// Responsibilities:
//   • Define logical DPI (Map = 1000 units per inch).
//   • Define default page size (8.5 × 11 inches).
//   • Define default document size (same as page size unless overridden).
//   • Define default margin (0.25 inches).
//   • Provide conversion helpers:
//         InchesToLogical()
//         LogicalToInches()
//   • Provide access to the associated view.
//   • Provide virtual getters so derived document classes (e.g. PhotoPrinterDoc)
//     can override page count, page size, margins, and document height.
//
// Notes:
//   • CBaseDoc does not perform any rendering; it only defines geometry.
//   • All rendering is performed by CBaseView and its derived classes.
//   • Derived classes override GetPages(), GetHeight(), GetWidth(),
//     GetMargin(), GetHeightOfPage(), and GetWidthOfPage as needed.
//
// This class is intentionally minimal. It serves as the geometric foundation
// for the entire device‑independent rendering pipeline.
/////////////////////////////////////////////////////////////////////////////
class CBaseDoc : public CDocument
{
protected: // create from serialization only
	CBaseDoc();
	DECLARE_DYNCREATE(CBaseDoc)

	// properties
public:
	// logical pixels per inch
	static int GetMap()
	{
		return 1000;
	}
	// logical pixels per inch
	__declspec( property( get = GetMap ) )
		int Map;

	// number of pages in the document
	virtual UINT GetPages()
	{
		return 1;
	}
	// number of pages in the document
	__declspec( property( get = GetPages ) )
		UINT Pages;

	// height of document in inches
	virtual double GetHeight()
	{
		return 11.0;
	}
	// height of document in inches
	__declspec( property( get = GetHeight ) )
		double Height;

	// width of document in inches
	virtual double GetWidth()
	{
		return 8.5;
	}
	// width of document in inches
	__declspec( property( get = GetWidth ) )
		double Width;

	// margin of document in inches
	virtual double GetMargin()
	{
		return 0.25;
	}
	// margin of document in inches
	__declspec( property( get = GetMargin ) )
		double Margin;

	// get a pointer to the view
	CView* GetView()
	{
		POSITION pos = GetFirstViewPosition();
		CView* value = 0;
		while ( value == 0 && pos )
		{
			value = GetNextView( pos );
		}

		return value;
	}
	// get a pointer to the view
	__declspec( property( get = GetView ) )
		CView* View;

	// convert logical co-ordinate value to inches
	double LogicalToInches( int nValue )
	{
		const int nMap = Map;
		return ( double( nValue ) / nMap );
	}

	// convert inches to logical co-ordinate value
	int InchesToLogical( double dValue )
	{
		const int nMap = Map;
		return int( dValue * nMap );
	}

	// height of page in inches
	virtual double GetHeightOfPage()
	{
		return 11.0;
	}
	// height of page in inches
	__declspec(property(get = GetHeightOfPage))
		double HeightOfPage;

	// width of page in inches
	virtual double GetWidthOfPage()
	{
		return 8.5;
	}
	// width of page in inches
	__declspec(property(get = GetWidthOfPage))
		double WidthOfPage;

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CBaseDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};
