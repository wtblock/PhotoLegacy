/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoExplorer.h"
#include "ImageProperties.h"
#include "PhotoExplorerDoc.h"
#include "ShellListView.h"
#include "ImageView.h"
#include "MainFrm.h"
#include "CHelper.h"
#include "ExifRotation.h"

constexpr auto uipDelayAdd = 1;
constexpr auto uipDelayDel = 2;
constexpr auto uipFolder = 3;
constexpr auto uipTabs = 4;

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CShellListView, CListView)

/////////////////////////////////////////////////////////////////////////////
CShellListView::CShellListView()
{
	ThumbnailSize = 128;
}

/////////////////////////////////////////////////////////////////////////////
CShellListView::~CShellListView()
{
}

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CShellListView, CListView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_MESSAGE( WM_UPDATE_THUMBNAIL, &CShellListView::OnUpdateThumbnail )
	ON_MESSAGE( WM_FOLDER_DELETE_EVENT, &CShellListView::OnFolderDeleteEvent )
	ON_MESSAGE( WM_FOLDER_ADD_EVENT, &CShellListView::OnFolderAddEvent )
	ON_WM_TIMER()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void CShellListView::AssertValid() const
{
	CListView::AssertValid();
}

	#ifndef _WIN32_WCE
	void CShellListView::Dump(CDumpContext& dc) const
	{
		CListView::Dump(dc);
	}
	#endif

	CPhotoExplorerDoc* CShellListView::GetDocument() // non-debug version is inline
	{
		ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CPhotoExplorerDoc ) ) );
		return (CPhotoExplorerDoc*)m_pDocument;
	}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
int CShellListView::OnCreate( LPCREATESTRUCT lpCreateStruct )
{
	if ( CListView::OnCreate( lpCreateStruct ) == -1 )
		return -1;

	ListControl->Create
	( 
		WS_CHILD | WS_VISIBLE | LVS_ICON, CRect( 0, 0, 0, 0 ), this, 1 
	);

	// size of images
	const int nSize = (int)ThumbnailSize;

	// Recreate the image list
	ImageList->Create( nSize, nSize, ILC_COLOR32 | ILC_MASK, 0, 1 );

	// Assign the new image list to the list control
	ListControl->SetImageList( ImageList, LVSIL_NORMAL );

	return 0;
} // OnCreate

/////////////////////////////////////////////////////////////////////////////
void CShellListView::ClearImageList()
{
	// Remove all images from the image list
	ImageList->DeleteImageList();

	// size of images
	const int nSize = (int)ThumbnailSize;

	// Recreate the image list
	ImageList->Create( nSize, nSize, ILC_COLOR32 | ILC_MASK, 0, 1 );

	// Reassign the new image list to the list control
	ListControl->SetImageList( ImageList, LVSIL_NORMAL );

} // ClearImageList

/////////////////////////////////////////////////////////////////////////////
void CShellListView::ClearItems()
{
	// Clear all items in the list control
	ListControl->DeleteAllItems();

	// Clear the image list
	ClearImageList();

} // ClearItems

/////////////////////////////////////////////////////////////////////////////
void CShellListView::ClearCache()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	CKeyedCollection<CString, Bitmap>& pThumbnails = pDoc->Thumbnails;
	pThumbnails.clear();

} // ClearCache

/////////////////////////////////////////////////////////////////////////////
void CShellListView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	// Position the control to fill the client area
	CRect rectClient;
	GetClientRect( &rectClient );
	ListControl->MoveWindow( &rectClient );

	SetTimer( uipFolder, 200, nullptr );

} // OnInitialUpdate

/////////////////////////////////////////////////////////////////////////////
void CShellListView::OnSize( UINT nType, int cx, int cy )
{
	CListView::OnSize( nType, cx, cy );

	CPhotoExplorerDoc* pDoc = GetDocument();
	pDoc->SelectedImages.clear();

	CShellListCtrl* pCtrl = ListControl;

	// Adjust the list control layout
	if (pCtrl->GetSafeHwnd() )
	{
		pCtrl->DeselectAllItems();

		CRect rect;
		GetClientRect( &rect );
		pCtrl->SetWindowPos
		( 
			NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE 
		);
		
		PopulateList();
	}
} // OnSize

/////////////////////////////////////////////////////////////////////////////
void CShellListView::SetProgressText( CString value )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COutputWnd* pOut = pFrame->OutputPane;
	pOut->ProgressText = value;
}

