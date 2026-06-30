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
// CMainFrame
//
// The main frame window for Photo Explorer. This class manages the overall
// application layout, including toolbars, docking windows, navigation panes,
// the properties pane, the output pane, and the Outlook-style navigation bar.
// It also coordinates threading, modal UI updates, and provides convenient
// access to the active views and document.
//
// Purpose:
//   • Serve as the top-level frame window for the MDI application.
//   • Create and manage all docking panes (navigator, properties, output,
//     calendar, Outlook bar).
//   • Provide fast access to the active image view, list view, and document.
//   • Display status information such as the selected folder.
//   • Manage UI updates during background operations via ThreadHelp.
//   • Handle application look-and-feel, customization, and window management.
//
// Why this class exists:
//   Photo Explorer is a pane-heavy application: navigation tree, properties
//   pane, output pane, calendar bar, and multiple views. CMainFrame centralizes
//   the creation, docking, and management of these components. It also provides
//   utility functions for modal UI updates and exposes pointers to key panes
//   so other components can interact with the UI cleanly.
//
// Responsibilities:
//   • Create and initialize all docking windows (CreateDockingWindows).
//   • Manage the Outlook-style navigation bar and its pages.
//   • Provide accessors for:
//       – ImageView (preview pane)
//       – ListView (thumbnail or file list)
//       – Document (active image/document)
//       – Navigator (folder tree)
//       – OutputPane (logging/output messages)
//       – PropertiesPane (metadata display)
//   • Update the status bar with the current folder (SetFolder).
//   • Handle modal UI updates during long operations (GoModal, Wait).
//   • Manage application look (OnApplicationLook, OnUpdateApplicationLook).
//   • Handle window creation, closing, and customization events.
//   • Provide message routing for toolbar creation and Outlook bar behavior.
//
// Interaction with other components:
//   • CNavigator — folder tree navigation.
//   • CPropertiesWnd — metadata display and editing.
//   • COutputWnd — logging and diagnostic output.
//   • CImageView — main image preview and label rendering.
//   • ThreadHelp — ensures UI responsiveness during background tasks.
//   • CMFCOutlookBar — provides Outlook-style navigation tabs.
//   • PhotoExplorerDoc — active document containing image and metadata.
//
// Key Features:
//   • Full docking window management with MFC’s advanced docking architecture.
//   • Outlook-style navigation bar with multiple pages (tree, calendar, etc.).
//   • ThreadHelp integration for smooth modal waits.
//   • Status bar updates for current folder path.
//   • Centralized access to all major panes and views.
//   • Support for application themes and customization dialogs.
//   • Message handlers for window manager, toolbar creation, and settings changes.
//
// Internal Structure:
//   • m_wndMenuBar — main menu bar.
//   • m_wndToolBar — main toolbar.
//   • m_wndStatusBar — status bar with path indicator.
//   • m_wndOutput — output/logging pane.
//   • m_wndProperties — metadata properties pane.
//   • m_wndNavigationBar — Outlook-style navigation bar.
//   • m_wndTree — folder navigator.
//   • m_wndCalendar — calendar pane.
//   • m_pCurrOutlookWnd / m_pCurrOutlookPage — track active Outlook tab.
//
// This class forms the backbone of Photo Explorer’s user interface, managing
// the layout, panes, navigation, and UI responsiveness that define the overall
// user experience.
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


