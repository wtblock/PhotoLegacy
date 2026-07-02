/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "MainFrm.h"
#include "PhotoExplorer.h"
#include "PhotoExplorerDoc.h"
#include "PhotoExplorerView.h"
#include "ImageView.h"
#include "CNavigator.h"
#include "CHelper.h"
#include "PlusGDI.h"
#include "ThumbnailDialog.h"
#include "EditShortcutDialog.h"
#include <propkey.h>
#include <xmllite.h>
#include <shlwapi.h>

#pragma comment(lib, "xmllite.lib")

using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CPhotoExplorerDoc, CDocument)

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPhotoExplorerDoc, CDocument)
	ON_COMMAND( ID_ALBUM_TITLE, &CPhotoExplorerDoc::OnAlbumTitle )
	ON_UPDATE_COMMAND_UI( ID_ALBUM_TITLE, &CPhotoExplorerDoc::OnUpdateAlbumTitle )
	ON_COMMAND( ID_ALBUM_LOCATION, &CPhotoExplorerDoc::OnAlbumLocation )
	ON_UPDATE_COMMAND_UI( ID_ALBUM_LOCATION, &CPhotoExplorerDoc::OnUpdateAlbumLocation )
	ON_COMMAND( ID_ALBUM_COMMENT, &CPhotoExplorerDoc::OnAlbumComment )
	ON_UPDATE_COMMAND_UI( ID_ALBUM_COMMENT, &CPhotoExplorerDoc::OnUpdateAlbumComment )
	ON_COMMAND( ID_ALBUM_DATE, &CPhotoExplorerDoc::OnAlbumDate )
	ON_UPDATE_COMMAND_UI( ID_ALBUM_DATE, &CPhotoExplorerDoc::OnUpdateAlbumDate )
	ON_COMMAND( ID_ALBUM_ARTIST, &CPhotoExplorerDoc::OnAlbumArtist )
	ON_UPDATE_COMMAND_UI( ID_ALBUM_ARTIST, &CPhotoExplorerDoc::OnUpdateAlbumArtist )
	ON_COMMAND( ID_ALBUM_COPYRIGHT, &CPhotoExplorerDoc::OnAlbumCopyright )
	ON_UPDATE_COMMAND_UI( ID_ALBUM_COPYRIGHT, &CPhotoExplorerDoc::OnUpdateAlbumCopyright )
	ON_COMMAND( ID_ALBUM_SOFTWARE, &CPhotoExplorerDoc::OnAlbumSoftware )
	ON_UPDATE_COMMAND_UI( ID_ALBUM_SOFTWARE, &CPhotoExplorerDoc::OnUpdateAlbumSoftware )
	ON_COMMAND( ID_ALBUM_KEYWORDS, &CPhotoExplorerDoc::OnAlbumKeywords )
	ON_UPDATE_COMMAND_UI( ID_ALBUM_KEYWORDS, &CPhotoExplorerDoc::OnUpdateAlbumKeywords )
	ON_COMMAND( ID_EDIT_EXPORTLABELS, &CPhotoExplorerDoc::OnEditExportLabels )
	ON_UPDATE_COMMAND_UI( ID_EDIT_EXPORTLABELS, &CPhotoExplorerDoc::OnUpdateEditExportLabels )
    ON_COMMAND( ID_EDIT_SHORTCUTS, &CPhotoExplorerDoc::OnEditShortcuts )
	ON_UPDATE_COMMAND_UI( ID_EDIT_SHORTCUTS, &CPhotoExplorerDoc::OnUpdateEditShortcuts )
    ON_COMMAND( ID_EDIT_ALBUM_LABEL, &CPhotoExplorerDoc::OnEditAlbumLabel )
	ON_UPDATE_COMMAND_UI( ID_EDIT_ALBUM_LABEL, &CPhotoExplorerDoc::OnUpdateEditAlbumLabel )
	ON_COMMAND( ID_EDIT_RENAME, &CPhotoExplorerDoc::OnEditRename )
	ON_UPDATE_COMMAND_UI( ID_EDIT_RENAME, &CPhotoExplorerDoc::OnUpdateEditRename )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// parses lines of shortcut definitions:
//		"@key data"
// and maps the data for rapid searches
// and guaranteed unique keys
CString CPhotoExplorerDoc::MapShortcuts
(
	CString source,
	CKeyedCollection<CString, CString>& mapShortcuts
)
{
	CString value;
	mapShortcuts.clear();
	int nStart = 0;
	CString csToken = source.Tokenize( L"\n ", nStart );
	while ( !csToken.IsEmpty() )
	{
		CString csKey = csToken;
		csToken = source.Tokenize( L"\r\n", nStart );
		if ( !csToken.IsEmpty() )
		{
			if ( !mapShortcuts.Exists[ csKey ] )
			{
				mapShortcuts.add
				(
					csKey,
					shared_ptr<CString>( new CString( csToken ) )
				);
			}

			csToken = source.Tokenize( L"\n ", nStart );
		}
	}

	for ( auto& node : mapShortcuts.Items )
	{
		value += node.first;
		value += L" ";
		value += *node.second;
		value += L"\r\n";
	}

	return value;
}

/////////////////////////////////////////////////////////////////////////////
// shortcuts in the source string are expanded to 
// their real values and the resolved string is
// returned with all shortcuts replaced
CString CPhotoExplorerDoc::ResolveShortcuts( CString source )
{
	CString value = source;

	// if there are no at signs, then we are done
	if ( source.FindOneOf( L"@" ) == -1 )
	{
		return value;
	}

	// parse out the keys, i.e. text beginning with '@'
	// and collect the keys in this array
	vector<CString> arrKeys;
	const int nLen = source.GetLength();
	CString csKey;
	bool bKey = false;
	for ( int nIndex = 0; nIndex < nLen; nIndex++ )
	{
		WCHAR wChar = source[ nIndex ];
		if ( bKey )
		{
			if ( iswspace( wChar ) || iswpunct( wChar ) )
			{
				bKey = false;
				arrKeys.push_back( csKey );

			} else
			{
				csKey += wChar;
			}
		} else
		{
			if ( wChar == L'@' )
			{
				bKey = true;
				csKey = L"@";
			}
		}
	}

	// there can be a key at the end of the line
	if ( bKey )
	{
		arrKeys.push_back( csKey );
	}

	// replace keys with their shortcuts
	CString csShortcut;
	for ( auto& node : arrKeys )
	{
		if ( Shortcuts.Exists[ node ] )
		{
			csShortcut = Shortcut[ node ];
			value.Replace( node, csShortcut );
		}
	}

	return value;
} // ResolveShortcuts

/////////////////////////////////////////////////////////////////////////////
CPhotoExplorerDoc::CPhotoExplorerDoc() noexcept : 
	m_bStopThread( false ), m_pThread( nullptr ), m_hCompletionPort( NULL )
{
	m_hStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	// initialize registry data
	CWinApp* pApp = AfxGetApp();
	const CString csSec( L"Settings" );

	// short cut data
	CString csValue = AfxGetApp()->GetProfileString
	(
		csSec, L"Shortcuts"
	);

	// index the short cuts
	MapShortcuts( csValue, Shortcuts );

	AlbumTitle = 0 != pApp->GetProfileIntW( csSec, L"AlbumTitle", 0 );
	AlbumLocation = 0 != pApp->GetProfileIntW( csSec, L"AlbumLocation", 0 );
	AlbumComment = 0 != pApp->GetProfileIntW( csSec, L"AlbumComment", 0 );
	AlbumDate = 0 != pApp->GetProfileIntW( csSec, L"AlbumDate", 0 );
	AlbumArtist = 0 != pApp->GetProfileIntW( csSec, L"AlbumArtist", 0 );
	AlbumCopyright = 0 != pApp->GetProfileIntW( csSec, L"AlbumCopyright", 0 );
	AlbumSoftware = 0 != pApp->GetProfileIntW( csSec, L"AlbumSoftware", 0 );
	AlbumKeywords = 0 != pApp->GetProfileIntW( csSec, L"AlbumKeywords", 0 );

	m_nPendingOperation = 0;

} // CPhotoExplorerDoc

/////////////////////////////////////////////////////////////////////////////
CPhotoExplorerDoc::~CPhotoExplorerDoc()
{
	StopMonitorThread();
	::Sleep( 500 );
	CloseHandle( m_hStopEvent );
	if ( m_hCompletionPort )
	{
		CloseHandle( m_hCompletionPort );
	}
}

/////////////////////////////////////////////////////////////////////////////
CPhotoExplorerView* CPhotoExplorerDoc::GetPhotoExplorerView()
{
	CView* pView = nullptr;
	CPhotoExplorerView* value = nullptr;
	POSITION pos = GetFirstViewPosition();
	while ( pos )
	{
		pView = GetNextView( pos );
		if ( pView->IsKindOf( RUNTIME_CLASS( CPhotoExplorerView ) ) )
		{
			value = static_cast<CPhotoExplorerView*>( pView );
			break;
		}
	}
	return value;
} // GetPhotoExplorerView

/////////////////////////////////////////////////////////////////////////////
CImageView* CPhotoExplorerDoc::GetImageView()
{
	CView* pView = nullptr;
	CImageView* value = nullptr;
	POSITION pos = GetFirstViewPosition();
	while ( pos )
	{
		pView = GetNextView( pos );
		if ( pView->IsKindOf( RUNTIME_CLASS( CImageView ) ) )
		{
			value = static_cast<CImageView*>( pView );
			break;
		}
	}
	return value;
} // GetImageView

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::SetProgressText( CString value )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COutputWnd* pOut = pFrame->OutputPane;
	pOut->ProgressText = value;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::SetWarningsText( CString value )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COutputWnd* pOut = pFrame->OutputPane;
	pOut->WarningsText = value;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::SetErrorsText( CString value )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	COutputWnd* pOut = pFrame->OutputPane;
	pOut->ErrorsText = value;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoExplorerDoc::OnOpenDocument( LPCTSTR lpszPathName )
{
	BOOL value = FALSE;
	if ( Open( lpszPathName ))
	{
		value = true;
	}
	return value;
} // OnOpenDocument

