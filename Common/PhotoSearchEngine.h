/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>
#include <atlstr.h>

class CPhotoSearchEngine
{
public:
	CPhotoSearchEngine();
	virtual ~CPhotoSearchEngine();

	// Inputs
	void SetQuery(const CString& aqsQuery);
	void SetStartFolder(const CString& startFolder);
	void SetEndFolder(const CString& endFolder);

	// Execute search
	// Returns true on success, false on failure
	bool Execute();

	// Output: list of fully-qualified file paths
	const std::vector<CString>& GetResults() const;

protected:
	// Internal helpers
	CString BuildSQLFromAQS();
	bool RunSQLQuery(const CString& sql);
	bool IsWithinFolderRange(const CString& path) const;
	bool IsLabeledImage(const CString& path) const;

protected:
	CString m_query;
	CString m_startFolder;
	CString m_endFolder;
	std::vector<CString> m_results;
};
