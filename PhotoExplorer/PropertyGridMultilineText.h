/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CPropertyGridMultilineText
//
// Specialized property‑grid item used for editing multi‑line text fields
// within Photo Explorer. This class extends CMFCPropertyGridProperty to
// provide a custom button‑click handler that launches the multi‑line text
// dialog (CMultilineTextDialog). It is used for EXIF fields such as comments,
// descriptions, and user notes that require more than a single‑line edit box.
//
// Purpose:
//   • Represent a multi‑line metadata field inside the property grid.
//   • Provide a clickable button that opens a dedicated multi‑line editor.
//   • Allow users to enter long text values cleanly and safely.
//   • Integrate with the property grid’s editing and update mechanisms.
//
// Why this class exists:
//   CMFCPropertyGridProperty supports only single‑line editing by default.
//   Many EXIF fields (e.g., XPComment, ImageDescription, UserComment) require
//   multi‑line input. This class provides the necessary UI hook to launch a
//   modal dialog that supports multi‑line editing, proper line‑break handling,
//   and maximum‑length enforcement.
//
// Responsibilities:
//   • Display a property item with a button for multi‑line editing.
//   • Override OnClickButton to open CMultilineTextDialog.
//   • Return TRUE for HasValueField to ensure the property grid treats this
//     item as editable.
//   • Store and return the edited multi‑line value.
//
// Interaction with other components:
//   • CMultilineTextDialog — invoked when the user clicks the property’s button.
//   • CPropertiesWnd — hosts the property grid and receives change notifications.
//   • CImageProperties — stores and retrieves the underlying EXIF metadata.
//   • CMFCPropertyGridCtrl — manages property layout and editing behavior.
//
// Key Features:
//   • Clean integration with the property grid’s button‑style editing.
//   • Dedicated multi‑line dialog for long text fields.
//   • Full support for metadata fields requiring multiple lines.
//   • Simple construction with name, initial value, and description.
//
// Internal Structure:
//   • Constructor stores name, value, and description.
//   • OnClickButton launches the multi‑line editor and updates the property value.
//   • HasValueField returns TRUE to indicate editable content.
//
// This class provides the property‑grid interface for multi‑line EXIF fields,
// enabling users to edit long comments and descriptions with clarity and ease.
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
