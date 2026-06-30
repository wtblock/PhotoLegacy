/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VersionInfo.h"

/////////////////////////////////////////////////////////////////////////////
// CSplash
//
// Dialog that displays Photo Explorer’s splash/about screen, including
// version information, module name, and custom-drawn branding. This dialog
// uses GDI+ to render text and images with precise control over fonts,
// layout, and appearance. It also retrieves version metadata from the
// executable using CVersionInfo.
//
// Purpose:
//   • Present an “About / Version Information” splash screen.
//   • Display module name, version number, build details, and copyright.
//   • Render custom text and graphics using GDI+ for a polished appearance.
//   • Provide a lightweight, modal dialog that can be shown at startup or
//     via an “About” menu command.
//
// Why this class exists:
//   Photo Explorer includes detailed version metadata (file version, product
//   version, build date, company name). A standard dialog would not provide
//   the visual polish or layout flexibility needed to present this
//   information cleanly. CSplash uses GDI+ drawing to create a professional,
//   branded splash screen that reflects the quality of the application.
//
// Responsibilities:
//   • Retrieve version information via CVersionInfo (m_version).
//   • Determine the module name (executable without path or extension).
//   • Draw labels, text, and images using GDI+ (DrawLabels, DrawImage,
//     DrawText).
//   • Handle WM_PAINT to render the splash screen.
//   • Provide CreateDlg() for non-modal creation when needed.
//   • Maintain locale-specific character set information (m_nCharSet).
//
// Interaction with other components:
//   • CVersionInfo — supplies version metadata extracted from the executable.
//   • GDI+ Graphics, Font, Brush, and StringFormat — used for rendering.
//   • CPaintDC — used for device-context drawing in OnPaint.
//   • Application startup or “About” command — triggers display of this dialog.
//
// Key Features:
//   • Custom GDI+ rendering for high-quality splash screen visuals.
//   • Dynamic layout of large and small fonts for version information.
//   • Module name retrieval for display and diagnostics.
//   • Clean separation of drawing logic into DrawLabels, DrawImage, DrawText.
//   • Support for both modal and modeless display via CreateDlg().
//
// Internal Structure:
//   • m_nCharSet — locale-specific character set for font rendering.
//   • m_version — encapsulated version information for the executable.
//   • OnInitDialog — initializes fonts, layout, and version metadata.
//   • DrawLabels — draws version text using large/small fonts.
//   • DrawImage — draws splash image or branding graphics.
//   • DrawText — draws module name and additional text.
//   • OnPaint — orchestrates all drawing operations.
//
// This dialog provides Photo Explorer’s polished, GDI+-rendered splash/about
// screen, presenting version information and branding with clarity and style.
/////////////////////////////////////////////////////////////////////////////
class CSplash : public CDialogEx
{
	DECLARE_DYNAMIC(CSplash)

protected:
	// character set for locales
	BYTE m_nCharSet;

	// class to contain all version information
	CVersionInfo m_version;

public:
	CSplash(CWnd* pParent = NULL);
	virtual ~CSplash();

	// Dialog Data
	enum
	{
		IDD = IDD_ABOUTVERSIONS
	};

	// attributes
public:
	// get the module name (executable without path or extension)
	CString GetModuleName();
	// get the module name (executable without path or extension)
	__declspec( property( get = GetModuleName ))
		CString ModuleName;

// public methods
public:
	void CreateDlg();

// protected methods
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	void DrawLabels
	(
		Graphics& graphics, PointF pointText,
		Gdiplus::Font& largeFont, Gdiplus::Font& smallFont,
		int nLargeHeight, int nSmallHeight,
		SolidBrush& brush, StringFormat& format
	);
	void DrawImage( CPaintDC& dc );
	void DrawText( CPaintDC& dc );
	afx_msg void OnPaint();
};

/////////////////////////////////////////////////////////////////////////////
