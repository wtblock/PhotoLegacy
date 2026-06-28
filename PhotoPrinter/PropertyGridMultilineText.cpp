/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoPrinter.h"
#include "PropertyGridMultilineText.h"
#include "MultilineTextDialog.h"

#define AFX_PROP_HAS_BUTTON 0x0002


/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CPropertyGridMultilineText, CMFCPropertyGridProperty)

/////////////////////////////////////////////////////////////////////////////
CPropertyGridMultilineText::CPropertyGridMultilineText
(
	LPCTSTR pcszName,
	LPCTSTR pcszValue,
	LPCTSTR pcszDesc
) : CMFCPropertyGridProperty( pcszName, COleVariant( pcszValue ), pcszDesc )
{
	m_dwFlags = AFX_PROP_HAS_BUTTON;
}

/////////////////////////////////////////////////////////////////////////////
CPropertyGridMultilineText::~CPropertyGridMultilineText()
{
}

/////////////////////////////////////////////////////////////////////////////
void CPropertyGridMultilineText::OnClickButton( CPoint point )
{	// If the item is disabled, then do not pop up any dialog
	if ( !IsEnabled() )
	{
		return;
	}

	COleVariant var = GetValue();
	CString csValue( var.bstrVal );
	CMultilineTextDialog dlg;
	dlg.Title = GetName();
	dlg.SetValue( csValue, _T( "\\n" ) );

	// setup the position of the dialog based on the property co-ordinates
	const CRect rectList = m_pWndList->GetListRect();
	CRect rectProp = GetRect();
	const int nColumnWidth = m_pWndList->GetPropertyColumnWidth();
	CPoint pt( rectList.left + nColumnWidth + 1, rectProp.bottom + 1 );
	m_pWndList->ClientToScreen( &pt );
	dlg.UpperLeftCorner = pt;

	if ( IDOK == dlg.DoModal() )
	{
		CString csText = dlg.GetValue( _T( "\n" ) );
		CPropertyGridMultilineText::SetValue( csText );
	}
} // OnClickButton

/////////////////////////////////////////////////////////////////////////////
