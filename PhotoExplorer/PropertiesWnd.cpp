/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "ShellListView.h"
#include "ImageView.h"
#include "PhotoExplorerDoc.h"
#include "PhotoExplorer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar
CPropertiesWnd::CPropertiesWnd() noexcept
{
	m_nComboHeight = 0;
}

/////////////////////////////////////////////////////////////////////////////
CPropertiesWnd::~CPropertiesWnd()
{
}

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	ON_COMMAND(ID_PROPERTIES_LABEL, OnImageLabel)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES_LABEL, OnUpdateImageLabel)
	ON_COMMAND(ID_PROPERTIES_CANCEL, OnPropertiesCancel)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES_CANCEL, OnUpdatePropertiesCancel)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChange )
	ON_COMMAND( ID_BUTTON_PREVIEW, &CPropertiesWnd::OnButtonPreview )
	ON_UPDATE_COMMAND_UI( ID_BUTTON_PREVIEW, &CPropertiesWnd::OnUpdateButtonPreview )
	ON_COMMAND( ID_BUTTON_EXPORT, &CPropertiesWnd::OnButtonExport )
	ON_UPDATE_COMMAND_UI( ID_BUTTON_EXPORT, &CPropertiesWnd::OnUpdateButtonExport )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
LRESULT CPropertiesWnd::OnPropertyChange
( 
	WPARAM /*WP*/, // not used
	LPARAM lp // property grid property pointer
)
{
	LRESULT value = 0;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	// the shell list view contains a collection of properties defined
	// as CImageProperties
	CShellListView* pView = (CShellListView*)pFrame->ListView;

	// retrieve the class that hosts image properties that were read
	// when the user changed folders to the current one. This class
	// contains cross references to allow easy access to image
	// property data
	CImageProperties* pProperties = pView->ImageProperties;

	// this is the index of the selected image's properties with
	// a key in the format of "group|name"
	CKeyedCollection<CString, CImageProperty>* pImageProps = 
		pProperties->Properties;

	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
	
	// the message passed in the property panel property that changed
	// which is displaying a copy of the information in the selected
	// image's corresponding property
	CMFCPropertyGridProperty* pProp = (CMFCPropertyGridProperty*)lp;

	// the current value from the property panel
	const _variant_t varIn = pProp->GetValue();

	// the property panel name of the property
	CString csName = pProp->GetName();

	// the commented text has to do with the old 
	// dropdown date entry that is no longer in use
	//WORD dtIndex = pProperties->DateTimeID[ csName ];
	//bool bDT = dtIndex != 9999;

	// the parent of the property on the panel which contains
	// the group the property belongs to
	CMFCPropertyGridProperty* pParent = pProp->GetParent();
	//CMFCPropertyGridProperty* pGrandParent = pParent->GetParent();
	//CString csGroup = bDT ? pGrandParent->GetName() : pParent->GetName();
	//CString csLabel = bDT ? pParent->GetName() : csName;
	CString csGroup = pParent->GetName();
	CString csLabel = csName;

	// test to see if an album property changed
	if ( csGroup == L"Album" )
	{
		if ( csName == L"Title" )
		{
			pDoc->Title = CString( varIn );

		} else if ( csName == L"Location" )
		{
			pDoc->Location = CString( varIn );

		} else if ( csName == L"Comment" )
		{
			pDoc->Comment = CString( varIn );

		} else if ( csName == L"Date" )
		{
			pDoc->Date = CString( varIn );

		} else if ( csName == L"Artist" )
		{
			pDoc->Artist = CString( varIn );

		} else if ( csName == L"Copyright" )
		{
			pDoc->Copyright = CString( varIn );

		} else if ( csName == L"Software" )
		{
			pDoc->Software = CString( varIn );

		} else if ( csName == L"Keywords" )
		{
			pDoc->Keywords = CString( varIn );
		}

		CString csAlbum = pDoc->AlbumPath;
		if ( csAlbum.IsEmpty())
		{
			CString csFolder = pDoc->Folder;
			CString csTitle = pDoc->Title;
			if ( csTitle.IsEmpty())
			{
				csFolder.TrimRight( L"\\" );
				CString csData = CHelper::GetDataName( csFolder );
				csAlbum = csFolder + L"\\" + csData + L".album";
				pDoc->Title = csData;

			} else
			{
				csFolder.TrimRight( L"\\" );
				csAlbum = csFolder + L"\\" + csTitle + L".album";
			}
		} 

		pDoc->WriteAlbumXml( csAlbum );
		pProp->SetOriginalValue( varIn );

		return value;
	}

	// the key to the properties in the property collection is in
	// the form of "group|name"
	CString csKey;
	csKey.Format( L"%s|%s", (LPCTSTR)csGroup, (LPCTSTR)csLabel );

	// just to be safe, verify this key actually exist (it should
	// since it was used to populate the panel in the first place)
	bool bExists = pImageProps->Exists[ csKey ];

	// ask the index for our property information
	if ( bExists == true )
	{
		shared_ptr<CImageProperty> pProperty = pImageProps->find( csKey );
		ULONG ulID = pProperty->PropertyID;
		pProperty->Value = varIn;
	}

	return value;
} // OnPropertyChange

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar message handlers
void CPropertiesWnd::AdjustLayout()
{
	if 
	(
		GetSafeHwnd() == nullptr || 
		(AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic())
	)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndObjectCombo.SetWindowPos
	(
		nullptr, rectClient.left, rectClient.top, rectClient.Width(), 
		m_nComboHeight, SWP_NOACTIVATE | SWP_NOZORDER
	);
	m_wndToolBar.SetWindowPos
	(
		nullptr, rectClient.left, rectClient.top + m_nComboHeight, rectClient.Width(), 
		cyTlb, SWP_NOACTIVATE | SWP_NOZORDER
	);
	m_wndPropList.SetWindowPos
	(
		nullptr, rectClient.left, rectClient.top + m_nComboHeight + cyTlb, rectClient.Width(), 
		rectClient.Height() -(m_nComboHeight+cyTlb), SWP_NOACTIVATE | SWP_NOZORDER
	);
}

