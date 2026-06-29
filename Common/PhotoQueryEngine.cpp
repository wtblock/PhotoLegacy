/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoQueryEngine.h"
#include <algorithm>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
struct IndexHeader
{
	uint32_t magic;
	uint32_t version;
	uint32_t imageCount;
	uint32_t tokenCount;
};

/////////////////////////////////////////////////////////////////////////////
CPhotoQueryEngine::CPhotoQueryEngine()
{
}

/////////////////////////////////////////////////////////////////////////////
CPhotoQueryEngine::~CPhotoQueryEngine()
{
}

/////////////////////////////////////////////////////////////////////////////
// DumpIndex
//
// Prints the entire in‑memory index to the console for debugging.
// Shows:
//   - All image paths
//   - All tokens
//   - Optional posting lists (image IDs for each token)
//
// This is a diagnostic tool used to verify that the binary index
// was loaded correctly and that the inverted index structure is valid.
/////////////////////////////////////////////////////////////////////////////
void CPhotoQueryEngine::DumpIndex(bool showPostings)
{
	wprintf(L"===== PHOTO INDEX DUMP =====\n");
	wprintf(L"Image count: %zu\n", m_imagePaths.size());
	wprintf(L"Token count: %zu\n\n", m_invertedIndex.size());

	wprintf(L"-- Image Paths --\n");
	for (size_t i = 0; i < m_imagePaths.size(); i++)
	{
		wprintf(L"[%zu] %s\n", i, m_imagePaths[i].GetString());
	}

	wprintf(L"\n-- Tokens --\n");
	for (const auto& kv : m_invertedIndex)
	{
		const wstring& token = kv.first;
		const vector<uint32_t>& postings = kv.second;

		wprintf(L"%s  (%zu hits)\n", token.c_str(), postings.size());

		if (showPostings)
		{
			wprintf(L"   IDs: ");
			for (uint32_t id : postings)
				wprintf(L"%u ", id);
			wprintf(L"\n");
		}
	}

	wprintf(L"===== END OF INDEX DUMP =====\n");
} // DumpIndex

/////////////////////////////////////////////////////////////////////////////
// LoadBinaryIndex
//
// Loads a .phix binary index file from disk and reconstructs:
//   - The image path table
//   - The token table
//   - The inverted index (token → posting list)
//
// Handles:
//   - UTF‑16 string decoding
//   - Relative path expansion (".\" → working directory)
//   - Sorted posting lists for fast AND/OR operations
//
// Returns true on success, false on any format or I/O error.
/////////////////////////////////////////////////////////////////////////////
bool CPhotoQueryEngine::LoadBinaryIndex(const CString& indexPath)
{
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, indexPath, L"rb") != 0 || !fp)
		return false;

	IndexHeader hdr;
	if (fread(&hdr, sizeof(hdr), 1, fp) != 1)
	{
		fclose(fp);
		return false;
	}

	if (hdr.magic != 0x50484958)  // 'PHIX'
	{
		fclose(fp);
		return false;
	}

	// ------------------------------------------------------------
	// IMAGE TABLE
	// ------------------------------------------------------------
	// The index stores image paths as UTF‑16 strings, each prefixed
	// by a uint16_t length (number of WCHARs, not including null).
	//
	// Paths may be:
	//   - absolute (C:\Users\...\IMG_1234.jpg)
	//   - relative (.\Family\MaryBeth\IMG_1234.jpg)
	//
	// Relative paths use the same convention as Photo Printer:
	//     ".\" means "relative to the working directory".
	//
	// We expand relative paths here so the query engine always
	// works with absolute paths internally.
	// ------------------------------------------------------------

	m_imagePaths.clear();
	m_imagePaths.reserve(hdr.imageCount);

	// Determine working directory for relative path expansion
	WCHAR cwdBuf[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cwdBuf);
	CString workingDir(cwdBuf);
	if (!workingDir.IsEmpty() && workingDir.Right(1) != L"\\")
		workingDir += L"\\";

	for (uint32_t i = 0; i < hdr.imageCount; i++)
	{
		uint16_t len = 0;
		fread(&len, sizeof(len), 1, fp);

		CString path;
		WCHAR* buf = path.GetBuffer(len);
		fread(buf, sizeof(WCHAR), len, fp);
		path.ReleaseBuffer(len);

		// Expand ".\" prefix to working directory
		if (path.Left(2) == L".\\")
		{
			CString full = workingDir + path.Mid(2);
			m_imagePaths.push_back(full);
		}
		else
		{
			m_imagePaths.push_back(path);
		}
	}

	// ------------------------------------------------------------
	// TOKEN TABLE + POSTING LISTS
	// ------------------------------------------------------------
	// For each token:
	//
	//   uint16_t len        → number of WCHARs in token
	//   WCHAR[len]          → UTF‑16 token text
	//   uint32_t count      → number of image IDs
	//   uint32_t[count]     → sorted posting list
	//
	// This reconstructs the inverted index:
	//     token → [imageID, imageID, ...]
	//
	// Posting lists are already sorted, enabling fast AND/OR.
	// ------------------------------------------------------------

	m_invertedIndex.clear();
	m_invertedIndex.reserve(hdr.tokenCount);

	for (uint32_t i = 0; i < hdr.tokenCount; i++)
	{
		uint16_t len = 0;
		fread(&len, sizeof(len), 1, fp);

		wstring token;
		token.resize(len);
		fread(&token[0], sizeof(WCHAR), len, fp);

		uint32_t count = 0;
		fread(&count, sizeof(count), 1, fp);

		vector<uint32_t> postings(count);
		fread(postings.data(), sizeof(uint32_t), count, fp);

		m_invertedIndex[token] = std::move(postings);
	}

	fclose(fp);
	return true;
} // LoadBinaryIndex

