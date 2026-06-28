/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "PhotoExplorer.h"
#include "ImageView.h"
#include "PhotoExplorerDoc.h"
#include "PhotoExplorerView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

/////////////////////////////////////////////////////////////////////////////
const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE
	(
		ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, 
		&CMainFrame::OnApplicationLook
	)
	ON_UPDATE_COMMAND_UI_RANGE
	(
		ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_WINDOWS_7, 
		&CMainFrame::OnUpdateApplicationLook
	)
	ON_WM_SETTINGCHANGE()
	ON_REGISTERED_MESSAGE
	( 
		QUEUE_QUIT_MESSAGE, OnQueueQuitMessage
	)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
	ID_INDICATOR_PATH,
};

/////////////////////////////////////////////////////////////////////////////
CMainFrame::CMainFrame() noexcept
{
	// TODO: add member initialization code here
	theApp.m_nAppLook = 
		theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_WINDOWS_7);

} // CMainFrame

/////////////////////////////////////////////////////////////////////////////
// return true if any open document has a pending operation
bool CMainFrame::GetPendingOperation()
{
	CWinApp* pApp = AfxGetApp();
	POSITION posTemplate = pApp->GetFirstDocTemplatePosition();
	while ( posTemplate )
	{
		CDocTemplate* pTemplate = pApp->GetNextDocTemplate( posTemplate );
		POSITION posDocument = pTemplate->GetFirstDocPosition();
		while ( posDocument )
		{
			CDocument* pDoc = pTemplate->GetNextDoc( posDocument );
			if ( pDoc->IsKindOf( RUNTIME_CLASS( CPhotoExplorerDoc ) ) )
			{
				CPhotoExplorerDoc* pPED = (CPhotoExplorerDoc*)pDoc;
				if ( pPED->PendingOperation )
				{
					return true;
				}
			}
		}
	}

	return false;
} // GetPendingOperation

/////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnClose()
{
	// do not allow a close during a pending operation 
	// like the generation of thumbnails
	const bool bPendingOp = PendingOperation;

	if ( bPendingOp )
	{
		return;
	}

	CMDIFrameWndEx::OnClose();
} // OnClose

/////////////////////////////////////////////////////////////////////////////
CView* CMainFrame::GetImageView()
{
	CView* value = nullptr;
	CMDIChildWnd *pChild = (CMDIChildWnd*)GetActiveFrame();
	if ( pChild == nullptr )
	{
		return value;
	}

	CDocument* pDoc = pChild->GetActiveDocument();
	if ( !pDoc )
	{
		return value;
	}

	POSITION pos = pDoc->GetFirstViewPosition();
	while ( pos )
	{
		CView* pView = pDoc->GetNextView( pos );
		if ( pView->IsKindOf( RUNTIME_CLASS( CImageView ) ) )
		{
			value = pView;
			break;
		}
	}

	return value;
} // GetImageView

/////////////////////////////////////////////////////////////////////////////
// Get the pointer for the list view.
CView* CMainFrame::GetListView()
{
	CView* value = nullptr;
	CMDIChildWnd* pChild = (CMDIChildWnd*)GetActiveFrame();
	if ( pChild == nullptr )
	{
		return value;
	}

	CDocument* pDoc = pChild->GetActiveDocument();
	if ( !pDoc )
	{
		return value;
	}

	POSITION pos = pDoc->GetFirstViewPosition();
	while ( pos )
	{
		CView* pView = pDoc->GetNextView( pos );
		if ( pView->IsKindOf( RUNTIME_CLASS( CPhotoExplorerView ) ) )
		{
			value = pView;
			break;
		}
	}

	return value;
} // GetListView

/////////////////////////////////////////////////////////////////////////////
CDocument* CMainFrame::GetDocument()
{
	CDocument* value = nullptr;
	CMDIChildWnd *pChild = (CMDIChildWnd*)GetActiveFrame();
	if ( pChild == nullptr )
	{
		return value;
	}

	CDocument* pDoc = pChild->GetActiveDocument();
	if ( !pDoc )
	{
		return value;
	}

	value = pDoc;

	return value;
} // GetDocument

