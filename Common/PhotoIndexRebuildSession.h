/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "PhotoIndexBuilder.h"
#include "CHelper.h"
#include <set>
#include <vector>
#include <unordered_map>

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

	// Initialize the rebuild session
	// baseFolder = root of the photo library
	bool Initialize(const CString& baseFolder);

	// Process a single image
	// Returns false if an unrecoverable error occurs
	bool Step();

	// True when all images have been processed
	bool IsDone() const;

	// Final result (valid only when IsDone() == true)
	const Result& GetResult() const;

	// Progress information
	int GetProcessedCount() const;
	int GetTotalCount() const;

	// Optional: last error encountered during Step()
	CString GetLastError() const;

	const std::vector<CString>& GetAllImages() const { return m_allImages; }

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

	// a helper to make tokenization of comments work correctly
	static void NormalizeComment(CString& s)
	{
		s.MakeLower();
		NormalizeWhitespace(s);
		s.Trim();
	}

private:
	// Internal helpers
	bool DiscoverLabeledFolders(const CString& baseFolder);
	bool DiscoverLabeledImages();

	bool ProcessImage(const CString& absPath);

private:
	// Internal state
	std::vector<CString> m_allImages;   // absolute paths
	size_t m_currentIndex = 0;

	Result m_result;

	// Temporary structures used during rebuild
	std::map<std::wstring, std::set<uint32_t>> m_tempInverted;
};