/////////////////////////////////////////////////////////////////////////////
// Tokenize
//
// Converts an input string into a list of lowercase alphanumeric tokens.
// Splits on any non‑alphanumeric character.
// Used for both:
//   - Query tokenization
//   - Internal AND‑group tokenization
//
// Example:
//   "Mary Beth Block, Sr." → ["mary", "beth", "block", "sr"]
/////////////////////////////////////////////////////////////////////////////
void CPhotoQueryEngine::Tokenize
(
	const CString& text, vector<CString>& outTokens
)
{
	CString token;
	int len = text.GetLength();

	for (int i = 0; i < len; i++)
	{
		WCHAR ch = text[i];

		if (iswalnum(ch))
		{
			token.AppendChar(towlower(ch));
		}
		else
		{
			if (!token.IsEmpty())
			{
				outTokens.push_back(token);
				token.Empty();
			}
		}
	}

	if (!token.IsEmpty())
		outTokens.push_back(token);
} // Tokenize

/////////////////////////////////////////////////////////////////////////////
// SplitOR
//
// Splits a full query string into OR‑groups using the '|' operator.
//
// Example:
//   "mary beth|block sr" → ["mary beth", "block sr"]
//
// Each group is later evaluated independently and OR‑merged.
/////////////////////////////////////////////////////////////////////////////
void CPhotoQueryEngine::SplitOR
(
	const CString& query, vector<CString>& outGroups
)
{
	int start = 0;
	int pos = 0;

	while ((pos = query.Find(L'|', start)) != -1)
	{
		outGroups.push_back(query.Mid(start, pos - start).Trim());
		start = pos + 1;
	}

	outGroups.push_back(query.Mid(start).Trim());
} // SplitOR

/////////////////////////////////////////////////////////////////////////////
// SplitAND
//
// Splits a single OR‑group into AND‑tokens.
// Uses Tokenize() to extract normalized alphanumeric tokens.
//
// Example:
//   "mary beth" → ["mary", "beth"]
/////////////////////////////////////////////////////////////////////////////
void CPhotoQueryEngine::SplitAND
(
	const CString& group, vector<CString>& outTokens
)
{
	Tokenize(group, outTokens);
} // SplitAND

