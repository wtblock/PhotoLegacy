/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "VersionInfo.h"

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
