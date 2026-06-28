/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoExplorer.h"
#include "PhotoExplorerDoc.h"
#include "PhotoExplorerView.h"
#include "CHelper.h"
#include "MainFrm.h"
#include "CNavigator.h"
#include <shlobj.h>
#include <shobjidl.h>

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CNavigator, CMFCShellTreeCtrl)

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP( CNavigator, CMFCShellTreeCtrl )
	ON_NOTIFY_REFLECT( TVN_SELCHANGED, &CNavigator::OnTvnSelchanged )
	ON_NOTIFY_REFLECT( NM_CUSTOMDRAW, &CNavigator::OnNMCustomdraw )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CNavigator::CNavigator()
{
	CString csOD = GetFolderDisplayName( FOLDERID_OneDrive );
	pair< CString, KNOWNFOLDERID> pairOD;
	pairOD.first = csOD.MakeLower();
	pairOD.second = FOLDERID_OneDrive;

	std::vector<std::pair<CString, KNOWNFOLDERID> > knownFolders =
	{
		{ _T( "desktop" ), FOLDERID_Desktop },
		{ _T( "documents" ), FOLDERID_Documents },
		{ _T( "downloads" ), FOLDERID_Downloads },
		{ _T( "pictures" ), FOLDERID_Pictures },
		{ _T( "cameraroll" ), FOLDERID_CameraRoll },
		{ _T( "music" ), FOLDERID_Music },
		{ _T( "home" ), FOLDERID_HomeGroup },
		{ _T( "libraries" ), FOLDERID_Libraries },
		{ _T( "videos" ), FOLDERID_Videos }
	};

	knownFolders.push_back( pairOD );

	m_keyKnown.clear();
	m_keyIds.clear();
	for ( const auto& folder : knownFolders )
	{
		GUID guid = folder.second;
		CString csID = CHelper::MakeStringGUID( guid );
		shared_ptr<CString> ptrID =
			shared_ptr<CString>( new CString( csID ) );
		m_keyIds.add( folder.first, ptrID );

		CString csKnown = GetKnownFolderPath( guid );
		if ( csKnown.IsEmpty())
		{
			continue;
		}
		CString csDisplay = GetFolderDisplayName( guid );
		shared_ptr<CString> ptrKnown =
			shared_ptr<CString>( new CString( csKnown ) );
		m_keyKnown.add( csDisplay, ptrKnown );
	}

} // CNavigator

/////////////////////////////////////////////////////////////////////////////
CNavigator::~CNavigator()
{
}

/////////////////////////////////////////////////////////////////////////////
CString CNavigator::GetRealFolder( CString csSymbolPath )
{
	CString value;
	int nStart = 0;
	CString csToken = csSymbolPath.Tokenize( L"\\", nStart );
	while ( !csToken.IsEmpty())
	{
		if ( m_keyKnown.Exists[ csToken ] )
		{
			value = *m_keyKnown.find( csToken );

		} else
		{
			value += csToken;
		}

		value += L"\\";
		csToken = csSymbolPath.Tokenize( L"\\", nStart );
	}

	return value;
} // GetRealFolder

/////////////////////////////////////////////////////////////////////////////
CString CNavigator::GetFolderDisplayName( REFKNOWNFOLDERID folderId )
{
	CComPtr<IShellItem> pShellItem;
	HRESULT hr = SHCreateItemInKnownFolder( folderId, 0, NULL, IID_PPV_ARGS( &pShellItem ) );
	if ( SUCCEEDED( hr ) )
	{
		LPWSTR pszName = NULL;
		hr = pShellItem->GetDisplayName( SIGDN_NORMALDISPLAY, &pszName );
		if ( SUCCEEDED( hr ) )
		{
			CString displayName( pszName );
			CoTaskMemFree( pszName );
			return displayName;
		}
	}

	return _T( "" );
} // GetFolderDisplayName

/////////////////////////////////////////////////////////////////////////////
// Helper function to resolve known folder GUID to path
CString CNavigator::GetKnownFolderPath( REFKNOWNFOLDERID folderId )
{
	PWSTR pszPath = NULL;
	HRESULT hr = SHGetKnownFolderPath( folderId, 0, NULL, &pszPath );
	if ( SUCCEEDED( hr ) )
	{
		CString strPath( pszPath );
		CoTaskMemFree( pszPath );
		return strPath;
	}

	return _T( "" );
} // GetKnownFolderPath

