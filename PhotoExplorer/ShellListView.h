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
