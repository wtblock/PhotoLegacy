/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include <vector>
#include <map>

using namespace std;

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


