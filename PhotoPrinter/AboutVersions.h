/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "afxdialogex.h"
#include "VersionInfo.h"

/////////////////////////////////////////////////////////////////////////////
// CAboutVersions
//
// Dialog class used for both the About Box and the application splash
// screen. It displays version information, copyright details, company
// information, and a descriptive summary of the PhotoPrinter application.
//
// Purpose:
//   • Present a visually rich About dialog using a background image and
//     stylized text rendered with GDI+.
//   • Serve as the splash screen shown during application startup.
//   • Display version metadata extracted from the executable via CVersionInfo.
//   • Provide a simple, single‑click dismissal mechanism.
//
// Why this class exists:
//   Standard MFC About dialogs are plain and text‑only. PhotoPrinter uses
//   a custom About/Splash dialog to present a professional, branded
//   introduction to the application, including:
//       – Product name
//       – File version
//       – Legal copyright
//       – Company name
//       – File description (split into multiple lines)
//       – A background logo image scaled to fit the dialog
//
// Responsibilities:
//   • Load and draw the background bitmap (IDB_LOGO) with correct aspect ratio.
//   • Render version text using GDI+ fonts, shadows, and color styling.
//   • Retrieve version metadata using CVersionInfo.
//   • Center the dialog and capture mouse input so a single click dismisses it.
//   • Act as both modal About dialog and non‑modal splash screen.
//
// Rendering:
//   • DrawImage() scales and centers the background logo.
//   • DrawLabels() draws product name, version, copyright, company,
//     and description text using GDI+.
//   • DrawText() orchestrates text layout, shadow effect, and color styling.
//
// Interaction:
//   • A left mouse click (OnLButtonDown) closes the dialog.
//   • OnPaint() draws both the background image and the text.
//
// This dialog provides the first visual impression of PhotoPrinter and
// reinforces the application's identity and professionalism.
/////////////////////////////////////////////////////////////////////////////
class CAboutVersions : public CDialogEx
{
	DECLARE_DYNAMIC( CAboutVersions )

protected:
	// character set for locales
	BYTE m_nCharSet;

	// class to contain all version information
	CVersionInfo m_version;

public:
	CAboutVersions( CWnd* pParent = NULL );   // standard constructor
	virtual ~CAboutVersions();

	// Dialog Data
	enum
	{
		IDD = IDD_ABOUTVERSIONS
	};

	// get the module name (executable without path or extension)
	CString GetModuleName();
	// get the module name (executable without path or extension)
	__declspec( property( get = GetModuleName ))
		CString ModuleName;

protected:

	virtual void DoDataExchange( CDataExchange* pDX );    // DDX/DDV support
	virtual BOOL OnInitDialog();

	void DrawImage( CPaintDC& dc );

	void DrawLabels
	(
		Graphics& graphics, PointF pointText,
		Gdiplus::Font& largeFont, Gdiplus::Font& smallFont,
		int nLargeHeight, int nSmallHeight,
		SolidBrush& brush, StringFormat& format
	);

	void DrawText( CPaintDC& dc );

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
	afx_msg void OnPaint();
};

/////////////////////////////////////////////////////////////////////////////
