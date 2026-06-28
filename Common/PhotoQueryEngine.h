/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "PhotoIndexBuilder.h"
#include <vector>
#include <unordered_map>

/////////////////////////////////////////////////////////////////////////////
class CPhotoQueryEngine
{
public:
	CPhotoQueryEngine();
	~CPhotoQueryEngine();

	bool LoadBinaryIndex(const CString& indexPath);

	// Query format:
	//   "mary beth cruise"
	//   "barry block|bobby block"
	bool Query(const CString& query, std::vector<CString>& outResults);
	void DumpIndex(bool showPostings);

private:
	// Data loaded from the index
	std::vector<CString> m_imagePaths;  // ID ? full path
	std::unordered_map<std::wstring, std::vector<uint32_t>> m_invertedIndex;

	// Helpers
	void Tokenize(const CString& text, std::vector<CString>& outTokens);
	void SplitOR(const CString& query, std::vector<CString>& outGroups);
	void SplitAND(const CString& group, std::vector<CString>& outTokens);

	void Intersect(const std::vector<uint32_t>& a,
		const std::vector<uint32_t>& b,
		std::vector<uint32_t>& out);

	void Union(const std::vector<uint32_t>& a,
		const std::vector<uint32_t>& b,
		std::vector<uint32_t>& out);
};

/////////////////////////////////////////////////////////////////////////////
