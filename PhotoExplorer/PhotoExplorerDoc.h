/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ExifRotation.h"
#include "PlusGDI.h"
#include "KeyedCollection.h"
#include "ThumbnailDialog.h"
#include "MapDialog.h"
#include "Date.h"
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
class CPhotoExplorerView;
class CImageView;

/////////////////////////////////////////////////////////////////////////////
// CPhotoExplorerDoc
//
// The central document class for Photo Explorer. This class manages all
// data associated with the currently selected folder, including image lists,
// thumbnails, metadata, album information, shortcuts, and background folder
// monitoring. It serves as the core data model for the application, providing
// the views with everything they need to display, annotate, and manipulate
// images.
//
// Purpose:
//   • Represent the active folder and all images within it.
//   • Maintain the selected image and its associated metadata.
//   • Manage album-level information (title, location, date, artist, etc.).
//   • Generate thumbnails and cache them for fast UI updates.
//   • Provide EXIF rotation, metadata extraction, and date-taken logic.
//   • Support multi-selection of images and batch operations.
//   • Monitor the folder for changes using a background thread.
//   • Provide Google/Bing map link generation from GPS metadata.
//   • Read and write album XML files and image detail files.
//   • Resolve shortcuts and symbolic paths for rapid metadata entry.
//
// Why this class exists:
//   Photo Explorer is built around folders of images. The document must
//   maintain a complete, consistent representation of the folder’s contents,
//   including thumbnails, metadata, album information, and selection state.
//   CPhotoExplorerDoc centralizes all of this logic so the views (list view,
//   image view, properties pane) can remain lightweight and responsive.
//
// Responsibilities:
//   • Track the active folder (m_csFolder) and its image list (m_ImageNames).
//   • Maintain the selected image (m_pSelectedBitmap) and its EXIF rotation.
//   • Manage album-level metadata fields and fallback rules:
//       – Title, Comment, Location, Date, Artist, Copyright,
//         Software, Keywords.
//   • Provide album fallback flags (m_bAlbumTitle, m_bAlbumLocation, etc.).
//   • Maintain collections:
//       – SelectedImages (multi-selection)
//       – AddedPaths / DeletedPaths (folder changes)
//       – Thumbnails (cached Bitmap objects)
//       – Shortcuts (metadata abbreviations)
//       – AlbumFolders / TwoDateFolders / EndDateFolders / NoDateFolders
//   • Provide metadata helpers:
//       – ResolveShortcuts
//       – MapShortcuts
//       – GetRealPath
//       – WriteAlbumToImage / WriteAlbumToBitmap
//       – ReadImageDetails / ParseAlbumDetails
//   • Provide GPS helpers:
//       – ConvertDMSToDecimal
//       – GenerateMapLink (Google/Bing)
//   • Provide folder monitoring via background thread:
//       – StartMonitorThread
//       – StopMonitorThread
//       – MonitorDirectoryThread
//   • Provide thumbnail generation (CreateThumbnail).
//   • Provide date-taken extraction and file renaming logic.
//
// Interaction with other components:
//   • CPhotoExplorerView — displays thumbnails and folder contents.
//   • CImageView — displays the selected image with labels.
//   • CImageProperties — manages metadata for the selected image.
//   • CPlusGDI — wraps GDI+ image loading and measurement.
//   • CExifRotation — corrects image orientation.
//   • CThumbnailDialog — used for batch thumbnail operations.
//   • CMapDialog — displays map links for GPS-tagged images.
//   • CMainFrame — coordinates UI updates and docking panes.
//   • COutputWnd — receives progress/warning/error messages.
//
// Key Features:
//   • Full album metadata system with fallback rules.
//   • Shortcut expansion for rapid metadata entry (e.g., @loc → “Nederland, TX”).
//   • GPS coordinate parsing and map link generation.
//   • Thumbnail caching for fast UI performance.
//   • Background folder monitoring using I/O completion ports.
//   • XML album file reading/writing for persistent album metadata.
//   • Image detail file parsing for folder-based metadata organization.
//   • Multi-selection support with progress logging.
//   • Date-taken extraction from EXIF or filename patterns.
//   • Automatic file renaming based on date-taken.
//
// Internal Structure:
//   • m_nPendingOperation — counts active operations for UI responsiveness.
//   • m_bStopThread / m_pThread / m_hStopEvent — folder monitoring thread.
//   • m_ImageNames — list of images in the active folder.
//   • m_Thumbnails — cached thumbnails for fast display.
//   • m_arrSelectedImages — multi-selection tracking.
//   • m_PlusGDI — wrapper for selected image loading and measurement.
//   • m_Rotator — EXIF rotation handler.
//   • Album metadata fields (m_csTitle, m_csComment, etc.).
//   • Album fallback flags (m_bAlbumTitle, m_bAlbumLocation, etc.).
//   • Shortcut collections and folder correction maps.
//
// This class forms the backbone of Photo Explorer’s data model, providing
// comprehensive management of images, metadata, thumbnails, album information,
// and folder monitoring. It enables the rest of the application to present a
// rich, responsive, and metadata-driven experience.
/////////////////////////////////////////////////////////////////////////////
class CPhotoExplorerDoc : public CDocument
{
protected: // create from serialization only
	CPhotoExplorerDoc() noexcept;
	DECLARE_DYNCREATE(CPhotoExplorerDoc)

// protected data
protected:
	// pending operation when count is non-zero
	int m_nPendingOperation;

