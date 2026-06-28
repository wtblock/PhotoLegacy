/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "CommandLine.h"

/////////////////////////////////////////////////////////////////////////////
CCommandLine::CCommandLine()
{
	LastFlag = paramNone;
}

/////////////////////////////////////////////////////////////////////////////
CCommandLine::~CCommandLine()
{
}

/////////////////////////////////////////////////////////////////////////////
// get raw parameter string
CString CCommandLine::GetParameters()
{
	CString value;
	if ( !WorkingFolder.IsEmpty())
	{
		value += _T( " /work " ) + WorkingFolder;
	}

	value.Trim();

	return value;
} // GetParameters

/////////////////////////////////////////////////////////////////////////////
// The framework calls this function to parse/interpret individual 
// parameters from the command line
void CCommandLine::ParseParam( LPCTSTR pszParam, BOOL bFlag, BOOL bLast )
{
	if ( bFlag )
	{
		if ( !ParseParamFlag( pszParam ))
		{
			return CCommandLineInfo::ParseParam( pszParam, bFlag, bLast );
		}
	} else
	{
		if ( !ParseParamNotFlag( pszParam ))
		{
			return CCommandLineInfo::ParseParam( pszParam, bFlag, bLast );
		}
	}

	ParseLast( bLast );
} // ParseParam

/////////////////////////////////////////////////////////////////////////////
// parse a parameter line flag
bool CCommandLine::ParseParamFlag( LPCTSTR flag )
{
	bool bOK = true;
	const CString csFlag = CString( flag ).MakeLower();
	if ( csFlag == _T( "work" ))
	{
		LastFlag = paramWork;
	
	} else // default processing
	{
		LastFlag = paramNone;
		bOK = false;
	}

	return bOK;
} // ParseParamFlag

/////////////////////////////////////////////////////////////////////////////
// parse a parameter line value
bool CCommandLine::ParseParamNotFlag( LPCTSTR value )
{
	bool bOK = true;
	
	PARAM_TYPES eType = LastFlag;
	switch ( eType )
	{
		case paramWork : WorkingFolder = value; break;
		default : // base processing needed
		{
			bOK = false;
		}
	}

	return bOK;
} // ParseParamNotFlag

/////////////////////////////////////////////////////////////////////////////
