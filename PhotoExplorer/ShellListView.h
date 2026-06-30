/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ShellListCtrl.h"
#include "KeyedCollection.h"
#include "PlusGDI.h"
#include "ImageProperties.h"
#include "PropertiesWnd.h"

using namespace Gdiplus;

/////////////////////////////////////////////////////////////////////////////
class CPhotoExplorerDoc;

/////////////////////////////////////////////////////////////////////////////
// CShellListView
//
// Shell-integrated list view used by Photo Explorer to display the contents
// of the active folder. This class extends CListView and embeds a custom
// CShellListCtrl, an image list for thumbnails, and a full CImageProperties
// object for metadata extraction. It serves as the primary UI component for
// browsing images, selecting items, updating thumbnails, and coordinating
// metadata with the document.
//
// Purpose:
//   • Display folder contents using a shell-style list control.
//   • Maintain and display thumbnails for each image.
//   • Track selected items and synchronize selection with the document.
//   • Provide metadata extraction via CImageProperties for the selected image.
//   • Handle folder-change events (add/delete) from the monitor thread.
//   • Provide progress/warning/error logging to the output pane.
//   • Support album metadata fallback logic (title, location, comment, date).
//
// Why this class exists:
//   Photo Explorer requires a fast, responsive, shell-like list view that
//   integrates tightly with metadata, thumbnails, and album logic. MFC’s
//   CListView provides the foundation, but CShellListView adds:
//     – Thumbnail caching and display
//     – Metadata extraction for selected items
//     – Integration with CPhotoExplorerDoc
//     – Album fallback logic
//     – Folder monitoring event handling
//     – Selection synchronization across views
//   This class is the backbone of the folder browsing experience.
//
// Responsibilities:
//   • Host the shell list control (m_ListCtrl).
//   • Maintain an image list for thumbnails (m_ImageList).
//   • Store and manage metadata for the selected image (m_ImageProperties).
//   • Track the currently selected item index (m_nSelectedItem).
//   • Provide accessors for:
//       – Document
//       – ListControl
//       – ImageList
//       – ImageProperties
//       – ThumbnailSize
//       – SelectedImage
//       – SelectedImages (multi-selection)
//   • Provide album fallback flags (AlbumTitle, AlbumLocation, etc.).
//   • Populate the list with images from the document (PopulateList).
//   • Add individual images to the list (AddImageToList).
//   • Clear list contents and cached thumbnails (ClearItems, ClearCache).
//   • Update document tabs when selection changes (UpdateDocumentTabs).
//   • Handle thumbnail update messages (OnUpdateThumbnail).
//   • Handle folder add/delete events from the monitor thread.
//
// Interaction with other components:
//   • CPhotoExplorerDoc — supplies folder contents, thumbnails, metadata,
//     album information, and selection state.
//   • CImageProperties — extracts metadata for the selected image.
//   • CMainFrame — coordinates docking panes and UI updates.
//   • CPropertiesWnd — displays metadata for the selected image.
//   • COutputWnd — receives progress/warning/error messages.
//   • CShellListCtrl — underlying list control for shell-style browsing.
//   • GDI+ — used for thumbnail generation and image loading.
//
// Key Features:
//   • Full thumbnail management with dynamic sizing.
//   • Multi-selection support with add/remove helpers.
//   • Album metadata fallback logic for title, location, comment, and date.
//   • Shell-style list control with custom drawing and selection behavior.
//   • Real-time folder monitoring integration (add/delete events).
//   • Automatic synchronization with the document and properties pane.
//   • Progress/warning/error logging routed through the document.
//
// Internal Structure:
//   • m_ListCtrl — custom list control for shell-style browsing.
//   • m_ImageList — thumbnail image list.
//   • m_ImageProperties — metadata extractor for selected image.
//   • m_uiThumbnailSize — current thumbnail size.
//   • m_nSelectedItem — index of the selected item.
//   • OnInitialUpdate — initializes list control and image list.
//   • OnUpdateThumbnail — updates thumbnails when generated.
//   • OnFolderAddEvent / OnFolderDeleteEvent — respond to monitor thread events.
//   • SelectItemByPath — programmatically select an item in the list.
//   • ClearCache — clears thumbnail and metadata caches.
//
// This class provides the core folder-browsing interface for Photo Explorer,
// integrating thumbnails, metadata, album logic, and shell-style navigation
// into a single cohesive view.
/////////////////////////////////////////////////////////////////////////////
class CShellListView : public CListView
{
	DECLARE_DYNCREATE(CShellListView)

// protected data
protected:
	CShellListCtrl m_ListCtrl;
	CImageList m_ImageList;
	UINT m_uiThumbnailSize;
	CImageProperties m_ImageProperties;
	int m_nSelectedItem;

// public properties
public:
	// get a pointer to the document
	CPhotoExplorerDoc* GetDocument();
	// get a pointer to the document
	__declspec( property( get = GetDocument ) )
		CPhotoExplorerDoc* Document;

