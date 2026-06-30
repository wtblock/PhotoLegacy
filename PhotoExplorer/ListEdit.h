/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CListEdit
//
// Lightweight in‑place editor used for editing text inside list controls.
// This class is typically created dynamically when the user begins editing
// a specific item/subitem in a list view. It behaves like a standard CEdit
// control but automatically commits its value back to the parent list and
// destroys itself when editing is complete.
//
// Purpose:
//   • Provide inline editing for list view cells (item + subitem).
//   • Capture user input and send the edited text back to the parent control.
//   • Cleanly self‑destruct after editing to avoid memory leaks.
//   • Handle Enter/Escape keys to finalize or cancel editing.
//
// Why this class exists:
//   MFC’s built‑in list controls do not provide a convenient mechanism for
//   editing subitems directly. CListEdit fills this gap by creating a temporary
//   edit control positioned over the cell being edited. Once the user finishes,
//   the control sends a notification message and deletes itself.
//
// Responsibilities:
//   • Store the item index, subitem index, and initial text.
//   • Display an edit box over the list cell being edited.
//   • On focus loss, send WM_USER_END_LABEL_EDIT to the parent with the new text.
//   • Destroy itself cleanly (OnNcDestroy).
//   • Handle Enter/Escape keys to end editing gracefully.
//
// Interaction with other components:
//   • Parent list control receives WM_USER_END_LABEL_EDIT and updates the cell.
//   • Photo Explorer uses this for renaming album entries, editing metadata
//     lists, or any UI element requiring inline text editing.
//   • Works seamlessly with CListCtrl or custom list views.
//
// Key Features:
//   • Self‑contained lifecycle: created dynamically, commits value, deletes itself.
//   • Keyboard handling for Enter/Escape.
//   • Focus‑based commit logic (OnKillFocus).
//   • No memory leaks — object deletes itself after window destruction.
//
// Internal Behavior:
//   • OnKillFocus:
//       – Retrieves the edited text.
//       – Sends WM_USER_END_LABEL_EDIT to the parent with item/subitem info.
//       – Calls DestroyWindow().
//   • OnNcDestroy:
//       – Deletes the CListEdit object.
//   • OnChar:
//       – Handles Enter/Escape to end editing.
//       – Otherwise passes input to CEdit.
//
// This class provides a simple, reliable mechanism for inline list editing,
// enhancing Photo Explorer’s UI by allowing users to edit text directly within
// list controls without dialog boxes or extra UI steps.
/////////////////////////////////////////////////////////////////////////////
class CListEdit : public CEdit
{
public:
	CListEdit( int iItem, int iSubItem, CString sInitText );
	virtual ~CListEdit();

protected:
	int m_iItem;
	int m_iSubItem;
	CString m_sInitText;

	afx_msg void OnKillFocus( CWnd* pNewWnd );
	afx_msg void OnNcDestroy();
	afx_msg void OnChar( UINT nChar, UINT nRepCnt, UINT nFlags );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
