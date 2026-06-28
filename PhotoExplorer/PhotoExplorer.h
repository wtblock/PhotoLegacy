/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

using namespace Gdiplus;

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

/////////////////////////////////////////////////////////////////////////////
class CPhotoExplorerApp : public CWinAppEx
{

// public data
public:
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

// protected methods
protected:
	// Gdiplus initialization
	GdiplusStartupInput m_gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;

// public methods
public:
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

// protected overrides
protected:

// public overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle( LONG lCount );
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

// public constructor/destructor
public:
	CPhotoExplorerApp() noexcept;

};

/////////////////////////////////////////////////////////////////////////////
extern CPhotoExplorerApp theApp;

/////////////////////////////////////////////////////////////////////////////
