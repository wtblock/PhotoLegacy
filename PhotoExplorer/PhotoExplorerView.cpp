/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "PhotoExplorer.h"
#include "PhotoExplorerDoc.h"
#include "ImageView.h"
#include "PhotoExplorerView.h"
#include "CHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CPhotoExplorerView, CShellListView)

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPhotoExplorerView, CShellListView)
	ON_WM_STYLECHANGED()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND( ID_VIEW_LARGEICON, &CPhotoExplorerView::OnViewLargeicon )
	ON_UPDATE_COMMAND_UI( ID_VIEW_LARGEICON, &CPhotoExplorerView::OnUpdateViewLargeicon )
	ON_COMMAND( ID_VIEW_LIST, &CPhotoExplorerView::OnViewList )
	ON_UPDATE_COMMAND_UI( ID_VIEW_LIST, &CPhotoExplorerView::OnUpdateViewList )
	ON_COMMAND( ID_VIEW_SMALLICON, &CPhotoExplorerView::OnViewSmallicon )
	ON_UPDATE_COMMAND_UI( ID_VIEW_SMALLICON, &CPhotoExplorerView::OnUpdateViewSmallicon )
	ON_COMMAND( ID_VIEW_DETAILS, &CPhotoExplorerView::OnViewDetails )
	ON_UPDATE_COMMAND_UI( ID_VIEW_DETAILS, &CPhotoExplorerView::OnUpdateViewDetails )
	ON_COMMAND( ID_EDIT_CUT, &CPhotoExplorerView::OnEditCut )
	ON_COMMAND( ID_EDIT_COPY, &CPhotoExplorerView::OnEditCopy )
	ON_COMMAND( ID_EDIT_PASTE, &CPhotoExplorerView::OnEditPaste )
	ON_UPDATE_COMMAND_UI( ID_EDIT_CUT, &CPhotoExplorerView::OnUpdateEditCut )
	ON_UPDATE_COMMAND_UI( ID_EDIT_COPY, &CPhotoExplorerView::OnUpdateEditCopy )
	ON_UPDATE_COMMAND_UI( ID_EDIT_PASTE, &CPhotoExplorerView::OnUpdateEditPaste )
	ON_COMMAND( ID_EDIT_DELETE, &CPhotoExplorerView::OnEditDelete )
	ON_UPDATE_COMMAND_UI( ID_EDIT_DELETE, &CPhotoExplorerView::OnUpdateEditDelete )
	ON_COMMAND( ID_EDIT_SELECT_ALL, &CPhotoExplorerView::OnEditSelectAll )
	ON_UPDATE_COMMAND_UI( ID_EDIT_SELECT_ALL, &CPhotoExplorerView::OnUpdateEditSelectAll )
	ON_COMMAND(ID_BING_MAP, &CPhotoExplorerView::OnBingMap)
	ON_UPDATE_COMMAND_UI(ID_BING_MAP, &CPhotoExplorerView::OnUpdateBingMap)
	ON_COMMAND(ID_GOOGLE_MAP, &CPhotoExplorerView::OnGoogleMap)
	ON_UPDATE_COMMAND_UI(ID_GOOGLE_MAP, &CPhotoExplorerView::OnUpdateGoogleMap)
    ON_COMMAND(ID_PROPERTIES_LABEL, &CPhotoExplorerView::OnPropertiesLabel)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES_LABEL, &CPhotoExplorerView::OnUpdatePropertiesLabel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CPhotoExplorerView::CPhotoExplorerView() noexcept
{
	CurrentView = ID_VIEW_DETAILS;

}