	// a volatile variable for thread awareness to stop an existing thread
	volatile bool m_bStopThread;

	// the actual thread
	CWinThread* m_pThread;

	// event to stop the thread
	HANDLE m_hStopEvent;

	HANDLE m_hCompletionPort;

	// the selected folder
	CString m_csFolder;

	// collection of pathnames when there is a multi-selection
	// or a single item representing a single selection
	CKeyedCollection<CString, int> m_arrSelectedImages;

	// pathnames added to active folder
	CKeyedCollection<CString, int> mapAddPaths;

	// pathnames deleted from active folder
	CKeyedCollection<CString, int> mapDelPaths;

	// the album file pathname
	CString m_csAlbumPath;

	// list of image files in the current folder
	CKeyedCollection<CString, int> m_ImageNames;

	// the currently selected image
	CPlusGDI m_PlusGDI;

	// selected bitmap
	shared_ptr<Bitmap> m_pSelectedBitmap;

	// rotation property of current image
	CExifRotation m_Rotator;

	// album title
	CString m_csTitle;

	// album comment
	CString m_csComment;

	// album location
	CString m_csLocation;

	// album date
	CString m_csDate;

	// album artist
	CString m_csArtist;

	// album copyright
	CString m_csCopyright;

	// album software
	CString m_csSoftware;

	// album keywords (semicolon separated)
	CString m_csKeywords;

	// if true, the title is replaced with the album title
	// if the image title is blank
	bool m_bAlbumTitle;

	// if true, the location is replaced with the album location
	// if the image location is blank
	bool m_bAlbumLocation;

	// if true, the comment is replaced with the album comment
	// if the image comment is blank
	bool m_bAlbumComment;

	// if true, the date is replaced with the album date
	// if the image date is blank
	bool m_bAlbumDate;

	// if true, the Artist is replaced with the album Artist
	// if the image Artist is blank
	bool m_bAlbumArtist;

	// if true, the Copyright is replaced with the album Copyright
	// if the image Copyright is blank
	bool m_bAlbumCopyright;

	// if true, the Software is replaced with the album Software
	// if the image Software is blank
	bool m_bAlbumSoftware;

	// if true, the Keywords is replaced with the album Keywords
	// if the image Keywords is blank
	bool m_bAlbumKeywords;

	// a collection of image thumbnails for the current folder
	CKeyedCollection<CString, Bitmap> m_Thumbnails;

	// a collection of short cuts to reduce repetative typing
	CKeyedCollection<CString, CString> m_Shortcuts;

