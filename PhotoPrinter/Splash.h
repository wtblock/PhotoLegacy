/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VersionInfo.h"

/////////////////////////////////////////////////////////////////////////////
// CSplash
//
// Lightweight splash‑screen dialog shown during PhotoPrinter startup.
// Although it shares the same dialog resource and rendering logic as
// CAboutVersions, CSplash is optimized for non‑modal display and automatic
// dismissal once initialization completes.
//
// Purpose:
//   • Display the PhotoPrinter logo and version information while the
//     application initializes its subsystems (MFC, OLE, COM, GDI+, UI
//     managers, document templates, etc.).
//   • Provide a visually polished startup experience without blocking the
//     main initialization thread.
//   • Reuse the same GDI+ rendering pipeline as the About dialog, ensuring
//     consistent branding across both splash and About screens.
//
// Why this class exists:
//   The About dialog (CAboutVersions) is modal and user‑driven. A splash
//   screen must instead be:
//       – Non‑modal
//       – Automatically positioned over the main frame
//       – Shown immediately during startup
//       – Dismissed programmatically when initialization finishes
//
//   CSplash provides this behavior while still using the same dialog
//   template and drawing code as CAboutVersions.
//
// Responsibilities:
//   • Create and position the splash dialog centered over the main frame.
//   • Draw the background logo (IDB_LOGO) scaled to fit the dialog.
//   • Draw product name, version, copyright, company, and description
//     using GDI+ fonts and shadowed text.
//   • Capture mouse input so a click can dismiss the splash if needed.
//   • Release capture and clean up when destroyed.
//
// Rendering:
//   • DrawImage() scales and centers the logo bitmap.
//   • DrawLabels() renders version metadata using CVersionInfo.
//   • DrawText() applies shadow and color styling identical to the About box.
//
// Interaction:
//   • Splash is typically dismissed programmatically by the main frame
//     after initialization.
//   • OnPaint() draws both the background image and the text.
//   • OnInitDialog() centers the window and captures mouse input.
//
// CSplash provides a polished, branded startup experience and visually
// ties the application together with the About dialog while remaining
// lightweight and non‑modal.
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
