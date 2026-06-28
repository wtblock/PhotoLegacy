/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "afxdialogex.h"
#include "VersionInfo.h"

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