/////////////////////////////////////////////////////////////////////////////
bool CPhotoExplorerDoc::Open
( 
	LPCTSTR szFilename, bool bRead, LPCTSTR pcszFileID 
)
{
	bool value = false;
	CString csPath( szFilename );
	if ( ::PathFileExists( csPath ))
	{
		CString csFolder = CHelper::GetFolder( csPath );
		CString csData = CHelper::GetDataName( csPath );
		SetFolder( csFolder, csData );
		value = true;
	}
	return value;
} // Open

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoExplorerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void CPhotoExplorerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPhotoExplorerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
UINT CPhotoExplorerDoc::GetThumbnailSize()
{
	UINT value = 128;
	CPhotoExplorerView* pView = PhotoExplorerView;
	if ( pView != nullptr )
	{
		value = pView->ThumbnailSize;
	}

	return value;
} // GetThumbnailSize

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::FindInterestingFiles( bool bStatus )
{
	// start trolling for files we are interested in
	m_ImageNames.clear();

	CString csCurrentPath = Folder;
	csCurrentPath.TrimRight( _T( "\\" ));
	CString csPath;
	csPath.Format( _T( "%s\\*.*" ), (LPCTSTR)csCurrentPath );
	if ( bStatus )
	{
		CString csProgress;
		csProgress.Format( _T( "Current path is: %s" ), (LPCTSTR)csCurrentPath );
		ProgressText = csCurrentPath;
	}

	CFileFind finder;
	BOOL bWorking = finder.FindFile( csPath );
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();

		// skip "." and ".." folder names
		if ( finder.IsDots() )
		{
			continue;
		}

		// if it's a directory, recursively search it
		if ( finder.IsDirectory() )
		{
			continue;

		} else if ( finder.GetLength() == 0 )
		{
			continue;

		} else // write the properties if it is a valid extension
		{
			// the pathname of the current file
			CString csPath = finder.GetFilePath();
			CString csFile = finder.GetFileName();
			CString csExt = CHelper::GetExtension( csFile );
			if ( CPlusGDI::IsImageFormatSaveSupported( csExt ))
			{
				shared_ptr<int> pInt = shared_ptr<int>( new int( 0 ));
				m_ImageNames.add( csFile, pInt );
			}
		}
	}

	finder.Close();

	if ( bStatus )
	{
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		COutputWnd* pOut = pFrame->OutputPane;
		pOut->ClearProgressOutput();
	}

	theApp.OnIdle( 0 );

	// Call the method to populate the list with thumbnails
	CShellListView* pView = PhotoExplorerView;

	// launch the progress dialog
	CThumbnailDialog dlg;
	dlg.Parent = pView;
	dlg.CreateDlg();
	dlg.ShowWindow( SW_SHOW );
	dlg.Invalidate();
	dlg.UpdateWindow();

	// do all of the hard work of creating thumbnails
	PopulateList( dlg );

	// done with the progress dialog
	dlg.DestroyWindow();

	pView->UpdateDocumentTabs();
} // FindInterestingFiles

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::UpdateTabText()
{
	CMainFrame* pFrame = static_cast<CMainFrame*>( AfxGetMainWnd() );
	if ( pFrame )
	{
		CMDIChildWnd* pMDIChild = pFrame->MDIGetActive();
		if ( pMDIChild )
		{
			CString csFolder = Folder;
			csFolder.TrimRight( L'\\' );
			CString csFolderName = CHelper::GetDataName( csFolder );
			CString csTitle = Title;
			if ( csTitle.IsEmpty())
			{
				csTitle = csFolderName;
			}

			pMDIChild->SetWindowText( csTitle );
		}
	}
} // UpdateTabText

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::UpdateDocumentTabs()
{
	// Get the main frame window
	CMainFrame* pFrame = static_cast<CMainFrame*>( AfxGetMainWnd() );
	if ( !pFrame )
		return;

	// Get the MDI client area window handle
	HWND hWndMDIClient = pFrame->m_hWndMDIClient;
	if ( !hWndMDIClient )
		return;

	// Get the first MDI child window
	HWND hWndChild = ::GetWindow( hWndMDIClient, GW_CHILD );

	// Iterate through all MDI child windows
	while ( hWndChild != NULL )
	{
		// Convert HWND to CWnd*
		CMDIChildWnd* pMDIChild = 
			DYNAMIC_DOWNCAST( CMDIChildWnd, CWnd::FromHandle( hWndChild ) );
		if ( pMDIChild != NULL )
		{
			CPhotoExplorerDoc* pDoc = 
				(CPhotoExplorerDoc*)pMDIChild->GetActiveDocument();
			if ( pDoc != NULL )
			{
				CString csTitle = pDoc->Title;
				if ( csTitle.IsEmpty() )
				{
					CString csFolder = Folder;
					csFolder.TrimRight( L'\\' );
					CString csFolderName = CHelper::GetDataName( csFolder );
					csTitle = csFolderName;
				}

				pMDIChild->SetWindowText( csTitle );
			}
		}

		// Move to the next MDI child window
		hWndChild = ::GetWindow( hWndChild, GW_HWNDNEXT );
	}
} // UpdateDocumentTabs

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::WriteAlbumToImage
(
	const CString& csKey, // property key
	CString& csValue, // value to write
	bool bPresent, // original value is present
	unique_ptr<Gdiplus::Image>& pImage
)
{
	// if the shift key is down, then overwrite existing values and not
	// just undefined values
	const bool bShift = CHelper::ShiftKeyDown();

	// if the value is already defined, leave it alone
	if ( bPresent )
	{
		return;
	}

	// the album value was empty, nothing to do unless the
	// user is holding down the shift key and wants to 
	// overwrite the value
	if ( !bShift && csValue.IsEmpty())
	{
		return;
	}

	CShellListView* pView = PhotoExplorerView;
	CImageProperties* pProperties = pView->ImageProperties;
	shared_ptr<CPropertyTag> pTag = pProperties->PropertyTag[ csKey ];

	// missing property?
	if ( pTag == nullptr )
	{
		return;
	}

	ULONG ulID = pTag->PropertyID;
	WORD wType = pTag->PropertyType;
	CStringA csValueA( csValue );
	void* pBuffer = nullptr;
	ULONG ulLength = 0;

	// ASCII vs. UNICODE
	if ( wType == PropertyTagTypeASCII )
	{
		ulLength = (ULONG)csValueA.GetLength() + 1;
		pBuffer = csValueA.GetBuffer();

	} else // UNICODE it is
	{
		wType = PropertyTagTypeByte;
		ulLength = 
			(ULONG)csValue.GetLength() * sizeof( WCHAR ) + sizeof( WCHAR );
		pBuffer = (void*)csValue.GetBuffer();
	}

	// create a GDI+ property item
	unique_ptr<CImageProperty::CPropertyItem>pItem =
		unique_ptr<CImageProperty::CPropertyItem>
		(
			new CImageProperty::CPropertyItem( ulID, wType, ulLength )
		);

	// copy the string into the proverty item value
	::CopyMemory( pItem->value, pBuffer, ulLength );

	// cast it to a pointer that will make the compiler happy
	Gdiplus::PropertyItem* pI =
		reinterpret_cast<Gdiplus::PropertyItem*>( pItem.get() );

	// finally our goal was to write the album value to the image
	pImage->SetPropertyItem( pI );

	// restor string buffer
	if ( wType == PropertyTagTypeASCII )
	{
		csValueA.ReleaseBuffer();

	} else
	{
		csValue.ReleaseBuffer();
	}

} // WriteAlbumToImage

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::WriteAlbumToBitmap
( 
	CString& csKey, 
	CString& csValue, bool bPresent, 
	unique_ptr<Gdiplus::Bitmap>& pBitmap 
)
{
	// if the value is already defined, leave it alone
	if ( bPresent )
	{
		return;
	}

	// the album value was empty, nothing to do
	if ( csValue.IsEmpty() )
	{
		return;
	}

	CShellListView* pView = PhotoExplorerView;
	CImageProperties* pProperties = pView->ImageProperties;
	shared_ptr<CPropertyTag> pTag = pProperties->PropertyTag[ csKey ];

	// missing property?
	if ( pTag == nullptr )
	{
		return;
	}

	ULONG ulID = pTag->PropertyID;
	WORD wType = pTag->PropertyType;
	CStringA csValueA( csValue );
	void* pBuffer = nullptr;
	ULONG ulLength = 0;

	// ASCII vs. UNICODE
	if ( wType == PropertyTagTypeASCII )
	{
		ulLength = (ULONG)csValueA.GetLength() + 1;
		pBuffer = csValueA.GetBuffer();

	} else // UNICODE it is
	{
		wType = PropertyTagTypeByte;
		ulLength =
			(ULONG)csValue.GetLength() * sizeof( WCHAR ) + sizeof( WCHAR );
		pBuffer = (void*)csValue.GetBuffer();
	}

	// create a GDI+ property item
	unique_ptr<CImageProperty::CPropertyItem>pItem =
		unique_ptr<CImageProperty::CPropertyItem>
		(
			new CImageProperty::CPropertyItem( ulID, wType, ulLength )
		);

	// copy the string into the property item value
	::CopyMemory( pItem->value, pBuffer, ulLength );

	// cast it to a pointer that will make the compiler happy
	Gdiplus::PropertyItem* pI =
		reinterpret_cast<Gdiplus::PropertyItem*>( pItem.get() );

	// finally our goal was to write the album value to the image
	Gdiplus::Status eStat = pBitmap->SetPropertyItem( pI );
	if ( eStat != Gdiplus::Ok )
	{
		CString csMessage;
		csMessage.Format
		( 
			L"FAiled to write %s: \n%s.", csKey, csValue 
		);
		ErrorsText = csMessage;
	}

	// restor string buffer
	if ( wType == PropertyTagTypeASCII )
	{
		csValueA.ReleaseBuffer();

	} else
	{
		csValue.ReleaseBuffer();
	}

} // WriteAlbumToBitmap