	// a collection of folders starting with "album" containing
	// a detail.txt file with the name the folder should be
	CKeyedCollection<CString, CString> m_AlbumFolders;

	// correcting an error in folders with two dates
	CKeyedCollection<CString, CString> m_TwoDateFolders;

	// correcting an inconsistency in folders with end dates
	CKeyedCollection<CString, CString> m_EndDateFolders;

	// correcting an error in folders with no dates
	CKeyedCollection<CString, CString> m_NoDateFolders;

// properties
public:
	// pending operation when count is non-zero
	inline bool GetPendingOperation()
	{
		return m_nPendingOperation != 0;
	}
	// pending operation when count is non-zero
	inline void SetPendingOperation( bool value )
	{
		if ( value )
		{
			m_nPendingOperation++;

		} else
		{
			if ( m_nPendingOperation > 0 )
			{
				m_nPendingOperation--;
			}
		}
	}
	// pending operation when count is non-zero
	__declspec 
		( property( get = GetPendingOperation, put = SetPendingOperation ) ) 
		bool PendingOperation;

	// the selected image
	CKeyedCollection<CString, Bitmap>& GetThumbnails()
	{
		return m_Thumbnails;
	}
	// the selected image
	__declspec( property( get = GetThumbnails ) )
		CKeyedCollection<CString, Bitmap>& Thumbnails;

	// get the thumbnail image associated with the given pathname
	shared_ptr<Bitmap> GetThumbnail( CString csPath )
	{
		shared_ptr<Bitmap> value;
		if ( m_Thumbnails.Exists[ csPath ] )
		{
			value = m_Thumbnails.find( csPath );
		}
		return value;
	}
	// get the thumbnail image associated with the given pathname
	__declspec( property( get = GetThumbnail ) )
		shared_ptr<Bitmap> Thumbnail[];

	// the associated shell list view
	CPhotoExplorerView* GetPhotoExplorerView();
	// the associated shell list view
	__declspec( property( get = GetPhotoExplorerView ) )
		CPhotoExplorerView* PhotoExplorerView;

	// the associated image view
	CImageView* GetImageView();
	// the associated image view
	__declspec( property( get = GetImageView ) )
		CImageView* ImageView;

	// the selected image
	shared_ptr<Bitmap> GetSelectedBitmap()
	{
		return m_pSelectedBitmap;
	}
	// the selected image
	__declspec( property( get = GetSelectedBitmap ) )
		shared_ptr<Bitmap> SelectedBitmap;

	// the selected image
	CPlusGDI* GetImage()
	{
		return &m_PlusGDI;
	}
	// the selected image
	__declspec( property( get = GetImage ) )
		CPlusGDI* Image;

	// the selected image
	CExifRotation* GetRotator()
	{
		return &m_Rotator;
	}
	// the selected image
	__declspec( property( get = GetRotator ) )
		CExifRotation* Rotator;

	// height of document in inches
	virtual double GetHeight()
	{
		double value = 0;
		if ( Image->IsOpen )
		{
			value = Image->GetHeightInches();
		}
		return value;
	}
	// height of document in inches
	__declspec( property( get = GetHeight ) )
		double Height;

	// width of document in inches
	virtual double GetWidth()
	{
		double value = 0;
		if ( Image->IsOpen )
		{
			value = Image->GetWidthInches();
		}
		return value;
	}
	// width of document in inches
	__declspec( property( get = GetWidth ) )
		double Width;

	// write text to the progress tab
	void SetProgressText( CString value );
	// write text to the progress tab
	__declspec( property( put = SetProgressText ) )
		CString ProgressText;

	// write text to the warnings tab
	void SetWarningsText( CString value );
	// write text to the warnings tab
	__declspec( property( put = SetWarningsText ) )
		CString WarningsText;

	// write text to the errors tab
	void SetErrorsText( CString value );
	// write text to the errors tab
	__declspec( property( put = SetErrorsText ) )
		CString ErrorsText;

