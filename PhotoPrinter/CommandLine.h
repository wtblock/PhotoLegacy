/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "afxwin.h"
#include <memory>

/////////////////////////////////////////////////////////////////////////////
// CCommandLine
//
// Lightweight command‑line parser for PhotoPrinter. Many of my applications
// use a more elaborate command‑line processor, but PhotoPrinter requires only
// a single optional parameter: a working folder override.
//
// Purpose:
//   • Interpret command‑line arguments passed to PhotoPrinter.
//   • Allow the user (or a batch file) to specify a working folder using:
//         /work <folder_path>
//   • Provide a simple, predictable interface for CPhotoPrinterApp to read
//     command‑line data exactly once during startup.
//
// Why this class exists:
//   MFC’s CCommandLineInfo provides basic parsing, but PhotoPrinter needs a
//   small amount of custom logic to support a “working folder” override.
//   Rather than embedding parsing logic inside the application class,
//   CCommandLine encapsulates it cleanly and consistently.
//
// Responsibilities:
//   • Track the most recent flag encountered (LastFlag).
//   • Recognize the “/work” flag and capture its associated folder path.
//   • Provide the WorkingFolder property to the application.
//   • Provide GetParameters() for reconstructing the raw command‑line string.
//   • Delegate unknown flags/values back to the base CCommandLineInfo parser.
//
// Parsing Behavior:
//   • Flags are processed by ParseParamFlag():
//         /work   → LastFlag = paramWork
//         anything else → not handled (base class processes it)
//   • Values are processed by ParseParamNotFlag():
//         paramWork → WorkingFolder = <value>
//         anything else → not handled (base class processes it)
//   • ParseParam() coordinates flag/value handling and calls ParseLast().
//
// Usage in PhotoPrinter:
//   • CPhotoPrinterApp creates a CCommandLine instance during InitInstance().
//   • ParseCommandLine() fills in WorkingFolder if provided.
//   • If no /work parameter is given, the application defaults to the current
//     directory.
//   • The working folder is then propagated to CPhotoPrinterDoc.
//
// This class intentionally remains simple. It provides just enough structure
// to support PhotoPrinter’s startup behavior without the complexity required
// in other applications.
/////////////////////////////////////////////////////////////////////////////
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

