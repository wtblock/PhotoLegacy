/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <afxpropertygridctrl.h>
#include "CHelper.h"

#define WM_SELECTALL (WM_USER + 100)

class CPropertiesWnd;

/////////////////////////////////////////////////////////////////////////////
// CPropertyGridCtrl
//
// Custom property grid control used by Photo Explorer to display and edit
// metadata properties. This class extends CMFCPropertyGridCtrl with additional
// capabilities needed for EXIF/GDI+ metadata editing, including flattening
// hierarchical property structures, detecting value changes, routing selection
// events, and communicating with the host pane (CPropertiesWnd).
//
// Purpose:
//   • Provide a metadata-aware property grid for EXIF/GDI+ properties.
//   • Flatten hierarchical property groups into a linear list for iteration,
//     validation, and update operations.
//   • Detect changes to property values using type-aware comparison logic.
//   • Route selection changes and “select all” commands to the host pane.
//   • Integrate tightly with CPropertiesWnd for property-change handling.
//
// Why this class exists:
//   CMFCPropertyGridCtrl provides a powerful base, but Photo Explorer requires
//   additional behavior:
//     – Flattening nested EXIF groups into a simple list for processing.
//     – Type-aware comparison of old/new values (string, integer, boolean,
//       floating-point, date).
//     – Custom message handling (WM_SELECTALL).
//     – Coordination with the host pane for metadata updates.
//   CPropertyGridCtrl encapsulates these behaviors cleanly.
//
// Responsibilities:
//   • Maintain a pointer to the host pane (m_pHost).
//   • Build a flat list of all editable properties (BuildFlatList).
//   • Recursively traverse property groups (AddPropertyRecursive).
//   • Provide type-aware comparison via ValuesAreDifferent.
//   • Handle “select all” operations (OnSelectAll).
//   • Override EndEditItem to finalize edits and notify the host.
//   • Override PreTranslateMessage for keyboard shortcuts.
//   • Override OnChangeSelection to update UI state.
//
// Interaction with other components:
//   • CPropertiesWnd — receives property-change notifications and updates
//     metadata accordingly.
//   • CImageProperties — supplies property definitions, types, and metadata.
//   • CMFCPropertyGridProperty — underlying property objects.
//   • CHelper — assists with converting MFC arrays to STL vectors.
//
// Key Features:
//   • Full recursive flattening of property hierarchy.
//   • Type-aware value comparison for detecting metadata changes.
//   • Host-aware behavior for property-change routing.
//   • Keyboard handling for selection and editing.
//   • Clean integration with the docking properties pane.
//
// Internal Structure:
//   • m_pHost — pointer to the owning CPropertiesWnd.
//   • BuildFlatList — produces a vector or CArray of all properties.
//   • AddPropertyRecursive — traverses groups and subitems.
//   • ValuesAreDifferent — compares COleVariant values by type.
//   • EndEditItem — commits edits and triggers host notifications.
//   • PreTranslateMessage — handles WM_SELECTALL and keyboard shortcuts.
//   • OnChangeSelection — updates selection state in the host.
//
// This class provides the metadata-editing intelligence behind Photo Explorer’s
// property pane, enabling structured, type-safe, and responsive editing of
// EXIF/GDI+ metadata.
/////////////////////////////////////////////////////////////////////////////
class CPropertyGridCtrl : public CMFCPropertyGridCtrl
{
// protected data
protected:
	CPropertiesWnd* m_pHost;

// public properties
public:
	// get a pointer to the host
	CPropertiesWnd* GetHost();
	void SetHost( CPropertiesWnd* value );
	__declspec( property( get = GetHost, put = SetHost)) 
		CPropertiesWnd* Host;

	void BuildFlatList(vector<CMFCPropertyGridProperty*>& flat)
	{
		CArray<CMFCPropertyGridProperty*, CMFCPropertyGridProperty*> list;
		BuildFlatList(list);
		flat = CHelper::ToVector(list);

	}

	void BuildFlatList
	(
		CArray<CMFCPropertyGridProperty*, CMFCPropertyGridProperty*>& list
	)
	{
		list.RemoveAll();

		int topCount = GetPropertyCount();
		for (int i = 0; i < topCount; i++)
		{
			CMFCPropertyGridProperty* pProp = GetProperty(i);
			if (pProp != nullptr)
			{
				AddPropertyRecursive(pProp, list);
			}
		}
	}

// protected methods
protected:
	void AddPropertyRecursive
	(
		CMFCPropertyGridProperty* pProp,
		CArray<CMFCPropertyGridProperty*, CMFCPropertyGridProperty*>& list
	)
	{
		// Skip group headers (they have children but no editable value)
		if (!pProp->IsGroup())
		{
			list.Add(pProp);
		}

		int childCount = pProp->GetSubItemsCount();
		for (int i = 0; i < childCount; i++)
		{
			AddPropertyRecursive(pProp->GetSubItem(i), list);
		}
	}

	LRESULT OnSelectAll(WPARAM, LPARAM);

// public methods
public:
	// compare variant data for equivalence 
	static bool ValuesAreDifferent
	(
		const COleVariant& oldVal,
		const COleVariant& newVal,
		VARTYPE vt
	)
	{
		switch (vt)
		{
		case VT_BSTR:
			return CString(oldVal.bstrVal) != CString(newVal.bstrVal);

		case VT_I4:
			return oldVal.lVal != newVal.lVal;

		case VT_BOOL:
			return oldVal.boolVal != newVal.boolVal;

		case VT_R8:
			return oldVal.dblVal != newVal.dblVal;

		case VT_DATE:
			return oldVal.date != newVal.date;

		default:
			// Fallback: compare string representations
			return CString(oldVal.bstrVal) != CString(newVal.bstrVal);
		}
	}

// protected overrides
protected:

// public overrides
public:
	virtual BOOL EndEditItem(BOOL bUpdateData = TRUE);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnChangeSelection
	(
		CMFCPropertyGridProperty* pNewSel, CMFCPropertyGridProperty* pOldSel
	);

// public constructor/destructor
public:
	CPropertyGridCtrl()
	{
		m_pHost = nullptr;
	}
	~CPropertyGridCtrl()
	{
	}

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