	// the selected folder
	CString GetFolder()
	{
		return m_csFolder;
	}
	// the selected folder
	void SetFolder( LPCTSTR value, LPCTSTR filename = 0 );
	// the selected folder
	__declspec( property( get = GetFolder, put = SetFolder ) )
		CString Folder;

	// the album file pathname
	CString GetAlbumPath()
	{
		return m_csAlbumPath;
	}
	// the album file pathname
	void SetAlbumPath( LPCTSTR value )
	{
		m_csAlbumPath = value;
	}
	// the album file pathname
	__declspec( property( get = GetAlbumPath, put = SetAlbumPath ) )
		CString AlbumPath;

	// the selected folder
	CString GetImagePath()
	{
		CString value;
		USHORT usCount = SelectedCount;
		if ( usCount == 1 )
		{
			for ( auto& node : SelectedImages.Items )
			{
				value = node.first;
				break;
			}
		}
		return value;
	}
	// the selected folder
	void SetImagePath( LPCTSTR value );
	// the selected folder
	__declspec( property( get = GetImagePath, put = SetImagePath ) )
		CString ImagePath;

	// collection of pathnames when there is a multi-selection
	// or a single item representing a single selection
	CKeyedCollection<CString, int>& GetSelectedImages()
	{
		return m_arrSelectedImages;
	}
	// collection of pathnames when there is a multi-selection
	// or a single item representing a single selection
	__declspec( property( get = GetSelectedImages ) )
		CKeyedCollection<CString, int>& SelectedImages;

	// pathnames added to active folder
	CKeyedCollection<CString, int>& GetAddedPaths()
	{
		return mapAddPaths;
	}
	// pathnames added to active folder
	__declspec( property( get = GetAddedPaths ) )
		CKeyedCollection<CString, int>& AddedPaths;

	// pathnames deleted from active folder
	CKeyedCollection<CString, int>& GetDeletedPaths()
	{
		return mapDelPaths;
	}
	// pathnames deleted from active folder
	__declspec( property( get = GetDeletedPaths ) )
		CKeyedCollection<CString, int>& DeletedPaths;

	// list of image files in the current folder
	CKeyedCollection<CString, int>& GetImageNames()
	{
		return m_ImageNames;
	}
	// list of image files in the current folder
	__declspec( property( get = GetImageNames ) )
		CKeyedCollection<CString, int>& ImageNames;

	// add a path to the selected images
	void SetAddSelectedImage( CString value )
	{
		// special case is an empty value...empty the collection
		if ( value.IsEmpty() )
		{
			SelectedImages.clear();
			return;
		}
		const bool bExists = SelectedImages.Exists[ value ];
		if ( !bExists )
		{
			SelectedImages.add
			(
				value, shared_ptr<int>( new int( 0 ) )
			);

			ProgressText = L"Multiple selection:";
			ProgressText = value;
		}
	}
	// add a path to the selected images
	__declspec( property( put = SetAddSelectedImage ) )
		CString AddSelectedImage;

	// remove a selected image
	void SetRemoveSelectedImage( CString value )
	{
		const bool bExists = SelectedImages.Exists[ value ];
		if ( bExists )
		{
			SelectedImages.remove( value );

			ProgressText = L"Multiple selection removed:";
			ProgressText = value;
		}
	}
	// remove a selected image
	__declspec( property( put = SetRemoveSelectedImage ) )
		CString RemoveSelectedImage;

	// number of selected images
	USHORT GetSelectedCount()
	{
		return (USHORT)SelectedImages.Count;
	}
	// number of selected images
	__declspec( property( get = GetSelectedCount ) )
		USHORT SelectedCount;

	// album title
	CString GetTitle()
	{
		CString value = ResolveShortcuts( m_csTitle );
		return value;
	}
	// album title
	void SetTitle( LPCTSTR value )
	{
		m_csTitle = ResolveShortcuts( value );
	}
	// album title
	__declspec( property( get = GetTitle, put = SetTitle ) )
		CString Title;

