/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <afxshelllistctrl.h>

/////////////////////////////////////////////////////////////////////////////
// CShellListCtrl
//
// Custom list control used by Photo Explorer to provide enhanced selection,
// drawing, and interaction behavior beyond what the standard MFC CListCtrl
// offers. This class is typically embedded inside CShellListView and provides
// the low-level event handling needed for shell-style list operations,
// including custom selection logic, background erasing, and item-change
// notifications.
//
// Purpose:
//   • Provide a specialized list control for folder and image browsing.
//   • Support custom selection operations (select all, deselect all).
//   • Handle item-change notifications for integration with the document.
//   • Provide custom drawing behavior for improved UI appearance.
//   • Integrate with shell-style list views while allowing application-specific
//     behavior.
//
// Why this class exists:
//   CShellListView wraps a shell-integrated list control, but the underlying
//   CListCtrl still requires custom behavior for Photo Explorer’s needs:
//     – Custom selection logic for multi-selection workflows.
//     – Custom drawing for thumbnails, highlight states, or themed UI.
//     – Clean deselection and select-all operations.
//     – Notification routing to the document and views.
//   CShellListCtrl encapsulates these enhancements cleanly.
//
// Responsibilities:
//   • Handle initialization messages (OnInitControl).
//   • Provide SelectAllItems and DeselectAllItems helpers.
//   • Handle LVN_ITEMCHANGED to update selection state.
//   • Provide custom drawing via NM_CUSTOMDRAW.
//   • Override background erasing for flicker-free rendering.
//
// Interaction with other components:
//   • CPhotoExplorerView — uses this control for folder browsing.
//   • CPhotoExplorerDoc — receives selection updates and uses them to update
//     thumbnails, metadata, and the image view.
//   • CImageView — displays the selected image when the list selection changes.
//   • MFC shell list infrastructure — provides folder enumeration and icons.
//
// Key Features:
//   • Full support for select-all and deselect-all operations.
//   • Custom-draw handler for enhanced UI appearance.
//   • Flicker-free background erasing.
//   • Clean integration with shell-style list views.
//   • Notification routing for item-change events.
//
// Internal Structure:
//   • OnInitControl — performs control initialization after creation.
//   • OnLvnItemchanged — handles selection changes.
//   • OnNMCustomdraw — provides custom drawing logic.
//   • OnEraseBkgnd — reduces flicker during redraw.
//   • SelectAllItems / DeselectAllItems — convenience methods for bulk selection.
//
// This class provides the enhanced list-control behavior needed for Photo
// Explorer’s shell-style browsing interface, ensuring smooth selection,
// drawing, and interaction throughout the application.
/////////////////////////////////////////////////////////////////////////////
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

