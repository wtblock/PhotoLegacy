/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <afxpropertygridctrl.h>

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


// protected methods
protected:

// public methods
public:

// protected overrides
protected:

// public overrides
public:

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