	// album comment
	CString GetComment()
	{
		CString value = ResolveShortcuts( m_csComment );
		return value;
	}
	// album comment
	void SetComment( LPCTSTR value )
	{
		m_csComment = ResolveShortcuts( value );
	}
	// album comment
	__declspec( property( get = GetComment, put = SetComment ) )
		CString Comment;

	// album location
	CString GetLocation()
	{
		CString value = ResolveShortcuts( m_csLocation );
		return value;
	}
	// album location
	void SetLocation( LPCTSTR value )
	{
		m_csLocation = ResolveShortcuts( value );
	}
	// album location
	__declspec( property( get = GetLocation, put = SetLocation ) )
		CString Location;

	// album date
	CString GetDate()
	{
		CString value = ResolveShortcuts( m_csDate );
		return value;
	}
	// album date
	void SetDate( LPCTSTR value )
	{
		m_csDate = ResolveShortcuts( value );
	}
	// album date
	__declspec( property( get = GetDate, put = SetDate ) )
		CString Date;

	// album artist
	CString GetArtist()
	{
		CString value = ResolveShortcuts( m_csArtist );
		return value;
	}
	// album artist
	void SetArtist( LPCTSTR value )
	{
		m_csArtist = ResolveShortcuts( value );
	}
	// album artist
	__declspec( property( get = GetArtist, put = SetArtist ) )
		CString Artist;

	// album copyright
	CString GetCopyright()
	{
		CString value = ResolveShortcuts( m_csCopyright );
		return value;
	}
	// album copyright
	void SetCopyright( LPCTSTR value )
	{
		m_csCopyright = ResolveShortcuts( value );
	}
	// album copyright
	__declspec( property( get = GetCopyright, put = SetCopyright ) )
		CString Copyright;

	// album software
	CString GetSoftware()
	{
		CString value = ResolveShortcuts( m_csSoftware );
		return value;
	}
	// album software
	void SetSoftware( LPCTSTR value )
	{
		m_csSoftware = ResolveShortcuts( value );
	}
	// album software
	__declspec( property( get = GetSoftware, put = SetSoftware ) )
		CString Software;

	// album keywords (semicolon separated)
	CString GetKeywords()
	{
		CString value = ResolveShortcuts( m_csKeywords );
		return value;
	}
	// album keywords (semicolon separated)
	void SetKeywords( LPCTSTR value )
	{
		m_csKeywords = ResolveShortcuts( value );
	}
	// album keywords (semicolon separated)
	__declspec( property( get = GetKeywords, put = SetKeywords ) )
		CString Keywords;

	// if true, the title is replaced with the album title
	// if the image title is blank
	inline bool GetAlbumTitle()
	{
		return m_bAlbumTitle;
	}
	// if true, the title is replaced with the album title
	// if the image title is blank
	inline void SetAlbumTitle( bool value )
	{
		AfxGetApp()->WriteProfileInt
		(
			L"Settings", L"AlbumTitle", value ? 1 : 0
		);
		m_bAlbumTitle = value;
	}
	// if true, the title is replaced with the album title
	// if the image title is blank
	__declspec( property( get = GetAlbumTitle, put = SetAlbumTitle ) )
		bool AlbumTitle;

	// if true, the location is replaced with the album location
	// if the image location is blank
	inline bool GetAlbumLocation()
	{
		return m_bAlbumLocation;
	}
	// if true, the location is replaced with the album location
	// if the image location is blank
	inline void SetAlbumLocation( bool value )
	{
		AfxGetApp()->WriteProfileInt
		(
			L"Settings", L"AlbumLocation", value ? 1 : 0
		);
		m_bAlbumLocation = value;
	}
	// if true, the location is replaced with the album location
	// if the image location is blank
	__declspec( property( get = GetAlbumLocation, put = SetAlbumLocation ) )
		bool AlbumLocation;

