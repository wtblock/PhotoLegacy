/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "afxwin.h"
#include <memory>

class CCommandLine : public CCommandLineInfo
{
// public definitions
public:
	// types of command line parameters
	typedef enum PARAM_TYPES
	{	paramNone, 
		paramWork, // /work working_folder
	} PARAM_TYPES;

// protected data
protected:
	// last command line parameter flag
	PARAM_TYPES m_eLastFlag;

	// optional working folder 
	CString m_csWorkingFolder;

// public properties
public:
	// get raw parameter string
	CString GetParameters();
	// get raw parameter string
	__declspec( property( get = GetParameters ) )
		CString Parameters;

	// type of plot enumeration
	inline PARAM_TYPES GetLastFlag()
	{
		return m_eLastFlag;
	}
	// type of plot enumeration
	inline void SetLastFlag( PARAM_TYPES value )
	{
		m_eLastFlag = value;
	}
	// type of plot enumeration
	__declspec( property( get=GetLastFlag, put=SetLastFlag )) 
		PARAM_TYPES LastFlag;

	// optional working folder 
	inline CString GetWorkingFolder()
	{
		return m_csWorkingFolder;
	}
	// optional working folder 
	inline void SetWorkingFolder( LPCTSTR value )
	{
		m_csWorkingFolder = value;
	}
	// optional working folder 
	__declspec( property( get = GetWorkingFolder, put = SetWorkingFolder ) )
		CString WorkingFolder;

// protected methods
protected:
	// parse a parameter line flag
	bool ParseParamFlag( LPCTSTR flag );

	// parse a parameter line value
	bool ParseParamNotFlag( LPCTSTR value );

// public methods
public:

// protected overrides
protected:

// public overrides
public:
	// The framework calls this function to parse/interpret individual 
	// parameters from the command line
	virtual void ParseParam( LPCTSTR pszParam, BOOL bFlag, BOOL bLast );

// public construction / destruction
public:
	CCommandLine();
	virtual ~CCommandLine();
};