/////////////////////////////////////////////////////////////////////////////
vector<CString> CPhotoExplorerDoc::ReadImageDetails( CString csImage )
{
	vector<CString> value;
	CString csFolder = CHelper::GetFolder( csImage );
	CString csFile = CHelper::GetFileName( csImage );
	CString csPath = csFolder + csFile + L"_details.txt";
	vector<CString> arrLines = CHelper::ReadText( csPath );

	CString csComment, csLocation, csDate, csWith;
	vector<CString> arrComments;

	// any lines could be missing
	for ( auto& csLine : arrLines )
	{
		if ( csDate.IsEmpty())
		{
			CString csTemp = CHelper::FormatDate( csLine );
			if ( !csTemp.IsEmpty())
			{
				csDate = csTemp;

			} else
			{
				csComment += csLine;
				csComment += L"\r\n";
			}
		} else
		{
			if ( L"With " == csLine.Left( 5 ))
			{
				csWith = csLine;

			} else
			{
				csLocation = csLine;
			}
		}
	}

	if ( !csWith.IsEmpty())
	{
		csComment += csWith;

	} else
	{
		csComment.TrimRight( L"\r\n" );
	}
	value.push_back( csComment );
	value.push_back( csDate );
	value.push_back( csLocation );

	return value;
} // ReadImageDetails

/////////////////////////////////////////////////////////////////////////////
vector<CString> CPhotoExplorerDoc::ParseAlbumDetails( CString csFolder )
{
	vector<CString> value;
	CString csDetails = csFolder + L"details.txt";
	vector<CString> arrLines = CHelper::ReadText( csDetails );

	CString csTitle, csLocation, csDate;
	vector<CString> arrComments;

	for ( auto& csLine : arrLines )
	{
		// Photo Explorer writes this to record the album name
		// prior to renaming the album
		if ( csLine.Left( 5 ) == L"Album" )
		{
			continue;
		}

		// the last line is the location
		if ( csTitle.IsEmpty())
		{
			csTitle = csLine;
			value.push_back( csTitle );

		} else if ( csDate.IsEmpty())
		{
			if ( csLine != L"Date unspecified" )
			{
				csDate = CHelper::FormatDate( csLine );
				value.push_back( csDate );
			}
		} else
		{
			csLocation = csLine;
			value.push_back( csLocation );
		}
	}

	return value;
} // ParseAlbumDetails

/////////////////////////////////////////////////////////////////////////////
CString CPhotoExplorerDoc::GetRealPath( const CString& input )
{
	CString value;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CNavigator* pNav = pFrame->Navigator;
	value = pNav->GetRealFolder( input );
	return value;
} // GetRealPath

/////////////////////////////////////////////////////////////////////////////
bool CPhotoExplorerDoc::FindAlbum()
{
	bool value = false;
	CPhotoExplorerView* ListView = PhotoExplorerView;
	// look for an album file
	CFileFind finder;
	CString csFolder = Folder;
	CString csWildcard = csFolder;
	csWildcard.TrimRight( L"\\" );
	csWildcard += L"\\*.album";

	BOOL bWorking = finder.FindFile( csWildcard );
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();

		// skip "." and ".." folder names
		if ( finder.IsDots() )
		{
			continue;
		}

		// if it's a directory, recursively search it
		if ( finder.IsDirectory() )
		{
			continue;

		} else // open the album 
		{
			const CString csPath = finder.GetFilePath();
			const CString csExt = CHelper::GetExtension( csPath ).MakeLower();
			const CString csFile = CHelper::GetFileName( csPath );

			if ( ReadAlbumXml( csPath ) )
			{
				AlbumPath = csPath;
				ListView->ImageProperties->UpdatePropertyPane();
				value = true;
				break;
			}
		}
	}

	finder.Close();

	if ( value == false )
	{
		vector<CString> arrLines = ParseAlbumDetails( csFolder );
		if ( !arrLines.empty())
		{
			int nLine = 1;
			for ( auto& node : arrLines )
			{
				switch ( nLine )
				{
					case 1 : Title = node; break;
					case 2 : Date = node; break;
					case 3 : Location = node;
				}
				nLine++;
			}
			CString csAlbum = csFolder + Title + L".album";
			if ( WriteAlbumXml( csAlbum ))
			{
				AlbumPath = csAlbum;
				ListView->ImageProperties->UpdatePropertyPane();
				value = true;
			}
		}
	}
	return value;
} // FindAlbum

/////////////////////////////////////////////////////////////////////////////
int CPhotoExplorerDoc::FindAlbumFolders()
{
	int value = 0;
	AlbumFolders.clear();

	CFileFind finder;
	CString csFolder = Folder;
	CString csWildcard = csFolder;
	csWildcard += L"album*";

	BOOL bWorking = finder.FindFile( csWildcard );
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();

		// skip "." and ".." folder names
		if ( finder.IsDots() )
		{
			continue;
		}

		// if it's a directory, recursively search it
		if ( finder.IsDirectory() )
		{
			CString csFolder = finder.GetFilePath();
			CString csDetails = csFolder + L"\\details.txt";
			if ( ::PathFileExists( csDetails ))
			{
				CString csTitle, csDate, csLocation;
				vector<CString> arrLines = CHelper::ReadText( csDetails );
				if ( !arrLines.empty())
				{
					int nLine = 1;
					for ( auto& node : arrLines )
					{
						switch ( nLine )
						{
							case 1: csTitle = node; break;
							case 2: csDate = CHelper::FormatDate( node ); break;
							case 3: csLocation = node;
						}
						nLine++;
					}
				}

				// if there is not data separated by dots
				if ( -1 == csTitle.FindOneOf( L"." ))
				{
					CString csPreface;

					// test for a single number that could be the year
					int nStart = 0;
					CString csToken = csTitle.Tokenize( L"- ", nStart );
					if ( !csToken.IsEmpty() )
					{
						const long lLong = _tstol( csToken );
						if ( lLong >= 1000 )
						{
							csPreface = csToken;
						}
					}

					// no leading year
					if ( csPreface.IsEmpty())
					{
						vector<CString> arrDates = CHelper::Words( csDate, L": " );
						int nDate = 0;
						for ( auto& node : arrDates )
						{
							if ( nDate > 2 )
							{
								break;
							}
							csPreface += node + L".";
							nDate++;
						}

						csPreface.TrimRight( L"." );
						csPreface += L" ";
						csPreface += csTitle;
						csTitle = csPreface;
						csTitle.Trim();
					}
				}

				// pathnames are restricted for containing some characters
				csTitle = CHelper::ReplaceReservedCharacters( csTitle );

				AlbumFolders.add( csFolder, shared_ptr<CString>( new CString( csTitle ) ) );
			}
		}
	}

	value = AlbumFolders.Count;

	FindTwoDateFolders();
	FindEndDateFolders();
	FindNoDateFolders();

	return value;
} // FindAlbumFolders

/////////////////////////////////////////////////////////////////////////////
int CPhotoExplorerDoc::FindNoDateFolders()
{
	int value = 0;
	NoDateFolders.clear();

	CFileFind finder;
	CString csFolder = Folder;
	CString csWildcard = csFolder;
	csWildcard += L"*.*";

	BOOL bWorking = finder.FindFile( csWildcard );
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();

		// skip "." and ".." folder names
		if ( finder.IsDots() )
		{
			continue;
		}

		// if it's a directory, recursively search it
		if ( finder.IsDirectory() )
		{
			CString csFolder = finder.GetFilePath();
			if ( AlbumFolders.Exists[ csFolder ] )
			{
				continue;
			}

			if ( TwoDateFolders.Exists[ csFolder ] )
			{
				continue;
			}

			if ( EndDateFolders.Exists[ csFolder ] )
			{
				continue;
			}

			CString csData = CHelper::GetDataName( csFolder );
			CString csPath = CHelper::GetFolder( csFolder );

			vector<CString> arrWords = CHelper::Words( csData );
			const int nSize = (int)arrWords.size();
			if ( nSize == 0 )
			{
				continue;
			}
			CString csLeading = arrWords[ 0 ];
			const int nLeading = _tstoi( csLeading );
			if ( nLeading > 1000 )
			{
				continue;
			}
			CString csDetails = csFolder + L"\\details.txt";
			if ( ::PathFileExists( csDetails ))
			{
				CString csTitle, csDate, csLocation;
				vector<CString> arrLines = CHelper::ReadText( csDetails );
				if ( !arrLines.empty())
				{
					int nLine = 1;
					for ( auto& node : arrLines )
					{
						switch ( nLine )
						{
							case 1: csTitle = node; break;
							case 2: csDate = CHelper::FormatDate( node ); break;
							case 3: csLocation = node;
						}
						nLine++;
					}
				}

				if ( csDate.IsEmpty())
				{
					continue;
				}

				CString csNew;
				vector<CString> arrDates = CHelper::Words( csDate, L": " );
				int nDate = 0;
				for ( auto& node : arrDates )
				{ 
					if ( nDate > 2 )
					{
						break;
					}
					csNew += node + L".";
					nDate++;
				}

				csNew.TrimRight( L"." );
				csNew += L" ";
				csNew += csData;

				// pathnames are restricted for containing some characters
				csNew = CHelper::ReplaceReservedCharacters( csNew );

				csPath += csNew;
				csNew = csPath;

				NoDateFolders.add( csFolder, shared_ptr<CString>( new CString( csNew ) ) );
			}
		}
	}

	value = NoDateFolders.Count;

	return value;
} // FindNoDateFolders