/////////////////////////////////////////////////////////////////////////////
CMainFrame::~CMainFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	BOOL bNameValid;

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE; // other styles available...
	mdiTabParams.m_bActiveTabCloseButton = TRUE;      // set to FALSE to place close button at right of tab area
	mdiTabParams.m_bTabIcons = FALSE;    // set to TRUE to enable document icons on MDI taba
	mdiTabParams.m_bAutoColor = TRUE;    // set to FALSE to disable auto-coloring of MDI tabs
	mdiTabParams.m_bDocumentMenu = TRUE; // enable the document menu at the right edge of the tab area
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	if (!m_wndMenuBar.Create(this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetPaneStyle
	(
		m_wndMenuBar.GetPaneStyle() | 
		CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY
	);

	// prevent the menu bar from taking the focus on activation
	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if 
	(
		!m_wndToolBar.CreateEx
		(
			this, TBSTYLE_FLAT, 
			WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER |
			 CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC
		) ||
		!m_wndToolBar.LoadToolBar
		(
			theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME
		)
	)
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	CString strToolBarName;
	bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	// Allow user-defined toolbars operations:
	InitUserToolbars(nullptr, uiFirstUserToolBarId, uiLastUserToolBarId);

	/////////////////////////////////////////////////////////////////////////////
	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// Get the correct index for the new pane
	int nPathPaneIndex = m_wndStatusBar.CommandToIndex( ID_INDICATOR_PATH );
	if ( nPathPaneIndex != -1 )
	{
		m_wndStatusBar.SetPaneInfo( nPathPaneIndex, ID_INDICATOR_PATH, SBPS_NORMAL, 600 );

	} else
	{
		TRACE0( "Failed to find the path pane\n" );
	}

	/////////////////////////////////////////////////////////////////////////////
	// TODO: Delete these five lines if you don't want the toolbar and 
	// menubar to be dockable
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// enable Visual Studio 2005 style docking window behavior
	CDockingManager::SetDockingMode(DT_SMART);
	// enable Visual Studio 2005 style docking window auto-hide behavior
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// Navigation pane will be created at left, so temporary disable 
	// docking at the left side:
	EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM | CBRS_ALIGN_RIGHT);

	// Create and setup "Outlook" navigation bar:
	if 
	(
		!CreateOutlookBar
		(
			m_wndNavigationBar, ID_VIEW_NAVIGATION, m_wndTree, m_wndCalendar, 250
		)
	)
	{
		TRACE0("Failed to create navigation pane\n");
		return -1;      // fail to create
	}

	// Outlook bar is created and docking on the left side should be allowed.
	EnableDocking(CBRS_ALIGN_LEFT);
	EnableAutoHidePanes(CBRS_ALIGN_RIGHT);

	// create docking windows
	if (!CreateDockingWindows())
	{
		TRACE0("Failed to create docking windows\n");
		return -1;
	}

	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);

	// set the visual manager and style based on persisted value
	OnApplicationLook(theApp.m_nAppLook);

	// Enable enhanced windows management dialog
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);

	// Enable toolbar and docking window menu replacement
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);

	// enable quick (Alt+drag) toolbar customization
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == nullptr)
	{
		// load user-defined toolbar images
		if (m_UserImages.Load(_T(".\\UserImages.bmp")))
		{
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	// enable menu personalization (most-recently used commands)
	// TODO: define your own basic commands, ensuring that each pulldown menu has at least one basic command.
	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_FILE_NEW);
	lstBasicCommands.AddTail(ID_FILE_OPEN);
	lstBasicCommands.AddTail(ID_FILE_SAVE);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_WINDOWS_7);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);

	// Switch the order of document name and application name on the window title bar. This
	// improves the usability of the taskbar because the document name is visible with the thumbnail.
	ModifyStyle(0, FWS_PREFIXTITLE);

	return 0;
} // OnCreate

