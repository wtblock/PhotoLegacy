/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "PropertyGridCtrl.h"

/////////////////////////////////////////////////////////////////////////////
class CPropertiesToolBar : public CMFCToolBar
{
public:
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

/////////////////////////////////////////////////////////////////////////////
class CPropertiesWnd : public CDockablePane
{

// protected data
protected:
	CFont m_fntPropList;
	CComboBox m_wndObjectCombo;
	CPropertiesToolBar m_wndToolBar;
	CPropertyGridCtrl m_wndPropList;
	int m_nComboHeight;

// public properties
public:
	// the list control displaying the properties 
	inline CPropertyGridCtrl* GetPropList()
	{
		return &m_wndPropList;
	}
	// the list control displaying the properties 
	__declspec( property( get = GetPropList ) )
		CPropertyGridCtrl* PropList;

// public methods
public:
	void AdjustLayout();

	void SetVSDotNetLook(BOOL bSet)
	{
		m_wndPropList.SetVSDotNetLook(bSet);
		m_wndPropList.SetGroupNameFullWidth(bSet);
	}

	void Clear()
	{
		m_wndPropList.RemoveAll();
		m_wndPropList.RedrawWindow();
		AdjustLayout();
		Invalidate();
	}

// protected methods
protected:
	void InitPropList();
	void SetPropListFont();

	// handler for a property change
	afx_msg LRESULT OnPropertyChange
	( 
		WPARAM wp, // not used
		LPARAM lp // property grid property pointer
	);

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnExpandAllProperties();
	afx_msg void OnUpdateExpandAllProperties(CCmdUI* pCmdUI);
	afx_msg void OnSortProperties();
	afx_msg void OnUpdateSortProperties(CCmdUI* pCmdUI);
	afx_msg void OnImageLabel();
	afx_msg void OnUpdateImageLabel(CCmdUI* pCmdUI);
	afx_msg void OnPropertiesCancel();
	afx_msg void OnUpdatePropertiesCancel(CCmdUI* pCmdUI);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);

	DECLARE_MESSAGE_MAP()

// protected overrides
protected:

// public overrides
public:

// public constructor/destructor
public:
	CPropertiesWnd() noexcept;
	virtual ~CPropertiesWnd();

	afx_msg void OnButtonPreview();
	afx_msg void OnUpdateButtonPreview( CCmdUI* pCmdUI );
	afx_msg void OnButtonExport();
	afx_msg void OnUpdateButtonExport( CCmdUI* pCmdUI );
};

/////////////////////////////////////////////////////////////////////////////
