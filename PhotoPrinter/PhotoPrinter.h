/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include <gdiplus.h>
#include "CommandLine.h"
#include <memory>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;

/////////////////////////////////////////////////////////////////////////////
class CPhotoPrinterApp : public CWinAppEx
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

	// optional working folder 
	CString m_csWorkingFolder;

	// pointer to the command line data
	unique_ptr<CCommandLine> m_pCommandLine;
	
// public properties
public:
	// pointer to the command line data which will only be read once
	// because reading it will destroy it (prevents subsequent opening
	// of documents from having the wrong command line data).
	unique_ptr<CCommandLine>& GetCommandLine()
	{
		return m_pCommandLine;
	}
	// pointer to the command line data which will only be read once
	// because reading it will destroy it (prevents subsequent opening
	// of documents from having the wrong command line data).
	inline void SetCommandLine(unique_ptr<CCommandLine> value)
	{
		m_pCommandLine = move(value);
	}
	// pointer to the command line data which will only be read once
	// because reading it will destroy it (prevents subsequent opening
	// of documents from having the wrong command line data).
	__declspec(property(get = GetCommandLine, put = SetCommandLine))
		unique_ptr<CCommandLine> CommandLine;

	// optional working folder 
	inline CString GetWorkingFolder()
	{
		return m_csWorkingFolder;
	}
	// optional working folder 
	inline void SetWorkingFolder(LPCTSTR value)
	{
		m_csWorkingFolder = value;
	}
	// optional working folder 
	__declspec(property(get = GetWorkingFolder, put = SetWorkingFolder))
		CString WorkingFolder;

// public overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

// public constructor/destructor
public:
	CPhotoPrinterApp() noexcept;

	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern CPhotoPrinterApp theApp;

/////////////////////////////////////////////////////////////////////////////
