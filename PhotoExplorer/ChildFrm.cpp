/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "PhotoExplorer.h"

#include "ChildFrm.h"
#include "ImageView.h"
#include "PhotoExplorerView.h"
#include "PhotoExplorerDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_UPDATE_COMMAND_UI_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, &CChildFrame::OnUpdateViewStyles)
	ON_COMMAND_RANGE(AFX_ID_VIEW_MINIMUM, AFX_ID_VIEW_MAXIMUM, &CChildFrame::OnViewStyle)
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

// CChildFrame construction/destruction

CChildFrame::CChildFrame() noexcept
{
	// TODO: add member initialization code here
}

CChildFrame::~CChildFrame()
{
}

BOOL CChildFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS( CPhotoExplorerView ), CSize(500, 100), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CImageView), CSize(100, 100), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}
	return TRUE;
}

BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}
#endif //_DEBUG

// CChildFrame message handlers
CPhotoExplorerView* CChildFrame::GetRightPane()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
	CPhotoExplorerView* pView = DYNAMIC_DOWNCAST(CPhotoExplorerView, pWnd);
	return pView;
}

void CChildFrame::OnUpdateViewStyles(CCmdUI* pCmdUI)
{
	if (!pCmdUI)
		return;

	// TODO: customize or extend this code to handle choices on the View menu.
	CPhotoExplorerView* pView = GetRightPane(); 

	// if the right-hand pane hasn't been created or isn't a view, disable commands in our range
	if (pView == nullptr)
		pCmdUI->Enable(FALSE);
	else
	{
		DWORD dwStyle = pView->GetStyle() & LVS_TYPEMASK;
		// if the command is ID_VIEW_LINEUP, only enable command
		// when we're in LVS_ICON or LVS_SMALLICON mode
		if (pCmdUI->m_nID == ID_VIEW_LINEUP)
		{
			if (dwStyle == LVS_ICON || dwStyle == LVS_SMALLICON)
				pCmdUI->Enable();
			else
				pCmdUI->Enable(FALSE);
		}
		else
		{
			// otherwise, use dots to reflect the style of the view
			pCmdUI->Enable();
			BOOL bChecked = FALSE;

			switch (pCmdUI->m_nID)
			{
			case ID_VIEW_DETAILS:
				bChecked = (dwStyle == LVS_REPORT);
				break;

			case ID_VIEW_SMALLICON:
				bChecked = (dwStyle == LVS_SMALLICON);
				break;

			case ID_VIEW_LARGEICON:
				bChecked = (dwStyle == LVS_ICON);
				break;

			case ID_VIEW_LIST:
				bChecked = (dwStyle == LVS_LIST);
				break;

			default:
				bChecked = FALSE;
				break;
			}

			pCmdUI->SetRadio(bChecked ? 1 : 0);
		}
	}
}

void CChildFrame::OnViewStyle(UINT nCommandID)
{
	// TODO: customize or extend this code to handle choices on the View menu.
	CPhotoExplorerView* pView = GetRightPane();

	// if the right-hand pane has been created and is a CPhotoExplorerView, process the menu commands...
	if (pView != nullptr)
	{
		int nStyle = -1;

		switch (nCommandID)
		{
		case ID_VIEW_LINEUP:
			{
				// ask the list control to snap to grid
				CListCtrl& refListCtrl = pView->GetListCtrl();
				refListCtrl.Arrange(LVA_SNAPTOGRID);
			}
			break;

		// other commands change the style on the list control
		case ID_VIEW_DETAILS:
			nStyle = LVS_REPORT;
			break;

		case ID_VIEW_SMALLICON:
			nStyle = LVS_SMALLICON;
			break;

		case ID_VIEW_LARGEICON:
			nStyle = LVS_ICON;
			break;

		case ID_VIEW_LIST:
			nStyle = LVS_LIST;
			break;
		}

		// change the style; window will repaint automatically
		if (nStyle != -1)
			pView->ModifyStyle(LVS_TYPEMASK, nStyle);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CChildFrame::OnMDIActivate( BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd )
{
	CMDIChildWndEx::OnMDIActivate( bActivate, pActivateWnd, pDeactivateWnd );

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	if ( pActivateWnd != this )
	{
		return;
	}

	CDocument* pDoc = GetActiveDocument();
	if ( pDoc == NULL )
	{
		return;
	}

	POSITION pos = pDoc->GetFirstViewPosition();
	CView* pView = 0;
	while ( pos )
	{
		pView = pDoc->GetNextView( pos );
		if ( pView->IsKindOf( RUNTIME_CLASS( CShellListView ) ) )
		{
			SetActiveView( pView );

			CShellListView* pShellView = (CShellListView*)pView;
			pShellView->ImageProperties->UpdatePropertyPane();


			CPhotoExplorerDoc* pPhotoDoc = 
				(CPhotoExplorerDoc*)pView->GetDocument();
			CString csFolder = pPhotoDoc->Folder;
			CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
			CNavigator* pNav = pFrame->Navigator;
			pNav->CurrentPath = csFolder;
			pNav->SelectPath( csFolder );
			break;
		}
	}

	// Tell the application there has been a change of project
	if ( bActivate )
	{
	}

} // OnMDIActivate
