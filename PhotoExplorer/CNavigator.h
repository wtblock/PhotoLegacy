/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include <vector>
#include <map>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CNavigator
//
// Wrapper around CMFCShellTreeCtrl used as the Folder Explorer pane in
// Photo Explorer. This control displays the hierarchical folder structure
// of the user’s photo archive and allows navigation by year, event, and
// custom grouping. Although CMFCShellTreeCtrl is provided by the MFC
// Application Wizard, Photo Explorer extends it with symbolic folder
// resolution, custom path mapping, and metadata-aware navigation.
//
// Purpose:
//   • Provide a tree-based navigation pane for browsing photo folders.
//   • Display the user’s album hierarchy (e.g., “2022.01.06 A Walk in the Park”).
//   • Track the currently selected folder and notify the application when
//     the selection changes.
//   • Support symbolic folder names and known-folder mappings.
//   • Integrate with the document/view system to load images when a folder
//     is selected.
//
// Why this class exists:
//   The Wizard-generated CMFCShellTreeCtrl provides a basic shell folder
//   browser, but Photo Explorer requires additional capabilities:
//       – Mapping symbolic folder names to real paths
//       – Tracking the current folder selection
//       – Custom drawing for enhanced readability
//       – Integration with Photo Explorer’s album-loading pipeline
//
// Responsibilities:
//   • Maintain the current path (CurrentPath property).
//   • Convert tree items into real filesystem paths (GetItemPath).
//   • Resolve known folders (GetKnownFolderPath / GetFolderDisplayName).
//   • Translate symbolic album paths into actual disk locations
//     (GetRealFolder).
//   • Handle selection-change notifications (OnTvnSelchanged) and trigger
//     document updates.
//   • Provide optional custom drawing via OnNMCustomdraw.
//
// Wizard‑generated portions:
//   • Base class (CMFCShellTreeCtrl).
//   • Message map structure.
//   • PreCreateWindow override.
//   • Basic tree control initialization.
//
// Application‑specific additions:
//   • Symbolic folder mapping using CKeyedCollection.
//   • Current-path tracking.
//   • Custom selection-change handler that loads the selected folder into
//     the active document.
//   • Optional custom drawing for folder items.
//   • Helper methods for resolving known folders and symbolic paths.
//
// This class forms the backbone of Photo Explorer’s folder navigation
// system, enabling users to browse large photo archives quickly and
// intuitively while keeping the document/view system synchronized with
// the selected folder.
/////////////////////////////////////////////////////////////////////////////
// wraps the tree control CMFCShellTreeCtrl
class CNavigator : public CMFCShellTreeCtrl
{
	DECLARE_DYNAMIC(CNavigator)

public:
	//typedef map<CString, REFKNOWNFOLDERID> MAP_PAIR_ID;

// protected data
protected:
	vector<CString> m_arrItems;
	//MAP_PAIR_ID m_mapKnownIds;
	CKeyedCollection<CString, CString> m_keyIds;
	CKeyedCollection<CString, CString> m_keyKnown;
	CString m_csCurrentPath;

// public properties
public:
	// the tree control's current path
	CString GetCurrentPath()
	{
		return m_csCurrentPath;
	}
	// the tree control's current path
	void SetCurrentPath( CString value )
	{
		m_csCurrentPath = value;
	}
	// the tree control's current path
	__declspec( property( get = GetCurrentPath, put = SetCurrentPath ) )
		CString CurrentPath;

public:
	CNavigator();
	virtual ~CNavigator();
	CString GetItemPath( HTREEITEM hItem );
	CString GetKnownFolderPath( REFKNOWNFOLDERID folderId );
	CString GetFolderDisplayName( REFKNOWNFOLDERID folderId );
	CString GetRealFolder( CString csSymbolPath );

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnTvnSelchanged( NMHDR* pNMHDR, LRESULT* pResult );
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
	afx_msg void OnNMCustomdraw( NMHDR* pNMHDR, LRESULT* pResult );
};


