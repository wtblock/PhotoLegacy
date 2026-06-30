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
// CPropertiesWnd
//
// Dockable Properties Pane used to display and edit all document-level
// metadata for PhotoPrinter. Although the MFC Application Wizard generated
// the initial docking pane and toolbar scaffolding, the class has evolved
// into a central UI component that synchronizes user-editable properties
// with the underlying CPhotoPrinterDoc.
//
// Purpose:
//   • Provide a Visual Studio–style dockable pane containing a property grid.
//   • Allow the user to edit title page metadata, folder range, query text,
//     and export settings directly from the UI.
//   • Display a dynamic Table of Contents generated from the document’s
//     album structure.
//   • Support navigation: selecting a TOC entry jumps the view to that page.
//
// Why this class exists:
//   The MFC Wizard creates a placeholder “Properties Window” for docking
//   applications, but PhotoPrinter requires a fully functional metadata
//   editor. CPropertiesWnd extends the wizard-generated pane with:
//       – A custom property grid (CPropertyGridCtrl)
//       – Document synchronization (UpdatePropertiesFromDocument)
//       – Change handling (OnPropertyChange)
//       – Dynamic TOC population (UpdateTableOfContents)
//       – Page navigation (OnGoTo)
//       – Layout management and toolbar integration
//
// Responsibilities:
//   • Build and initialize the property grid (InitPropList).
//   • Reflect document state in the UI (UpdatePropertiesFromDocument).
//   • Push user edits back into the document (OnPropertyChange).
//   • Rebuild the Table of Contents when the document changes.
//   • Navigate the view when the user selects a TOC entry.
//   • Maintain docking pane layout and appearance.
//   • Provide a consistent editing experience for all document metadata.
//
// Interaction with other components:
//   • Reads and writes properties on CPhotoPrinterDoc.
//   • Notifies CPhotoPrinterView to refresh scrollbars and redraw when
//     metadata changes affect layout.
//   • Called by CMainFrame when documents activate (via ChildFrame).
//
// Wizard-generated portions:
//   • Dockable pane creation and toolbar setup.
//   • Basic message map and layout plumbing.
//   • Font management and docking behavior.
//
// Application-specific additions:
//   • Full property grid construction (Title Page, Folder Range, Export).
//   • Custom multiline text editor (CPropertyGridMultilineText).
//   • Dynamic Table of Contents group.
//   • Document synchronization and validation logic.
//   • Page navigation support.
//
// This pane is the user’s primary interface for configuring a PhotoPrinter
// document and plays a key role in the book-building workflow.
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