/////////////////////////////////////////////////////////////////////////////
int CPhotoExplorerDoc::FindTwoDateFolders()
{
	int value = 0;
	TwoDateFolders.clear();

	CFileFind finder;
	CString csFolder = Folder;
	CString csWildcard = csFolder;
	csWildcard += L"*.*";

	BOOL bWorking = finder.FindFile( csWildcard );
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();

		// skip "." and ".." folder names
		if ( finder.IsDots() )
		{
			continue;
		}

		// if it's a directory, recursively search it
		if ( finder.IsDirectory() )
		{
			CString csFolder = finder.GetFilePath();
			CString csData = CHelper::GetDataName( csFolder );
			CString csPath = CHelper::GetFolder( csFolder );
			int nStart = 0;
			int nLeft = 0;
			int nLen = csFolder.GetLength();
			CString csFirst = csData.Tokenize( L" ", nStart );
			if ( !csFirst.IsEmpty())
			{
				nLeft = nStart;
				CString csSecond = csData.Tokenize( L"- ", nStart );

				// is the first token is a date?
				if ( -1 != csFirst.FindOneOf( L"." ) )
				{
					const long lValue = _tstol( csSecond );
					if ( lValue > 1000 )
					{
						CString csNew = csData.Mid( nLeft );
						TwoDateFolders.add
						( 
							csFolder, shared_ptr<CString>( new CString( csPath + csNew ))
						);
					}
				}

			}
		}
	}

	value = TwoDateFolders.Count;

	return value;
} // FindTwoDateFolders

/////////////////////////////////////////////////////////////////////////////
int CPhotoExplorerDoc::FindEndDateFolders()
{
	int value = 0;
	EndDateFolders.clear();

	CFileFind finder;
	CString csFolder = Folder;
	CString csWildcard = csFolder;
	csWildcard += L"*.*";

	BOOL bWorking = finder.FindFile( csWildcard );
	while ( bWorking )
	{
		bWorking = finder.FindNextFile();

		// skip "." and ".." folder names
		if ( finder.IsDots() )
		{
			continue;
		}

		// if it's a directory, recursively search it
		if ( finder.IsDirectory() )
		{
			CString csFolder = finder.GetFilePath();
			CString csData = CHelper::GetDataName( csFolder );
			CString csPath = CHelper::GetFolder( csFolder );
			CString csEndDate = CHelper::EndDate( csData );
			if ( csEndDate.IsEmpty())
			{
				continue;
			}
			vector<CString> arrWords = CHelper::Words( csData );
			int nWords = (int)arrWords.size();
			if ( nWords > 0 )
			{
				// if the leading value of the line is numeric
				// then continue with the next folder
				int nValue = (int)_tstoi( arrWords[ 0 ] );
				if ( nValue != 0 )
				{
					continue;
				}
			}
			vector<CString> arrDate = CHelper::Words( csEndDate, L"." );
			int nDateValues = (int)arrDate.size();
			arrWords.resize( size_t( nWords - nDateValues ));
			CString csNew( csEndDate + L" " );
			for ( auto& node : arrWords )
			{
				csNew += node + L" ";
			}
			csNew.TrimRight();
			EndDateFolders.add
			(
				csFolder, shared_ptr<CString>( new CString( csPath + csNew ) )
			);
		}
	}

	value = EndDateFolders.Count;

	return value;
} // FindEndDateFolders

/////////////////////////////////////////////////////////////////////////////
// the selected path
void CPhotoExplorerDoc::SetFolder( LPCTSTR value, LPCTSTR filename )
{
	// switching to a new folder resets the selection
	// which is accomplished by adding a blank pathname
	AddSelectedImage = L"";

	CString csSymbol( value );
	CString csValue = GetRealPath( csSymbol );

	if ( csValue.IsEmpty())
	{
		m_csFolder = csValue;
		return;
	}

	StopMonitorThread();

	if ( csValue != m_csFolder )
	{
		m_csFolder = csValue;
		AfxGetApp()->WriteProfileString( L"Settings", L"Folder", csValue );
		const int nAlbums = FindAlbumFolders();

		CMainFrame* pMain = (CMainFrame*)AfxGetMainWnd();
		pMain->Folder = csValue;

		CPhotoExplorerView* ListView = PhotoExplorerView;
		CImageView* pImageView = ImageView;
		CPropertiesWnd* pProperties = pMain->PropertiesPane;

		AlbumPath = L"";
		Title = L"";
		Location = L"";
		Comment = L"";
		Date = L"";

		Artist = L"";
		Copyright = L"";
		Software = L"";
		Keywords = L"";

		if ( ListView != nullptr )
		{
			ListView->ClearCache();
			ListView->ImageProperties->ClearProperties();
			ListView->SelectedImage = L"";
		}

		if ( pProperties != nullptr )
		{
			pProperties->Clear();
			if ( ListView != nullptr )
			{
				ListView->ImageProperties->UpdatePropertyPane();
			}
		}

		if ( filename == nullptr )
		{
			const bool bAlbum = FindAlbum();

		} else
		{
			const CString csPath = csValue + filename;
			if ( ReadAlbumXml( csPath ) )
			{
				AlbumPath = csPath;
				ListView->ImageProperties->UpdatePropertyPane();
			}
		}

		ImagePath = L"";

		UpdateDocumentTabs();
		FindInterestingFiles();
	}

	StartMonitorThread();
} // SetFolder

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::SetImagePath( LPCTSTR value )
{
	CString csCurrent = ImagePath;
	if ( value != csCurrent )
	{
		// clear the multi-selection
		AddSelectedImage = L"";

		// add the single selection
		AddSelectedImage = value;

		CImageView* pImageView = ImageView;
		pImageView->Invalidate();
	}

} // SetImagePath

/////////////////////////////////////////////////////////////////////////////
// Save the data inside pImage to the given filename but relocated to the 
// sub-folder "Details"
bool CPhotoExplorerDoc::SaveWithDetails
( 
	CString& csPath, Gdiplus::Image* pImage 
)
{
	// writing to the same file will fail, so save to a corrected folder
	// below the image being corrected
	const CString csDetails( L"Details" );
	const CString csFolder = CHelper::GetFolder( csPath ) + csDetails;
	if ( !::PathFileExists( csFolder ) )
	{
		if ( !CHelper::CreatePath( csFolder ) )
		{
			return false;
		}
	}

	// filename plus extension
	const CString csData = CHelper::GetDataName( csPath );

	// create a new path from the pieces
	const CString csNewPath = csFolder + _T( "\\" ) + csData;

	// use the extension member class to get the class ID of the file
	CPhotoExplorerView* pView = PhotoExplorerView;
	
	CImageProperties* pProperties = pView->ImageProperties;
	CString csExt = CHelper::GetExtension( csPath );
	pProperties->Extension->FileExtension = csExt;
	CLSID classID = pProperties->Extension->ClassID;

	// save the image to the corrected folder
	Status status = pImage->Save( csNewPath, &classID );

	// return true if the save worked
	return status == Ok;
} // SaveWithDetails