/////////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::CreateDockingWindows()
{
	BOOL bNameValid;

	// Create output window
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if 
	(
		!m_wndOutput.Create
		(
			strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND, 
			WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
			CBRS_BOTTOM | CBRS_FLOAT_MULTI
		)
	)
	{
		TRACE0("Failed to create Output window\n");
		return FALSE; // failed to create
	}

	// Create properties window
	CString strPropertiesWnd;
	bNameValid = strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
	if 
	(
		!m_wndProperties.Create
		(
			strPropertiesWnd, this, CRect(0, 0, 200, 200), TRUE, 
			ID_VIEW_PROPERTIESWND, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | 
			WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI
		)
	)
	{
		TRACE0("Failed to create Properties window\n");
		return FALSE; // failed to create
	}

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hOutputBarIcon = 
		(HICON) ::LoadImage
		(
			::AfxGetResourceHandle(), 
			MAKEINTRESOURCE
			(
				bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND
			), 
			IMAGE_ICON, 
			::GetSystemMetrics(SM_CXSMICON), 
			::GetSystemMetrics(SM_CYSMICON), 0
		);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = 
		(HICON) 
		::LoadImage
		(
			::AfxGetResourceHandle(), 
			MAKEINTRESOURCE
			(
				bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND
			), 
			IMAGE_ICON, 
			::GetSystemMetrics(SM_CXSMICON), 
			::GetSystemMetrics(SM_CYSMICON), 0
		);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);

	UpdateMDITabbedBarsIcons();
} // SetDockingWindowIcons

/////////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::CreateOutlookBar
(
	CMFCOutlookBar& bar, 
	UINT uiID, 
	CMFCShellTreeCtrl& tree, 
	CCalendarBar& calendar, 
	int nInitialWidth
)
{
	bar.SetMode2003();

	BOOL bNameValid;
	CString strTemp;
	bNameValid = strTemp.LoadString(IDS_SHORTCUTS);
	ASSERT(bNameValid);
	if (!bar.Create(strTemp, this, CRect(0, 0, nInitialWidth, 32000), uiID, WS_CHILD | WS_VISIBLE | CBRS_LEFT))
	{
		return FALSE; // fail to create
	}

	CMFCOutlookBarTabCtrl* pOutlookBar = (CMFCOutlookBarTabCtrl*)bar.GetUnderlyingWindow();

	if (pOutlookBar == nullptr)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pOutlookBar->EnableInPlaceEdit(TRUE);

	static UINT uiPageID = 1;

	// can float, can autohide, can resize, CAN NOT CLOSE
	DWORD dwStyle = AFX_CBRS_FLOAT | AFX_CBRS_AUTOHIDE | AFX_CBRS_RESIZE;

	CRect rectDummy(0, 0, 0, 0);
	const DWORD dwTreeStyle = 
		WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	tree.Create(dwTreeStyle, rectDummy, &bar, 1200);
	bNameValid = strTemp.LoadString(IDS_FOLDERS);
	ASSERT(bNameValid);
	pOutlookBar->AddControl(&tree, strTemp, 2, TRUE, dwStyle);

	calendar.Create(rectDummy, &bar, 1201);
	bNameValid = strTemp.LoadString(IDS_CALENDAR);
	ASSERT(bNameValid);
	pOutlookBar->AddControl(&calendar, strTemp, 3, TRUE, dwStyle);

	bar.SetPaneStyle
	(
		bar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC
	);

	pOutlookBar->SetImageList
	(
		theApp.m_bHiColorIcons ? IDB_PAGES_HC : IDB_PAGES, 24
	);
	pOutlookBar->SetToolbarImageList
	(
		theApp.m_bHiColorIcons ? IDB_PAGES_SMALL_HC : IDB_PAGES_SMALL, 16
	);
	pOutlookBar->RecalcLayout();

	BOOL bAnimation = theApp.GetInt(_T("OutlookAnimation"), TRUE);
	CMFCOutlookBarTabCtrl::EnableAnimation(bAnimation);

	bar.SetButtonsFont(&afxGlobalData.fontBold);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

/////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = 
		new CMFCToolBarsCustomizeDialog(this, TRUE /* scan menus */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

/////////////////////////////////////////////////////////////////////////////
LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

/////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2008:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_WINDOWS_7:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	m_wndOutput.UpdateFonts();
	RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

/////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	// base class does the real work

	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
	{
		return FALSE;
	}


	// enable customization button for all user toolbars
	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++)
	{
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != nullptr)
		{
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CMDIFrameWndEx::OnSettingChange(uFlags, lpszSection);
	m_wndOutput.UpdateFonts();
}

/////////////////////////////////////////////////////////////////////////////
LRESULT CMainFrame::OnQueueQuitMessage( WPARAM wp, LPARAM lp )
{
	return ThreadHelp::OnQueueQuitMessage( wp, lp );

} // OnQueueQuitMessage

/////////////////////////////////////////////////////////////////////////////
