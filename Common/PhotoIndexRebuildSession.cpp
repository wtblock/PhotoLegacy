/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoIndexRebuildSession.h"
#include "PhotoIndexBuilder.h"
#include "CHelper.h"
#include <gdiplus.h>
#include <algorithm>

using namespace Gdiplus;

// ------------------------------------------------------------
// CPhotoIndexRebuildSession
// ------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CPhotoIndexRebuildSession
//
// Drives a full or incremental rebuild of the photo index by walking the
// folder tree, discovering all labeled images, extracting metadata, and
// producing:
//
//   • imageTable      → vector<ImageEntry>
//   • invertedIndex   → map<wstring, set<uint32_t>>
//
// The rebuild is performed incrementally via Step(), allowing UI progress
// bars in PhotoPrinter and PhotoExplorer.
//
// This class is used by:
//   • BuildPhotoIndex (full rebuild)
//   • PhotoPrinter (incremental rebuild/update)
//   • PhotoExplorer (index refresh)
//
// The session is stateful and processes one image per Step().
/////////////////////////////////////////////////////////////////////////////
CPhotoIndexRebuildSession::CPhotoIndexRebuildSession()
	: m_currentIndex(0)
{
}

/////////////////////////////////////////////////////////////////////////////
// Destructor — no special cleanup required
/////////////////////////////////////////////////////////////////////////////
CPhotoIndexRebuildSession::~CPhotoIndexRebuildSession()
{
}

