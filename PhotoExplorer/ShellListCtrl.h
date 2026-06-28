/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <afxshelllistctrl.h>

class CShellListCtrl : public CListCtrl
{
public:

protected:

	DECLARE_MESSAGE_MAP()
	
	LRESULT OnInitControl( WPARAM wParam, LPARAM lParam );

public:
	void DeselectAllItems();
	afx_msg void OnLvnItemchanged( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnNMCustomdraw( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg void SelectAllItems();
};