/////////////////////////////////////////////////////////////////////////////
void CShellListView::SetWarningsText( CString value )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COutputWnd* pOut = pFrame->OutputPane;
	pOut->WarningsText = value;
}

/////////////////////////////////////////////////////////////////////////////
void CShellListView::SetErrorsText( CString value )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COutputWnd* pOut = pFrame->OutputPane;
	pOut->ErrorsText = value;
}

/////////////////////////////////////////////////////////////////////////////
bool CShellListView::GetAlbumTitle()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumTitle;
}

/////////////////////////////////////////////////////////////////////////////
bool CShellListView::GetAlbumLocation()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumLocation;
}

/////////////////////////////////////////////////////////////////////////////
bool CShellListView::GetAlbumComment()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumComment;
}

/////////////////////////////////////////////////////////////////////////////
bool CShellListView::GetAlbumDate()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumDate;
}

/////////////////////////////////////////////////////////////////////////////
void CShellListView::AddImageToList
(
	const CString& imagePath 
)
{
	try
	{
		CPhotoExplorerDoc* pDoc = Document;
		shared_ptr<Bitmap> pThumbnail = pDoc->Thumbnail[ imagePath ];
		const UINT uiCount = pThumbnail->GetPropertyCount();

		// Convert the GDI+ thumbnail to HBITMAP
		HBITMAP hThumbnail = NULL;
		pThumbnail->GetHBITMAP( Color( 255, 255, 255 ), &hThumbnail );

		// Add the thumbnail to the image list
		int imageIndex =
			ImageList->Add( CBitmap::FromHandle( hThumbnail ), RGB( 255, 255, 255 ) );
		const int nItems = ListControl->GetItemCount();
		int listIndex = ListControl->InsertItem( nItems, imagePath, imageIndex );

		const CString csFile = CHelper::GetDataName( imagePath );
		CString csMessage;
		csMessage.Format( L"%04d--%s", listIndex + 1, csFile );
		ProgressText = csMessage;

	} catch ( ... )
	{
	}
} // AddImageToList

/////////////////////////////////////////////////////////////////////////////
void CShellListView::UpdateDocumentTabs()
{
	SetTimer( uipTabs, 200, nullptr );

} // UpdateDocumentTabs


/////////////////////////////////////////////////////////////////////////////
void CShellListView::OnTimer( UINT_PTR nIDEvent )
{
	CPhotoExplorerDoc* pDoc = Document;
	switch ( nIDEvent )
	{
		case uipTabs :
		{
			KillTimer( uipTabs );
			pDoc->UpdateDocumentTabs();
			break;
		}
		case uipDelayAdd :
		{
			KillTimer( uipDelayAdd );
			CKeyedCollection<CString, int>& mapAddPaths = pDoc->AddedPaths;
			pDoc->FindInterestingFiles( false );
			mapAddPaths.clear();
			break;
		}
		case uipDelayDel:
		{
			KillTimer( uipDelayDel );
			CKeyedCollection<CString, int>& mapDelPaths = pDoc->DeletedPaths;

			// preventing a crash caused by editing a file with Photos
			// that is in the list. Temp file are created and deleted
			// that causes an exception in CPhotoExplorerDoc::PopulateList
			// method. When those temp files are deleted, I am ignoring 
			// them to avoid the exception.
			bool bTemp = false;
			for ( auto& node : mapDelPaths.Items )
			{
				CString csExt = CHelper::GetExtension( node.first );
				csExt.MakeLower();
				if ( csExt == L".tmp" )
				{
					bTemp = true;
				}
			}

			if ( !bTemp )
			{
				pDoc->FindInterestingFiles( false );
			}

			mapDelPaths.clear();
			break;
		}
		case uipFolder :
		{
			KillTimer( uipFolder );
			CString csFolder =
				AfxGetApp()->GetProfileString( L"Settings", L"Folder" );
			if ( !csFolder.IsEmpty())
			{
				BOOL bExists = ::PathFileExists( csFolder );
				if ( !bExists )
				{
					csFolder = CHelper::GetWellKnownFolder( CSIDL_MYPICTURES );
					if ( csFolder.IsEmpty())
					{
						csFolder = CHelper::GetWellKnownFolder( CSIDL_DESKTOP );
					}
				}
				CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
				CNavigator* pNav = pMain->Navigator;
				pNav->CurrentPath = csFolder;
				pNav->SelectPath( csFolder );
				pDoc->Folder = csFolder;
			}
			break;
		}
	}

	CListView::OnTimer( nIDEvent );
} // OnTimer