/////////////////////////////////////////////////////////////////////////////
// write information from image detail text files
unique_ptr<Gdiplus::Image> CPhotoExplorerDoc::WriteImageDetails
(
	CString& csImage
)
{
	vector<CString> arrDetails = ReadImageDetails( csImage );
	CString csComment, csDate, csLocation;
	if ( 3 == arrDetails.size())
	{
		csComment = arrDetails[ 0 ];
		csDate = arrDetails[ 1 ];
		csLocation = arrDetails[ 2 ];
	}

	// replace empty values with album data if enabled
	// to do so.
	if ( csDate.IsEmpty() && AlbumDate )
	{
		csDate = Date;
	}
	if ( csLocation.IsEmpty() && AlbumLocation )
	{
		csLocation = Location;
	}
	CString csTitle = AlbumTitle ? Title : L"";

	CPhotoExplorerView* pView = PhotoExplorerView;
	CImageProperties* pProperties = pView->ImageProperties;
	CString csPath = pProperties->Pathname;
	if ( csPath.IsEmpty())
	{
		pProperties->Pathname = csImage;
		CString csExt = CHelper::GetExtension( csImage );
		pProperties->Extension->FileExtension = csExt;
	}

	CString csTitleKey = L"Label|1. Title";
	CString csLocationKey = L"Label|2. Location";
	CString csCommentKey = L"Label|3. Comment";
	CString csDateKey = L"Label|4. Date taken";

	shared_ptr<CPropertyTag> pTitleTag = pProperties->PropertyTag[ csTitleKey ];
	shared_ptr<CPropertyTag> pLocationTag = pProperties->PropertyTag[ csLocationKey ];
	shared_ptr<CPropertyTag> pCommentTag = pProperties->PropertyTag[ csCommentKey ];
	shared_ptr<CPropertyTag> pDateTag = pProperties->PropertyTag[ csDateKey ];

	ULONG ulTitleID = pTitleTag->PropertyID;
	ULONG ulLocationID = pLocationTag->PropertyID;
	ULONG ulCommentID = pCommentTag->PropertyID;
	ULONG ulDateID = pDateTag->PropertyID;

	bool bTitle = csTitle.IsEmpty();
	bool bLocation = csLocation.IsEmpty();
	bool bComment = csComment.IsEmpty();;
	bool bDate = csDate.IsEmpty();

	CString csExt = CHelper::GetExtension( csImage );
	pProperties->Extension->FileExtension = csExt;

	// context block to allow the pImage to go out of context
	{
		//// Load the image using GDI+
		//unique_ptr<Gdiplus::Bitmap>
		//	pImage( Gdiplus::Bitmap::FromFile( (LPCTSTR)csImage ) );
		CString csError;
		unique_ptr<Bitmap> pImage =
			CHelper::LoadBitmapFromFile( csImage, csError );
		if ( pImage )
		{
			// Preserve all metadata
			const UINT uiPropertyCount = pImage->GetPropertyCount();
			PROPID* propIDs = new PROPID[ uiPropertyCount ];
			pImage->GetPropertyIdList( uiPropertyCount, propIDs );

			// loop through the metadata properties of the original image and 
			// copy them to the new image
			for ( UINT i = 0; i < uiPropertyCount; ++i )
			{
				UINT size = pImage->GetPropertyItemSize( propIDs[ i ] );
				Gdiplus::PropertyItem* pItem = (Gdiplus::PropertyItem*)malloc( size );
				pImage->GetPropertyItem( propIDs[ i ], size, pItem );

				// test for orientation item and if it exists, apply the rotation
				// to the thumbnail
				const ULONG ulId = pItem->id;
				if ( ulId == ulTitleID )
				{
					CStringA strA( (char*)pItem->value );
					strA.Trim();
					bTitle = !strA.IsEmpty();

				} else if ( ulId == ulLocationID )
				{
					bLocation = true;

				} else if ( ulId == ulCommentID )
				{
					bComment = true;

				} else if ( ulId == ulDateID )
				{
					bDate = true;
				}

				free( pItem );
			}

			WriteAlbumToBitmap( csTitleKey, csTitle, bTitle, pImage );
			WriteAlbumToBitmap( csLocationKey, csLocation, bLocation, pImage );
			WriteAlbumToBitmap( csCommentKey, csComment, bComment, pImage );
			WriteAlbumToBitmap( csDateKey, csDate, bDate, pImage );

			const UINT uiCount = pImage->GetPropertyCount();

			// clean up
			delete[] propIDs;

			// save changes if any
			if ( !bTitle || !bLocation || !bComment || !bDate )
			{
				CLSID classID = pProperties->Extension->ClassID;
				Status status = pImage->Save( csImage, &classID );
				if ( status != Ok )
				{
					CString csMessage;
					csMessage.Format( L"Failed to save: %s", csImage );
					ErrorsText = csMessage;
				}
			}
		}
	}

	// reopen the file after updating its properties from the image details
	unique_ptr<Gdiplus::Bitmap> 
		pBitmap( Gdiplus::Bitmap::FromFile( (LPCTSTR)csImage ) );

	return pBitmap;

} // WriteImageDetails

/////////////////////////////////////////////////////////////////////////////
// create a thumbnail from given path to an image
bool CPhotoExplorerDoc::CreateThumbnail
(
	CString& imagePath, shared_ptr<Gdiplus::Bitmap>& pThumbnail
)
{
	bool value = false;

	// if there are detail text files in the folder, per Photomyne,
	// then automatically incorporate that data into the image
	unique_ptr<Gdiplus::Image> pImage = WriteImageDetails( imagePath );

	// cast to a bitmap
	unique_ptr<Bitmap> pBitmap( dynamic_cast<Bitmap*>( pImage.release() ) );
	if ( pBitmap )
	{
		// calculate the aspect ratio of the image
		const UINT uiWidth = pBitmap->GetWidth();
		const UINT uiHeight = pBitmap->GetHeight();
		const int nSize = ThumbnailSize;
		const float fAspect =
			CHelper::GetAspectRatio( uiWidth, uiHeight );

		// scale the new image to the thumbnail size and calculate
		// the new left and top corner of the image in the thumbnail
		int nWidth = nSize;
		int nHeight = nSize;
		int nTop = 0;
		int nLeft = 0;

		// aspect is width divided by height so an aspect greater
		// than one is landscape which will change the height and top
		if ( fAspect > 1.0f )
		{
			nHeight = int( float( nHeight ) / fAspect );
			nTop = ( nSize - nHeight ) / 2;

		} else // portrait change width and left
		{
			nWidth = int( float( nWidth ) * fAspect );
			nLeft = ( nSize - nWidth ) / 2;
		}

#ifdef _DEBUG
#undef new
#endif
		// Create a thumbnail the same size as the image list size
		pThumbnail = shared_ptr<Bitmap>
		(
			new Bitmap( nSize, nSize, pBitmap->GetPixelFormat() )
		);
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

		// create a Gdiplus::Graphics class from the thumbnail
		Gdiplus::Graphics graphics( pThumbnail.get() );

		// initialize the thumbnail to white
		graphics.Clear( Gdiplus::Color( 255, 255, 255 ) );

		// draw the original image onto the thumbnail
		graphics.DrawImage( pBitmap.get(), nLeft, nTop, nWidth, nHeight );

		// Preserve all metadata
		const UINT uiPropertyCount = pBitmap->GetPropertyCount();
		PROPID* propIDs = new PROPID[ uiPropertyCount ];
		pBitmap->GetPropertyIdList( uiPropertyCount, propIDs );

		// loop through the metadata properties of the original image and 
		// copy them to the new image
		for ( UINT i = 0; i < uiPropertyCount; ++i )
		{
			UINT size = pBitmap->GetPropertyItemSize( propIDs[ i ] );
			Gdiplus::PropertyItem* pItem = (Gdiplus::PropertyItem*)malloc( size );
			pBitmap->GetPropertyItem( propIDs[ i ], size, pItem );
			Bitmap* pThumb = pThumbnail.get();

			// test for orientation item and if it exists, apply the rotation
			// to the thumbnail
			const ULONG ulId = pItem->id;
			switch ( ulId )
			{
				case PropertyTagOrientation:
				{
					USHORT* pValue = reinterpret_cast<USHORT*>( pItem->value );
					CExifRotation rotator;
					rotator.Rotation = (CExifRotation::ExifRotations)pValue[ 0 ];
					rotator.ApplyOrientation( pThumb );
					break;
				}
				default:
				{
				}
			}

			pThumb->SetPropertyItem( pItem );
			free( pItem );
		}

		const UINT uiCount = pThumbnail->GetPropertyCount();

		// clean up
		delete[] propIDs;

		value = true;
	}

	return value;
} // CreateThumbnail

/////////////////////////////////////////////////////////////////////////////
// populate the list with thumbnail images
void CPhotoExplorerDoc::PopulateList( CThumbnailDialog& dlg )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CNavigator* pNav = pFrame->Navigator;
	CPhotoExplorerView* pView = PhotoExplorerView;
	if ( pView == nullptr )
	{
		return;
	}
	HWND hWnd = pView->GetSafeHwnd();

	pView->ClearItems();

	CString csFolder = Folder;
	csFolder.TrimRight( _T( "\\" ) );

	const int nImages = m_ImageNames.Count;
	if ( nImages == 0 )
	{
		return;
	}

	dlg.TotalImages = nImages;

	int nImage = 0;

	// prevent the user from closing the application or
	// changing folders while this is running
	PendingOperation = true;

	// gray out the navigation pane to disable it
	pNav->EnableWindow( FALSE );

	// loop through all of the image name we found earlier
	for ( auto& node : m_ImageNames.Items )
	{
		// let the user cancel out
		if ( dlg.Cancel )
		{
			break;
		}

		// update the progress dialog's status
		dlg.CurrentImage = nImage++;

		// build a full pathname from the folder and the 
		// current image name
		const CString csFile = node.first;
		CString csPath;
		csPath.Format
		(
			_T( "%s\\%s" ), (LPCTSTR)csFolder, (LPCTSTR)csFile
		);

		// use the existing thumbnail if available
		shared_ptr<Bitmap> pThumbnail;
		if ( m_Thumbnails.Exists[ csPath ] )
		{
			pThumbnail = m_Thumbnails.find( csPath );

		} else // create a thumbnail
		{
			CreateThumbnail( csPath, pThumbnail );
			m_Thumbnails.add( csPath, pThumbnail );
		}
		const UINT uiCount = pThumbnail->GetPropertyCount();

		// tell the list view to display the thumbnail 
		::PostMessage
		(
			hWnd, WM_UPDATE_THUMBNAIL, 0,
			reinterpret_cast<LPARAM>( new CString( csPath ) )
		);

		// wait ten milliseconds while letting normal 
		// window messaging to run
		pFrame->Wait( 10 );
	}

	// reenable the navigation panel
	pNav->EnableWindow();

	// remove restrictions
	PendingOperation = false;

} // PopulateList