	// if true, the comment is replaced with the album comment
	// if the image comment is blank
	inline bool GetAlbumComment()
	{
		return m_bAlbumComment;
	}
	// if true, the comment is replaced with the album comment
	// if the image comment is blank
	inline void SetAlbumComment( bool value )
	{
		AfxGetApp()->WriteProfileInt
		(
			L"Settings", L"AlbumComment", value ? 1 : 0
		);
		m_bAlbumComment = value;
	}
	// if true, the comment is replaced with the album comment
	// if the image comment is blank
	__declspec( property( get = GetAlbumComment, put = SetAlbumComment ) )
		bool AlbumComment;

	// if true, the date is replaced with the album date
	// if the image date is blank
	inline bool GetAlbumDate()
	{
		return m_bAlbumDate;
	}
	// if true, the date is replaced with the album date
	// if the image date is blank
	inline void SetAlbumDate( bool value )
	{
		AfxGetApp()->WriteProfileInt
		(
			L"Settings", L"AlbumDate", value ? 1 : 0
		);
		m_bAlbumDate = value;
	}
	// if true, the date is replaced with the album date
	// if the image date is blank
	__declspec( property( get = GetAlbumDate, put = SetAlbumDate ) )
		bool AlbumDate;

	// if true, the Artist is replaced with the album Artist
	// if the image Artist is blank
	inline bool GetAlbumArtist()
	{
		return m_bAlbumArtist;
	}
	// if true, the Artist is replaced with the album Artist
	// if the image Artist is blank
	inline void SetAlbumArtist( bool value )
	{
		AfxGetApp()->WriteProfileInt
		(
			L"Settings", L"AlbumArtist", value ? 1 : 0
		);
		m_bAlbumArtist = value;
	}
	// if true, the Artist is replaced with the album Artist
	// if the image Artist is blank
	__declspec( property( get = GetAlbumArtist, put = SetAlbumArtist ) )
		bool AlbumArtist;

	// if true, the Copyright is replaced with the album Copyright
	// if the image Copyright is blank
	inline bool GetAlbumCopyright()
	{
		return m_bAlbumCopyright;
	}
	// if true, the Copyright is replaced with the album Copyright
	// if the image Copyright is blank
	inline void SetAlbumCopyright( bool value )
	{
		AfxGetApp()->WriteProfileInt
		(
			L"Settings", L"AlbumCopyright", value ? 1 : 0
		);
		m_bAlbumCopyright = value;
	}
	// if true, the Copyright is replaced with the album Copyright
	// if the image Copyright is blank
	__declspec( property( get = GetAlbumCopyright, put = SetAlbumCopyright ) )
		bool AlbumCopyright;

	// if true, the Software is replaced with the album Software
	// if the image Software is blank
	inline bool GetAlbumSoftware()
	{
		return m_bAlbumSoftware;
	}
	// if true, the Software is replaced with the album Software
	// if the image Software is blank
	inline void SetAlbumSoftware( bool value )
	{
		AfxGetApp()->WriteProfileInt
		(
			L"Settings", L"AlbumSoftware", value ? 1 : 0
		);
		m_bAlbumSoftware = value;
	}
	// if true, the Software is replaced with the album Software
	// if the image Software is blank
	__declspec( property( get = GetAlbumSoftware, put = SetAlbumSoftware ) )
		bool AlbumSoftware;

	// if true, the Keywords is replaced with the album Keywords
	// if the image Keywords is blank
	inline bool GetAlbumKeywords()
	{
		return m_bAlbumKeywords;
	}
	// if true, the Keywords is replaced with the album Keywords
	// if the image Keywords is blank
	inline void SetAlbumKeywords( bool value )
	{
		AfxGetApp()->WriteProfileInt
		( 
			L"Settings", L"AlbumKeywords", value ? 1 : 0 
		);
		m_bAlbumKeywords = value;
	}
	// if true, the Keywords is replaced with the album Keywords
	// if the image Keywords is blank
	__declspec( property( get = GetAlbumKeywords, put = SetAlbumKeywords ) )
		bool AlbumKeywords;

	// height and width of image thumbnails
	UINT GetThumbnailSize();
	// height and width of image thumbnails
	__declspec( property( get = GetThumbnailSize ) )
		UINT ThumbnailSize;
	