/////////////////////////////////////////////////////////////////////////////
LRESULT CShellListView::OnUpdateThumbnail( WPARAM wParam, LPARAM lParam )
{
	// wait a half second for the thread to gather all of the new files
	CString* pImagePath = reinterpret_cast<CString*>( lParam );
	if ( pImagePath )
	{
		try
		{
			AddImageToList( *pImagePath );
			delete pImagePath;
			Invalidate();
			UpdateWindow();

		} catch (...)
		{
		}
	}

	return 0;
} // OnUpdateThumbnail

/////////////////////////////////////////////////////////////////////////////
LRESULT CShellListView::OnFolderDeleteEvent( WPARAM wParam, LPARAM lParam )
{
	SetTimer( uipDelayDel, 1000, nullptr );

	return LRESULT();
}

/////////////////////////////////////////////////////////////////////////////
LRESULT CShellListView::OnFolderAddEvent( WPARAM wParam, LPARAM lParam )
{
	SetTimer( uipDelayAdd, 1000, nullptr );

	return LRESULT();
}

/////////////////////////////////////////////////////////////////////////////
void CShellListView::PopulateList()
{
	ClearItems();
	SelectedImage = L"";

	CPhotoExplorerDoc* pDoc = GetDocument();

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COutputWnd* pOut = pFrame->OutputPane;
	pOut->ClearProgressOutput();

	for ( auto& node : pDoc->Thumbnails.Items )
	{
		const CString csPath = node.first;
		const CString csFile = CHelper::GetFileName( csPath );
		AddImageToList( csPath );
	}

	CString csMessage;
	csMessage.Format( _T( "Number of images: %d" ), pDoc->Thumbnails.Count );
	ProgressText = csMessage;

} // PopulateList


/////////////////////////////////////////////////////////////////////////////
// the collection of selected image paths
CKeyedCollection<CString, int>& CShellListView::GetSelectedImages()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
	return pDoc->SelectedImages;

} // GetSelectedImages

/////////////////////////////////////////////////////////////////////////////
// add a path to the selected images with the special
// case of an empty value which will clear the collection
void CShellListView::SetAddSelectedImage( CString value )
{
	CPhotoExplorerDoc* pDoc = Document;
	pDoc->AddSelectedImage = value;
	USHORT usDoc = pDoc->SelectedCount;
	USHORT usView = SelectedImageCount;
	if (usDoc == 1 && usView == 1)
	{
		SelectedImage = value;

	} else
	{
		SelectedImage = L"";
	}

} // SetAddSelectedImage

/////////////////////////////////////////////////////////////////////////////
void CShellListView::SetRemoveSelectedImage( CString value )
{
	CPhotoExplorerDoc* pDoc = Document;
	pDoc->RemoveSelectedImage = value;
	USHORT usDoc = pDoc->SelectedCount;
	USHORT usView = SelectedImageCount;
	if ( usDoc == 1 && usView == 1)
	{
		SelectedImage = pDoc->ImagePath;

	} else
	{
		SelectedImage = L"";
	}

} // SetRemoveSelectedImage

/////////////////////////////////////////////////////////////////////////////
void CShellListView::SelectItemByPath( const CString& path )
{
	// Get the list control
	CListCtrl& listCtrl = GetListCtrl();

	// Iterate through the items in the list control
	int itemCount = listCtrl.GetItemCount();
	for ( int i = 0; i < itemCount; ++i )
	{
		// Retrieve the file path associated with the item
		// Assuming the path is stored in the first column
		CString itemPath = listCtrl.GetItemText( i, 0 ); 

		// Compare the item path with the given path
		if ( itemPath.CompareNoCase( path ) == 0 )
		{
			// Select the item if the paths match
			listCtrl.SetItemState
			( 
				i, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED 
			);

			// Ensure the item is visible
			listCtrl.EnsureVisible( i, FALSE );

			// Stop the iteration since the item is found
			break;
		}
	}
} // SelectItemByPath

/////////////////////////////////////////////////////////////////////////////
CString CShellListView::GetSelectedImage()
{
	CPhotoExplorerDoc* pDoc = Document;
	CString value = pDoc->ImagePath;
	return value;

} // GetSelectedImage

