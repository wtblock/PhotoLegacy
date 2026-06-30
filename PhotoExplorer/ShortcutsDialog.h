/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include "ListEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CShortcutsDialog
//
// Dialog for managing user-defined metadata shortcuts in Photo Explorer.
// Shortcuts allow users to define symbolic keys (e.g., "@loc", "@event",
// "@family") that expand into full text values when entering metadata.
// This dialog provides a simple list-based editor for creating, modifying,
// and deleting these shortcuts, and persists them in the Windows registry.
//
// Purpose:
//   • Provide a UI for editing shortcut definitions used throughout Photo Explorer.
//   • Allow users to define symbolic keys that expand into full metadata text.
//   • Load and save shortcut definitions from the registry.
//   • Support in-place editing of shortcut keys and values via CListCtrl.
//   • Maintain a unique, keyed collection of shortcuts.
//
// Why this dialog exists:
//   Metadata entry can be repetitive — locations, people, events, and comments
//   often recur across many images. Shortcuts dramatically reduce typing by
//   allowing users to enter symbolic tokens that expand automatically. This
//   dialog gives users full control over those tokens, making metadata entry
//   faster, more consistent, and less error-prone.
//
// Responsibilities:
//   • Host a list control (m_listCtrl) for displaying shortcut key/value pairs.
//   • Maintain a unique collection of shortcuts (m_mapShortcuts).
//   • Load shortcuts from the registry at initialization.
//   • Save shortcuts back to the registry when the user confirms changes.
//   • Provide in-place editing via CListEdit for both key and value fields.
//   • Handle label-edit completion (OnEndLabelEdit, OnLvnEndlabeleditListCtrl).
//   • Handle double-click editing (OnDblclkListControl).
//   • Provide OK/Cancel handlers for committing or discarding changes.
//
// Interaction with other components:
//   • CPhotoExplorerDoc — uses shortcuts to expand metadata strings.
//   • CImageProperties — resolves shortcuts when writing metadata.
//   • CListEdit — provides in-place editing for list control cells.
//   • CKeyedCollection — stores shortcut key/value pairs uniquely.
//   • Windows Registry — persists shortcut definitions across sessions.
//
// Key Features:
//   • Full in-place editing of shortcut keys and values.
//   • Automatic uniqueness enforcement via CKeyedCollection.
//   • Registry persistence under:
//         HKEY_CURRENT_USER\Software\PhotoExplorer\PhotoExplorer
//   • Double-click editing for fast workflow.
//   • Clean integration with metadata expansion logic.
//   • Simple, intuitive UI for managing reusable metadata tokens.
//
// Internal Structure:
//   • m_listCtrl — list control displaying shortcut pairs.
//   • m_mapShortcuts — keyed collection storing shortcut definitions.
//   • GetRegistryBaseKey — returns the registry path used for persistence.
//   • LoadShortcutsFromRegistry — populates the list and collection.
//   • SaveShortcutsToRegistry — writes updated shortcuts back to registry.
//   • OnEndLabelEdit / OnLvnEndlabeleditListCtrl — commit edits to the collection.
//   • OnDblclkListControl — begin editing the selected shortcut.
//
// This dialog provides the user-facing interface for Photo Explorer’s shortcut
// system, enabling fast, consistent metadata entry through reusable symbolic
// tokens.
/////////////////////////////////////////////////////////////////////////////
class CShortcutsDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CShortcutsDialog)

// protected data
protected:
	CListCtrl m_listCtrl;

	// unique collection of short cuts
	CKeyedCollection<CString, CString> m_mapShortcuts;

#ifdef AFX_DESIGN_TIME
	enum
	{
		IDD = IDD_SHORTCUTS_DIALOG
	};
#endif

// public properties
public:
	// unique collection of short cuts
	CKeyedCollection<CString, CString>& GetShortcuts()
	{
		return m_mapShortcuts;
	} 
	// unique collection of short cuts
	__declspec( property( get = GetShortcuts ))
		CKeyedCollection<CString, CString>& Shortcuts;

	// the key where Photo Explorer data is stored
	CString GetRegistryBaseKey()
	{
		CString value
		(
			L"HKEY_CURRENT_USER\\Software\\PhotoExplorer\\PhotoExplorer"
		);
		return value;
	}
	// the key where Photo Explorer data is stored
	__declspec( property( get = GetRegistryBaseKey ) )
		CString RegistryBaseKey;

// protected methods
protected:
	DECLARE_MESSAGE_MAP()

// public methods
public:
	void LoadShortcutsFromRegistry();
	void SaveShortcutsToRegistry();

	afx_msg void OnOk();
	afx_msg void OnCancel();
	afx_msg LRESULT OnEndLabelEdit( WPARAM wParam, LPARAM lParam );
	afx_msg void OnLvnEndlabeleditListCtrl( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnDblclkListControl( NMHDR* pNMHDR, LRESULT* pResult );

// protected overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

// public overrides
public:

// public constructor/destructor
public:
	CShortcutsDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CShortcutsDialog();

};

/////////////////////////////////////////////////////////////////////////////
