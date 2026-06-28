/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "OutputWnd.h"
#include "PropertiesWnd.h"
#include "CalendarBar.h"
#include "CNavigator.h"
#include "Resource.h"
#include "ThreadHelp.h"

/////////////////////////////////////////////////////////////////////////////
class COutlookBar : public CMFCOutlookBar
{
	virtual BOOL AllowShowOnPaneMenu() const
	{
		return TRUE;
	}
	virtual void GetPaneName( CString& strName ) const
	{
		BOOL bNameValid = strName.LoadString( IDS_OUTLOOKBAR );
		ASSERT( bNameValid );
		if ( !bNameValid ) strName.Empty();
	}
};

/////////////////////////////////////////////////////////////////////////////
class CMainFrame : public CMDIFrameWndEx, public ThreadHelp
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame() noexcept;

// properties
public:
	// pending operation when count is non-zero
	bool GetPendingOperation();
	// pending operation when count is non-zero
	__declspec
		( property( get = GetPendingOperation ) )
		bool PendingOperation;

	// Get the pointer for the image view.
	CView* GetImageView();
	// Get the pointer for the image view.
	__declspec( property( get = GetImageView ) )
		CView* ImageView;

	// Get the pointer for the list view.
	CView* GetListView();
	// Get the pointer for the list view.
	__declspec( property( get = GetListView ) )
		CView* ListView;

	// Get the active document.
	CDocument* GetDocument();
	// Get the active document.
	__declspec( property( get = GetDocument ) )
		CDocument* Document;

	// Get the navigator
	CNavigator* GetNavigator()
	{
		return &m_wndTree;
	}
	// Get the navigator
	__declspec( property( get = GetNavigator ) )
		CNavigator* Navigator;

	// Get the output pane
	COutputWnd* GetOutputPane()
	{
		return &m_wndOutput;
	}
	// Get the output pane
	__declspec( property( get = GetOutputPane ) )
		COutputWnd* OutputPane;

	// Get the properties pane
	CPropertiesWnd* GetPropertiesPane()
	{
		return &m_wndProperties;
	}
	// Get the properties pane
	__declspec( property( get = GetPropertiesPane ) )
		CPropertiesWnd* PropertiesPane;

	// the selected folder
	void SetFolder( LPCTSTR value )
	{
		if ( ::IsWindow( m_wndStatusBar.m_hWnd ))
		{
			int nPathPaneIndex = 
				m_wndStatusBar.CommandToIndex( ID_INDICATOR_PATH );
			CString csMessage;
			csMessage.Format( L"Pathname: %s", value );
			m_wndStatusBar.SetPaneText( nPathPaneIndex, csMessage );
		}
	}
	// the selected folder
	__declspec( property( put = SetFolder ) )
		CString Folder;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame
	(
		UINT nIDResource, 
		DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, 
		CWnd* pParentWnd = nullptr, 
		CCreateContext* pContext = nullptr
	);

// Implementation
public:
	// go modal on them
	void GoModal()
	{
		ThreadHelp::UpdateUI();
	}

	// modal wait given number of milliseconds
	void Wait( DWORD dwMilliseconds )
	{
		const ULONGLONG ullStart = GetTickCount64();
		ULONGLONG ullDelta = 0;
		do
		{
			GoModal();
			::Sleep( 10 );
			const ULONGLONG ullNow = GetTickCount64();
			ullDelta = ullNow - ullStart;
		}
		while ( ullDelta < dwMilliseconds );
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
	COutputWnd			m_wndOutput;
	CPropertiesWnd		m_wndProperties;
	COutlookBar			m_wndNavigationBar;
	CNavigator			m_wndTree;
	CCalendarBar		m_wndCalendar;

// Generated message map functions
protected:
	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnQueueQuitMessage( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);
	BOOL CreateOutlookBar(CMFCOutlookBar& bar, UINT uiID, CMFCShellTreeCtrl& tree, CCalendarBar& calendar, int nInitialWidth);

	int FindFocusedOutlookWnd(CMFCOutlookBarTabCtrl** ppOutlookWnd);

	CMFCOutlookBarTabCtrl* FindOutlookParent(CWnd* pWnd);
	CMFCOutlookBarTabCtrl* m_pCurrOutlookWnd;
	CMFCOutlookBarPane*    m_pCurrOutlookPage;
};


