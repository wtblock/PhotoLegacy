/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "BuildPhotoIndex.h"
#include "PhotoIndexRebuildSession.h"
#include "PhotoIndexTypes.h"
#include "CHelper.h"
#include "PhotoIndexBuilder.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// BuildPhotoIndex.cpp
//
// A lightweight console application that orchestrates the creation or
// incremental update of the photo index used by Legacy applications.
//
// Architecture:
//   • All heavy lifting (scanning, metadata extraction, tokenization,
//     inverted index construction, binary serialization) is performed by
//     the Common library.
//   • BuildPhotoIndex simply initializes system services, parses command
//     line arguments, selects the correct indexing path (full rebuild or
//     incremental update), and reports progress.
//
// Responsibilities:
//   • Initialize MFC, COM, OLE, and GDI+
//   • Parse command line arguments
//   • Load existing index if present
//   • Perform full rebuild via CPhotoIndexRebuildSession when needed
//   • Perform incremental update via CPhotoIndexBuilder::UpdateIndex
//   • Write console progress output
//   • Shut down GDI+ cleanly
//
// This file intentionally contains no indexing logic. All indexing,
// metadata extraction, and file system crawling is delegated to the
// Common library.
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// The one and only application object
CWinApp theApp;

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// InitGdiplus
//
// Starts the GDI+ subsystem and stores the startup token globally.
// Required for reading EXIF metadata, XPComment, and image dimensions.
//
// Returns true if GDI+ initialized successfully.
/////////////////////////////////////////////////////////////////////////////
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
// TerminateGdiplus
//
// Shuts down the GDI+ subsystem and clears the global token.
// Safe to call even if initialization failed.
/////////////////////////////////////////////////////////////////////////////
void TerminateGdiplus()
{
	GdiplusShutdown(m_gdiplusToken);
	m_gdiplusToken = NULL;

}// TerminateGdiplus

/////////////////////////////////////////////////////////////////////////////
// _tmain
//
// Main entry point for the BuildPhotoIndex console application.
//
// Flow:
//   1. Initialize MFC, COM, OLE, and GDI+
//   2. Parse command line arguments
//   3. Determine base folder and index file location
//   4. Attempt to load existing index
//   5. If no index exists → FULL REBUILD
//        • Use CPhotoIndexRebuildSession
//        • Step through all images incrementally
//        • Save final index via SaveBinaryIndexFromComponents
//   6. If index exists → INCREMENTAL UPDATE
//        • Use CPhotoIndexBuilder::UpdateIndex
//        • Detect new, modified, and deleted images
//        • Save updated index automatically
//   7. Report results to console
//   8. Shut down GDI+ and exit
//
// Notes:
//   • Full rebuild is used only when no index file exists.
//   • Incremental update is used whenever an index is present.
//   • All indexing logic is delegated to the Common library.
/////////////////////////////////////////////////////////////////////////////
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);

	CStdioFile fOut(stdout);
	CStdioFile fErr(stderr);

	if (hModule != nullptr)
	{
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			fErr.WriteString(L"Fatal Error: MFC initialization failed\n");
			return 1;
		}

		// Start COM + GDI+
		AfxOleInit();
		::CoInitialize(NULL);
		InitGdiplus();

		// Parse command line
		vector<CString> arrArgs = 
			CHelper::CorrectedCommandLine(argc, argv);
		CString csBaseFolder;

		if (arrArgs.size() >= 2)
		{
			csBaseFolder = arrArgs[1];
		}
		else
		{
			csBaseFolder = CHelper::GetCurrentDirectory();
		}

		// Fallback if base folder does not exist
		if (!::PathFileExists(csBaseFolder))
			csBaseFolder = L"C:\\Users\\wtblo\\LocalDocs\\Photographs";

		CString csIndexFile = csBaseFolder + L"\\PhotoIndex.phix";

		// Try to load existing index
		CPhotoIndexBuilder::LoadedIndex existingIndex;
		bool bHasExistingIndex = 
			CPhotoIndexBuilder::LoadBinaryIndex(csIndexFile, existingIndex);

		/////////////////////////////////////////////////////////////////////////////
		// Full Rebuild
		//
		// Triggered when no existing index file is found.
		//
		// Uses CPhotoIndexRebuildSession to:
		//   • Crawl the base folder
		//   • Extract metadata from each image
		//   • Tokenize comments and XPComment
		//   • Build the image table and inverted index incrementally
		//
		// Progress is printed every 100 images.
		// The final index is saved using SaveBinaryIndexFromComponents.
		/////////////////////////////////////////////////////////////////////////////
		if (!bHasExistingIndex)
		{
			fOut.WriteString(L"No existing index found. Full rebuild.\n");

			CPhotoIndexRebuildSession session;
			if (!session.Initialize(csBaseFolder))
			{
				fErr.WriteString(L"Initialization failed: " + session.GetLastError() + L"\n");
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

			TerminateGdiplus();
			return 0;
		}

		/////////////////////////////////////////////////////////////////////////////
		// Incremental Update
		//
		// Triggered when an existing index file is successfully loaded.
		//
		// Uses CPhotoIndexBuilder::UpdateIndex to:
		//   • Detect new images
		//   • Detect modified images (timestamp or size change)
		//   • Detect deleted images
		//   • Rebuild only the affected portions of the index
		//
		// The updated index is saved automatically.
		// Console output lists counts of new, modified, and deleted images.
		/////////////////////////////////////////////////////////////////////////////
		auto result = CPhotoIndexBuilder::UpdateIndex(csBaseFolder, csIndexFile);

		if (!result.success)
		{
			fErr.WriteString(L"Update failed: " + result.errorMessage + L"\n");
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

		TerminateGdiplus();
		return 0;
	}

	/////////////////////////////////////////////////////////////////////////////
	// Error Handling
	//
	// All failures (MFC init, COM init, GDI+ init, rebuild failure,
	// incremental update failure) are reported to stderr via CStdioFile.
	//
	// The application exits with non-zero status on failure.
	/////////////////////////////////////////////////////////////////////////////
	fErr.WriteString(L"Fatal Error: GetModuleHandle failed\n");
	return 1;

} // _tmain

/////////////////////////////////////////////////////////////////////////////