/////////////////////////////////////////////////////////////////////////////
// Initialize
//
// Prepares the rebuild session by:
//
//   1. Clearing previous results
//   2. Discovering all labeled folders under baseFolder
//   3. Discovering all labeled images (*.jpg) inside those folders
//   4. Pre‑allocating the image table
//
// On success, the session is ready for Step() calls.
// On failure, errorMessage is populated.
//
// Returns true if initialization succeeded.
/////////////////////////////////////////////////////////////////////////////
bool CPhotoIndexRebuildSession::Initialize(const CString& baseFolder)
{
	m_result = Result{};
	m_allImages.clear();
	m_currentIndex = 0;
	m_tempInverted.clear();

	// 1) Discover labeled folders
	if (!DiscoverLabeledFolders(baseFolder))
	{
		m_result.success = false;
		if (m_result.errorMessage.IsEmpty())
			m_result.errorMessage = L"Failed to discover labeled folders.";
		return false;
	}

	// 2) Discover labeled images
	if (!DiscoverLabeledImages())
	{
		m_result.success = false;
		if (m_result.errorMessage.IsEmpty())
			m_result.errorMessage = L"Failed to discover labeled images.";
		return false;
	}

	// Pre‑allocate image table
	m_result.imageTable.clear();
	m_result.imageTable.reserve(m_allImages.size());

	m_result.success = true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Step
//
// Processes exactly ONE image from m_allImages:
//
//   • Loads the image
//   • Extracts XPComment metadata
//   • Normalizes comment text
//   • Tokenizes comment into search tokens
//   • Inserts tokens into temporary inverted index
//   • Appends ImageEntry to imageTable
//
// When the final image is processed, Step() finalizes the inverted index
// by moving m_tempInverted → m_result.invertedIndex.
//
// Returns true even on non‑fatal image errors (bad JPG, missing metadata).
/////////////////////////////////////////////////////////////////////////////
bool CPhotoIndexRebuildSession::Step()
{
	if (IsDone())
		return true; // nothing to do

	const CString absPath = m_allImages[m_currentIndex];

	if (!ProcessImage(absPath))
	{
		// Non‑fatal: we log error but continue
		// If you want fatal behavior, flip this to return false.
	}

	++m_currentIndex;

	// When done, finalize inverted index into result
	if (IsDone())
	{
		m_result.invertedIndex = std::move(m_tempInverted);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// IsDone
//
// Returns true when all images have been processed.
// Used by UI loops:
//
//     while (!session.IsDone())
//         session.Step();
//
/////////////////////////////////////////////////////////////////////////////
bool CPhotoIndexRebuildSession::IsDone() const
{
	return m_currentIndex >= m_allImages.size();
}

/////////////////////////////////////////////////////////////////////////////
// GetResult
//
// Returns the final rebuild result:
//
//   • success
//   • errorMessage
//   • imageTable
//   • invertedIndex
//
// Valid only after IsDone() == true.
/////////////////////////////////////////////////////////////////////////////
const CPhotoIndexRebuildSession::Result& 
CPhotoIndexRebuildSession::GetResult() const
{
	return m_result;
}

/////////////////////////////////////////////////////////////////////////////
// GetProcessedCount
//
// Returns number of images processed so far.
// Used for progress bars.
/////////////////////////////////////////////////////////////////////////////
int CPhotoIndexRebuildSession::GetProcessedCount() const
{
	return static_cast<int>(m_currentIndex);
}

/////////////////////////////////////////////////////////////////////////////
// GetTotalCount
//
// Returns total number of images discovered.
// Used for progress bars and UI status.
/////////////////////////////////////////////////////////////////////////////
int CPhotoIndexRebuildSession::GetTotalCount() const
{
	return static_cast<int>(m_allImages.size());
}

/////////////////////////////////////////////////////////////////////////////
// GetLastError
//
// Returns the most recent non‑fatal error encountered during Step().
// Useful for logging or UI display.
/////////////////////////////////////////////////////////////////////////////
CString CPhotoIndexRebuildSession::GetLastError() const
{
	return m_result.errorMessage;
}

/////////////////////////////////////////////////////////////////////////////
// DiscoverLabeledFolders
//
// Recursively walks the folder tree starting at baseFolder.
//
// For each directory:
//   • If a "Labeled" subfolder exists, collect all *.jpg files inside it
//   • Recurse into subdirectories
//
// Populates m_allImages with absolute paths.
//
// This mirrors the original ListLabeledFolders logic used by PhotoPrinter,
// but stores results in memory instead of writing to disk.
//
// Returns true unless a filesystem error occurs.
/////////////////////////////////////////////////////////////////////////////
bool CPhotoIndexRebuildSession::DiscoverLabeledFolders(const CString& baseFolder)
{
	// Recursively walk the folder tree and find all "Labeled" subfolders.
	// For each labeled folder, collect all JPG files into m_allImages.

	CFileFind finder;
	CString search = baseFolder + L"\\*.*";

	BOOL bWorking = finder.FindFile(search);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsDots())
			continue;

		if (finder.IsDirectory())
		{
			CString folderPath = finder.GetFilePath();

			// Check for "Labeled" subfolder
			CString labeledPath = folderPath + L"\\Labeled";
			if (::PathFileExists(labeledPath))
			{
				// Found a labeled folder — collect images
				CFileFind ff;
				CString pattern = labeledPath + L"\\*.jpg";

				BOOL bImg = ff.FindFile(pattern);
				while (bImg)
				{
					bImg = ff.FindNextFile();
					if (ff.IsDots() || ff.IsDirectory())
						continue;

					CString absPath = ff.GetFilePath();
					m_allImages.push_back(absPath);
				}
				ff.Close();
			}

			// Recurse into subfolder
			DiscoverLabeledFolders(folderPath);
		}
	}

	finder.Close();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// DiscoverLabeledImages
//
// Validates that DiscoverLabeledFolders() found at least one image.
//
// Returns false if no labeled images exist.
/////////////////////////////////////////////////////////////////////////////
bool CPhotoIndexRebuildSession::DiscoverLabeledImages()
{
	// Nothing to do — m_allImages is already populated
	// by DiscoverLabeledFolders().
	//
	// But we return false if no images were found.

	if (m_allImages.empty())
	{
		m_result.errorMessage = L"No labeled images found.";
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// ProcessImage
//
// Processes a single image:
//
//   1. Load image via GDI+
//   2. Extract XPComment metadata
//   3. Normalize comment (lowercase, punctuation, whitespace)
//   4. Tokenize comment into raw tokens
//   5. Normalize each token (possessive handling, cleanup)
//   6. Insert tokens into m_tempInverted[token].insert(imageID)
//   7. Build ImageEntry (relative path, timestamp, size)
//   8. Append ImageEntry to imageTable
//
// Non‑fatal errors (bad JPG, missing metadata) are logged but do not stop
// the rebuild.
//
// Returns true on success, false on non‑fatal image load errors.
/////////////////////////////////////////////////////////////////////////////
bool CPhotoIndexRebuildSession::ProcessImage(const CString& absPath)
{
	CString csError;

	// Load image
	std::shared_ptr<Image> pImage =
		CHelper::LoadImageFromFile(absPath, csError);

	if (pImage == nullptr)
	{
		// Non‑fatal: record last error, continue
		m_result.errorMessage = csError;
		return false;
	}

	// Extract XPComment
	CString csComment = CHelper::GetXPComment(pImage.get());

	// Normalize comment
	NormalizeComment(csComment);

	// Build ImageEntry
	uint64_t t = 0, s = 0;
	CHelper::GetFileTimestampAndSize(absPath, t, s);

	CString relPath = CHelper::ToRelative(absPath);

	CPhotoIndexBuilder::ImageEntry entry;
	entry.path = relPath;
	entry.lastWriteTime = t;
	entry.fileSize = s;

	uint32_t newID = static_cast<uint32_t>(m_result.imageTable.size());
	m_result.imageTable.push_back(entry);

	// Tokenize comment and update inverted index
	std::vector<CString> rawTokens;
	CHelper::Tokenize(csComment, rawTokens);

	for (CString tok : rawTokens)
	{
		CHelper::NormalizeToken(tok);   // per-token cleanup
		if (!tok.IsEmpty())
		{
			std::wstring w(tok.GetString());
			m_tempInverted[w].insert(newID);   // <-- deterministic + deduped
		}
	}

	return true;
}

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
CPhotoIndexBuilder::UpdateResult CPhotoIndexBuilder::UpdateIndex
(
	const CString& baseFolder, const CString& indexPath
)
{
	UpdateResult result;

	// ------------------------------------------------------------
	// 1. Load existing index (if any)
	// ------------------------------------------------------------
	LoadedIndex existing;
	bool hasExisting = LoadBinaryIndex(indexPath, existing);

	// ------------------------------------------------------------
	// 2. Discover all labeled images (absolute paths)
	// ------------------------------------------------------------
	std::vector<CString> allImages;

	// Reuse the rebuild session’s folder discovery logic
	{
		CPhotoIndexRebuildSession session;
		if (!session.Initialize(baseFolder))
		{
			result.success = false;
			result.errorMessage = session.GetLastError();
			return result;
		}

		// We only want the discovered image list, not a rebuild
		allImages.reserve(session.GetTotalCount());
		const auto& imgs = session.GetAllImages();
		for (const CString& path : imgs)
			allImages.push_back(path);
	}

	// ------------------------------------------------------------
	// 3. If no existing index → full rebuild
	// ------------------------------------------------------------
	if (!hasExisting)
	{
		// Use the rebuild session for full rebuild
		CPhotoIndexRebuildSession session;
		if (!session.Initialize(baseFolder))
		{
			result.success = false;
			result.errorMessage = session.GetLastError();
			return result;
		}

		while (!session.IsDone())
			session.Step();

		const auto& r = session.GetResult();

		SaveBinaryIndexFromComponents(r.imageTable, r.invertedIndex, indexPath);

		result.success = true;
		result.newImages = allImages; // everything is new
		return result;
	}

	// ------------------------------------------------------------
	// 4. Build lookup of existing images by relative path
	// ------------------------------------------------------------
	std::unordered_map<std::wstring, size_t> existingMap;
	for (size_t i = 0; i < existing.images.size(); ++i)
		existingMap[existing.images[i].path.GetString()] = i;

	std::vector<bool> seen(existing.images.size(), false);

	// ------------------------------------------------------------
	// 5. Detect new + modified images
	// ------------------------------------------------------------
	for (const CString& absPath : allImages)
	{
		CString relPath = CHelper::ToRelative(absPath);
		std::wstring wrel(relPath.GetString());

		uint64_t t = 0, s = 0;
		CHelper::GetFileTimestampAndSize(absPath, t, s);

		auto it = existingMap.find(wrel);
		if (it == existingMap.end())
		{
			result.newImages.push_back(absPath);
		}
		else
		{
			size_t idx = it->second;
			seen[idx] = true;

			const auto& old = existing.images[idx];
			if (old.lastWriteTime != t || old.fileSize != s)
				result.modifiedImages.push_back(absPath);
		}
	}

	// ------------------------------------------------------------
	// 6. Detect deleted images
	// ------------------------------------------------------------
	for (size_t i = 0; i < seen.size(); ++i)
	{
		if (!seen[i])
			result.deletedImages.push_back(existing.images[i].path);
	}

	// ------------------------------------------------------------
	// 7. Update image table in place (ID-stable)
	// ------------------------------------------------------------
	std::vector<ImageEntry> newTable = existing.images;  // start from existing

	// Deleted: mark as deleted but KEEP the slot
	for (const CString& rel : result.deletedImages)
	{
		auto it = existingMap.find(rel.GetString());
		if (it != existingMap.end())
		{
			size_t idx = it->second;
			// keep path so ID is preserved
			newTable[idx].lastWriteTime = 0;
			newTable[idx].fileSize = 0;
			// (optional) you could also add a "deleted" flag in the future
		}
	}

	// Modified: update metadata IN PLACE, keep same ID
	for (const CString& absPath : result.modifiedImages)
	{
		uint64_t t = 0, s = 0;
		CHelper::GetFileTimestampAndSize(absPath, t, s);

		CString rel = CHelper::ToRelative(absPath);
		auto it = existingMap.find(rel.GetString());
		if (it != existingMap.end())
		{
			size_t idx = it->second;
			newTable[idx].lastWriteTime = t;
			newTable[idx].fileSize = s;
		}
	}

	// New: APPEND ONLY, new IDs start after existing.size()
	for (const CString& absPath : result.newImages)
	{
		uint64_t t = 0, s = 0;
		CHelper::GetFileTimestampAndSize(absPath, t, s);

		CString rel = CHelper::ToRelative(absPath);

		ImageEntry e;
		e.path = rel;
		e.lastWriteTime = t;
		e.fileSize = s;

		newTable.push_back(e);
	}

	// ------------------------------------------------------------
	// 8. Build new inverted index
	// ------------------------------------------------------------
	std::map<std::wstring, std::set<uint32_t>> newInverted = existing.invertedIndex;

	auto removePosting = [&](uint32_t id)
	{
		for (auto& kv : newInverted)
			kv.second.erase(id);
	};

	// Deleted
	for (const CString& rel : result.deletedImages)
	{
		auto it = existingMap.find(rel.GetString());
		if (it != existingMap.end())
			removePosting((uint32_t)it->second);
	}

	// Modified
	for (const CString& abs : result.modifiedImages)
	{
		CString rel = CHelper::ToRelative(abs);
		auto it = existingMap.find(rel.GetString());
		if (it != existingMap.end())
			removePosting((uint32_t)it->second);
	}

	auto addPostings = [&](uint32_t id, CString& comment)
	{
		CPhotoIndexRebuildSession::NormalizeComment(comment);

		std::vector<CString> rawTokens;
		CHelper::Tokenize(comment, rawTokens);

		for (CString tok : rawTokens)
		{
			CHelper::NormalizeToken(tok);
			if (!tok.IsEmpty())
			{
				std::wstring w(tok.GetString());
				newInverted[w].insert(id);
			}
		}
	};

	// Modified: re-add with SAME ID
	for (const CString& abs : result.modifiedImages)
	{
		CString rel = CHelper::ToRelative(abs);
		auto it = existingMap.find(rel.GetString());
		if (it != existingMap.end())
		{
			uint32_t id = (uint32_t)it->second;
			CString comment = CHelper::GetXPCommentFromFile(abs);
			addPostings(id, comment);
		}
	}

	// New: IDs start at existing.images.size()
	uint32_t nextID = (uint32_t)existing.images.size();
	for (const CString& abs : result.newImages)
	{
		CString comment = CHelper::GetXPCommentFromFile(abs);
		addPostings(nextID, comment);
		nextID++;
	}

	// ------------------------------------------------------------
	// 9. Save updated index
	// ------------------------------------------------------------
	SaveBinaryIndexFromComponents(newTable, newInverted, indexPath);

	result.success = true;
	return result;
}

/////////////////////////////////////////////////////////////////////////////
