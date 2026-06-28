/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "PhotoPrinterDoc.h"
#include "PhotoPrinterView.h"
#include "PhotoPrinter.h"
#include "PropertyGridMultilineText.h"

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
	m_pTableOfContents = nullptr;
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
	ON_COMMAND(ID_PROPERTIES_GO_TO, OnGoTo)
	ON_UPDATE_COMMAND_UI(ID_PROPERTIES_GO_TO, OnUpdateGoTo)
	ON_WM_SETFOCUS()
	ON_WM_SETTINGCHANGE()
	ON_REGISTERED_MESSAGE( AFX_WM_PROPERTY_CHANGED, OnPropertyChange )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CPhotoPrinterDoc* CPropertiesWnd::GetPhotoPrinterDocument()
{
	CPhotoPrinterDoc* value = nullptr;
	CMainFrame* pFrame = nullptr;
	try
	{
		pFrame = (CMainFrame*)AfxGetMainWnd();
		value = pFrame->PhotoPrinterDocument;
	}
	catch (...)
	{
		return value;
	}

	return value;
} // GetPhotoPrinterDocument

/////////////////////////////////////////////////////////////////////////////
CPhotoPrinterView* CPropertiesWnd::GetPhotoPrinterView()
{
	CPhotoPrinterView* value = nullptr;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	value = pFrame->PhotoPrinterView;
	return value;
} // GetPhotoPrinterView

