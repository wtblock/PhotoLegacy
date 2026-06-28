/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "PhotoPrinter.h"
#include "PhotoPrinterDoc.h"
#include "ChildFrm.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CChildFrame, CMDIChildWndEx)

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CChildFrame, CMDIChildWndEx)
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CChildFrame::CChildFrame() noexcept
{
	// TODO: add member initialization code here
}

/////////////////////////////////////////////////////////////////////////////
CChildFrame::~CChildFrame()
{
}

/////////////////////////////////////////////////////////////////////////////
BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CMDIChildWndEx::PreCreateWindow(cs) )
		return FALSE;

	cs.style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
		| FWS_ADDTOTITLE | WS_THICKFRAME;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	if (bActivate) // A new document is becoming active
	{
		CDocument* pDoc = GetActiveDocument();
		if (pDoc)
		{
			CMainFrame* pMainFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
			if (pMainFrame)
			{
				CPropertiesWnd* pProps = pMainFrame->PropertiesPane;
				pProps->UpdatePropertiesFromDocument
				(
					static_cast<CPhotoPrinterDoc*>(pDoc)
				);
				pProps->UpdateTableOfContents();
			}
		}
	}
} // OnMDIActivate

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG

/////////////////////////////////////////////////////////////////////////////
void CChildFrame::AssertValid() const
{
	CMDIChildWndEx::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////
void CChildFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWndEx::Dump(dc);
}

/////////////////////////////////////////////////////////////////////////////
#endif //_DEBUG

