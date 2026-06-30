/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ShellListView.h"
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CPhotoExplorerView
//
// The primary folder and file browsing view for Photo Explorer. This class
// extends CShellListView to provide a rich, Explorer-style interface for
// navigating image folders, selecting images, performing file operations,
// and launching metadata-related actions such as map links and property
// editing. It serves as the main list-based UI component of the application.
//
// Purpose:
//   • Display the contents of the active folder using an Explorer-style list.
//   • Support multiple view modes (large icons, small icons, list, details).
//   • Handle selection, context menus, and file operations (cut/copy/paste/delete).
//   • Integrate with the document to reflect current selections and folder state.
//   • Provide quick access to Google/Bing map links for GPS-tagged images.
//   • Provide UI entry points for metadata label editing and property display.
//
// Why this class exists:
//   Photo Explorer requires a fast, familiar, and flexible way to browse
//   folders of images. CShellListView provides a shell-integrated list control,
//   but CPhotoExplorerView adds application-specific behavior such as album
//   metadata integration, map commands, selection tracking, and custom view
//   switching. This class is the user’s primary interface for navigating
//   image collections.
//
// Responsibilities:
//   • Maintain the current list view mode (m_nCurrentView).
//   • Track cut/copy buffers for file operations (m_arrCut, m_arrCopy).
//   • Handle right-click context menus and command routing.
//   • Provide view-switching commands:
//       – Large Icons
//       – Small Icons
//       – List
//       – Details
//   • Forward selection changes to the document.
//   • Support cut/copy/paste/delete/select-all operations.
//   • Provide map commands (OnGoogleMap, OnBingMap) using GPS metadata.
//   • Provide metadata label editing commands (OnPropertiesLabel).
//   • Integrate with the document to retrieve thumbnails, metadata, and paths.
//
// Interaction with other components:
//   • CPhotoExplorerDoc — supplies folder contents, thumbnails, and metadata.
//   • CMainFrame — coordinates docking panes and UI updates.
//   • CImageView — displays the selected image when the user clicks an item.
//   • COutputWnd — receives progress/warning/error messages from operations.
//   • Metadata and GPS subsystems — used for map link generation.
//   • Shell list control — provides native folder browsing behavior.
//
// Key Features:
//   • Full Explorer-style browsing with multiple view modes.
//   • Context menu support for file operations and metadata actions.
//   • Cut/copy/paste/delete/select-all functionality integrated with the shell.
//   • GPS map integration for Bing and Google Maps.
//   • Property label editing for metadata-driven workflows.
//   • Automatic UI updates when folder contents change.
//   • PreTranslateMessage override for keyboard shortcuts and navigation.
//
// Internal Structure:
//   • m_nCurrentView — current list view mode.
//   • m_arrCut — list of items pending cut operation.
//   • m_arrCopy — list of items pending copy operation.
//   • OnInitialUpdate — initializes the shell list view after creation.
//   • View-switching handlers (OnViewLargeicon, OnViewList, etc.).
//   • Edit command handlers (OnEditCut, OnEditCopy, OnEditPaste, OnEditDelete).
//   • Map command handlers (OnGoogleMap, OnBingMap).
//   • Metadata label command handler (OnPropertiesLabel).
//
// This class provides the main navigation interface for Photo Explorer,
// enabling users to browse folders, select images, perform file operations,
// and access metadata-driven features with speed and clarity.
/////////////////////////////////////////////////////////////////////////////
class CPhotoExplorerView : public CShellListView
{
protected:
	// create from serialization only
	CPhotoExplorerView() noexcept;
	DECLARE_DYNCREATE(CPhotoExplorerView)

// protected data
protected:
	int m_nCurrentView;
	vector<CString> m_arrCut;
	vector<CString> m_arrCopy;

// public properties
public:
	// the list control's current view
	int GetCurrentView()
	{
		return m_nCurrentView;
	}
	void SetCurrentView( int value )
	{
		m_nCurrentView = value;
	}
	// Get the pointer for the list control.
	__declspec( property( get = GetCurrentView, put = SetCurrentView ) )
		int CurrentView;

// Attributes
public:
	CPhotoExplorerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();

protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CPhotoExplorerView();
	BOOL PreTranslateMessage( MSG* pMsg );
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewLargeicon();
	afx_msg void OnUpdateViewLargeicon( CCmdUI* pCmdUI );
	afx_msg void OnViewList();
	afx_msg void OnUpdateViewList( CCmdUI* pCmdUI );
	afx_msg void OnViewSmallicon();
	afx_msg void OnUpdateViewSmallicon( CCmdUI* pCmdUI );
	afx_msg void OnViewDetails();
	afx_msg void OnUpdateViewDetails( CCmdUI* pCmdUI );

    afx_msg void OnEditCut();
    afx_msg void OnEditCopy();
    afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditCut( CCmdUI* pCmdUI );
	afx_msg void OnUpdateEditCopy( CCmdUI* pCmdUI );
	afx_msg void OnUpdateEditPaste( CCmdUI* pCmdUI );
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete( CCmdUI* pCmdUI );
    afx_msg void OnEditSelectAll();
    afx_msg void OnUpdateEditSelectAll( CCmdUI* pCmdUI );
    afx_msg void OnBingMap();
    afx_msg void OnUpdateBingMap(CCmdUI* pCmdUI);
    afx_msg void OnGoogleMap();
    afx_msg void OnUpdateGoogleMap(CCmdUI* pCmdUI);
    afx_msg void OnPropertiesLabel();
    afx_msg void OnUpdatePropertiesLabel(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////
#ifndef _DEBUG  // debug version in PhotoExplorerView.cpp
inline CPhotoExplorerDoc* CPhotoExplorerView::GetDocument() const
   { return reinterpret_cast<CPhotoExplorerDoc*>(m_pDocument); }
#endif

/////////////////////////////////////////////////////////////////////////////
