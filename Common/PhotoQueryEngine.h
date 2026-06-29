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
	bool LoadBinaryIndex(const CString& indexPath);

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
	bool Query(const CString& query, std::vector<CString>& outResults);

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
	void DumpIndex(bool showPostings);

private:
	// Data loaded from the index
	std::vector<CString> m_imagePaths;  // ID ? full path
	std::unordered_map<std::wstring, std::vector<uint32_t>> m_invertedIndex;

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
	void Tokenize(const CString& text, std::vector<CString>& outTokens);

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
	void SplitOR(const CString& query, std::vector<CString>& outGroups);

	/////////////////////////////////////////////////////////////////////////////
	// SplitAND
	//
	// Splits a single OR‑group into AND‑tokens.
	// Uses Tokenize() to extract normalized alphanumeric tokens.
	//
	// Example:
	//   "mary beth" → ["mary", "beth"]
	/////////////////////////////////////////////////////////////////////////////
	void SplitAND(const CString& group, std::vector<CString>& outTokens);

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
	void Intersect(const std::vector<uint32_t>& a,
		const std::vector<uint32_t>& b,
		std::vector<uint32_t>& out);

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
	void Union(const std::vector<uint32_t>& a,
		const std::vector<uint32_t>& b,
		std::vector<uint32_t>& out);
};

/////////////////////////////////////////////////////////////////////////////
