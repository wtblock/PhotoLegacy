/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputWnd::COutputWnd() noexcept
{
	m_pFont = new CFont();
	m_pFont->CreatePointFont( 120, _T( "Lucida Console" ) ); // 120 means 12-point font
}

COutputWnd::~COutputWnd()
{
	delete m_pFont;
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create tabs window:
	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_3D_ROUNDED, rectDummy, this, 1))
	{
		TRACE0("Failed to create output tab window\n");
		return -1;      // fail to create
	}

	// Create output panes:
	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if (!m_wndOutputProgress.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
		!m_wndOutputWarnings.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
		!m_wndOutputErrors.Create(dwStyle, rectDummy, &m_wndTabs, 4))
	{
		TRACE0("Failed to create output windows\n");
		return -1;      // fail to create
	}

	UpdateFonts();

	CString strTabName;
	BOOL bNameValid;

	// Attach list windows to tab:
	bNameValid = strTabName.LoadString(IDS_PROGRESS_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputProgress, strTabName, (UINT)0);
	bNameValid = strTabName.LoadString(IDS_WARNINGS_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputWarnings, strTabName, (UINT)1);
	bNameValid = strTabName.LoadString(IDS_ERRORS_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputErrors, strTabName, (UINT)2);

	// Fill output tabs with some dummy text (nothing magic here)
	FillOutputWindow();
	FillWarningsWindow();
	FillErrorsWindow();

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	// Tab control should cover the whole client area:
	m_wndTabs.SetWindowPos (nullptr, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontRegular);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++)
	{
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, (int)dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

void COutputWnd::FillOutputWindow()
{
	ProgressText = 
		_T("Progress while creating thumbnails and other processing.");
}

void COutputWnd::FillWarningsWindow()
{
	WarningsText =
		_T("Warning information about the current activity.");
}

void COutputWnd::FillErrorsWindow()
{
	ErrorsText = 
		_T( "Error information about the current activity." );
}

void COutputWnd::UpdateFonts()
{
	m_wndOutputProgress.SetFont( m_pFont );
	m_wndOutputWarnings.SetFont( m_pFont );
	m_wndOutputErrors.SetFont( m_pFont );

	//m_wndOutputProgress.SetFont(&afxGlobalData.fontRegular);
	//m_wndOutputWarnings.SetFont(&afxGlobalData.fontRegular);
	//m_wndOutputErrors.SetFont(&afxGlobalData.fontRegular);
}

/////////////////////////////////////////////////////////////////////////////
// show one of the output tabs
void COutputWnd::ShowOutputTab( COutputList& refWnd )
{
	const int nTab =
		m_wndTabs.GetTabFromHwnd( refWnd.GetSafeHwnd() );

	m_wndTabs.SetActiveTab( nTab );
	CWnd* pParent = GetParent();
	if ( pParent == 0 )
	{
		return;
	}
	const BOOL bTab = pParent->IsKindOf( RUNTIME_CLASS( CMFCTabCtrl ) );
	if ( !bTab )
	{
		return;
	}

	CMFCTabCtrl* pParentTab = (CMFCTabCtrl*)pParent;
	
	// Get the tab index of the message panel
	const int nMessageWndTab = pParentTab->GetTabFromHwnd( GetSafeHwnd() );
	
	// Bring the message panel to the top
	pParentTab->SetActiveTab( nMessageWndTab );

} // ShowOutputTab

/////////////////////////////////////////////////////////////////////////////
// COutputList1

COutputList::COutputList() noexcept
{
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
void COutputList::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

/////////////////////////////////////////////////////////////////////////////
void COutputList::OnEditCopy()
{
	// Get the number of items in the list box
	int itemCount = GetCount();
	if ( itemCount == LB_ERR || itemCount == 0 )
		return;

	// Create a string to hold the content
	CStringA strContentA; // Use CStringA for ASCII
	CString strContentW;  // Use CString for Unicode

	// Iterate through the list box items and append them to the string
	for ( int i = 0; i < itemCount; ++i )
	{
		CString strItemText;
		GetText( i, strItemText ); // Directly retrieve into a CString

		CStringA strItemTextA( strItemText ); // Convert to ASCII CStringA
		strContentA += strItemTextA + "\n";
	}

	// Convert ASCII string to Unicode string
	strContentW = CString( strContentA );

	// Open the clipboard
	if ( OpenClipboard() )
	{
		// Empty the clipboard
		EmptyClipboard();

		LONGLONG nLen = strContentW.GetLength() + 1;

		// Allocate global memory for the string
		HGLOBAL hGlobal = GlobalAlloc( GMEM_MOVEABLE, nLen * sizeof( TCHAR ) );
		if ( hGlobal )
		{
			// Lock the global memory and copy the string to it
			LPTSTR pGlobal = (LPTSTR)GlobalLock( hGlobal );
			if ( pGlobal )
			{
				_tcscpy_s( pGlobal, nLen, (LPCTSTR)strContentW );
				GlobalUnlock( hGlobal );

				// Set the clipboard data
				SetClipboardData( CF_UNICODETEXT, hGlobal );
			}
		}
		// Close the clipboard
		CloseClipboard();
	}

} // OnEditCopy

/////////////////////////////////////////////////////////////////////////////
void COutputList::OnEditClear()
{
	ResetContent();

}

/////////////////////////////////////////////////////////////////////////////
void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != nullptr && pParentBar != nullptr)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
} // OnViewOutput

/////////////////////////////////////////////////////////////////////////////

