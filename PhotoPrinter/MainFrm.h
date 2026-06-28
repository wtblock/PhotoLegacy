/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Resource.h"
#include "PropertiesWnd.h"
#include "ThreadHelp.h"

/////////////////////////////////////////////////////////////////////////////
class CPhotoPrinterView;
class CPhotoPrinterDoc;

/////////////////////////////////////////////////////////////////////////////
class CMainFrame : public CMDIFrameWndEx, public ThreadHelp
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame() noexcept;

// Attributes
public:
	// Get the properties pane
	CPropertiesWnd* GetPropertiesPane()
	{
		return &m_wndProperties;
	}
	// Get the properties pane
	__declspec(property(get = GetPropertiesPane))
		CPropertiesWnd* PropertiesPane;

	// Get the active document.
	CPhotoPrinterDoc* GetPhotoPrinterDocument();
	// Get the active document.
	__declspec(property(get = GetPhotoPrinterDocument))
		CPhotoPrinterDoc* PhotoPrinterDocument;

	// Get the pointer for the image view.
	CPhotoPrinterView* GetPhotoPrinterView();
	// Get the pointer for the image view.
	__declspec(property(get = GetPhotoPrinterView))
		CPhotoPrinterView* PhotoPrinterView;


// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = nullptr, CCreateContext* pContext = nullptr);

// Implementation
public:
	// go modal on them
	void GoModal()
	{
		ThreadHelp::UpdateUI();
	}

	// modal wait given number of milliseconds
	void Wait(DWORD dwMilliseconds)
	{
		const ULONGLONG ullStart = GetTickCount64();
		ULONGLONG ullDelta = 0;
		do
		{
			GoModal();
			::Sleep(10);
			const ULONGLONG ullNow = GetTickCount64();
			ullDelta = ullNow - ullStart;
		} while (ullDelta < dwMilliseconds);
	}

	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CMFCMenuBar			m_wndMenuBar;
	CMFCToolBar			m_wndToolBar;
	CMFCStatusBar		m_wndStatusBar;
	CMFCToolBarImages	m_UserImages;
	CPropertiesWnd		m_wndProperties;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);

};

/////////////////////////////////////////////////////////////////////////////

