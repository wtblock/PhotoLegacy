/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "PropertyGridCtrl.h"

/////////////////////////////////////////////////////////////////////////////
class CPhotoPrinterView;
class CPhotoPrinterDoc;

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
	CPropertiesToolBar m_wndToolBar;
	CPropertyGridCtrl m_wndPropList;
	int m_nComboHeight;
	CMFCPropertyGridProperty* m_pTableOfContents;

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

	// Get the active document.
	CPhotoPrinterDoc* GetPhotoPrinterDocument();
	// Get the active document.
	__declspec(property(get = GetPhotoPrinterDocument))
		CPhotoPrinterDoc* PhotoPrinterDocument;

	// Get the pointer for the image view.
	CPhotoPrinterView* GetPhotoPrinterView();
	// Get the pointer for the image view.
	__declspec(property(get = GetPhotoPrinterView))
		CPhotoPrinterView* PhotoPrinterView;


// public methods
public:
	void UpdateTableOfContents();

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

	void UpdatePropertiesFromDocument(CPhotoPrinterDoc* pDoc);

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
	afx_msg void OnGoTo();
	afx_msg void OnUpdateGoTo(CCmdUI* pCmdUI);
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

};

/////////////////////////////////////////////////////////////////////////////
