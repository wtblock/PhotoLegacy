/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoIndexTypes.h"
#include "PhotoIndexBuilder.h"
#include <algorithm>
#include "CHelper.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
struct IndexHeader
{
	uint32_t magic;        // 'PHIX'
	uint32_t version;      // 2  (was 1)
	uint32_t imageCount;
	uint32_t tokenCount;
	uint64_t indexTimestamp;   // NEW: when this index was written
};

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
CString CPhotoIndexBuilder::MakeRelativePath(const CString& fullPath)
{
	return CHelper::ToRelative(fullPath);
}

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
bool CPhotoIndexBuilder::SaveBinaryIndex
(
	MAP_INDEX& mapIndex, const CString& outputPath
)
{
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, outputPath, L"wb") != 0 || !fp)
		return false;

	// Determine the root folder (working directory)
	WCHAR cwdBuf[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, cwdBuf);
	CString rootFolder(cwdBuf);
	if (!rootFolder.IsEmpty() && rootFolder.Right(1) != L"\\")
		rootFolder += L"\\";

	// Determine index timestamp (now)
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	ULARGE_INTEGER uli;
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	uint64_t indexTimestamp = uli.QuadPart;

	// ------------------------------------------------------------
	// 1. Assign numeric IDs to images and build filename table
	// ------------------------------------------------------------
	// We convert absolute paths to relative paths (.\...) so the
	// index becomes portable across machines and drives.
	// ------------------------------------------------------------

	vector<CString> imagePaths;
	imagePaths.reserve(mapIndex.Count);

	unordered_map<wstring, vector<uint32_t>> invertedIndex;
	invertedIndex.reserve(mapIndex.Count * 4);

	uint32_t imageID = 0;

	for (auto& pair : mapIndex.Items)
	{
		const CString& absPath = pair.first;
		const CString& comment = *(pair.second);

		// Convert to relative path if under root
		CString relPath = MakeRelativePath(absPath);
		imagePaths.push_back(relPath);

		// Tokenize comment
		vector<CString> tokens;
		Tokenize(comment, tokens);

		// Insert tokens into inverted index
		for (const CString& tok : tokens)
		{
			wstring key(tok.GetString());
			invertedIndex[key].push_back(imageID);
		}

		imageID++;
	}

	// ------------------------------------------------------------
	// 2. Sort posting lists for fast AND/OR queries
	// ------------------------------------------------------------
	for (auto& entry : invertedIndex)
	{
		auto& vec = entry.second;
		sort(vec.begin(), vec.end());
		vec.erase(unique(vec.begin(), vec.end()), vec.end());
	}

	// ------------------------------------------------------------
	// 3. Write header
	// ------------------------------------------------------------
	IndexHeader hdr;
	hdr.magic = 0x50484958;   // 'PHIX'
	hdr.version = 2;
	hdr.imageCount = (uint32_t)imagePaths.size();
	hdr.tokenCount = (uint32_t)invertedIndex.size();
	hdr.indexTimestamp = indexTimestamp;

	fwrite(&hdr, sizeof(hdr), 1, fp);

	// ------------------------------------------------------------
	// 4. Write image table
	// ------------------------------------------------------------
	// Each entry:
	//   uint16_t len      → number of WCHARs
	//   WCHAR[len]        → UTF‑16 path (relative or absolute)
	// ------------------------------------------------------------
	for (const CString& path : imagePaths)
	{
		// write path
		uint16_t len = (uint16_t)path.GetLength();
		fwrite(&len, sizeof(len), 1, fp);
		fwrite(path.GetString(), sizeof(WCHAR), len, fp);

		// get file attributes for timestamp + size
		WIN32_FILE_ATTRIBUTE_DATA fad = {};
		uint64_t lastWriteTime = 0;
		uint64_t fileSize = 0;

		CString absPath = path;
		// if stored as .\relative, convert back to absolute using rootFolder
		if (path.Left(2) == L".\\")
		{
			absPath = rootFolder + path.Mid(2);
		}

		if (GetFileAttributesEx(absPath, GetFileExInfoStandard, &fad))
		{
			ULARGE_INTEGER uliTime;
			uliTime.LowPart = fad.ftLastWriteTime.dwLowDateTime;
			uliTime.HighPart = fad.ftLastWriteTime.dwHighDateTime;
			lastWriteTime = uliTime.QuadPart;

			ULARGE_INTEGER uliSize;
			uliSize.LowPart = fad.nFileSizeLow;
			uliSize.HighPart = fad.nFileSizeHigh;
			fileSize = uliSize.QuadPart;
		}

		fwrite(&lastWriteTime, sizeof(lastWriteTime), 1, fp);
		fwrite(&fileSize, sizeof(fileSize), 1, fp);
	}

	// ------------------------------------------------------------
	// 5. Write token table + posting lists
	// ------------------------------------------------------------
	// Each token:
	//   uint16_t len      → WCHAR count
	//   WCHAR[len]        → token text
	//   uint32_t count    → posting list size
	//   uint32_t[count]   → sorted image IDs
	// ------------------------------------------------------------
	for (auto& entry : invertedIndex)
	{
		const wstring& token = entry.first;
		const vector<uint32_t>& postings = entry.second;

		uint16_t len = (uint16_t)token.length();
		fwrite(&len, sizeof(len), 1, fp);
		fwrite(token.data(), sizeof(WCHAR), len, fp);

		uint32_t count = (uint32_t)postings.size();
		fwrite(&count, sizeof(count), 1, fp);

		fwrite(postings.data(), sizeof(uint32_t), count, fp);
	}

	fclose(fp);
	return true;
}

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
bool CPhotoIndexBuilder::SaveBinaryIndexFromComponents
(
	const std::vector<ImageEntry>& imageTable,
	const std::map<std::wstring, std::set<uint32_t>>& invertedIndex,
	const CString& outputPath
)
{
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, outputPath, L"wb") != 0 || !fp)
		return false;

	// Compute index timestamp (now)
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	ULARGE_INTEGER uli;
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	uint64_t indexTimestamp = uli.QuadPart;

	// ------------------------------------------------------------
	// 1. Write header
	// ------------------------------------------------------------
	IndexHeader hdr;
	hdr.magic = 0x50484958;   // 'PHIX'
	hdr.version = 2;
	hdr.imageCount = (uint32_t)imageTable.size();
	hdr.tokenCount = (uint32_t)invertedIndex.size();
	hdr.indexTimestamp = indexTimestamp;

	fwrite(&hdr, sizeof(hdr), 1, fp);

	// ------------------------------------------------------------
	// 2. Write image table
	//    Each entry:
	//      uint16_t len
	//      WCHAR[len] path
	//      uint64_t lastWriteTime
	//      uint64_t fileSize
	// ------------------------------------------------------------
	for (const auto& img : imageTable)
	{
		uint16_t len = (uint16_t)img.path.GetLength();
		fwrite(&len, sizeof(len), 1, fp);
		fwrite(img.path.GetString(), sizeof(WCHAR), len, fp);

		fwrite(&img.lastWriteTime, sizeof(img.lastWriteTime), 1, fp);
		fwrite(&img.fileSize, sizeof(img.fileSize), 1, fp);
	}

	// ------------------------------------------------------------
	// 3. Write token table + posting lists
	//    Each token:
	//      uint16_t len
	//      WCHAR[len] token
	//      uint32_t count
	//      uint32_t[count] imageIDs
	// ------------------------------------------------------------
	for (const auto& kv : invertedIndex)
	{
		const std::wstring& token = kv.first;
		const std::set<uint32_t>& idSet = kv.second;

		// Convert set → vector for writing (keeps on‑disk format unchanged)
		std::vector<uint32_t> postings(idSet.begin(), idSet.end());

		uint16_t len = (uint16_t)token.length();
		fwrite(&len, sizeof(len), 1, fp);
		fwrite(token.data(), sizeof(WCHAR), len, fp);

		uint32_t count = (uint32_t)postings.size();
		fwrite(&count, sizeof(count), 1, fp);

		if (count > 0)
		{
			fwrite(postings.data(), sizeof(uint32_t), count, fp);
		}
	}

	fclose(fp);
	return true;
}

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
bool CPhotoIndexBuilder::LoadBinaryIndex
(
	const CString& inputPath,
	LoadedIndex& outIndex
)
{
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, inputPath, L"rb") != 0 || !fp)
		return false;

	IndexHeader hdr = {};
	if (fread(&hdr, sizeof(hdr), 1, fp) != 1)
	{
		fclose(fp);
		return false;
	}

	// Validate header
	if (hdr.magic != 0x50484958 || hdr.version != 2)
	{
		fclose(fp);
		return false; // not our format
	}

	outIndex.indexTimestamp = hdr.indexTimestamp;
	outIndex.images.clear();
	outIndex.images.reserve(hdr.imageCount);
	outIndex.invertedIndex.clear();   // map: no reserve()

	// ------------------------------
	// Read image table
	// ------------------------------
	for (uint32_t i = 0; i < hdr.imageCount; ++i)
	{
		uint16_t len = 0;
		if (fread(&len, sizeof(len), 1, fp) != 1)
		{
			fclose(fp);
			return false;
		}

		std::vector<WCHAR> buf(len + 1, 0);
		if (fread(buf.data(), sizeof(WCHAR), len, fp) != len)
		{
			fclose(fp);
			return false;
		}

		CString path(buf.data(), len);

		uint64_t lastWriteTime = 0;
		uint64_t fileSize = 0;

		if (fread(&lastWriteTime, sizeof(lastWriteTime), 1, fp) != 1 ||
			fread(&fileSize, sizeof(fileSize), 1, fp) != 1)
		{
			fclose(fp);
			return false;
		}

		ImageEntry entry;
		entry.path = path;
		entry.lastWriteTime = lastWriteTime;
		entry.fileSize = fileSize;

		outIndex.images.push_back(entry);
	}

	// ------------------------------
	// Read token table + posting lists
	// ------------------------------
	for (uint32_t t = 0; t < hdr.tokenCount; ++t)
	{
		uint16_t len = 0;
		if (fread(&len, sizeof(len), 1, fp) != 1)
		{
			fclose(fp);
			return false;
		}

		std::vector<WCHAR> buf(len + 1, 0);
		if (fread(buf.data(), sizeof(WCHAR), len, fp) != len)
		{
			fclose(fp);
			return false;
		}

		std::wstring token(buf.data(), len);

		uint32_t count = 0;
		if (fread(&count, sizeof(count), 1, fp) != 1)
		{
			fclose(fp);
			return false;
		}

		std::set<uint32_t> idSet;

		if (count > 0)
		{
			for (uint32_t i = 0; i < count; ++i)
			{
				uint32_t id = 0;
				if (fread(&id, sizeof(id), 1, fp) != 1)
				{
					fclose(fp);
					return false;
				}
				idSet.insert(id);   // dedup + sorted
			}
		}

		outIndex.invertedIndex.emplace(std::move(token), std::move(idSet));
	}

	fclose(fp);
	return true;
}

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
void CPhotoIndexBuilder::Tokenize
(
	const CString& comment,
	vector<CString>& outTokens
)
{
	CHelper::Tokenize(comment, outTokens);
}

/////////////////////////////////////////////////////////////////////////////