	// a collection of short cuts to reduce repetative typing
	inline CKeyedCollection<CString, CString>& GetShortcuts()
	{
		return m_Shortcuts;
	}
	// a collection of short cuts to reduce repetative typing
	inline void SetShortcuts( CKeyedCollection<CString, CString>& value )
	{
		m_Shortcuts = value;
	}
	// a collection of short cuts to reduce repetative typing
	__declspec( property( get = GetShortcuts, put = SetShortcuts ) )
		CKeyedCollection<CString, CString>& Shortcuts;

	// translate a shortcut into its value
	inline CString GetShortcut( CString csKey )
	{
		CString value;
		if ( Shortcuts.Exists[ csKey ] )
		{
			value = *Shortcuts.find( csKey );
		}
		return value;
	}
	// translate a shortcut into its value
	__declspec( property( get = GetShortcut ) )
		CString Shortcut[];

	// a collection of short cuts to reduce repetative typing
	inline CKeyedCollection<CString, CString>& GetAlbumFolders()
	{
		return m_AlbumFolders;
	}
	// a collection of short cuts to reduce repetative typing
	inline void SetAlbumFolders( CKeyedCollection<CString, CString>& value )
	{
		m_AlbumFolders = value;
	}
	// a collection of short cuts to reduce repetative typing
	__declspec( property( get = GetAlbumFolders, put = SetAlbumFolders ) )
		CKeyedCollection<CString, CString>& AlbumFolders;

	// correcting an error in folders with two dates
	inline CKeyedCollection<CString, CString>& GetTwoDateFolders()
	{
		return m_TwoDateFolders;
	}
	// correcting an error in folders with two dates
	inline void SetTwoDateFolders( CKeyedCollection<CString, CString>& value )
	{
		m_TwoDateFolders = value;
	}
	// correcting an error in folders with two dates
	__declspec( property( get = GetTwoDateFolders, put = SetTwoDateFolders ) )
		CKeyedCollection<CString, CString>& TwoDateFolders;

	// correcting an inconsistency in folders with end dates
	inline CKeyedCollection<CString, CString>& GetEndDateFolders()
	{
		return m_EndDateFolders;
	}
	// correcting an inconsistency in folders with end dates
	inline void SetEndDateFolders( CKeyedCollection<CString, CString>& value )
	{
		m_EndDateFolders = value;
	}
	// correcting an inconsistency in folders with end dates
	__declspec( property( get = GetEndDateFolders, put = SetEndDateFolders ) )
		CKeyedCollection<CString, CString>& EndDateFolders;

	// correcting an error in folders with no dates
	inline CKeyedCollection<CString, CString>& GetNoDateFolders()
	{
		return m_NoDateFolders;
	}
	// correcting an error in folders with no dates
	inline void SetNoDateFolders( CKeyedCollection<CString, CString>& value )
	{
		m_NoDateFolders = value;
	}
	// correcting an error in folders with no dates
	__declspec( property( get = GetNoDateFolders, put = SetNoDateFolders ) )
		CKeyedCollection<CString, CString>& NoDateFolders;


// Operations
public:
	// Helper to convert Degrees, Minutes, and Seconds (DMS) to decimal degrees
	double ConvertDMSToDecimal(Gdiplus::PropertyItem* item, char ref);

	// Main function to generate Google Maps link
	CString GenerateMapLink(const CString& imagePath, bool bBing = true);