/////////////////////////////////////////////////////////////////////////////
// Intersect
//
// Computes the sorted intersection (logical AND) of two posting lists.
// Posting lists contain sorted image IDs.
//
// Example:
//   a = [1, 3, 5]
//   b = [3, 4, 5]
//   result = [3, 5]
//
// Used to evaluate AND‑conditions inside a single OR‑group.
/////////////////////////////////////////////////////////////////////////////
void CPhotoQueryEngine::Intersect
(
	const vector<uint32_t>& a,
	const vector<uint32_t>& b,
	vector<uint32_t>& out
)
{
	out.clear();
	size_t i = 0, j = 0;

	while (i < a.size() && j < b.size())
	{
		if (a[i] == b[j])
		{
			out.push_back(a[i]);
			i++; j++;
		}
		else if (a[i] < b[j])
		{
			i++;
		}
		else
		{
			j++;
		}
	}
} // Intersect

/////////////////////////////////////////////////////////////////////////////
// Union
//
// Computes the sorted union (logical OR) of two posting lists.
// Posting lists contain sorted image IDs.
//
// Example:
//   a = [1, 3, 5]
//   b = [3, 4, 5]
//   result = [1, 3, 4, 5]
//
// Used to merge results from multiple OR‑groups.
/////////////////////////////////////////////////////////////////////////////
void CPhotoQueryEngine::Union
(
	const vector<uint32_t>& a,
	const vector<uint32_t>& b,
	vector<uint32_t>& out
)
{
	out.clear();
	size_t i = 0, j = 0;

	while (i < a.size() || j < b.size())
	{
		if (j >= b.size() || (i < a.size() && a[i] < b[j]))
		{
			out.push_back(a[i++]);
		}
		else if (i >= a.size() || b[j] < a[i])
		{
			out.push_back(b[j++]);
		}
		else
		{
			out.push_back(a[i]);
			i++; j++;
		}
	}
} // Union

/////////////////////////////////////////////////////////////////////////////
// Query
//
// Evaluates a full query string using OR and AND semantics.
//
// Steps:
//   1. Split query into OR‑groups
//   2. For each group:
//        a. Tokenize into AND‑tokens
//        b. Start with first token’s posting list
//        c. Intersect with remaining tokens
//   3. OR‑merge all group results
//   4. Convert final image IDs into full paths
//
// Returns true if the query was processed (even if no results).
/////////////////////////////////////////////////////////////////////////////
bool CPhotoQueryEngine::Query
(
	const CString& query, vector<CString>& outResults
)
{
	outResults.clear();

	// Split on OR
	vector<CString> groups;
	SplitOR(query, groups);

	vector<uint32_t> finalResults;

	for (const CString& group : groups)
	{
		// Split into AND tokens
		vector<CString> tokens;
		SplitAND(group, tokens);

		if (tokens.empty())
			continue;

		// Start with the first token's posting list
		wstring tok0(tokens[0].GetString());
		auto it = m_invertedIndex.find(tok0);
		if (it == m_invertedIndex.end())
			continue;

		vector<uint32_t> groupResult = it->second;

		// AND with remaining tokens
		for (size_t i = 1; i < tokens.size(); i++)
		{
			wstring tok(tokens[i].GetString());
			auto it2 = m_invertedIndex.find(tok);
			if (it2 == m_invertedIndex.end())
			{
				groupResult.clear();
				break;
			}

			vector<uint32_t> temp;
			Intersect(groupResult, it2->second, temp);
			groupResult.swap(temp);

			if (groupResult.empty())
				break;
		}

		// OR into final results
		if (finalResults.empty())
		{
			finalResults = groupResult;
		}
		else
		{
			vector<uint32_t> temp;
			Union(finalResults, groupResult, temp);
			finalResults.swap(temp);
		}
	}

	// Convert IDs → filenames
	for (uint32_t id : finalResults)
	{
		if (id < m_imagePaths.size())
			outResults.push_back(m_imagePaths[id]);
	}

	return true;
} // Query

/////////////////////////////////////////////////////////////////////////////