/////////////////////////////////////////////////////////////////////////////
CString CNavigator::GetItemPath( HTREEITEM hItem )
{
	CString csPath;
	m_arrItems.clear();

	while ( hItem != nullptr )
	{
		CString csItem = GetItemText( hItem );
		m_arrItems.push_back( csItem );
		hItem = GetParentItem( hItem );
	}

	const size_t nCount = m_arrItems.size();

	// only check for symbols on the first item
	bool bFirst = true;

	// the items are supplied in reverse order
	for ( int n = (int)nCount - 1; n >= 0; n-- )
	{
		CString csValue = m_arrItems[ n ];
		CString csItem = csValue;
		csValue.MakeLower();

		// ignore desktop
		if ( csValue == L"desktop" || csValue == L"this pc" )
		{
			continue;
		}

		// the beginning of a path is formated as "volumn name (drive letter:)"
		// so attempt to parse the drive letter and colon from the value
		if ( csValue.FindOneOf( L":" ) != -1 )
		{
			CString csDrive = csValue.Right( 4 );
			if ( csDrive.Left( 1 ) == L"(" && csDrive.Right( 1 ) == L")" )
			{
				csDrive.Trim( L"()" );
				if ( csDrive.Right( 1 ) == L":" )
				{
					csItem = csDrive;
				}
			}
		}

		if ( bFirst && m_keyIds.Exists[ csValue ] )
		{
			CString csGUID = *m_keyIds.find( csValue );
			GUID guid;
			CHelper::MakeGUID( csGUID, guid );
			csPath = GetKnownFolderPath( guid );
		}
		else
		{
			csPath += csItem;
		}

		bFirst = false;
		csPath += _T( "\\" );
	}

	CurrentPath = csPath;

	return csPath;
} // GetItemPath

/////////////////////////////////////////////////////////////////////////////
void CNavigator::OnTvnSelchanged( NMHDR* pNMHDR, LRESULT* pResult )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CDocument* pDoc = pFrame->Document;
	if ( pDoc == nullptr )
	{
		*pResult = 0;
		return;
	}

	CPhotoExplorerDoc* pDocument = (CPhotoExplorerDoc*)pDoc;
	if ( pDocument->PendingOperation )
	{
		*pResult = 0;
		return;
	}

	CString csPath;
	if ( CMFCShellTreeCtrl::GetItemPath( csPath ) )
	{
		*pResult = 0;
		pDocument->Folder = csPath;
		return;
	}

	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>( pNMHDR );

	HTREEITEM hOldItem = pNMTreeView->itemOld.hItem;
	HTREEITEM hNewItem = pNMTreeView->itemNew.hItem;

	if ( /*hOldItem != 0 &&*/ hOldItem != hNewItem )
	{
		// Selection change
		CString strSelectedPath = GetItemPath( hNewItem );
		if ( hOldItem != 0 && !strSelectedPath.IsEmpty() )
		{
			pDocument->Folder = strSelectedPath;
		}
	} else
	{
		// Handle expansion separately if needed
	}


	*pResult = 0;
} // OnTvnSelchanged

/////////////////////////////////////////////////////////////////////////////
BOOL CNavigator::PreCreateWindow( CREATESTRUCT& cs )
{
	if ( CMFCShellTreeCtrl::PreCreateWindow( cs ) )
	{
		// Add the LVS_SHOWSELALWAYS style
		cs.style |= TVS_SHOWSELALWAYS;
		return TRUE;
	}
	return FALSE;
} // PreCreateWindow

///////////////////////////////////////////////////////////////////////////////
void CNavigator::OnNMCustomdraw( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMTVCUSTOMDRAW pNMTVCD = reinterpret_cast<LPNMTVCUSTOMDRAW>( pNMHDR );

	switch ( pNMTVCD->nmcd.dwDrawStage )
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;

		case CDDS_ITEMPREPAINT:
			if ( pNMTVCD->nmcd.uItemState & CDIS_SELECTED )
			{
				pNMTVCD->clrTextBk = RGB( 0, 120, 215 ); // Windows 10 blue highlight color
				pNMTVCD->clrText = RGB( 255, 255, 255 ); // White text
			}
			*pResult = CDRF_DODEFAULT;
			break;

		default:
			*pResult = CDRF_DODEFAULT;
			break;
	}
} // OnNMCustomdraw

///////////////////////////////////////////////////////////////////////////////
