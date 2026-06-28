/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <afxpropertygridctrl.h>
#include "CHelper.h"

#define WM_SELECTALL (WM_USER + 100)

class CPropertiesWnd;

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
