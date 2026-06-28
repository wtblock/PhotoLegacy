/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PropertiesWnd.h"
#include "PropertyGridCtrl.h"
#include "MainFrm.h"
#include "PhotoPrinterDoc.h"

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP( CPropertyGridCtrl, CMFCPropertyGridCtrl )
	ON_MESSAGE(WM_SELECTALL, OnSelectAll)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CPropertiesWnd* CPropertyGridCtrl::GetHost()
{
	return m_pHost;

} // GetHost

/////////////////////////////////////////////////////////////////////////////
void CPropertyGridCtrl::SetHost( CPropertiesWnd* value )
{
	m_pHost = value;

} // SetHost

/////////////////////////////////////////////////////////////////////////////
void CPropertyGridCtrl::OnChangeSelection
(
	CMFCPropertyGridProperty* pNewSel,
	CMFCPropertyGridProperty* pOldSel
)
{
	CMFCPropertyGridCtrl::OnChangeSelection(pNewSel, pOldSel);

	if (pNewSel != nullptr)
	{
		// Forward the selection to the parent (CPropertiesWnd)
		CPropertiesWnd* pHost = Host;
		if (pHost != nullptr)
		{
			pHost->SendMessage(AFX_WM_PROPERTY_CHANGED, 0, (LPARAM)pNewSel);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
LRESULT CPropertyGridCtrl::OnSelectAll(WPARAM, LPARAM)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->Wait(100);

	CWnd* pEdit = GetFocus();
	if (pEdit && pEdit->IsKindOf(RUNTIME_CLASS(CEdit)))
	{
		((CEdit*)pEdit)->SetSel(0, -1);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPropertyGridCtrl::EndEditItem(BOOL bUpdateData)
{
	CMFCPropertyGridProperty* pProp = GetCurSel();

	// Capture old value BEFORE calling base
	COleVariant oldValue;
	VARTYPE vt = VT_EMPTY;

	if (pProp != nullptr)
	{
		oldValue = pProp->GetValue();
		vt = oldValue.vt;
	}

	// Let MFC commit the edit
	CMFCPropertyGridCtrl::EndEditItem(bUpdateData);

	// Now compare old vs new
	if (pProp != nullptr)
	{
		COleVariant newValue = pProp->GetValue();
		bool bDiff = ValuesAreDifferent(oldValue, newValue, vt);
		if (bDiff)   // ⭐ only mark modified if value changed
		{
			CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	
			// Now mark the document as modified
			CPhotoPrinterDoc* pDoc = pFrame->GetPhotoPrinterDocument();;
			if (pDoc)
			{
				pDoc->SetModifiedFlag(TRUE);
			}
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPropertyGridCtrl::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB)
	{
		// Commit current edit if one is active
		CMFCPropertyGridProperty* pSel = GetCurSel();
		if (pSel != nullptr) 
		{
			EndEditItem();
			
			// Build flat list of editable properties
			CArray<CMFCPropertyGridProperty*, CMFCPropertyGridProperty*> flat;
			BuildFlatList(flat);

			// Find current property in the flat list
			int nSize = (int)flat.GetSize();
			for (int i = 0; i < nSize; i++)
			{
				CMFCPropertyGridProperty* pCur = flat[i];
				if (pCur == pSel)
				{
					int nNext = i + 1;
					if (nNext < nSize)
					{
						CMFCPropertyGridProperty* pNext = flat[nNext];
						SetCurSel(pNext);
						
						// ⭐ Open the edit box automatically
						EditItem(pNext);

						// Post a message so selection happens AFTER MFC sets focus
						PostMessage(WM_SELECTALL, 0, 0);
					}
					break;
				}
			}
		}

		return TRUE; // swallow the key
	}
	return CMFCPropertyGridCtrl::PreTranslateMessage(pMsg);
}