/////////////////////////////////////////////////////////////////////////////
CPhotoExplorerView::~CPhotoExplorerView()
{
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoExplorerView::PreTranslateMessage( MSG* pMsg )
{
	CPhotoExplorerDoc* pDoc = Document;

	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( pMsg->wParam == 'A' && ( GetKeyState( VK_CONTROL ) & 0x8000 ) )
		{
			// Handle Ctrl+A
			OnEditSelectAll();
			ListControl->SetFocus();
			return TRUE;

		} else if ( pMsg->wParam == VK_DELETE )
		{
			// Handle Delete key
			if ( ListControl->GetSelectedCount() > 0 )
			{
				OnEditDelete();
			}
			return TRUE;

		} else if ( pMsg->wParam == 'L' && ( GetKeyState( VK_CONTROL ) & 0x8000 ) )
		{
			// Handle Ctrl+F10 key
			if ( ListControl->GetSelectedCount() > 0 )
			{
				pDoc->OnEditExportLabels();
			}
			return TRUE;
		}
	}

	return CView::PreTranslateMessage( pMsg );
} // PreTranslateMessage

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoExplorerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CShellListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnInitialUpdate()
{
	CShellListView::OnInitialUpdate();

	ListControl->SetParent( this );

	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu
	(
		IDR_POPUP_EDIT, point.x, point.y, this, TRUE
	);
}

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void CPhotoExplorerView::AssertValid() const
{
	CShellListView::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::Dump(CDumpContext& dc) const
{
	CShellListView::Dump(dc);
}

/////////////////////////////////////////////////////////////////////////////
CPhotoExplorerDoc* CPhotoExplorerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPhotoExplorerDoc)));
	return (CPhotoExplorerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPhotoExplorerView message handlers
void CPhotoExplorerView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	CListView::OnStyleChanged(nStyleType,lpStyleStruct);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnViewLargeicon()
{
	ListControl->SetView( LVS_ICON );
	CurrentView = ID_VIEW_LARGEICON;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateViewLargeicon( CCmdUI* pCmdUI )
{
	const bool bOn = CurrentView == ID_VIEW_LARGEICON;
	pCmdUI->SetRadio( bOn == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnViewList()
{
	ListControl->SetView( LVS_LIST );
	CurrentView = ID_VIEW_LIST;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateViewList( CCmdUI* pCmdUI )
{
	const bool bOn = CurrentView == ID_VIEW_LIST;
	pCmdUI->SetRadio( bOn == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnViewSmallicon()
{
	ListControl->SetView( LVS_SMALLICON );
	CurrentView = ID_VIEW_SMALLICON;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateViewSmallicon( CCmdUI* pCmdUI )
{
	const bool bOn = CurrentView == ID_VIEW_SMALLICON;
	pCmdUI->SetRadio( bOn == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnViewDetails()
{
	ListControl->SetView( LVS_REPORT );
	CurrentView = ID_VIEW_DETAILS;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateViewDetails( CCmdUI* pCmdUI )
{
	const bool bOn = CurrentView == ID_VIEW_DETAILS;
	pCmdUI->SetRadio( bOn == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::PreSubclassWindow()
{
	CListCtrl& listCtrl = GetListCtrl();
	listCtrl.SetExtendedStyle
	( 
		listCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_HIDELABELS 
	);
	CShellListView::PreSubclassWindow();
} // PreSubclassWindow

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnEditDelete()
{
	CWnd* pFocus = GetFocus();
	if (pFocus && pFocus->IsKindOf(RUNTIME_CLASS(CEdit)))
	{
		// Replace the current selection with an empty string (i.e., delete it)
		pFocus->SendMessage(EM_REPLACESEL, TRUE, (LPARAM)_T(""));
		return;
	}

	CPhotoExplorerDoc* pDoc = Document;
	CString csFolder = pDoc->Folder;
	CKeyedCollection<CString, Bitmap>& pThumbnails = pDoc->Thumbnails;
	CString csFiles;

	// Get selected items and add to m_CutFileList
	POSITION pos = ListControl->GetFirstSelectedItemPosition();
	while ( pos )
	{
		int nItem = ListControl->GetNextSelectedItem( pos );

		// Assume column 0 contains the path
		CString strPath = ListControl->GetItemText( nItem, 0 );

		// remove the cached thumbnail
		if ( pThumbnails.Exists[ strPath ] )
		{
			pThumbnails.remove( strPath );
		}

		CString csData = CHelper::GetDataName( strPath );
		csFiles += csData;
		csFiles += L";";
	}

	pDoc->StopMonitorThread();

	csFiles.TrimRight( L";" );
	CHelper::ShellDelete( csFolder, csFiles );

	// repopulate the list without the deleted items
	pDoc->FindInterestingFiles();

	// reset the selection
	pDoc->AddSelectedImage = L"";
	pDoc->ImageView->Invalidate();

	pDoc->StartMonitorThread();

} // OnEditDelete

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateEditDelete( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( FALSE );
	if ( ListControl->GetSelectedCount() > 0 )
	{
		pCmdUI->Enable();
	}
} // OnUpdateEditDelete

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnEditCut()
{
	m_arrCut.clear();
	m_arrCopy.clear();

	// Get selected items and add to m_CutFileList
	POSITION pos = ListControl->GetFirstSelectedItemPosition();
	while ( pos )
	{
		int nItem = ListControl->GetNextSelectedItem( pos );

		// Assume column 0 contains the path
		CString strPath = ListControl->GetItemText( nItem, 0 ); 
		m_arrCut.push_back( strPath );
	}

} // OnEditCut

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateEditCut( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( FALSE );
	if ( ListControl->GetSelectedCount() > 0 )
	{
		pCmdUI->Enable();
	}
} // OnUpdateEditCut

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnEditCopy()
{
	m_arrCut.clear();
	m_arrCopy.clear();

	// Get selected items and add to m_CutFileList
	POSITION pos = ListControl->GetFirstSelectedItemPosition();
	while ( pos )
	{
		int nItem = ListControl->GetNextSelectedItem( pos );

		// Assume column 0 contains the path
		CString strPath = ListControl->GetItemText( nItem, 0 );
		m_arrCopy.push_back( strPath );
	}

} // OnEditCopy

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateEditCopy( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( FALSE );
	if ( ListControl->GetSelectedCount() > 0 )
	{
		pCmdUI->Enable();
	}
} // OnUpdateEditCopy

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnEditPaste()
{
	CPhotoExplorerDoc* pDoc = Document;
	CString csDestination = pDoc->Folder;
	CString csSource;
	CString csFiles;

	pDoc->StopMonitorThread();

	if ( !m_arrCut.empty())
	{
		for ( auto& csPathname : m_arrCut )
		{
			CString csData = CHelper::GetDataName( csPathname );
			csFiles += csData;
			csFiles += L";";
			csSource = CHelper::GetFolder( csPathname );
		}
		csFiles.TrimRight( L";" );
		CHelper::ShellCopy( csSource, csDestination, csFiles, true );
		m_arrCut.clear();

	} else
	{
		for ( auto& csPathname : m_arrCopy )
		{
			CString csData = CHelper::GetDataName( csPathname );
			csFiles += csData;
			csFiles += L";";
			csSource = CHelper::GetFolder( csPathname );
		}
		csFiles.TrimRight( L";" );
		CHelper::ShellCopy( csSource, csDestination, csFiles );
		m_arrCopy.clear();
	}
	pDoc->FindInterestingFiles();

	pDoc->StartMonitorThread();

} // OnEditPaste

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateEditPaste( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( FALSE );
	if ( !m_arrCut.empty() || !m_arrCopy.empty())
	{
		pCmdUI->Enable();
	}
} // OnUpdateEditPaste

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnEditSelectAll()
{
	CWnd* pFocus = GetFocus();
	if (pFocus && pFocus->IsKindOf(RUNTIME_CLASS(CEdit)))
	{
		// Select all text in the edit control
		pFocus->SendMessage(EM_SETSEL, 0, -1);
		return;
	}

	CShellListCtrl* pCtrl = ListControl;
	pCtrl->SelectAllItems();

} // OnEditSelectAll

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateEditSelectAll( CCmdUI* pCmdUI )
{
	pCmdUI->Enable();

} // OnUpdateEditSelectAll

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnBingMap()
{
	CPhotoExplorerDoc* pDoc = Document;
	CString csImage = pDoc->ImagePath;
	CString csLink = pDoc->GenerateMapLink(csImage);
	ShellExecute(nullptr, L"open", csLink, nullptr, nullptr, SW_SHOWNORMAL);
	//CMapDialog dlg(csLink);
	//dlg.DoModal();

} // OnBingMap

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateBingMap(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
	CPhotoExplorerDoc* pDoc = Document;
	CString csImage = pDoc->ImagePath;
	if (!csImage.IsEmpty())
	{
		CString csLink = pDoc->GenerateMapLink(csImage);
		if (!csLink.IsEmpty())
		{
			pCmdUI->Enable();
		}
	}
} // OnUpdateBingMap

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnGoogleMap()
{
	CPhotoExplorerDoc* pDoc = Document;
	CString csImage = pDoc->ImagePath;
	CString csLink = pDoc->GenerateMapLink(csImage, false );
	ShellExecute(nullptr, L"open", csLink, nullptr, nullptr, SW_SHOWNORMAL);
} // OnGoogleMap

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdateGoogleMap(CCmdUI* pCmdUI)
{
	OnUpdateBingMap(pCmdUI);
} // OnUpdateGoogleMap

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnPropertiesLabel()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CShellListView* pView = (CShellListView*)pFrame->ListView;
	CImageView* pImageView = (CImageView*)pFrame->ImageView;

	pView->SelectedImage = L"~~~";

	pImageView->Invalidate();
} // OnPropertiesLabel

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerView::OnUpdatePropertiesLabel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
	if (pDoc != nullptr)
	{
		// updating album labels to the selected image
		bool bAlbumLabel =
			pDoc->AlbumComment || pDoc->AlbumDate ||
			pDoc->AlbumLocation || pDoc->AlbumTitle;

		CShellListView* pView = (CShellListView*)pFrame->ListView;
		const WORD wModified = pView->ImageProperties->ModifiedValues;
		if (bAlbumLabel || wModified > 0)
		{
			pCmdUI->Enable();
		}
	}

} // OnUpdatePropertiesLabel

/////////////////////////////////////////////////////////////////////////////
