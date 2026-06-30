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
// Custom property grid control used by the Properties pane. Although the
// MFC Application Wizard provides a basic CMFCPropertyGridCtrl, PhotoPrinter
// requires additional behavior for navigation, document synchronization,
// and edit‑tracking. This derived class adds those capabilities while
// preserving full compatibility with the MFC property grid infrastructure.
//
// Purpose:
//   • Provide enhanced keyboard navigation (Tab to next editable property).
//   • Automatically select all text when an edit box opens.
//   • Notify the host pane (CPropertiesWnd) whenever the selection changes.
//   • Track modifications accurately by comparing old vs. new values.
//   • Build a flattened list of editable properties for sequential traversal.
//   • Support custom commands such as WM_SELECTALL.
//
// Why this class exists:
//   CMFCPropertyGridCtrl is powerful but limited in three areas:
//     1. It does not provide “Tab to next property” behavior.
//     2. It marks properties modified even when the value hasn’t changed.
//     3. It does not notify the host when selection changes.
//   PhotoPrinter’s workflow depends heavily on keyboard‑friendly editing,
//   precise modification tracking, and tight integration with the document
//   model. This class implements those behaviors cleanly.
//
// Responsibilities:
//   • Host pointer management (link back to CPropertiesWnd).
//   • Flatten hierarchical property groups into a linear list for navigation.
//   • Recursive property enumeration (AddPropertyRecursive).
//   • Accurate value‑change detection (ValuesAreDifferent).
//   • Override EndEditItem() to mark the document modified only when needed.
//   • Override PreTranslateMessage() to implement Tab‑navigation and
//     automatic text selection.
//   • Forward selection changes to the host via AFX_WM_PROPERTY_CHANGED.
//
// Interaction with other components:
//   • Sends property‑change notifications to CPropertiesWnd.
//   • CPropertiesWnd pushes changes into CPhotoPrinterDoc.
//   • CPhotoPrinterView refreshes layout when document metadata changes.
//   • CMainFrame’s Wait() method is used to keep the UI responsive during
//     selection transitions.
//
// Wizard‑generated portions:
//   • Base class (CMFCPropertyGridCtrl) and message map structure.
//
// Application‑specific additions:
//   • Full Tab‑navigation system.
//   • WM_SELECTALL handler for auto‑selecting text.
//   • Flattened property list builder.
//   • Value‑change comparison logic.
//   • Host‑notification mechanism.
//
// This class transforms the standard MFC property grid into a polished,
// keyboard‑friendly metadata editor that integrates tightly with the
// PhotoPrinter document/view architecture.
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