/////////////////////////////////////////////////////////////////////////////
bool CPhotoExplorerDoc::WriteAlbumXml( const CString& filePath )
{
	bool value = false;
	IStream* pFileStream = nullptr;
	HRESULT hr = SHCreateStreamOnFileW
	( 
		filePath, STGM_CREATE | STGM_WRITE, &pFileStream 
	);

	if ( FAILED( hr ) )
	{
		ErrorsText = L"Failed to create file stream.";
		return value;
	}

	IXmlWriter* pWriter = nullptr;
	hr = CreateXmlWriter
	( 
		__uuidof( IXmlWriter ), reinterpret_cast<void**>( &pWriter ), nullptr 
	);
	if ( FAILED( hr ) )
	{
		ErrorsText = L"Failed to create XML writer.";
		pFileStream->Release();
		return value;
	}

	hr = pWriter->SetOutput( pFileStream );
	if ( FAILED( hr ) )
	{
		ErrorsText = L"Failed to set output for XML writer.";
		pWriter->Release();
		pFileStream->Release();
		return value;
	}

	// ⭐ Enable pretty-print formatting
	pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);

	value = true;
	pWriter->WriteStartDocument( XmlStandalone_Yes );

	pWriter->WriteStartElement( nullptr, L"Album", nullptr );

	pWriter->WriteStartElement( nullptr, L"Title", nullptr );
	pWriter->WriteAttributeString( nullptr, L"value", nullptr, Title );
	pWriter->WriteAttributeString( nullptr, L"type", nullptr, L"string" );
	pWriter->WriteEndElement();

	pWriter->WriteStartElement( nullptr, L"Comment", nullptr );
	pWriter->WriteAttributeString( nullptr, L"value", nullptr, Comment );
	pWriter->WriteAttributeString( nullptr, L"type", nullptr, L"string" );
	pWriter->WriteEndElement();

	pWriter->WriteStartElement( nullptr, L"Location", nullptr );
	pWriter->WriteAttributeString( nullptr, L"value", nullptr, Location );
	pWriter->WriteAttributeString( nullptr, L"type", nullptr, L"string" );
	pWriter->WriteEndElement();

	pWriter->WriteStartElement( nullptr, L"Date", nullptr );
	pWriter->WriteAttributeString( nullptr, L"value", nullptr, Date );
	pWriter->WriteAttributeString( nullptr, L"type", nullptr, L"string" );
	pWriter->WriteEndElement();

	pWriter->WriteStartElement( nullptr, L"Artist", nullptr );
	pWriter->WriteAttributeString( nullptr, L"value", nullptr, Artist );
	pWriter->WriteAttributeString( nullptr, L"type", nullptr, L"string" );
	pWriter->WriteEndElement();

	pWriter->WriteStartElement( nullptr, L"Copyright", nullptr );
	pWriter->WriteAttributeString( nullptr, L"value", nullptr, Copyright );
	pWriter->WriteAttributeString( nullptr, L"type", nullptr, L"string" );
	pWriter->WriteEndElement();

	pWriter->WriteStartElement( nullptr, L"Software", nullptr );
	pWriter->WriteAttributeString( nullptr, L"value", nullptr, Software );
	pWriter->WriteAttributeString( nullptr, L"type", nullptr, L"string" );
	pWriter->WriteEndElement();

	pWriter->WriteStartElement( nullptr, L"Keywords", nullptr );
	pWriter->WriteAttributeString( nullptr, L"value", nullptr, Keywords );
	pWriter->WriteAttributeString( nullptr, L"type", nullptr, L"string" );
	pWriter->WriteEndElement();

	pWriter->WriteEndElement();
	pWriter->WriteEndDocument();

	pWriter->Release();
	pFileStream->Release();

	return value;
} // WriteAlbumXml

