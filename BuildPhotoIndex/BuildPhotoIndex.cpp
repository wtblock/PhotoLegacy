/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "BuildPhotoIndex.h"
#include "PhotoIndexRebuildSession.h"
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
	GdiplusShutdown(m_gdiplusToken);
	m_gdiplusToken = NULL;

}// TerminateGdiplus

/////////////////////////////////////////////////////////////////////////////
// a console application that can crawl through the file
// system and build an index of the comments in JPG files
// located in folders named 'Labeled'
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

		// If no existing index → FULL REBUILD using rebuild session
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

		// ------------------------------------------------------------
		// Incremental update path (NEW)
		// ------------------------------------------------------------
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

	fErr.WriteString(L"Fatal Error: GetModuleHandle failed\n");
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
