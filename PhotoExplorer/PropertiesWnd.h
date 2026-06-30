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
// CPropertiesWnd
//
// Dockable pane that displays and edits metadata properties for the selected
// image. This pane hosts the property grid control used throughout Photo
// Explorer to present EXIF/GDI+ metadata in a structured, grouped, and
// user-friendly format. It provides editing, sorting, expansion, and preview
// capabilities, and integrates tightly with CImageProperties to reflect
// real-time metadata changes.
//
// Purpose:
//   • Display all metadata properties for the selected image.
//   • Provide a property grid with grouping, sorting, and editing support.
//   • Handle property-change notifications and write updates back to the
//     underlying image metadata.
//   • Provide toolbar commands for expanding, sorting, previewing, and
//     exporting metadata labels.
//   • Integrate with the docking system for flexible UI layout.
//
// Why this pane exists:
//   Photo Explorer is metadata-driven. Users need a clear, organized, and
//   editable view of all EXIF properties, including camera settings, GPS
//   coordinates, thumbnail info, XP tags, and album labels. MFC’s property
//   grid provides a powerful foundation, but CPropertiesWnd adds application-
//   specific behavior such as album label editing, preview/export commands,
//   and integration with CImageProperties.
//
// Responsibilities:
//   • Host and manage the property grid control (m_wndPropList).
//   • Provide a toolbar for property-related commands (expand, sort, preview,
//     export).
//   • Handle property-change messages (OnPropertyChange) and forward updates
//     to the document/image metadata system.
//   • Adjust layout dynamically when resized (AdjustLayout).
//   • Support Visual Studio–style appearance (SetVSDotNetLook).
//   • Clear and refresh the property grid when the selected image changes.
//   • Maintain font and combo-box height for consistent UI appearance.
//
// Interaction with other components:
//   • CImageProperties — supplies property definitions, types, descriptions,
//     and enumerations.
//   • CPhotoExplorerDoc — provides the selected image and metadata context.
//   • CMainFrame — hosts the pane and coordinates docking behavior.
//   • CPropertyGridCtrl — underlying grid control for property display.
//   • CPropertiesToolBar — toolbar for property commands.
//   • CImageView — uses property values for label preview/export.
//
// Key Features:
//   • Full property grid with grouping, sorting, and editing.
//   • Expand-all and sort-by-category commands.
//   • Preview and export buttons for metadata label rendering.
//   • Property-change notifications routed through OnPropertyChange.
//   • Visual Studio–style appearance option.
//   • Dynamic resizing and layout adjustment.
//   • Clear() method for resetting the grid when no image is selected.
//
// Internal Structure:
//   • m_wndPropList — main property grid control.
//   • m_wndToolBar — toolbar for property commands.
//   • m_wndObjectCombo — optional object selector (reserved for future use).
//   • m_fntPropList — font used for property grid text.
//   • m_nComboHeight — height of the combo box area.
//   • InitPropList — initializes grid settings and categories.
//   • SetPropListFont — applies consistent font styling.
//   • OnPropertyChange — handles edits made by the user.
//
// This pane provides the metadata editing backbone of Photo Explorer,
// enabling users to inspect, modify, preview, and export image properties
// with clarity and precision.
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
