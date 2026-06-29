/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "PhotoIndexBuilder.h"
#include "CHelper.h"
#include <set>
#include <vector>
#include <unordered_map>

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
class CPhotoIndexRebuildSession
{
public:
	struct Result
	{
		bool success = false;
		CString errorMessage;

		std::vector<CPhotoIndexBuilder::ImageEntry> imageTable;

		// Deterministic token ordering + deduped, sorted posting lists
		std::map<std::wstring, std::set<uint32_t>> invertedIndex;
	};

public:
	CPhotoIndexRebuildSession();
	~CPhotoIndexRebuildSession();

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
	bool Initialize(const CString& baseFolder);

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
	bool Step();

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
	bool IsDone() const;

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
	const Result& GetResult() const;

	/////////////////////////////////////////////////////////////////////////////
	// GetProcessedCount
	//
	// Returns number of images processed so far.
	// Used for progress bars.
	/////////////////////////////////////////////////////////////////////////////
	int GetProcessedCount() const;

	/////////////////////////////////////////////////////////////////////////////
	// GetTotalCount
	//
	// Returns total number of images discovered.
	// Used for progress bars and UI status.
	/////////////////////////////////////////////////////////////////////////////
	int GetTotalCount() const;

	/////////////////////////////////////////////////////////////////////////////
	// GetLastError
	//
	// Returns the most recent non‑fatal error encountered during Step().
	// Useful for logging or UI display.
	/////////////////////////////////////////////////////////////////////////////
	CString GetLastError() const;

	const std::vector<CString>& GetAllImages() const { return m_allImages; }

	/////////////////////////////////////////////////////////////////////////////
	// IsApostropheLike
	//
	// Returns true if the character is any “apostrophe‑like” Unicode variant
	// that should be treated as a normal ASCII apostrophe ('), including:
	//
	//   U+0027  '   ASCII apostrophe
	//   U+2019  ’   right single quotation mark
	//   U+2018  ‘   left single quotation mark
	//   U+201B  ‛   single high‑reversed‑9 quotation mark
	//   U+FF07  ＇  fullwidth apostrophe
	//   U+02BC  ʼ  modifier letter apostrophe
	//   U+02C8  ˈ  modifier letter vertical line
	//   U+A78C  ꞌ  Latin small letter saltillo
	//
	// Used by NormalizeWhitespace() to canonicalize all of these to ASCII '
	// so that tokenization and possessive handling behave consistently.
	/////////////////////////////////////////////////////////////////////////////
	static bool IsApostropheLike(WCHAR c)
	{
		switch (c)
		{
		case 0x0027: return true; // '
		case 0x2019: return true; // ’
		case 0x2018: return true; // ‘
		case 0x201B: return true; // ‛
		case 0xFF07: return true; // ＇
		case 0x02BC: return true; // ʼ
		case 0x02C8: return true; // ˈ
		case 0xA78C: return true; // ꞌ
		default: return false;
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// NormalizeWhitespace
	//
	// Normalizes whitespace and apostrophe‑like characters in a CString:
	//
	//   • Converts all apostrophe‑like Unicode characters → ASCII '
	//   • Converts all Unicode whitespace (including NBSP U+00A0) → space ' '
	//   • Replaces zero‑width characters (U+200B, U+FEFF) with space
	//
	// This ensures that comments coming from mixed sources (Word, browsers,
	// scanners, phones) are normalized into a clean, tokenizable form where
	// apostrophes and spaces behave predictably.
	/////////////////////////////////////////////////////////////////////////////
	static void NormalizeWhitespace(CString& s)
	{
		for (int i = 0; i < s.GetLength(); i++)
		{
			WCHAR c = s[i];

			if (IsApostropheLike(c))
				s.SetAt(i, L'\''); // normalize to ASCII apostrophe

			// Normalize ALL Unicode whitespace
			if (iswspace(c) || c == 0xA0) // NBSP
			{
				s.SetAt(i, L' ');
			}

			// Remove zero-width characters
			else if (c == 0x200B || c == 0xFEFF)
			{
				s.SetAt(i, L' ');
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// NormalizeComment
	//
	// High‑level normalization for XPComment strings before tokenization:
	//
	//   • Converts to lowercase
	//   • Normalizes whitespace and apostrophes via NormalizeWhitespace()
	//   • Trims leading/trailing spaces
	//
	// This is the canonical pre‑processing step for all comment text used in
	// indexing and querying, ensuring that different input sources produce
	// consistent tokens.
	/////////////////////////////////////////////////////////////////////////////
	static void NormalizeComment(CString& s)
	{
		s.MakeLower();
		NormalizeWhitespace(s);
		s.Trim();
	}

private:
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
	bool DiscoverLabeledFolders(const CString& baseFolder);

	/////////////////////////////////////////////////////////////////////////////
	// DiscoverLabeledImages
	//
	// Validates that DiscoverLabeledFolders() found at least one image.
	//
	// Returns false if no labeled images exist.
	/////////////////////////////////////////////////////////////////////////////
	bool DiscoverLabeledImages();

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
	bool ProcessImage(const CString& absPath);

private:
	// Internal state
	std::vector<CString> m_allImages;   // absolute paths
	size_t m_currentIndex = 0;

	Result m_result;

	// Temporary structures used during rebuild
	std::map<std::wstring, std::set<uint32_t>> m_tempInverted;
};