/////////////////////////////////////////////////////////////////////////////
LRESULT CPropertiesWnd::OnPropertyChange
( 
	WPARAM /*WP*/, // not used
	LPARAM lp // property grid property pointer
)
{
	LRESULT value = 0;

	CPhotoPrinterDoc* pDoc = PhotoPrinterDocument;
	CPhotoPrinterView* pView = PhotoPrinterView;

	// the message passed in the property panel property that changed
	// which is displaying a copy of the information in the selected
	// image's corresponding property
	CMFCPropertyGridProperty* pProp = (CMFCPropertyGridProperty*)lp;

	// the current value from the property panel
	const _variant_t varIn = pProp->GetValue();

	// the property panel name of the property
	CString csName = pProp->GetName();

	// the parent of the property on the panel which contains
	// the group the property belongs to
	CMFCPropertyGridProperty* pParent = pProp->GetParent();
	if (pParent == nullptr)
	{
		return value;
	}
	
	CString csGroup = pParent->GetName();
	CString csLabel = csName;

	if (csGroup == L"Table of Contents")
	{
		OnGoTo();
		return value;
	}
	// keep track of folder changes
	bool bFolder = false;
	CString csStart = pDoc->StartFolder;
	CString csEnd = pDoc->EndFolder;

	// test to see if an album property changed
	if ( csGroup == L"Title Page Properties" )
	{
		if ( csName == L"Title" )
		{
			pDoc->Title = CString( varIn );

		} else if ( csName == L"Subtitle" )
		{
			pDoc->Subtitle = CString( varIn );

		} else if ( csName == L"Publisher" )
		{
			pDoc->Publisher = CString( varIn );

		} else if ( csName == L"ISBN" )
		{
			pDoc->ISBN = CString( varIn );

		} else if ( csName == L"Description" )
		{
			pDoc->Description = CString( varIn );

		} else if ( csName == L"Copyright" )
		{
			pDoc->Copyright = CString( varIn );
		}
	}
	else if (csGroup == L"Folder Collection Range")
	{
		if (csName == L"Start Folder")
		{
			CString csFolder = CString(varIn);
			csFolder.TrimRight(L"\\");
			if (csFolder != csStart)
			{
				CString csStartFolder = 
					pDoc->CorrectForWorkingFolder(csFolder);
				if (::PathFileExists(csStartFolder))
				{
					pDoc->StartFolder = csFolder;
					bFolder = true;
					csStart = pDoc->StartFolder;
				}
				else
				{
					CString csMessage;
					csMessage.Format
					(
						L"Pathname does not exist:\n%s", 
						csStartFolder
					);
					AfxMessageBox(csMessage);
				}
			}
		}
		else if (csName == L"End Folder")
		{
			CString csFolder = CString(varIn);
			csFolder.TrimRight(L"\\");
			if (csFolder != csEnd)
			{
				CString csEndFolder = 
					pDoc->CorrectForWorkingFolder(csFolder);
				if (::PathFileExists(csEndFolder))
				{
					pDoc->EndFolder = csFolder;
					bFolder = true;
					csEnd = pDoc->EndFolder;
				}
				else
				{
					CString csMessage;
					csMessage.Format
					(
						L"Pathname does not exist:\n%s", 
						csEndFolder
					);
					AfxMessageBox(csMessage);
				}
			}
		}
		else if (csName == L"Query")
		{
			pDoc->Query = CString(varIn);
		}
	}
	else if (csGroup == L"Export Properties")
	{
		if (csName == L"Export Folder")
		{
			pDoc->ExportFolder = CString(varIn);
		}
		else if (csName == L"Export Pages")
		{
			pDoc->ExportPages = CString(varIn);
		}
		else if (csName == L"Export DPI")
		{
			CString csDPI = CString(varIn);
			csDPI.Trim();
			if (csDPI.IsEmpty())
			{
				csDPI = L"400";
			}
			pDoc->ExportDPI = (UINT)_tstol(csDPI);
		}
		else if (csName == L"Export Quality")
		{
			CString csQuality = CString(varIn);
			csQuality.Trim();
			if (csQuality.IsEmpty())
			{
				csQuality = L"75";
			}
			pDoc->ExportQuality = (UINT)_tstol(csQuality);
		}
	}

	pProp->SetOriginalValue(varIn);

	// if the user changed a folder, initialize the document
	if (bFolder)
	{
		pDoc->InitDocument();
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->Wait(1000);
		UpdateTableOfContents();
	}

	// update the view
	pView->SetupScrollBars();
	pView->Invalidate();

	return value;
} // OnPropertyChange

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::UpdatePropertiesFromDocument(CPhotoPrinterDoc* pDoc)
{
	CPhotoPrinterView* pView = PhotoPrinterView;

	// Iterate through top-level properties
	for (int i = 0; i < m_wndPropList.GetPropertyCount(); ++i)
	{
		CMFCPropertyGridProperty* pCategory = m_wndPropList.GetProperty(i);

		if (!pCategory) continue;

		// Iterate through child properties within the category
		for (int j = 0; j < pCategory->GetSubItemsCount(); ++j)
		{
			CMFCPropertyGridProperty* pProp = pCategory->GetSubItem(j);

			CString csName = pProp->GetName();
			if (csName == L"Title")
			{
				CString csTitle = pDoc->Title;
				pProp->SetValue(csTitle);
			}
			else if (csName == L"Subtitle")
			{
				CString csSubtitle = pDoc->Subtitle;
				pProp->SetValue(csSubtitle);
			}
			else if (csName == L"Publisher")
			{
				CString csPublisher = pDoc->Publisher;
				pProp->SetValue(csPublisher);
			}
			else if (csName == L"ISBN")
			{
				CString csISBN = pDoc->ISBN;
				pProp->SetValue(csISBN);
			}
			else if (csName == L"Description")
			{
				CString csDescription = pDoc->Description;
				pProp->SetValue(csDescription);
			}
			else if (csName == L"Copyright")
			{
				CString csCopyright = pDoc->Copyright;
				pProp->SetValue(csCopyright);
			}
			else if (csName == L"Start Folder")
			{
				CString csStartFolder = pDoc->StartFolder;
				pProp->SetValue(csStartFolder);
			}
			else if (csName == L"End Folder")
			{
				CString csEndFolder = pDoc->EndFolder;
				pProp->SetValue(csEndFolder);
			}
			else if (csName == L"Query")
			{
				CString csQuery = pDoc->Query;
				pProp->SetValue(csQuery);
			}
			else if (csName == L"Export Folder")
			{
				CString csExportFolder = pDoc->ExportFolder;
				pProp->SetValue(csExportFolder);
			}
			else if (csName == L"Export Pages")
			{
				CString csExportPages = pDoc->ExportPages;
				pProp->SetValue(csExportPages);
			}
			else if (csName == L"Export DPI")
			{
				CString csValue;
				csValue.Format(L"%d", pDoc->ExportDPI);
				if (csValue.IsEmpty())
				{
					csValue = L"400";
				}
				pProp->SetValue(csValue);
			}
			else if (csName == L"Export Quality")
			{
				CString csValue;
				csValue.Format(L"%d", pDoc->ExportQuality);
				if (csValue.IsEmpty())
				{
					csValue = L"75";
				}
				pProp->SetValue(csValue);
			}
		}
	}

	// Refresh the property grid to reflect updates
	m_wndPropList.RedrawWindow();

} // UpdatePropertiesFromDocument

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
void CPropertiesWnd::OnGoTo()
{
	CPhotoPrinterDoc* pDoc = PhotoPrinterDocument;
	if (!pDoc)
	{
		return;
	}
	CPhotoPrinterView* pView = PhotoPrinterView;

	CMFCPropertyGridProperty* pSelectedProp = m_wndPropList.GetCurSel();
	CString propValue = pSelectedProp->GetValue().bstrVal;
	const int nPage = (int)_tstoi(propValue);
	pDoc->Page = nPage;
	const double dPageHeight = pDoc->HeightOfPage;
	const double dTop = dPageHeight * (nPage - 1);
	pView->TopOfView = dTop;
	pView->SetupScrollBars();
	pView->Invalidate();
} // OnGoTo

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnUpdateGoTo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
	int nPages = 0;
	try
	{
		CPhotoPrinterDoc* pDoc = PhotoPrinterDocument;
		if (!pDoc)
		{
			return;
		}
		nPages = pDoc->Pages;
	}
	catch (...)
	{
		return;
	}

	CMFCPropertyGridProperty* pSelectedProp = m_wndPropList.GetCurSel();
	if (pSelectedProp)
	{
		CMFCPropertyGridProperty* pParent = pSelectedProp->GetParent();
		if (pParent)
		{
			CString csParent = pParent->GetName();
			if (csParent == L"Table of Contents")
			{
				CString propName = pSelectedProp->GetName();
				CString propValue = pSelectedProp->GetValue().bstrVal; 
				const int nPage = (int)_tstoi(propValue);
				if (1 <= nPage && nPage <= nPages)
				{
					pCmdUI->Enable(TRUE);
				}
			}
		}
	}
} // OnUpdateGoTo

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::InitPropList()
{
	SetPropListFont();

	m_wndPropList.RemoveAll();
	m_wndPropList.EnableHeaderCtrl(FALSE);
	m_wndPropList.EnableDescriptionArea();
	m_wndPropList.SetVSDotNetLook();
	m_wndPropList.MarkModifiedProperties();
	m_wndPropList.Host = this;

	CMFCPropertyGridProperty* pGroup1 = 
		new CMFCPropertyGridProperty(L"Title Page Properties");
	pGroup1->SetDescription
	(
		L"Properties of the document to be displayed on the title page."
	);

	pGroup1->AddSubItem
	(
		new CMFCPropertyGridProperty
		(
			L"Title", 
			(_variant_t) L"",
			L"The title of the document being created."
		)
	);

	pGroup1->AddSubItem
	(
		new CMFCPropertyGridProperty
		(
			L"Subtitle", 
			(_variant_t) L"",
			L"Brief additional information about this title."
		)
	);

	pGroup1->AddSubItem
	(
		new CMFCPropertyGridProperty
		(
			L"Publisher", 
			(_variant_t) L"",
			L"The name of the person or organization publishing the document."
		)
	);

	pGroup1->AddSubItem
	(
		new CMFCPropertyGridProperty
		(
			L"ISBN", 
			(_variant_t) L"",
			L"The International Standard Book Number of the document."
		)
	);

	pGroup1->AddSubItem
	(
		new CMFCPropertyGridProperty
		(
			L"Copyright", 
			(_variant_t) L"Copyright © 2026",
			L"Copyright information for the document."
		)
	);

	pGroup1->AddSubItem
	(
		new CPropertyGridMultilineText
		(
			L"Description", 
			L"",
			L"A description of the contents of this document."
		)
	);

	m_wndPropList.AddProperty(pGroup1);

	CMFCPropertyGridProperty* pGroup2 = 
		new CMFCPropertyGridProperty(L"Folder Collection Range");
	pGroup2->SetDescription
	(
		L"Specifies the inclusive range of folders between which "
		L"labeled images are collected to populate the document."
	);

	CMFCPropertyGridFileProperty* pProp1 = new CMFCPropertyGridFileProperty
	(
		L"Start Folder",
		L""
	);
	pProp1->SetDescription(L"The album folder to begin collecting images.");
	pGroup2->AddSubItem(pProp1);

	CMFCPropertyGridFileProperty* pProp2 = new CMFCPropertyGridFileProperty
	(
		L"End Folder",
		L""
	);
	pProp2->SetDescription(L"The last album folder where images are collected.");
	pGroup2->AddSubItem(pProp2);

	pGroup2->AddSubItem
	(
		new CPropertyGridMultilineText
		(
			L"Query",
			L"",
			L"A query string to filter the contents based on metadata."
		)
	);

	m_wndPropList.AddProperty(pGroup2);

	CMFCPropertyGridProperty* pGroup3 = 
		new CMFCPropertyGridProperty(L"Export Properties");
	pGroup2->SetDescription
	(
		L"Specifies the information on how and where the document "
		L"will be exported."
	);

	CMFCPropertyGridFileProperty* pProp3 = new CMFCPropertyGridFileProperty
	(
		L"Export Folder",
		L""
	);
	pProp3->SetDescription(L"The folder where exported documents are written to.");
	pGroup3->AddSubItem(pProp3);

	pGroup3->AddSubItem
	(
		new CMFCPropertyGridProperty
		(
			L"Export Pages",
			(_variant_t)L"",
			L"The pages to be exported where:\n"
			L"  * A blank value is for all pages.\n"
			L"  * A range is spcified by a start and end separated with a dash.\n"
			L"  * Individual page numbers separated by commas.\n"
			L"  * Example: 5-8, 10-12, 15, 20\n"
			L"      would output pages 5, 6, 7, 8, 10, 11, 12, 15 and 20\n"
		)
	);

	pGroup3->AddSubItem
	(
		new CMFCPropertyGridProperty
		(
			L"Export DPI",
			(_variant_t)L"400",
			L"The dots per inch used when exporting page images from the document."
		)
	);

	pGroup3->AddSubItem
	(
		new CMFCPropertyGridProperty
		(
			L"Export Quality",
			(_variant_t)L"75",
			L"The quality of images in percent between 1 and 100 where a higher quality generates sharper images and a lower quality generates smaller images."
		)
	);

	m_wndPropList.AddProperty(pGroup3);

} // InitPropList

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::UpdateTableOfContents()
{
	CPhotoPrinterDoc* pDoc = PhotoPrinterDocument;

	if (!pDoc || !m_wndPropList.GetSafeHwnd())
		return;

	// Ensure the Table of Contents group exists
	if (!m_pTableOfContents)
	{
		m_pTableOfContents = new CMFCPropertyGridProperty(_T("Table of Contents"));
		m_pTableOfContents->SetDescription
		(
			L"A list of albums with their corresponding page numbers."
		);
		m_wndPropList.AddProperty(m_pTableOfContents);
	}
	else
	{
		// Remove existing sub-items
		int subItemCount = m_pTableOfContents->GetSubItemsCount();
		for (int i = subItemCount - 1; i >= 0; --i) // Remove from last to first
		{
			CMFCPropertyGridProperty* pSubItem = m_pTableOfContents->GetSubItem(0);
			m_pTableOfContents->RemoveSubItem(pSubItem);
			delete pSubItem; // Prevent memory leaks
		}
	}

	// Populate new sub-items from the document
	vector<pair<CString, int>>& arrTableOfContents = 
		pDoc->AlbumTableOfContents;
	for (auto& album : arrTableOfContents) 
	{
		CString csAlbum = album.first;
		int nPage = album.second;
		CString csPage;
		csPage.Format(L"%d", nPage);

		CMFCPropertyGridProperty* pSubItem =
			new CMFCPropertyGridProperty
			(
				csAlbum, (_variant_t)csPage,
				L"Page number of the named album."
			);
		pSubItem->AllowEdit(FALSE); // Make it read-only
		m_pTableOfContents->AddSubItem(pSubItem);
	}

	m_pTableOfContents->Expand();
	m_wndPropList.ExpandAll();
	m_wndPropList.RedrawWindow(); // Ensure UI updates
} // UpdateTableOfContents

/////////////////////////////////////////////////////////////////////////////
void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndPropList.SetFocus();
} // OnSetFocus

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

}

/////////////////////////////////////////////////////////////////////////////
