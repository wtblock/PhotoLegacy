/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

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
