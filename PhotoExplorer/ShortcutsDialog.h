/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include "ListEdit.h"

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
