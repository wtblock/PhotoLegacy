/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CPropertyGridMultilineText
//
// Custom property‑grid item that provides multi‑line text editing support.
// The standard CMFCPropertyGridProperty only supports single‑line edit
// fields, which is insufficient for PhotoPrinter’s “Description” and
// “Query” fields. This derived class adds a button that opens a dedicated
// multi‑line editor dialog.
//
// Purpose:
//   • Allow editing of long text fields (e.g., Description, Query) using a
//     proper multi‑line dialog instead of a cramped single‑line edit box.
//   • Integrate seamlessly with the existing CMFCPropertyGridCtrl UI.
//   • Preserve the property’s name, description, and value semantics.
//
// Why this class exists:
//   CMFCPropertyGridProperty does not natively support multi‑line editing.
//   PhotoPrinter requires a comfortable way for users to enter long
//   descriptive text. This class provides that capability by attaching a
//   “…” button to the property and launching a custom dialog.
//
// Responsibilities:
//   • Display a button next to the property value (AFX_PROP_HAS_BUTTON).
//   • Launch CMultilineTextDialog when the button is clicked.
//   • Transfer the edited text back into the property grid item.
//   • Position the dialog intelligently based on the property’s screen
//     coordinates.
//
// Interaction with other components:
//   • Uses CMultilineTextDialog to perform the actual multi‑line editing.
//   • Works inside CPropertyGridCtrl and CPropertiesWnd without additional
//     integration code.
//   • The edited value flows into CPhotoPrinterDoc through the normal
//     property‑change pipeline.
//
// Wizard‑generated portions:
//   • Base class (CMFCPropertyGridProperty) and dynamic‑creation macros.
//
// Application‑specific additions:
//   • Multi‑line dialog invocation.
//   • Button flag setup.
//   • Screen‑coordinate positioning logic.
//
// This class enhances the property grid with a polished, user‑friendly
// multi‑line editor, essential for editing descriptive metadata in
// PhotoPrinter.
/////////////////////////////////////////////////////////////////////////////
class CPropertyGridMultilineText : public CMFCPropertyGridProperty
{
	DECLARE_DYNAMIC(CPropertyGridMultilineText)

// Construction
public:
	CPropertyGridMultilineText
	( 
		LPCTSTR pcszName,
		LPCTSTR pcszValue,
		LPCTSTR pcszDesc
	);
	virtual ~CPropertyGridMultilineText();


// Properties
public:

// Overrides
public:
	virtual void OnClickButton( CPoint point );
	inline virtual BOOL HasValueField() const
	{
		return TRUE;
	}

};

/////////////////////////////////////////////////////////////////////////////