	// parses lines of shortcut definitions:
	//		"@key data"
	// and maps the data for rapid searches
	// and guaranteed unique keys
	CString MapShortcuts
	( 
		CString source,
		CKeyedCollection<CString, CString>& mapShortcuts
	);
	// shortcuts in the source string are expanded to 
	// their real values and the resolved string is
	// returned with all shortcuts replaced
	CString ResolveShortcuts( CString source );
	CString GetRealPath( const CString& symbolicPath );
	bool FindAlbum();
	int FindAlbumFolders();
	int FindNoDateFolders();
	int FindTwoDateFolders();
	int FindEndDateFolders();
	vector<CString> ReadImageDetails( CString csImage );
	vector<CString> ParseAlbumDetails( CString csFolder );
	BOOL OnOpenDocument( LPCTSTR lpszPathName );
	void FindInterestingFiles( bool bStatus = true );
	void UpdateDocumentTabs();
	void PopulateList( CThumbnailDialog& dlg );
	bool WriteAlbumXml( const CString& filePath );
	bool ReadAlbumXml( const CString& filePath );
	void UpdateTabText();
	void WriteAlbumToImage
	(
		const CString& csKey, // property key
		CString& csValue, // value to write
		bool bPresent, // original value is present
		unique_ptr<Gdiplus::Image>& pImage 
	);
	void WriteAlbumToBitmap
	(
		CString& csKey, // property key
		CString& csValue, // value to write
		bool bPresent, // original value is present
		unique_ptr<Gdiplus::Bitmap>& pBitmap 
	);
	// create a thumbnail from given path to an image
	bool CreateThumbnail
	(
		CString& imagePath, shared_ptr<Gdiplus::Bitmap>& pThumbnail
	);

    bool SaveWithDetails( CString& csPath, Gdiplus::Image* pImage );

    // write information from image detail text files
	unique_ptr<Gdiplus::Image> WriteImageDetails( CString& csImage );

	void StartMonitorThread();
	void StopMonitorThread();
	static UINT MonitorDirectoryThread( LPVOID pParam );

	// get the date taken, if any, from the given bitmap
	CString GetDateTaken
	(
		shared_ptr<Bitmap>& pBitmap,
		CDate& date
	);
	// rename the given file by the date taken property in the date parameter
	CString RenameFile( LPCTSTR lpszPathName, CDate& date );

// Overrides
public:
	// open a data file
	virtual bool Open
	( 
		LPCTSTR szFilename, // name of data file to open
		bool bRead = true, // do an initial read after open
		LPCTSTR pcszFileID = _T( "F1" )
	);

	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CPhotoExplorerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnAlbumTitle();
	afx_msg void OnUpdateAlbumTitle( CCmdUI* pCmdUI );
	afx_msg void OnAlbumLocation();
	afx_msg void OnUpdateAlbumLocation( CCmdUI* pCmdUI );
	afx_msg void OnAlbumComment();
	afx_msg void OnUpdateAlbumComment( CCmdUI* pCmdUI );
	afx_msg void OnAlbumDate();
	afx_msg void OnUpdateAlbumDate( CCmdUI* pCmdUI );
	afx_msg void OnAlbumArtist();
	afx_msg void OnUpdateAlbumArtist( CCmdUI* pCmdUI );
	afx_msg void OnAlbumCopyright();
	afx_msg void OnUpdateAlbumCopyright( CCmdUI* pCmdUI );
	afx_msg void OnAlbumSoftware();
	afx_msg void OnUpdateAlbumSoftware( CCmdUI* pCmdUI );
	afx_msg void OnAlbumKeywords();
	afx_msg void OnUpdateAlbumKeywords( CCmdUI* pCmdUI );
	afx_msg void OnEditExportLabels();
	afx_msg void OnUpdateEditExportLabels( CCmdUI* pCmdUI );
    afx_msg void OnEditShortcuts();
    afx_msg void OnUpdateEditShortcuts( CCmdUI* pCmdUI );
	afx_msg void OnEditAlbumLabel();
	afx_msg void OnUpdateEditAlbumLabel( CCmdUI* pCmdUI );
	void OnEditRename();
	afx_msg void OnUpdateEditRename( CCmdUI* pCmdUI );
    virtual void OnCloseDocument();
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
    virtual BOOL SaveModified();
	virtual BOOL IsModified() 
	{ 
		return FALSE;
	};
	virtual void SetModifiedFlag(BOOL bModified = TRUE)
	{
		CDocument::SetModifiedFlag(FALSE);
	}

};

/////////////////////////////////////////////////////////////////////////////
