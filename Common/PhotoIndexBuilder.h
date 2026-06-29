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

	/////////////////////////////////////////////////////////////////////////////
	// SaveBinaryIndex
	//
	// Builds and writes a complete binary PHIX index from a MAP_INDEX structure.
	// This is the main index builder used by BuildPhotoIndex.
	//
	// Responsibilities:
	//   1. Convert absolute image paths → relative paths for portability
	//   2. Tokenize each image's comment text into search tokens
	//   3. Build an inverted index: token → [imageID, imageID, ...]
	//   4. Sort and deduplicate posting lists
	//   5. Write the PHIX header (magic, version, counts, timestamp)
	//   6. Write the image table (path, lastWriteTime, fileSize)
	//   7. Write the token table + posting lists
	//
	// Output format is identical to the original PHIX v1 format,
	// with additions for v2 (indexTimestamp, file metadata).
	//
	// Returns true on success, false on any I/O or format error.
	/////////////////////////////////////////////////////////////////////////////
	static bool SaveBinaryIndex(MAP_INDEX& mapIndex, const CString& outputPath);

	/////////////////////////////////////////////////////////////////////////////
	// SaveBinaryIndexFromComponents
	//
	// Writes a PHIX binary index using pre‑constructed components:
	//
	//   - imageTable: vector<ImageEntry>
	//   - invertedIndex: map<wstring, set<uint32_t>>
	//
	// This version is used by PhotoPrinter and PhotoExplorer when they
	// rebuild or update an index incrementally.
	//
	// Differences from SaveBinaryIndex():
	//   • Accepts already‑tokenized and already‑sorted data
	//   • Uses std::set for posting lists (auto‑sorted + deduped)
	//   • Does not compute relative paths — caller must supply them
	//
	// Writes:
	//   1. Header (magic, version, counts, timestamp)
	//   2. Image table (path, lastWriteTime, fileSize)
	//   3. Token table + posting lists
	//
	// Returns true on success.
	/////////////////////////////////////////////////////////////////////////////
	static bool SaveBinaryIndexFromComponents
	(
		const std::vector<ImageEntry>& imageTable,
		const std::map<std::wstring, std::set<uint32_t>>& invertedIndex,
		const CString& outputPath
	);

	/////////////////////////////////////////////////////////////////////////////
	// LoadBinaryIndex
	//
	// Reads a PHIX v2 binary index from disk and reconstructs:
	//
	//   • indexTimestamp
	//   • image table (path, lastWriteTime, fileSize)
	//   • inverted index (token → sorted set of image IDs)
	//
	// This loader is used by:
	//   • QueryPhotoIndex
	//   • PhotoPrinter (to detect stale images)
	//   • PhotoExplorer (to support fast metadata search)
	//
	// Format read:
	//   Header:
	//     magic ('PHIX'), version (2), imageCount, tokenCount, timestamp
	//
	//   Image table:
	//     uint16_t len
	//     WCHAR[len] path
	//     uint64_t lastWriteTime
	//     uint64_t fileSize
	//
	//   Token table:
	//     uint16_t len
	//     WCHAR[len] token
	//     uint32_t count
	//     uint32_t[count] imageIDs
	//
	// Posting lists are stored as sets to maintain sorted order and dedup.
	//
	// Returns true on success, false on any I/O or format error.
	/////////////////////////////////////////////////////////////////////////////
	static bool LoadBinaryIndex(const CString& inputPath, LoadedIndex& outIndex);

	/////////////////////////////////////////////////////////////////////////////
	// Tokenize
	//
	// Tokenizes a comment string into normalized search tokens.
	// Delegates to CHelper::Tokenize(), which performs:
	//
	//   • lowercase conversion
	//   • punctuation stripping
	//   • possessive handling ("block's" → "block")
	//   • splitting on whitespace and punctuation
	//
	// This wrapper exists so PhotoIndexBuilder always uses the same
	// tokenization logic as QueryPhotoIndex and PhotoPrinter.
	//
	// Produces tokens suitable for insertion into the inverted index.
	/////////////////////////////////////////////////////////////////////////////
	static void Tokenize(const CString& comment, std::vector<CString>& outTokens);

	/////////////////////////////////////////////////////////////////////////////
	// UpdateIndex  (CPhotoIndexBuilder)
	//
	// Performs an incremental index update:
	//
	//   1. Load existing index (if present)
	//   2. Discover all labeled images
	//   3. Detect:
	//        • new images
	//        • modified images (timestamp or size changed)
	//        • deleted images
	//   4. Preserve existing image IDs (ID‑stable update)
	//   5. Remove postings for deleted/modified images
	//   6. Re‑tokenize modified images and re‑insert postings
	//   7. Append new images with new IDs
	//   8. Save updated index
	//
	// Returns UpdateResult containing lists of:
	//   • newImages
	//   • modifiedImages
	//   • deletedImages
	//   • success/errorMessage
	//
	// This is the engine behind PhotoPrinter’s “Update Index” feature.
	/////////////////////////////////////////////////////////////////////////////
	static UpdateResult UpdateIndex
	(
		const CString& baseFolder, const CString& indexPath
	);

private:
	/////////////////////////////////////////////////////////////////////////////
	// MakeRelativePath
	//
	// Converts an absolute image path into a portable relative path (".\...").
	// This ensures the binary index can be moved between machines, drives,
	// or directory structures without breaking image references.
	//
	// Uses CHelper::ToRelative(), which compares the path against the current
	// working directory and rewrites it as:
	//
	//     C:\Users\wtblo\LocalDocs\Photographs\1901\IMG.jpg
	//     → .\1901\IMG.jpg
	//
	// If the path is not under the working directory, it is returned unchanged.
	/////////////////////////////////////////////////////////////////////////////
	static CString MakeRelativePath(const CString& fullPath);
};