/////////////////////////////////////////////////////////////////////////////
// the selected list control pathname
void CShellListView::SetSelectedImage( CString value )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPhotoExplorerDoc* pDoc = Document;
	CImageView* pImageView = pDoc->ImageView;
	CImageProperties* pProperties = ImageProperties;
	CString csSelectedImage = pProperties->Pathname;
	if ( !csSelectedImage.IsEmpty() && value != csSelectedImage )
	{
		pProperties->ApplyAlbumLabels();
		pProperties->SaveProperties();
	}
	
	// keep the original selected image
	if ( value == L"~~~")
	{
		value = csSelectedImage;

	} else // change the selected image
	{
		csSelectedImage = value;
		pDoc->ImagePath = value;
	}

	const CString csExt = CHelper::GetExtension( value );
	pProperties->Extension->FileExtension = csExt;

	pDoc->ImagePath = value;
	pProperties->Pathname = value;

	COutputWnd* pOut = pFrame->OutputPane;
	pOut->ClearWarningOutput();

	pProperties->ClearProperties();
	if (pImageView != nullptr)
	{
		pImageView->Invalidate();
	}

	if ( value.IsEmpty())
	{
		pFrame->PropertiesPane->Clear();
		return;
	}

	if ( pDoc->Thumbnails.Exists[ value ] )
	{
		Bitmap* pBitmap = pDoc->Thumbnails.find( value ).get();

		const UINT uiPropertyCount = pBitmap->GetPropertyCount();
		PROPID* propIDs = new PROPID[ uiPropertyCount ];
		pBitmap->GetPropertyIdList( uiPropertyCount, propIDs );

		// loop through the metadata properties of the thumbnail
		for ( UINT i = 0; i < uiPropertyCount; ++i )
		{
			UINT size = pBitmap->GetPropertyItemSize( propIDs[ i ] );
			Gdiplus::PropertyItem* pItem = (Gdiplus::PropertyItem*)malloc( size );
			pBitmap->GetPropertyItem( propIDs[ i ], size, pItem );

			// test for orientation item and if it exists, apply the rotation
			// to the thumbnail
			const ULONG ulId = pItem->id;
			if ( ulId == PropertyTagOrientation )
			{
				USHORT* pValue = reinterpret_cast<USHORT*>( pItem->value );
				CExifRotation* pRotator = pDoc->Rotator;
				pRotator->Rotation = (CExifRotation::ExifRotations)pValue[ 0 ];
			}
			// cache the property data
			ImageProperties->AddProperty( pItem );

			free( pItem );
		}

		// clean up
		delete[] propIDs;
	}

	CString csValue;
	{
		CString csKey( L"Label|1. Title" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			ImageProperties->CreateStringProperty( csKey, csValue );
		}
	}
	{
		CString csKey( L"Label|2. Location" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			ImageProperties->CreateStringProperty( csKey, csValue );
		}
	}
	{
		CString csKey( L"Label|3. Comment" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			ImageProperties->CreateStringProperty( csKey, csValue );
		}
	}
	{
		CString csKey( L"User|5. Keywords" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			ImageProperties->CreateStringProperty( csKey, csValue );
		}
	}
	{
		CString csKey( L"User|1. Filename" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			const CString csPathname = SelectedImage;
			CString csFilename = CHelper::GetDataName( csPathname );
			ImageProperties->CreateStringProperty( csKey, csFilename );
		}
	}
	{
		CString csKey( L"User|2. Artist" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			ImageProperties->CreateStringProperty( csKey, csValue );
		}
	}
	{
		CString csKey( L"User|3. Copyright" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			ImageProperties->CreateStringProperty( csKey, csValue );
		}
	}
	{
		CString csKey( L"Label|4. Date taken" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			ImageProperties->CreateStringProperty( csKey, csValue );
		}
	}
	{
		CString csKey( L"User|4. Software" );
		CString csValue( L"Photo Explorer" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			ImageProperties->CreateStringProperty( csKey, csValue );
		}
	}
	{
		CString csKey( L"Image|Orientation" );
		if ( false == ImageProperties->Properties->Exists[ csKey ] )
		{
			USHORT usValue( 1 );
			ImageProperties->CreateShortProperty( csKey, usValue );
		}
	}

	// output the metadata to the warnings output
	for ( auto& node : ImageProperties->Properties->Items )
	{
		CImageProperty* pProp = node.second.get();
		CString csCSV = pProp->CommaSeparatedValue;
		WarningsText = csCSV;
	}

	ImageProperties->UpdatePropertyPane();
	if (pImageView != nullptr)
	{
		pImageView->SetFocus();
	}

} // SetSelectedImage

/////////////////////////////////////////////////////////////////////////////
