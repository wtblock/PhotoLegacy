/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "pch.h"
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "Version.lib")

/////////////////////////////////////////////////////////////////////////////
class CVersionInfo
{
// protected data
protected:
	CString m_csCompanyName;
	CString m_csFileDescription;
	CString m_csFileVersion;
	CString m_csProductName;
	CString m_csLegalCopyright;

// public properties
public:
	// company name
	CString GetCompanyName()
	{
		return m_csCompanyName;
	}
	// company name
	void SetCompanyName( CString value )
	{
		m_csCompanyName = value;
	}
	// company name
	__declspec( property( get = GetCompanyName, put = SetCompanyName ) )
		CString CompanyName;

	// description of the application
	CString GetFileDescription()
	{
		return m_csFileDescription;
	}
	// description of the application
	void SetFileDescription( CString value )
	{
		m_csFileDescription = value;
	}
	// description of the application
	__declspec( property( get = GetFileDescription, put = SetFileDescription ) )
		CString FileDescription;

	// version of the application
	CString GetFileVersion()
	{
		return m_csFileVersion;
	}
	// version of the application
	void SetFileVersion( CString value )
	{
		m_csFileVersion = value;
	}
	// version of the application
	__declspec( property( get = GetFileVersion, put = SetFileVersion ) )
		CString FileVersion;

	// name of the product
	CString GetProductName()
	{
		return m_csProductName;
	}
	// name of the product
	void SetProductName( CString value )
	{
		m_csProductName = value;
	}
	// name of the product
	__declspec( property( get = GetProductName, put = SetProductName ) )
		CString ProductName;

	// legal copyright declaration
	CString GetLegalCopyright()
	{
		return m_csLegalCopyright;
	}
	// company name
	void SetLegalCopyright( CString value )
	{
		m_csLegalCopyright = value;
	}
	// 
	__declspec( property( get = GetLegalCopyright, put = SetLegalCopyright ) )
		CString LegalCopyright;

// protected methods
protected:
	// read version information from the properties
	void ReadVersionData()
	{
		TCHAR szFileName[ MAX_PATH ];
		GetModuleFileName( NULL, szFileName, MAX_PATH );

		DWORD dwHandle;
		DWORD dwSize = GetFileVersionInfoSize( szFileName, &dwHandle );
		if ( dwSize > 0 )
		{
			std::vector<BYTE> buffer( dwSize );
			if ( GetFileVersionInfo( szFileName, 0, dwSize, buffer.data() ) )
			{
				TCHAR* pValue;
				UINT uLen;
				BOOL bOK = VerQueryValue
				(
					buffer.data(), L"\\StringFileInfo\\040904b0\\CompanyName",
					(LPVOID*)&pValue, &uLen
				);
				if ( bOK )
				{
					CompanyName = pValue;
				}
				bOK = VerQueryValue
				(
					buffer.data(), L"\\StringFileInfo\\040904b0\\LegalCopyright",
					(LPVOID*)&pValue, &uLen
				);
				if ( bOK )
				{
					LegalCopyright = pValue;
				}
				bOK = VerQueryValue
				(
					buffer.data(), L"\\StringFileInfo\\040904b0\\FileDescription",
					(LPVOID*)&pValue, &uLen
				);
				if ( bOK )
				{
					FileDescription = pValue;
				}
				bOK = VerQueryValue
				(
					buffer.data(), L"\\StringFileInfo\\040904b0\\ProductName",
					(LPVOID*)&pValue, &uLen
				);
				if ( bOK )
				{
					ProductName = pValue;
				}
				bOK = VerQueryValue
				(
					buffer.data(), L"\\StringFileInfo\\040904b0\\FileVersion",
					(LPVOID*)&pValue, &uLen
				);
				if ( bOK )
				{
					FileVersion = pValue;
				}
			}
		}
	}

// public methods
public:

// protected overrides
protected:

// public overrides
public:

// public constructor/destructor
public:
	CVersionInfo()
	{
		ReadVersionData();
	}
	virtual ~CVersionInfo()
	{
	}

};

/////////////////////////////////////////////////////////////////////////////
