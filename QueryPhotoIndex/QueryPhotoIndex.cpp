/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "QueryPhotoIndex.h"
#include "PhotoIndexRebuildSession.h"
#include <set>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object
CWinApp theApp;

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// initialize GDI+
bool InitGdiplus()
{
	GdiplusStartupInput gdiplusStartupInput;
	Status status = GdiplusStartup
	(
		&m_gdiplusToken,
		&gdiplusStartupInput,
		NULL
	);
	return (Ok == status);
} // InitGdiplus

/////////////////////////////////////////////////////////////////////////////
// remove reference to GDI+
void TerminateGdiplus()
{
	// remove gdi images from memory before shutting down gdi
	m_mapAlbums.clear();

	GdiplusShutdown(m_gdiplusToken);
	m_gdiplusToken = NULL;

}// TerminateGdiplus

/////////////////////////////////////////////////////////////////////////////
// Load the binary index from disk
void DumpIndex(bool showPostings)
{
	size_t nImages = m_loadedIndex.images.size();
	size_t nInverted = m_loadedIndex.invertedIndex.size();

	wprintf(L"===== PHOTO INDEX DUMP =====\n");
	wprintf(L"Image count: %zu\n", nImages);
	wprintf(L"Token count: %zu\n\n", nInverted);

	wprintf(L"-- Image Paths --\n");
	for (size_t i = 0; i < nImages; i++)
	{
		wprintf(L"[%zu] %s\n", i, m_loadedIndex.images[i].path.GetString());
	}

	wprintf(L"\n-- Tokens --\n");
	for (const auto& kv : m_loadedIndex.invertedIndex)
	{
		const std::wstring& token = kv.first;
		const std::set<uint32_t>& postings = kv.second;  // <-- changed from vector to set

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
}

/////////////////////////////////////////////////////////////////////////////
// cache the images within the labeled folders
// cache the queried images using the new inverted index
// cache the images within the labeled folders
// cache the queried images using the new inverted index
void CacheQueriedImages()
{
	m_queryResults.clear();

	CString csQuery = m_csQuery;
	if (csQuery.IsEmpty())
		return;

	// This will hold the final absolute paths for either OR or AND logic
	std::vector<CString> arrImages;

	// ===============================================================
	// Detect OR operator
	// ===============================================================
	bool hasOr = (csQuery.Find(L'|') != -1);

	// ===============================================================
	// OR LOGIC
	// ===============================================================
	if (hasOr)
	{
		// Split into OR groups
		std::vector<CString> orGroups;
		{
			int start = 0;
			int pos = 0;
			while ((pos = csQuery.Find(L'|', start)) != -1)
			{
				CString part = csQuery.Mid(start, pos - start).Trim();
				if (!part.IsEmpty())
					orGroups.push_back(part);
				start = pos + 1;
			}
			CString last = csQuery.Mid(start).Trim();
			if (!last.IsEmpty())
				orGroups.push_back(last);
		}

		std::set<uint32_t> finalIDs;

		// Evaluate each OR group independently
		for (const CString& group : orGroups)
		{
			std::vector<CString> tokens;
			CHelper::Tokenize(group, tokens);

			if (tokens.empty())
				continue;

			for (CString& t : tokens)
			{
				CHelper::NormalizeToken(t);
			}

			bool firstToken = true;
			std::set<uint32_t> groupIDs;

			for (const CString& tok : tokens)
			{
				std::wstring w(tok.GetString());

				auto it = m_loadedIndex.invertedIndex.find(w);
				if (it == m_loadedIndex.invertedIndex.end())
				{
					groupIDs.clear();
					break;
				}

				// CHANGED: vector -> set
				const std::set<uint32_t>& posting = it->second;

				if (firstToken)
				{
					groupIDs.insert(posting.begin(), posting.end());
					firstToken = false;
				}
				else
				{
					std::set<uint32_t> temp;
					for (uint32_t id : posting)
					{
						if (groupIDs.find(id) != groupIDs.end())
							temp.insert(id);
					}
					groupIDs.swap(temp);
				}

				if (groupIDs.empty())
					break;
			}

			// Union this group's results
			finalIDs.insert(groupIDs.begin(), groupIDs.end());
		}

		// Convert IDs to absolute paths
		for (uint32_t id : finalIDs)
		{
			const auto& entry = m_loadedIndex.images[id];
			CString abs = CHelper::ToAbsolute(entry.path);
			arrImages.push_back(abs);
		}
	}
	else
	{
		// ===============================================================
		// AND LOGIC (original behavior)
		// ===============================================================

		std::vector<CString> tokens;
		CHelper::Tokenize(csQuery, tokens);

		for (CString& t : tokens)
		{
			CHelper::NormalizeToken(t);
		}

		std::set<uint32_t> resultIDs;
		bool firstToken = true;

		for (const CString& tok : tokens)
		{
			std::wstring w(tok.GetString());

			auto it = m_loadedIndex.invertedIndex.find(w);
			if (it == m_loadedIndex.invertedIndex.end())
			{
				resultIDs.clear();
				break;
			}

			// CHANGED: vector -> set
			const std::set<uint32_t>& posting = it->second;

			if (firstToken)
			{
				resultIDs.insert(posting.begin(), posting.end());
				firstToken = false;
			}
			else
			{
				std::set<uint32_t> temp;
				for (uint32_t id : posting)
				{
					if (resultIDs.find(id) != resultIDs.end())
						temp.insert(id);
				}
				resultIDs.swap(temp);
			}

			if (resultIDs.empty())
				break;
		}

		// Convert IDs to absolute paths
		for (uint32_t id : resultIDs)
		{
			const auto& entry = m_loadedIndex.images[id];
			CString abs = CHelper::ToAbsolute(entry.path);
			arrImages.push_back(abs);
		}
	}

	// ===============================================================
	// Sort results (shared by both OR and AND)
	// ===============================================================
	std::sort(arrImages.begin(), arrImages.end(),
		[](const CString& a, const CString& b)
	{
		return a.CompareNoCase(b) < 0;
	});

	if (arrImages.empty())
	{
		AfxMessageBox(L"The query did not yield any results.", MB_OK);
		return;
	}

	// For your own tracking: keep the filtered list
	m_queryResults = arrImages;

	// This still sets Images the way your old code expected
	m_nImages = (UINT)arrImages.size();

	int nImage = 0;
	CString csCurrentAlbum;

	nImage = 0;

	std::vector<CString> arrAlbumImages;

	// clear old albums
	m_mapAlbums.clear();
	m_keyFolders.clear();

	for (auto& node : arrImages)
	{
		CString csImage = node;
		const CString csFile = CHelper::GetFileName(csImage);

		CString csFolder = CHelper::GetFolder(csImage);
		csFolder.TrimRight(L"\\");
		CString csAlbum = CHelper::GetFolder(csFolder);
		csAlbum.TrimRight(L"\\");

		if (csCurrentAlbum.IsEmpty())
		{
			csCurrentAlbum = csAlbum;
			arrAlbumImages.push_back(csImage);
		}
		else
		{
			// we are at the end or album has changed
			if (csAlbum != csCurrentAlbum)
			{
				// a collection of image names and the cached image
				shared_ptr<MAP_IMAGES> pImages(new MAP_IMAGES);

				for (auto& image : arrAlbumImages)
				{
					CString csError;
					shared_ptr<Image> pImage =
						CHelper::LoadImageFromFile(image, csError);

					CString csData = CHelper::GetDataName(image);
					pImages->add(csData, pImage);
				}

				m_mapAlbums.add(csCurrentAlbum, pImages);
				m_keyFolders.add(csCurrentAlbum, 0);

				// start collecting images for another album
				csCurrentAlbum = csAlbum;
				arrAlbumImages.clear();
				arrAlbumImages.push_back(csImage);
			}
			else // accumulate images for this album
			{
				arrAlbumImages.push_back(csImage);
			}
		}
	}

} // CacheQueriedImages

/////////////////////////////////////////////////////////////////////////////
// a console application that can crawl through the file
// system and build an index of the comments in JPG files
// located in folders named 'Labeled'
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);

	// console output
	CStdioFile fOut(stdout);

	// console error output
	CStdioFile fErr(stderr);

	if (hModule != nullptr)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			fErr.WriteString(L"Fatal Error: MFC initialization failed\n");
			nRetCode = 1;
		}
		else
		{
			// Start COM + GDI+
			AfxOleInit();
			::CoInitialize(NULL);
			InitGdiplus();

			CString csWorkingFolder(char(0), MAX_PATH);
			DWORD dwLen = 
				::GetCurrentDirectory(MAX_PATH, csWorkingFolder.GetBuffer());
			csWorkingFolder.ReleaseBuffer();
			CString csBaseFolder = csWorkingFolder;
			CString csQuery;
			
			// do some common command line argument corrections
			vector<CString> arrArgs = CHelper::CorrectedCommandLine(argc, argv);
			size_t nArgs = arrArgs.size();

			// the query and base folder can be passed in as a parameter 
			// to the command line
			if (nArgs == 2 || nArgs == 3)
			{
				bool bTest = false;
				if (nArgs == 2)
				{
					csQuery = arrArgs[1];
				}
				else if (nArgs == 3)
				{
					csQuery = arrArgs[1];
					CString csTemp = arrArgs[2];
					
					// if this argument is test, a delay will be added to 
					// every line output
					if (csTemp == L"test")
					{
						bTest = true;
					}
					else
					{
						csBaseFolder = csTemp;
					}
				}

				bool bDump = csQuery.CompareNoCase(L"dump") == 0;
				bool bPost = csQuery.CompareNoCase(L"post") == 0;;

				// worse case, default to my known photographs folder
				if (!::PathFileExists(csBaseFolder))
				{
					csBaseFolder = L"C:\\Users\\wtblo\\LocalDocs\\Photographs";
				}

				// build the output path
				CString csIndexFile = csBaseFolder + L"\\PhotoIndex.phix";

				// Try to load existing index
				bool bHasExistingIndex = CPhotoIndexBuilder::LoadBinaryIndex
				(
					csIndexFile, m_loadedIndex
				);

				// If no existing index → FULL REBUILD using rebuild session
				if (!bHasExistingIndex)
				{
					fOut.WriteString
					(
						L"No existing index found. Full rebuild.\n"
					);

					CPhotoIndexRebuildSession session;
					if (!session.Initialize(csBaseFolder))
					{
						fErr.WriteString
						(
							L"Initialization failed: " +
							session.GetLastError() + L"\n"
						);
						TerminateGdiplus();
						return 1;
					}

					// Process images incrementally
					while (!session.IsDone())
					{
						session.Step();

						int count = session.GetProcessedCount();
						if (count % 100 == 0)
						{
							CString msg;
							msg.Format(L"%05d ", count);
							if (count % 1000 == 0)
								msg += L"\n";
							fOut.WriteString(msg);
						}
					}

					// Save final index
					const auto& result = session.GetResult();
					CPhotoIndexBuilder::SaveBinaryIndexFromComponents
					(
						result.imageTable,
						result.invertedIndex,
						csIndexFile
					);

					// load the index again now that it has been built
					CPhotoIndexBuilder::LoadBinaryIndex
					(
						csIndexFile, m_loadedIndex
					);
				}
				else
				{
					// ------------------------------------------------------------
					// Incremental update path (NEW)
					// ------------------------------------------------------------
					auto result = CPhotoIndexBuilder::UpdateIndex
					(
						csBaseFolder, csIndexFile
					);

					if (!result.success)
					{
						fErr.WriteString
						(
							L"Update failed: " + result.errorMessage + L"\n"
						);
						TerminateGdiplus();
						return 1;
					}

					// Console output for incremental results
					CString msg;
					msg.Format(
						L"Incremental detection:\n"
						L"  New: %d\n"
						L"  Modified: %d\n"
						L"  Deleted: %d\n",
						(int)result.newImages.size(),
						(int)result.modifiedImages.size(),
						(int)result.deletedImages.size()
					);
					fOut.WriteString(msg);
				}

				m_csWorkingFolder = csBaseFolder;
				m_csQuery = csQuery;

				if (bDump)
				{
					DumpIndex(false);
				}
				else if (bPost)
				{
					DumpIndex(true);
				}
				else
				{
					CacheQueriedImages();

					for (const CString& path : m_queryResults)
					{
						if (bTest)
						{
							Sleep(100);
						}
						fOut.WriteString(path + L"\n");
					}

					CString csMessage;
					csMessage.Format
					(
						L"\n\nImages found: %d\n\n", (int)m_queryResults.size()
					);
					fOut.WriteString(csMessage);
				}
			}
			else // provide the user some help
			{
				fOut.WriteString(_T(".\n"));
				fOut.WriteString
				(
					_T("QueryPhotoIndex, Copyright (c) 2026, ")
					_T("by W. T. Block.\n")
				);

				fOut.WriteString
				(
					_T(".\n")
					_T("A Windows command line program to read a photo index\n")
					_T("  file called \"PhotoIndex.phix\" located by default in\n")
					_T("  the working folder.\n")
					_T(".\n")
					_T("The index relates to the subfolders containing \n")
					_T("  labeled photographs (.JPG) in subfolders called \"Labeled\".\n")
					_T(".\n")
					_T("The output represents a list of photograph pathnames\n")
					_T("  that have comment metadata that matches the query \n")
					_T("  parameters.\n")
					_T(".\n")
				);

				fOut.WriteString
				(
					_T(".\n")
					_T("Usage:\n")
					_T(".\n")
					_T(".  QueryPhotoIndex query [index_folder]\n")
					_T(".\n")
					_T("Where:\n")
					_T(".\n")
				);

				fOut.WriteString
				(
					_T(".\n")
					_T("Query format:\n")
					_T(".\n")
					_T(".  name or names separated with vertical bars: |.\n")
					_T(".  word or words separated with vertical bars: |.\n")
					_T(".\n")
					_T("Examples:\n")
					_T(".  \"Maria Elisabeth Block 'Mary Beth' Cruise\"\n")
					_T(".    will look for a specific person's name.\n")
					_T(".  \"Barry Wayne Block|Bobby Glenn 'Bob' Block\"\n")
					_T(".    will look for either or both people's names.\n")
					_T(".  \"Houston Zoo\"\n")
					_T(".    will look for references to Houston Zoo.\n")
					_T(".\n")
					_T("Special case:\n")
					_T(".  DUMP\n")
					_T(".    will dump the contents of the index.\n")
					_T(".  POST\n")
					_T(".    will dump the contents of the index and the postings.\n")
					_T(".\n")
				);
				nRetCode = 2;
			}
		}

		TerminateGdiplus();
	}
	else
	{
		fErr.WriteString(L"Fatal Error: GetModuleHandle failed\n");
		nRetCode = 1;
	}

	return nRetCode;
} // _tmain

/////////////////////////////////////////////////////////////////////////////
