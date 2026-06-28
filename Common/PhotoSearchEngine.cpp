/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoSearchEngine.h"

#include <atlbase.h>
#include <atlcom.h>
#include <comdef.h>
#include <SearchAPI.h>

#pragma comment(lib, "SearchSDK.lib")

/////////////////////////////////////////////////////////////////////////////
// ADO import (only once in the project; here is fine since you don't 
// use it elsewhere)
#import "C:\\Program Files\\Common Files\\System\\ado\\msado15.dll" \
    no_namespace rename("EOF","adoEOF")

/////////////////////////////////////////////////////////////////////////////
CPhotoSearchEngine::CPhotoSearchEngine()
{
}

/////////////////////////////////////////////////////////////////////////////
CPhotoSearchEngine::~CPhotoSearchEngine()
{
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoSearchEngine::SetQuery(const CString& aqsQuery)
{
	m_query = aqsQuery;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoSearchEngine::SetStartFolder(const CString& startFolder)
{
	m_startFolder = startFolder;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoSearchEngine::SetEndFolder(const CString& endFolder)
{
	m_endFolder = endFolder;
}

/////////////////////////////////////////////////////////////////////////////
const std::vector<CString>& CPhotoSearchEngine::GetResults() const
{
	return m_results;
}

/////////////////////////////////////////////////////////////////////////////
bool CPhotoSearchEngine::Execute()
{
	m_results.clear();

	// Basic validation
	if (m_query.IsEmpty())
		return false;

	// Initialize COM for this thread
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	bool bCoInitialized = SUCCEEDED(hr);

	if (!bCoInitialized && hr != RPC_E_CHANGED_MODE)
	{
		return false;
	}

	bool bOK = false;

	do
	{
		// Build SQL from AQS
		CString sql = BuildSQLFromAQS();
		if (sql.IsEmpty())
			break;

		// Run SQL and populate m_results
		if (!RunSQLQuery(sql))
			break;

		bOK = true;

	} while (false);

	if (bCoInitialized)
		CoUninitialize();

	return bOK;
}

/////////////////////////////////////////////////////////////////////////////
CString CPhotoSearchEngine::BuildSQLFromAQS()
{
	CString sql;

	CComPtr<ISearchManager> spSearchManager;
	HRESULT hr = spSearchManager.CoCreateInstance(CLSID_CSearchManager, nullptr, CLSCTX_INPROC_SERVER);
	if (FAILED(hr))
	{
		CString msg;
		msg.Format(L"CoCreateInstance failed: 0x%08X", hr);
		AfxMessageBox(msg);
		return sql;
	}
	CComPtr<ISearchCatalogManager> spCatalogManager;
	hr = spSearchManager->GetCatalog(L"SystemIndex", &spCatalogManager);
	if (FAILED(hr))
		return sql;

	CComPtr<ISearchQueryHelper> spQueryHelper;
	hr = spCatalogManager->GetQueryHelper(&spQueryHelper);
	if (FAILED(hr))
		return sql;

	// Set the AQS query text
	hr = spQueryHelper->put_QueryWhereRestrictions(m_query);
	if (FAILED(hr))
		return sql;

	// Optionally, we could restrict to file system results only
	// spQueryHelper->put_QueryWhereRestrictions(L"scope='file:'");

	LPWSTR pwszSQL = nullptr;
	hr = spQueryHelper->GenerateSQLFromUserQuery(
		m_query,        // AQS query text
		&pwszSQL        // Returned SQL string
	);
	if (FAILED(hr) || pwszSQL == nullptr)
		return sql;

	sql = pwszSQL;
	CoTaskMemFree(pwszSQL);

	return sql;
}

/////////////////////////////////////////////////////////////////////////////
bool CPhotoSearchEngine::RunSQLQuery(const CString& sql)
{
	try
	{
		_ConnectionPtr spConnection;
		HRESULT hr = spConnection.CreateInstance(__uuidof(Connection));
		if (FAILED(hr))
			return false;

		// Connection string for Windows Search via OLE DB
		_bstr_t connStr(L"Provider=Search.CollatorDSO;Extended Properties='Application=Windows';");

		hr = spConnection->Open(connStr, L"", L"", adConnectUnspecified);
		if (FAILED(hr))
			return false;

		_RecordsetPtr spRecordset;
		hr = spRecordset.CreateInstance(__uuidof(Recordset));
		if (FAILED(hr))
			return false;

		_bstr_t bstrSQL(sql);
		spRecordset->Open(bstrSQL,
			_variant_t((IDispatch*)spConnection, true),
			adOpenForwardOnly,
			adLockReadOnly,
			adCmdText);

		if (spRecordset == nullptr)
			return false;

		// Iterate through results
		while (!spRecordset->adoEOF)
		{
			_variant_t varPath = spRecordset->Fields->Item[L"System.ItemPathDisplay"]->Value;
			if (varPath.vt == VT_BSTR)
			{
				CString path((LPCWSTR)_bstr_t(varPath));

				// Filter to labeled images and folder range
				if (IsLabeledImage(path) && IsWithinFolderRange(path))
				{
					m_results.push_back(path);
				}
			}

			spRecordset->MoveNext();
		}

		spRecordset->Close();
		spConnection->Close();
	}
	catch (const _com_error&)
	{
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CPhotoSearchEngine::IsLabeledImage(const CString& path) const
{
	// Must contain "\Labeled\" in the path
	if (path.Find(L"\\Labeled\\") < 0)
		return false;

	// Simple extension check for .jpg / .jpeg
	CString lower = path;
	lower.MakeLower();

	if (lower.Right(4) == L".jpg" || lower.Right(5) == L".jpeg")
		return true;

	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool CPhotoSearchEngine::IsWithinFolderRange(const CString& path) const
{
	// If no range specified, accept everything
	if (m_startFolder.IsEmpty() && m_endFolder.IsEmpty())
		return true;

	// Normalize for case-insensitive compare
	CString p = path;
	CString s = m_startFolder;
	CString e = m_endFolder;

	p.MakeLower();
	s.MakeLower();
	e.MakeLower();

	// Basic prefix-based range check:
	// - path must start with the same root as your photo tree
	// - and be >= start folder and <= end folder lexicographically
	// This assumes your folder names are chronologically sortable,
	// which matches your existing design.
	if (!s.IsEmpty())
	{
		if (p.Left(s.GetLength()).Compare(s) < 0)
			return false;
	}

	if (!e.IsEmpty())
	{
		if (p.Left(e.GetLength()).Compare(e) > 0)
			return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