/////////////////////////////////////////////////////////////////////////////
int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// Create combo:
	const DWORD dwViewStyle = 
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | 
		CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1))
	{
		TRACE0("Failed to create Properties Combo \n");
		return -1;      // fail to create
	}

	m_wndObjectCombo.AddString(_T("Metadata"));
	m_wndObjectCombo.SetCurSel(0);

	CRect rectCombo;
	m_wndObjectCombo.GetClientRect (&rectCombo);

	m_nComboHeight = rectCombo.Height();

	if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2))
	{
		TRACE0("Failed to create Properties Grid \n");
		return -1;      // fail to create
	}

	InitPropList();

	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* Is locked */);
	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap
	(
		theApp.m_bHiColorIcons ? 
			IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* Locked */
	);

	m_wndToolBar.SetPaneStyle
	(
		m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY
	);
	m_wndToolBar.SetPaneStyle
	(
		m_wndToolBar.GetPaneStyle() & 
		~(
			CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | 
			CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT
		)
	);

	m_wndToolBar.SetOwner(this);

	// All commands will be routed via this control, not via the parent frame:
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnExpandAllProperties()
{
	m_wndPropList.ExpandAll();
}

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* /* pCmdUI */)
{
}

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnSortProperties()
{
	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
}

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
}

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnButtonPreview()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CImageView* pView = (CImageView*)pFrame->ImageView;

	// toggle the label display
	bool bLabel = pView->Label;
	pView->Label = !bLabel;
	pView->Invalidate();

} // OnButtonPreview

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnUpdateButtonPreview( CCmdUI* pCmdUI )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CImageView* pView = (CImageView*)pFrame->ImageView;
	if ( pView != nullptr )
	{
		bool bLabel = pView->Label;
		pCmdUI->SetCheck( bLabel == true );
	}
} // OnUpdateButtonPreview

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnButtonExport()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CImageView* pView = (CImageView*)pFrame->ImageView;
	pView->ExportImageWithLabels();

} // OnButtonExport

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnUpdateButtonExport( CCmdUI* pCmdUI )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CImageView* pView = (CImageView*)pFrame->ImageView;
	if ( pView != nullptr )
	{
		const bool bLabel = pView->Label;
		pCmdUI->Enable( bLabel == true );
	}

} // OnUpdateButtonExport

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnImageLabel()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CShellListView* pView = (CShellListView*)pFrame->ListView;
	CImageView* pImageView = (CImageView*)pFrame->ImageView;

	pView->SelectedImage = L"~~~";

	pImageView->Invalidate();
} // OnImageLabel

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnUpdateImageLabel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( FALSE );
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
	if ( pDoc != nullptr )
	{
		// updating album labels to the selected image
		bool bAlbumLabel =
			pDoc->AlbumComment || pDoc->AlbumDate ||
			pDoc->AlbumLocation || pDoc->AlbumTitle;

		CShellListView* pView = (CShellListView*)pFrame->ListView;
		const WORD wModified = pView->ImageProperties->ModifiedValues;
		if ( bAlbumLabel || wModified > 0 )
		{
			pCmdUI->Enable();
		}
	}

} // OnUpdateImageLabel

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnPropertiesCancel()
{
	int nReply = AfxMessageBox
	(
		L"Press OK to discard modifications.", MB_OKCANCEL
	);
	if ( nReply == IDOK )
	{
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		CShellListView* pView = (CShellListView*)pFrame->ListView;

		// select the current image to prevent it from saving the
		// current changes
		const CString csImage = pView->SelectedImage;
		pView->SelectedImage = csImage;
		pView->ImageProperties->ModifiedValues = 0;
	}
} // OnPropertiesCancel

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnUpdatePropertiesCancel(CCmdUI* pCmdUI)
{
	pCmdUI->Enable( FALSE );
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CShellListView* pView = (CShellListView*)pFrame->ListView;
	if ( pView != nullptr )
	{
		const WORD wModified = pView->ImageProperties->ModifiedValues;
		if ( wModified > 0 )
		{
			pCmdUI->Enable();
		}
	}
} // OnUpdatePropertiesCancel

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.RemoveAll();
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();

}

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
}

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CDockablePane::OnSettingChange(uFlags, lpszSection);
	SetPropListFont();
}

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::SetPropListFont()
{
	::DeleteObject(m_fntPropList.Detach());

	LOGFONT lf;
	afxGlobalData.fontRegular.GetLogFont(&lf);

	NONCLIENTMETRICS info;
	info.cbSize = sizeof(info);

	afxGlobalData.GetNonClientMetrics(info);

	lf.lfHeight = info.lfMenuFont.lfHeight;
	lf.lfWeight = info.lfMenuFont.lfWeight;
	lf.lfItalic = info.lfMenuFont.lfItalic;

	m_fntPropList.CreateFontIndirect(&lf);

	m_wndPropList.SetFont(&m_fntPropList);
	m_wndObjectCombo.SetFont(&m_fntPropList);
}

/////////////////////////////////////////////////////////////////////////////