	// Get the pointer for the list control.
	CShellListCtrl* GetListControl()
	{
		return &m_ListCtrl;
	}
	// Get the pointer for the list control.
	__declspec( property( get = GetListControl ) )
		CShellListCtrl* ListControl;

	// Get the pointer for the image list
	CImageList* GetImageList()
	{
		return &m_ImageList;
	}
	// Get the pointer for the image list
	__declspec( property( get = GetImageList ) )
		CImageList* ImageList;

	// a collection of image properties
	inline CImageProperties* GetImageProperties()
	{
		return &m_ImageProperties;
	}
	// a collection of image properties
	__declspec( property( get = GetImageProperties ) )
		CImageProperties* ImageProperties;

	// height and width of image thumbnails
	UINT GetThumbnailSize()
	{
		return m_uiThumbnailSize;
	}
	// height and width of image thumbnails
	void SetThumbnailSize( UINT value )
	{
		m_uiThumbnailSize = value;
	}
	// height and width of image thumbnails
	__declspec( property( get = GetThumbnailSize, put = SetThumbnailSize ) )
		UINT ThumbnailSize;

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

	// the selected image path
	CString GetSelectedImage();
	// the selected image path
	void SetSelectedImage( CString value );
	// the selected image path
	__declspec( property( get = GetSelectedImage, put = SetSelectedImage ) )
		CString SelectedImage;

	// the number of selected images
	USHORT GetSelectedImageCount()
	{
		USHORT value = 0;
		// Get selected items and add to m_CutFileList
		POSITION pos = ListControl->GetFirstSelectedItemPosition();
		while (pos)
		{
			value++;
			int nItem = ListControl->GetNextSelectedItem(pos);
		}

		return value;
	}
	// the number of selected images
	__declspec( property( get = GetSelectedImageCount) )
		USHORT SelectedImageCount;

	// the collection of selected image paths
	CKeyedCollection<CString, int>& GetSelectedImages();
	// the collection of selected image paths
	__declspec( property( get = GetSelectedImages ) )
		CKeyedCollection<CString, int>& SelectedImages;

	// add a path to the selected images with the special
	// case of an empty value which will clear the collection
	void SetAddSelectedImage( CString value );
	// add a path to the selected images with the special
	// case of an empty value which will clear the collection
	__declspec( property( put = SetAddSelectedImage ) )
		CString AddSelectedImage;

	// remove a path from the selected images 
	void SetRemoveSelectedImage( CString value );

	// select an image in the control
	void SelectItemByPath( const CString& path );

	// remove a path from the selected images 
	__declspec( property( put = SetRemoveSelectedImage ) )
		CString RemoveSelectedImage;

	// if true, the title is replaced with the album title
	// if the image title is blank
	bool GetAlbumTitle();
	// if true, the title is replaced with the album title
	// if the image title is blank
	__declspec( property( get = GetAlbumTitle ) )
		bool AlbumTitle;

	// if true, the location is replaced with the album location
	// if the image location is blank
	bool GetAlbumLocation();
	// if true, the location is replaced with the album location
	// if the image location is blank
	__declspec( property( get = GetAlbumLocation ) )
		bool AlbumLocation;

	// if true, the comment is replaced with the album comment
	// if the image comment is blank
	inline bool GetAlbumComment();
	// if true, the comment is replaced with the album comment
	// if the image comment is blank
	__declspec( property( get = GetAlbumComment ) )
		bool AlbumComment;

	// if true, the date is replaced with the album date
	// if the image date is blank
	inline bool GetAlbumDate();
	// if true, the date is replaced with the album date
	// if the image date is blank
	__declspec( property( get = GetAlbumDate ) )
		bool AlbumDate;

// public methods
public:
	void UpdateDocumentTabs();

	void AddImageToList( const CString& imagePath );

	void PopulateList();

	void ClearImageList();

	void ClearItems();

	void ClearCache();

// protected methods
protected:
	CShellListView();
	virtual ~CShellListView();

// public virtual
public:
	virtual void OnInitialUpdate();

#ifdef _DEBUG
	virtual void AssertValid() const;
	#ifndef _WIN32_WCE
		virtual void Dump(CDumpContext& dc) const;
	#endif
#endif

// protected message handlers
protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg int OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg LRESULT OnUpdateThumbnail( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnFolderDeleteEvent( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnFolderAddEvent( WPARAM wParam, LPARAM lParam );
	afx_msg void OnTimer( UINT_PTR nIDEvent );
};

/////////////////////////////////////////////////////////////////////////////
#ifndef _DEBUG  // debug version in ImageView.cpp
inline CPhotoExplorerDoc* CShellListView::GetDocument()
{
	return reinterpret_cast<CPhotoExplorerDoc*>( m_pDocument );
}
#endif

/////////////////////////////////////////////////////////////////////////////