/////////////////////////////////////////////////////////////////////////////
bool CPhotoExplorerDoc::ReadAlbumXml( const CString& filePath )
{
	bool value = false;

	IStream* pFileStream = nullptr;
	HRESULT hr = SHCreateStreamOnFileW( filePath, STGM_READ, &pFileStream );
	if ( FAILED( hr ) )
	{
		ErrorsText = L"Failed to create file stream.";
		return value;
	}

	IXmlReader* pReader = nullptr;
	hr = CreateXmlReader
	( 
		__uuidof( IXmlReader ), reinterpret_cast<void**>( &pReader ), nullptr 
	);
	if ( FAILED( hr ) )
	{
		ErrorsText = L"Failed to create XML reader.";
		pFileStream->Release();
		return value;
	}

	hr = pReader->SetInput( pFileStream );
	if ( FAILED( hr ) )
	{
		ErrorsText =  L"Failed to set input for XML reader.";
		pReader->Release();
		pFileStream->Release();
		return value;
	}

	XmlNodeType nodeType;
	while ( S_OK == pReader->Read( &nodeType ) )
	{
		if ( nodeType == XmlNodeType_Element )
		{
			const WCHAR* pwszLocalName = nullptr;
			pReader->GetLocalName( &pwszLocalName, nullptr );

			if 
			( 
				wcscmp( pwszLocalName, L"Title" ) == 0 || 
				wcscmp( pwszLocalName, L"Comment" ) == 0 ||
				wcscmp( pwszLocalName, L"Location" ) == 0 || 
				wcscmp( pwszLocalName, L"Date" ) == 0 ||
				wcscmp( pwszLocalName, L"Artist" ) == 0 || 
				wcscmp( pwszLocalName, L"Copyright" ) == 0 ||
				wcscmp( pwszLocalName, L"Software" ) == 0 || 
				wcscmp( pwszLocalName, L"Keywords" ) == 0 
			)
			{
				if ( pReader->MoveToFirstAttribute() == S_OK )
				{
					do
					{
						const WCHAR* pwszAttrName = nullptr;
						const WCHAR* pwszValue = nullptr;

						pReader->GetLocalName( &pwszAttrName, nullptr );
						pReader->GetValue( &pwszValue, nullptr );

						if ( wcscmp( pwszAttrName, L"value" ) == 0 )
						{
							CString elementName( pwszLocalName );
							CString elementValue( pwszValue );

							if ( elementName == L"Title" )
							{
								Title = elementValue;

							} else if ( elementName == L"Comment" )
							{
								Comment = elementValue;

							} else if( elementName == L"Location" )
							{
								Location = elementValue;

							} else if ( elementName == L"Date" )
							{
								Date = elementValue;

							} else if ( elementName == L"Artist" )
							{
								Artist = elementValue;

							} else if( elementName == L"Copyright" )
							{
								Copyright = elementValue;

							} else if ( elementName == L"Software" )
							{
								Software = elementValue;

							} else if ( elementName == L"Keywords" )
							{
								Keywords = elementValue;
							}
						}
					}
					while ( pReader->MoveToNextAttribute() == S_OK );
				}
			}
		}
	}
	value = true;
	pReader->Release();
	pFileStream->Release();
	return value;
} // ReadAlbumXml

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnAlbumTitle()
{
	bool bValue = AlbumTitle;
	AlbumTitle = !bValue;
	CImageView* pView = ImageView;
	pView->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateAlbumTitle( CCmdUI* pCmdUI )
{
	bool bValue = AlbumTitle;
	pCmdUI->SetCheck( bValue == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnAlbumLocation()
{
	bool bValue = AlbumLocation;
	AlbumLocation = !bValue;
	CImageView* pView = ImageView;
	pView->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateAlbumLocation( CCmdUI* pCmdUI )
{
	bool bValue = AlbumLocation;
	pCmdUI->SetCheck( bValue == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnAlbumComment()
{
	bool bValue = AlbumComment;
	AlbumComment = !bValue;
	CImageView* pView = ImageView;
	pView->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateAlbumComment( CCmdUI* pCmdUI )
{
	bool bValue = AlbumComment;
	pCmdUI->SetCheck( bValue == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnAlbumDate()
{
	bool bValue = AlbumDate;
	AlbumDate = !bValue;
	CImageView* pView = ImageView;
	pView->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateAlbumDate( CCmdUI* pCmdUI )
{
	bool bValue = AlbumDate;
	pCmdUI->SetCheck( bValue == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnAlbumArtist()
{
	bool bValue = AlbumArtist;
	AlbumArtist = !bValue;
	CImageView* pView = ImageView;
	pView->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateAlbumArtist( CCmdUI* pCmdUI )
{
	bool bValue = AlbumArtist;
	pCmdUI->SetCheck( bValue == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnAlbumCopyright()
{
	bool bValue = AlbumCopyright;
	AlbumCopyright = !bValue;
	CImageView* pView = ImageView;
	pView->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateAlbumCopyright( CCmdUI* pCmdUI )
{
	bool bValue = AlbumCopyright;
	pCmdUI->SetCheck( bValue == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnAlbumSoftware()
{
	bool bValue = AlbumSoftware;
	AlbumSoftware = !bValue;
	CImageView* pView = ImageView;
	pView->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateAlbumSoftware( CCmdUI* pCmdUI )
{
	bool bValue = AlbumSoftware;
	pCmdUI->SetCheck( bValue == true );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnAlbumKeywords()
{
	bool bValue = AlbumKeywords;
	AlbumKeywords = !bValue;
	CImageView* pView = ImageView;
	pView->Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateAlbumKeywords( CCmdUI* pCmdUI )
{
	bool bValue = AlbumKeywords;
	pCmdUI->SetCheck( bValue == true );
}

/////////////////////////////////////////////////////////////////////////////
// In your CPhotoExplorerDoc class implementation
UINT CPhotoExplorerDoc::MonitorDirectoryThread( LPVOID pParam )
{
	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pParam;
	CString strDirectory = pDoc->Folder;
	CKeyedCollection<CString, Bitmap>& pThumbnails = pDoc->Thumbnails;
	CPhotoExplorerView* pView = pDoc->PhotoExplorerView;
	HWND hWnd = pView->GetSafeHwnd();

	HANDLE hDir = CreateFile(
		strDirectory,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		NULL );

	if ( hDir == INVALID_HANDLE_VALUE )
		return 1;

	pDoc->m_hCompletionPort = CreateIoCompletionPort( hDir, NULL, 0, 1 );

	BYTE buffer[ 1024 ];
	DWORD dwBytesReturned = 0;
	OVERLAPPED overlapped = { 0 };

	while ( TRUE )
	{
		ZeroMemory( &overlapped, sizeof( overlapped ) );

		BOOL result = ReadDirectoryChangesW
		(
			hDir,
			buffer,
			sizeof( buffer ),
			TRUE, // Monitor subdirectories
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE | 
			FILE_NOTIFY_CHANGE_LAST_WRITE,
			&dwBytesReturned,
			&overlapped,
			NULL 
		);

		if ( !result )
		{
			AfxMessageBox( _T( "ReadDirectoryChangesW failed." ) );
			break;
		}

		DWORD dwBytes;
		ULONG_PTR key;
		LPOVERLAPPED lpOverlapped;

		BOOL status = GetQueuedCompletionStatus
		(
			pDoc->m_hCompletionPort,
			&dwBytes,
			&key,
			&lpOverlapped,
			INFINITE
		);

		if ( !status || pDoc->m_bStopThread )
		{
			CancelIoEx( hDir, &overlapped ); // Cancel the pending ReadDirectoryChangesW
			break;
		}

		if ( lpOverlapped )
		{
			FILE_NOTIFY_INFORMATION* pNotify = (FILE_NOTIFY_INFORMATION*)buffer;
			int offset = 0;

			CKeyedCollection<CString, int>& mapAddPaths = pDoc->AddedPaths;
			CKeyedCollection<CString, int>& mapDelPaths = pDoc->DeletedPaths;

			do
			{
				pNotify = (FILE_NOTIFY_INFORMATION*)( (BYTE*)buffer + offset );
				CString strFileName
				( 
					pNotify->FileName, pNotify->FileNameLength / sizeof( WCHAR ) 
				);

				CString strPath = strDirectory + strFileName;

				// Handle the notification
				switch ( pNotify->Action )
				{
					case FILE_ACTION_ADDED:
					{
						if ( !mapAddPaths.Exists[ strPath ] )
						{
							mapAddPaths.add( strPath, shared_ptr<int>( new int( 0 ) ) );
						}
						break;
					}
					case FILE_ACTION_REMOVED:
					{
						if ( !mapDelPaths.Exists[ strPath ] )
						{
							mapDelPaths.add( strPath, shared_ptr<int>( new int( 0 ) ) );
						}
						break;
					}
					case FILE_ACTION_MODIFIED:
						break;
				}

				offset += pNotify->NextEntryOffset;
			}
			while ( pNotify->NextEntryOffset != 0 );

			if ( mapDelPaths.Count > 0 )
			{
				::PostMessage( hWnd, WM_FOLDER_DELETE_EVENT, 0, 0 );
			}
			if ( mapAddPaths.Count > 0 )
			{
				::PostMessage( hWnd, WM_FOLDER_ADD_EVENT, 0, 0 );
			}
		}
	}

	CloseHandle( hDir );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::StopMonitorThread()
{
	if ( m_pThread )
	{
		m_bStopThread = true;
		SetEvent( m_hStopEvent ); // Signal the event to stop the thread
		if ( m_hCompletionPort )
		{
			PostQueuedCompletionStatus( m_hCompletionPort, 0, 0, NULL );
		}
		WaitForSingleObject( m_pThread->m_hThread, INFINITE );
		ResetEvent( m_hStopEvent ); // Reset the event for future use
		m_pThread = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::StartMonitorThread()
{
	StopMonitorThread(); // Ensure any previous thread is stopped

	m_bStopThread = false;
	m_pThread = AfxBeginThread( MonitorDirectoryThread, this );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnEditExportLabels()
{
	CKeyedCollection<CString, int> arrImages = SelectedImages;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CImageView* pImageView = ImageView;
	CShellListView* pListView = (CShellListView*)pFrame->ListView;

	// remember the label setting
	const bool bLabel = pImageView->Label;

	// clear the selection
	AddSelectedImage = L"";

	// loop through the previously selected image
	for ( auto& node : arrImages.Items )
	{
		pImageView->Export = true;
		pImageView->Label = true;

		// this will trigger a paint by setting
		// the path in the document
		pListView->SetSelectedImage( node.first );
		pFrame->Wait( 100 );
	}

	// clear the selection
	AddSelectedImage = L"";

	// restore the previous label setting
	pImageView->Label = bLabel;

} // OnEditExportLabels

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateEditExportLabels( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( FALSE );
	if ( SelectedCount > 0 )
	{
		pCmdUI->Enable();
	}
} // OnUpdateEditExportLabels

/////////////////////////////////////////////////////////////////////////////
// get the date taken, if any, from the given bitmap
CString CPhotoExplorerDoc::GetDateTaken
( 
	shared_ptr<Bitmap>& pBitmap,
	CDate& date
)
{
	CString value;

	// test the date properties stored in the given image
	CString csDateTaken =
		CHelper::GetStringProperty( pBitmap, PropertyTagExifDTOrig );

	// officially the original property is the date taken in this
	// format: "YYYY:MM:DD HH:MM:SS"
	date.DateTaken = csDateTaken;
	if ( date.Okay )
	{
		value = date.Date;
	}

	return value;
} // GetDateTaken

/////////////////////////////////////////////////////////////////////////////
// rename the given file by the date taken property in the date parameter
CString CPhotoExplorerDoc::RenameFile( LPCTSTR lpszPathName, CDate& date )
{
	CString value;
	const CString csFolder = CHelper::GetFolder( lpszPathName );
	const CString csFile = CHelper::GetFileName( lpszPathName );
	const CString csExtension = CHelper::GetExtension( lpszPathName );
	COleDateTime oDT = date.DateAndTime;

	// format the date in a globally sortable format suitable for a filename
	const CString csDate = oDT.Format( _T( "%Y_%m_%d_%H_%M_%S" ) );

	// related text file if it exists
	CString csText;
	csText.Format
	( 
		L"%s%s_details.txt", 
		csFolder, csFile 
	);

	int nCount = 0;

	// loop until a unique filename is generated
	do
	{
		// the first time through, do the normal generation
		if ( nCount++ == 0 )
		{
			value.Format
			( 
				_T( "%s%s%s" ), 
				csFolder, csDate, csExtension 
			);

		} else // after the first attempt, add the count to the filename
		{
			value.Format
			( 
				_T( "%s%s_%02d%s" ), 
				csFolder, csDate, nCount, csExtension 
			);
		}

		// if the original pathname matches the generated name
		// there is nothing to do, so get out
		if ( value == lpszPathName )
		{
			return value;
		}

		// if the path does not exist, break out of the loop
		if ( !::PathFileExists( value ) )
		{
			break;
		}

	}
	while ( true );

	// rename the file to the generated value
	try
	{
		CFile::Rename( lpszPathName, value );
		// if the rename fails for any reason, return a blank value
	}
	catch ( ... )
	{
		value = _T( "" );
	}

	// rename the corresponding text file if it exists so that they 
	// can still be associated together
	if ( ::PathFileExists( csText ) && !value.IsEmpty())
	{
		CString csFile = CHelper::GetFileName( value );
		CString csNewText;
		csNewText.Format
		( 
			L"%s%s_details.txt", csFolder, csFile 
		);
		CFile::Rename( csText, csNewText );
	}

	return value;
} // RenameFile

/////////////////////////////////////////////////////////////////////////////
// rename the selected images by their date taken
void CPhotoExplorerDoc::OnEditRename()
{
	CKeyedCollection<CString, int> arrImages = SelectedImages;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CShellListView* pListView = (CShellListView*)pFrame->ListView;
	const bool bShift = CHelper::ShiftKeyDown();
	const bool bAlbum = AlbumDate;

	// loop through the previously selected images
	for ( auto& node : arrImages.Items )
	{
		if ( Thumbnails.Exists[ node.first ] )
		{
			shared_ptr<Bitmap> pBitmap = Thumbnails.find( node.first );

			// use the album date if enabled and the user is holding
			// down the shift key
			CDate date;
			if ( bAlbum && bShift )
			{
				date.DateTaken = Date;

			} else
			{
				CString csDate = GetDateTaken( pBitmap, date );
				if ( csDate.IsEmpty() )
				{
					continue;
				}
			}
			int nHour = date.Hour;
			int nMinute = date.Minute;
			int nSecond = date.Second;
			if ( nHour == 0 && nMinute == 0 && nSecond == 0 )
			{
				// the file's status contains the modification time.
				CFileStatus fs;

				// if successful, write the modification time to the
				// date variable
				if ( CFile::GetStatus( node.first, fs ) )
				{
					date.Hour = fs.m_mtime.GetHour();
					date.Minute = fs.m_mtime.GetMinute();
					date.Second = fs.m_mtime.GetSecond();
				}
			}
			RenameFile( node.first, date );
		}
	}

	StopMonitorThread();

	// repopulate the list without the deleted items
	FindInterestingFiles();

	// reset the selection
	AddSelectedImage = L"";
	ImageView->Invalidate();

	StartMonitorThread();

} // OnEditRename

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateEditRename( CCmdUI* pCmdUI )
{
	pCmdUI->Enable( FALSE );
	if ( SelectedCount > 0 )
	{
		pCmdUI->Enable();
	}
} // OnUpdateEditRename

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnEditShortcuts()
{
    CEditShortcutDialog dlg;
	
	// setup the position of the dialog based on the property co-ordinates
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CRect rect;
	pFrame->GetWindowRect( &rect );
	CPoint pt( rect.left, rect.top );
	pFrame->ClientToScreen( &pt );
	dlg.UpperLeftCorner = pt;

	INT_PTR ipResult = dlg.DoModal();
	if ( ipResult == IDOK )
	{
		Shortcuts = dlg.Shortcuts;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateEditShortcuts( CCmdUI* pCmdUI )
{
	pCmdUI->Enable();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnEditAlbumLabel()
{
	theApp.OnIdle( 0 );
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CNavigator* pNav = pFrame->Navigator;

	// launch the progress dialog
	CThumbnailDialog dlg;
	dlg.Parent = pFrame;
	dlg.CreateDlg();
	dlg.ShowWindow( SW_SHOW );

	dlg.TotalImages = AlbumFolders.Count;

	int nFolder = 0;
	bool bAbort = false;

	for ( auto& node : AlbumFolders.Items )
	{
		// let the user cancel out
		if ( dlg.Cancel )
		{
			bAbort = true;
			break;
		}

		// update the progress dialog's status
		dlg.CurrentImage = nFolder++;

		if ( ::PathFileExists( node.first ))
		{
			CString csPath = node.first;
			CString csTitle = *node.second;
			CString csFolder = CHelper::GetFolder( csPath );
			csFolder += csTitle;
			
			CString csMessage( L"Renaming:" );
			csMessage += csPath;
			ProgressText = csMessage;
			CString csAlbum = CHelper::GetFileName( csPath );
			CString csDetails;
			csDetails.Format( L"%s\\Details.txt", (LPCTSTR)csPath );
			vector<CString> arrDetails = CHelper::ReadText( csDetails );
			int nDetails = (int)arrDetails.size();
			if ( nDetails != 0 )
			{
				CString csLast = arrDetails[ nDetails - 1 ];
				if ( csLast != csAlbum )
				{
					CHelper::AppendStringToTextFile( csDetails, L"\n" );
					CHelper::AppendStringToTextFile( csDetails, csAlbum );
				}
			}

			// creates a version of a pathname when there is a duplication where
			// the version is appended to the original name inside of open 
			// curly brackets
			csFolder = CHelper::NewPathVersion( csFolder );

			csMessage = L"To: ";
			csMessage += csFolder;
			ProgressText = csMessage;

			BOOL bOK = ::MoveFile( csPath, csFolder );

			if ( !bOK )
			{
				ErrorsText = L"Failed to rename.";
				CString csMessage = CHelper::LastError();
				ErrorsText = csMessage;
			}
		}

		// wait ten milliseconds while letting normal 
		// window messaging to run
		pFrame->Wait( 10 );
	}

	if ( !bAbort )
	{
		dlg.TotalImages = TwoDateFolders.Count;

		int nFolder = 0;
		for ( auto& node : TwoDateFolders.Items )
		{
			// let the user cancel out
			if ( dlg.Cancel )
			{
				bAbort = true;
				break;
			}

			// update the progress dialog's status
			dlg.CurrentImage = nFolder++;

			if ( ::PathFileExists( node.first ) )
			{
				CString csPath = node.first;
				CString csNew = *node.second;

				CString csMessage( L"Renaming:" );
				csMessage += csPath;
				ProgressText = csMessage;

				// creates a version of a pathname when there is a duplication where
				// the version is appended to the original name inside of open 
				// curly brackets
				csNew = CHelper::NewPathVersion( csNew );

				csMessage = L"To: ";
				csMessage += csNew;
				ProgressText = csMessage;

				BOOL bOK = ::MoveFile( csPath, csNew );

				if ( !bOK )
				{
					ErrorsText = L"Failed to rename.";
					CString csMessage = CHelper::LastError();
					ErrorsText = csMessage;
				}
			}

			// wait ten milliseconds while letting normal 
			// window messaging to run
			pFrame->Wait( 10 );
		}
	}

	if ( !bAbort )
	{
		dlg.TotalImages = EndDateFolders.Count;

		int nFolder = 0;
		for ( auto& node : EndDateFolders.Items )
		{
			// let the user cancel out
			if ( dlg.Cancel )
			{
				bAbort = true;
				break;
			}

			// update the progress dialog's status
			dlg.CurrentImage = nFolder++;

			if ( ::PathFileExists( node.first ) )
			{
				CString csPath = node.first;
				CString csNew = *node.second;

				CString csMessage( L"Renaming:" );
				csMessage += csPath;
				ProgressText = csMessage;

				// creates a version of a pathname when there is a duplication where
				// the version is appended to the original name inside of open 
				// curly brackets
				csNew = CHelper::NewPathVersion( csNew );

				csMessage = L"To: ";
				csMessage += csNew;
				ProgressText = csMessage;

				BOOL bOK = ::MoveFile( csPath, csNew );

				if ( !bOK )
				{
					ErrorsText = L"Failed to rename.";
					CString csMessage = CHelper::LastError();
					ErrorsText = csMessage;
				}
			}

			// wait ten milliseconds while letting normal 
			// window messaging to run
			pFrame->Wait( 10 );
		}
	}

	if ( !bAbort )
	{
		dlg.TotalImages = NoDateFolders.Count;

		int nFolder = 0;
		for ( auto& node : NoDateFolders.Items )
		{
			// let the user cancel out
			if ( dlg.Cancel )
			{
				bAbort = true;
				break;
			}

			// update the progress dialog's status
			dlg.CurrentImage = nFolder++;

			if ( ::PathFileExists( node.first ) )
			{
				CString csPath = node.first;
				CString csNew = *node.second;

				CString csMessage( L"Renaming:" );
				csMessage += csPath;
				ProgressText = csMessage;

				// creates a version of a pathname when there is a duplication where
				// the version is appended to the original name inside of open 
				// curly brackets
				csNew = CHelper::NewPathVersion( csNew );

				csMessage = L"To: ";
				csMessage += csNew;
				ProgressText = csMessage;

				BOOL bOK = ::MoveFile( csPath, csNew );

				if ( !bOK )
				{
					ErrorsText = L"Failed to rename.";
					CString csMessage = CHelper::LastError();
					ErrorsText = csMessage;
				}
			}

			// wait ten milliseconds while letting normal 
			// window messaging to run
			pFrame->Wait( 10 );
		}
	}

	// done with the progress dialog
	dlg.DestroyWindow();

	AlbumFolders.clear();

} // OnEditAlbumLabel

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnUpdateEditAlbumLabel( CCmdUI* pCmdUI )
{
	const int nAlbums = AlbumFolders.Count;
	const int nTwoDates = TwoDateFolders.Count;
	const int nEndDates = EndDateFolders.Count;
	const int nNoDates = NoDateFolders.Count;
	pCmdUI->Enable
	( 
		nAlbums != 0 || nTwoDates != 0 || nEndDates != 0 || nNoDates != 0
	);
}

/////////////////////////////////////////////////////////////////////////////
// Helper to convert Degrees, Minutes, and Seconds (DMS) to decimal degrees
double CPhotoExplorerDoc::ConvertDMSToDecimal
(
	Gdiplus::PropertyItem* item, char ref
) 
{
	if
	(
		!item ||
		item->type != PropertyTagTypeRational ||
		item->length < 3 * sizeof(ULONG) * 2
	)
	{
		return 0.0;
	}

	auto* data = reinterpret_cast<ULONG*>(item->value);
	double degrees = static_cast<double>(data[0]) / data[1];
	double minutes = static_cast<double>(data[2]) / data[3];
	double seconds = static_cast<double>(data[4]) / data[5];

	double decimal = degrees + (minutes / 60.0) + (seconds / 3600.0);
	if (ref == 'S' || ref == 'W')
	{
		decimal *= -1;
	}
	return decimal;
} // ConvertDMSToDecimal

/////////////////////////////////////////////////////////////////////////////
// Main function to generate Google Maps link
CString CPhotoExplorerDoc::GenerateMapLink
(
	const CString& imagePath, bool bBing
) 
{
	CString value;
	Gdiplus::Bitmap bitmap(imagePath);
	if (bitmap.GetLastStatus() != Ok)
	{
		return value;
	}

	Gdiplus::PropertyItem* latItem = nullptr;
	Gdiplus::PropertyItem* latRefItem = nullptr;
	Gdiplus::PropertyItem* lonItem = nullptr;
	Gdiplus::PropertyItem* lonRefItem = nullptr;

	UINT size = 0;

	// GPS Latitude
	size = bitmap.GetPropertyItemSize(PropertyTagGpsLatitude);
	if (size == 0)
	{
		return value;
	}
	latItem = (Gdiplus::PropertyItem*)malloc(size);
	bitmap.GetPropertyItem(PropertyTagGpsLatitude, size, latItem);

	// GPS Latitude Ref
	size = bitmap.GetPropertyItemSize(PropertyTagGpsLatitudeRef);
	latRefItem = (Gdiplus::PropertyItem*)malloc(size);
	bitmap.GetPropertyItem(PropertyTagGpsLatitudeRef, size, latRefItem);

	// GPS Longitude
	size = bitmap.GetPropertyItemSize(PropertyTagGpsLongitude);
	lonItem = (Gdiplus::PropertyItem*)malloc(size);
	bitmap.GetPropertyItem(PropertyTagGpsLongitude, size, lonItem);

	// GPS Longitude Ref
	size = bitmap.GetPropertyItemSize(PropertyTagGpsLongitudeRef);
	lonRefItem = (Gdiplus::PropertyItem*)malloc(size);
	bitmap.GetPropertyItem(PropertyTagGpsLongitudeRef, size, lonRefItem);

	CString csLat((char*)latRefItem->value);
	CString csLong((char*)lonRefItem->value);

	double latitude = ConvertDMSToDecimal(latItem, (char)csLat[0]);
	double longitude = ConvertDMSToDecimal(lonItem, (char)csLong[0]);

	free(latItem); free(latRefItem); free(lonItem); free(lonRefItem);

	if (!CHelper::NearlyEqual(latitude + longitude, 0.0))
	{
		if (bBing)
		{
			value.Format
			(
				L"https://www.bing.com/maps?q=%.7f,%.7f",
				latitude, longitude
			);
		}
		else // Google map
		{
			value.Format
			(
				L"https://www.google.com/maps?q=%.7f,%.7f",
				latitude, longitude
			);
		}
	}

	return value;
} // GenerateMapLink

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class

	CDocument::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoExplorerDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class

	return CDocument::OnSaveDocument(lpszPathName);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoExplorerDoc::SaveModified()
{
	// TODO: Add your specialized code here and/or call the base class

	return CDocument::SaveModified();
}

/////////////////////////////////////////////////////////////////////////////

