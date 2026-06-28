/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include <set>
#include <unordered_map>
#include <vector>
#include <memory>

class CPhotoIndexBuilder
{
public:
	typedef CKeyedCollection<CString, CString> MAP_INDEX;

	struct ImageEntry
	{
		CString  path;          // stored path (relative or absolute)
		uint64_t lastWriteTime; // FILETIME as 64-bit
		uint64_t fileSize;      // bytes
	};

	struct LoadedIndex
	{
		uint64_t indexTimestamp;   // unchanged
		std::vector<ImageEntry> images;

		// Updated to deterministic structure
		std::map<std::wstring, std::set<uint32_t>> invertedIndex;
	};

	struct UpdateResult
	{
		bool success = false;
		CString errorMessage;

		std::vector<CString> newImages;
		std::vector<CString> modifiedImages;
		std::vector<CString> deletedImages;
	};

	static bool SaveBinaryIndex(MAP_INDEX& mapIndex, const CString& outputPath);

	static bool SaveBinaryIndexFromComponents
	(
		const std::vector<ImageEntry>& imageTable,
		const std::map<std::wstring, std::set<uint32_t>>& invertedIndex,
		const CString& outputPath
	);

	// ⭐ NEW: load version‑2 index
	static bool LoadBinaryIndex(const CString& inputPath, LoadedIndex& outIndex);

	static void Tokenize(const CString& comment, std::vector<CString>& outTokens);

	static UpdateResult UpdateIndex
	(
		const CString& baseFolder, const CString& indexPath
	);

private:
	static CString MakeRelativePath(const CString& fullPath);
};

