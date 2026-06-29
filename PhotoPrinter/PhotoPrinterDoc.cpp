/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "resource.h"
#include "framework.h"
#include "PhotoIndexRebuildSession.h"
#include "PhotoPrinter.h"
#include "PhotoPrinterDoc.h"
#include "PhotoPrinterView.h"
#include "MainFrm.h"
#include "PropertiesWnd.h"
#include "ThumbnailDialog.h"
#include "CHelper.h"
#include "ImagePlus.h"
#include <propkey.h>
#include <xmllite.h>
#include <set>

/////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "xmllite.lib")

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CPhotoPrinterDoc, CBaseDoc)

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPhotoPrinterDoc, CBaseDoc)
	ON_COMMAND(ID_FILE_SAVE, &CPhotoPrinterDoc::OnFileSave)
	ON_COMMAND(ID_FILE_SAVE_AS, &CPhotoPrinterDoc::OnFileSaveAs)
	ON_COMMAND(ID_FILE_BUILDPAGES, &CPhotoPrinterDoc::OnFileBuildPages)
	ON_UPDATE_COMMAND_UI(ID_FILE_BUILDPAGES, &CPhotoPrinterDoc::OnUpdateFileBuildPages)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CPhotoPrinterDoc::CPhotoPrinterDoc() noexcept
{
	HeightOfPage = 11.0;
	WidthOfPage = 8.5;

	Pages = 1;
	Page = 1;
	Images = 0;
	ExportDPI = 400;
	ExportQuality = 75;

	//Title = L"Block Family End of Era";
	//Subtitle = L"Our Final Christmas Vacation";
	//Publisher = L"William Theodore Block III, Publisher";
	//ISBN = L"ISBN: 9798285957720";
	//Copyright = L"Copyright © 2025";
	//Description =
	//	L"A key tradition in our family was our Christmas holidays. During the years when the kids were younger, we would often visit Walt Disney World. As they grew older, we shifted to skiing and enjoyed entire days on the slopes together. This album captures our final Christmas before our son Ted was set to marry the following June. ";
	//StartFolder = 
	//	L"C:\\Users\\wtblo\\Documents\\Photomyne\\2009.12.19 Disney World";
	//StartFolder = 
	//	L"C:\\Users\\wtblo\\Documents\\Photomyne\\2009.12.04 Snow in December";
	//EndFolder =
	//	L"C:\\Users\\wtblo\\Documents\\Photomyne\\2009.12.04 Snow in December";
	//StartFolder =
	//	L"C:\\Users\\wtblo\\Documents\\Photomyne\\2009.12.18 Our Home";
	//EndFolder = 
	//	L"C:\\Users\\wtblo\\Documents\\Photomyne\\2009.12.18 Our Home";
	//EndFolder = 
	//	L"C:\\Users\\wtblo\\Documents\\Photomyne\\2009.12.29 Home";
	//EndFolder = 
	//	L"C:\\Users\\wtblo\\Documents\\Photomyne\\2009.12.22 Disney World";
}

/////////////////////////////////////////////////////////////////////////////
CPhotoPrinterDoc::~CPhotoPrinterDoc()
{
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoPrinterDoc::OnSaveDocument(CString& csPath)
{
	bool value = false;
	IStream* pFileStream = nullptr;
	HRESULT hr = SHCreateStreamOnFileW(
		csPath, STGM_CREATE | STGM_WRITE, &pFileStream);

	if (FAILED(hr))
	{
		AfxMessageBox(L"Failed to create file stream.");
		return value;
	}

	IXmlWriter* pWriter = nullptr;
	hr = CreateXmlWriter(__uuidof(IXmlWriter),
		reinterpret_cast<void**>(&pWriter),
		nullptr);
	if (FAILED(hr))
	{
		AfxMessageBox(L"Failed to create XML writer.");
		pFileStream->Release();
		return value;
	}

	hr = pWriter->SetOutput(pFileStream);
	if (FAILED(hr))
	{
		AfxMessageBox(L"Failed to set output for XML writer.");
		pWriter->Release();
		pFileStream->Release();
		return value;
	}

	// ⭐ Enable pretty-print formatting
	pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);

	value = true;

	pWriter->WriteStartDocument(XmlStandalone_Yes);
	pWriter->WriteStartElement(nullptr, L"PhotoPrinter", nullptr);

	auto WriteStringProp = [&](LPCWSTR name, const CString& val)
	{
		pWriter->WriteStartElement(nullptr, name, nullptr);
		pWriter->WriteAttributeString(nullptr, L"value", nullptr, val);
		pWriter->WriteAttributeString(nullptr, L"type", nullptr, L"string");
		pWriter->WriteEndElement();
	};

	WriteStringProp(L"Title", Title);
	WriteStringProp(L"Subtitle", Subtitle);
	WriteStringProp(L"Publisher", Publisher);
	WriteStringProp(L"ISBN", ISBN);
	WriteStringProp(L"Copyright", Copyright);
	WriteStringProp(L"Description", Description);
	WriteStringProp(L"StartFolder", StartFolder);
	WriteStringProp(L"EndFolder", EndFolder);
	WriteStringProp(L"Query", Query);
	WriteStringProp(L"ExportFolder", ExportFolder);
	WriteStringProp(L"ExportPages", ExportPages);

	CString csDpi;
	csDpi.Format(L"%d", ExportDPI > 0 ? ExportDPI : 400);
	WriteStringProp(L"ExportDPI", csDpi);

	CString csQuality;
	csQuality.Format(L"%d", ExportQuality > 0 ? ExportQuality : 75);
	WriteStringProp(L"ExportQuality", csQuality);

	pWriter->WriteEndElement();   // </PhotoPrinter>
	pWriter->WriteEndDocument();

	pWriter->Release();
	pFileStream->Release();

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPropertiesWnd* pProperties = pFrame->PropertiesPane;
	CPropertyGridCtrl* pProps = pProperties->PropList;
	pProperties->UpdatePropertiesFromDocument(this);

	// Build flat list of editable properties
	vector<CMFCPropertyGridProperty*> flat;
	pProps->BuildFlatList(flat);

	// clear modified state on every property so they are no 
	// longer bolded
	for ( auto& pProp : flat)
	{
		if (pProp->IsModified())
		{
			COleVariant var = pProp->GetValue();
			pProp->SetOriginalValue(var);
			pProp->ResetOriginalValue();
		}
	}

	SetModifiedFlag(FALSE);
	pProps->Invalidate();

	return value;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoPrinterDoc::PromptForFileName(CString& strFilePath)
{
	CFileDialog fileDlg
	(
		FALSE, L"pp", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"Photo Printer Files (*.pp)|*.pp|All Files (*.*)|*.*||"
	);

	if (fileDlg.DoModal() == IDOK)
	{
		strFilePath = fileDlg.GetPathName();
		return TRUE; // User selected a valid file name
	}

	return FALSE; // User canceled
} // PromptForFileName

/////////////////////////////////////////////////////////////////////////////
// open a data file
bool CPhotoPrinterDoc::Open(LPCTSTR szFilename, bool bRead, LPCTSTR pcszFileID)
{
	bool value = false;

	IStream* pFileStream = nullptr;
	HRESULT hr = SHCreateStreamOnFileW(szFilename, STGM_READ, &pFileStream);
	if (FAILED(hr))
	{
		AfxMessageBox( L"Failed to create file stream." );
		return value;
	}

	IXmlReader* pReader = nullptr;
	hr = CreateXmlReader
	(
		__uuidof(IXmlReader), reinterpret_cast<void**>(&pReader), nullptr
	);
	if (FAILED(hr))
	{
		AfxMessageBox(L"Failed to create XML reader." );
		pFileStream->Release();
		return value;
	}

	hr = pReader->SetInput(pFileStream);
	if (FAILED(hr))
	{
		AfxMessageBox(L"Failed to set input for XML reader." );
		pReader->Release();
		pFileStream->Release();
		return value;
	}

	XmlNodeType nodeType;
	while (S_OK == pReader->Read(&nodeType))
	{
		if (nodeType == XmlNodeType_Element)
		{
			const WCHAR* pwszLocalName = nullptr;
			pReader->GetLocalName(&pwszLocalName, nullptr);

			if
				(
					wcscmp(pwszLocalName, L"Title") == 0 ||
					wcscmp(pwszLocalName, L"Subtitle") == 0 ||
					wcscmp(pwszLocalName, L"Publisher") == 0 ||
					wcscmp(pwszLocalName, L"ISBN") == 0 ||
					wcscmp(pwszLocalName, L"Copyright") == 0 ||
					wcscmp(pwszLocalName, L"Description") == 0 ||
					wcscmp(pwszLocalName, L"StartFolder") == 0 ||
					wcscmp(pwszLocalName, L"EndFolder") == 0 ||
					wcscmp(pwszLocalName, L"Query") == 0 ||
					wcscmp(pwszLocalName, L"ExportFolder") == 0 ||
					wcscmp(pwszLocalName, L"ExportPages") == 0 ||
					wcscmp(pwszLocalName, L"ExportDPI") == 0 ||
					wcscmp(pwszLocalName, L"ExportQuality") == 0
				)
			{
				if (pReader->MoveToFirstAttribute() == S_OK)
				{
					do
					{
						const WCHAR* pwszAttrName = nullptr;
						const WCHAR* pwszValue = nullptr;

						pReader->GetLocalName(&pwszAttrName, nullptr);
						pReader->GetValue(&pwszValue, nullptr);

						if (wcscmp(pwszAttrName, L"value") == 0)
						{
							CString elementName(pwszLocalName);
							CString elementValue(pwszValue);

							if (elementName == L"Title")
							{
								Title = elementValue;

							}
							else if (elementName == L"Subtitle")
							{
								Subtitle = elementValue;

							}
							else if (elementName == L"Publisher")
							{
								Publisher = elementValue;

							}
							else if (elementName == L"ISBN")
							{
								ISBN = elementValue;

							}
							else if (elementName == L"Copyright")
							{
								Copyright = elementValue;

							}
							else if (elementName == L"Description")
							{
								Description = elementValue;

							}
							else if (elementName == L"StartFolder")
							{
								StartFolder = elementValue;

							}
							else if (elementName == L"EndFolder")
							{
								EndFolder = elementValue;
							}
							else if (elementName == L"Query")
							{
								Query = elementValue;
							}
							else if (elementName == L"ExportFolder")
							{
								ExportFolder = elementValue;
							}
							else if (elementName == L"ExportPages")
							{
								ExportPages = elementValue;
							}
							else if (elementName == L"ExportDPI")
							{
								if (elementValue.IsEmpty())
								{
									elementValue = L"400";
								}
								UINT nValue = (UINT)_tstol(elementValue);

								ExportDPI = nValue;
							}
							else if (elementName == L"ExportQuality")
							{
								if (elementValue.IsEmpty())
								{
									elementValue = L"75";
								}
								UINT nValue = (UINT)_tstol(elementValue);

								ExportQuality = nValue;
							}
						}
					} while (pReader->MoveToNextAttribute() == S_OK);
				}
			}
		}
	}
	value = true;
	pReader->Release();
	pFileStream->Release();
	return value;
} // Open

/////////////////////////////////////////////////////////////////////////////
// Save the document data to a file
// lpszPathName = path name where to save document file
// if lpszPathName is NULL then the user will be prompted (SaveAs)
// note: lpszPathName can be different than 'm_strPathName'
// if 'bReplace' is TRUE will change file name if successful (SaveAs)
// if 'bReplace' is FALSE will not change path name (SaveCopyAs)
BOOL CPhotoPrinterDoc::DoSave(CString& csPath, BOOL bReplace)
{
	CWaitCursor wait;
	if (csPath.IsEmpty())
	{
		// Prompt user for file name
		CString strNewPath;
		if (!PromptForFileName(strNewPath))
			return FALSE; // User canceled

		csPath = strNewPath;
	}

	// Perform the actual save operation
	if (!OnSaveDocument(csPath))
		return FALSE; // Save failed

	// If bReplace is TRUE, update the document's path
	if (bReplace)
		SetPathName(csPath);

	return TRUE;

} // DoSave

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterDoc::OnFileSave()
{
	// Get the current file path
	CString strFilePath = GetPathName();

	if (strFilePath.IsEmpty())
	{
		// If no file is set, prompt for "Save As"
		OnFileSaveAs();
		return;
	}

	// Save document data
	if (!DoSave(strFilePath))
	{
		AfxMessageBox(L"Error saving file.");
	}

} // OnFileSave

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoPrinterDoc::SaveModified()
{
	if (!IsModified())
		return TRUE;    // nothing to do, go ahead and close

	int res = AfxMessageBox(_T("Save changes?"), MB_YESNOCANCEL);

	if (res == IDCANCEL)
		return FALSE;   // ❌ cancel: do NOT close the document

	if (res == IDNO)
		return TRUE;    // ❌ no: close WITHOUT saving

	// ✅ yes: try to save
	OnFileSave();

	return TRUE;        // saved successfully → close
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterDoc::OnFileSaveAs()
{
	// Get the current file path
	CString strFilePath;

	// Save document data
	if (!DoSave(strFilePath, TRUE))
	{
		AfxMessageBox(L"Error saving file.");
	}

} // OnFileSaveAs

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoPrinterDoc::OnNewDocument()
{
	if (!CBaseDoc::OnNewDocument())
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
void CPhotoPrinterDoc::AssertValid() const
{
	CBaseDoc::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterDoc::Dump(CDumpContext& dc) const
{
	CBaseDoc::Dump(dc);
}

/////////////////////////////////////////////////////////////////////////////
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// the view associated with this document
CPhotoPrinterView* CPhotoPrinterDoc::GetPhotoPrinterView()
{
	CPhotoPrinterView* value = nullptr;
	CView* pView = nullptr;
	POSITION pos = GetFirstViewPosition();
	while (pos)
	{
		pView = GetNextView(pos);
		if (pView->IsKindOf(RUNTIME_CLASS(CPhotoPrinterView)))
		{
			value = static_cast<CPhotoPrinterView*>(pView);
			break;
		}
	}

	return value;

} // GetPhotoPrinterView

/////////////////////////////////////////////////////////////////////////////
// does the given path contain an album
bool CPhotoPrinterDoc::ContainsAnAlbum(CString folderPath)
{
	bool value = false;
	CFileFind finder;
	CString searchPath = folderPath + L"\\*.album";
	CString csPath;

	BOOL bWorking = finder.FindFile(searchPath);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// Ignore . and .. directories
		if (finder.IsDots())
			continue;

		// we are only interested in files at this point
		if (finder.IsDirectory())
			continue;

		csPath = finder.GetFilePath();
		value = true;
		break;
	}

	return value;
} // ContainsAnAlbum

/////////////////////////////////////////////////////////////////////////////
// find all folders contained in a given folder that are in the
// interested range
void CPhotoPrinterDoc::ListSubFolders(CString folderPath)
{
	CString csWorkingFolder = WorkingFolder;
	if (folderPath.Left(1) != L"\\")
	{
		folderPath += L"\\";
	}
	CFileFind finder;
	CString searchPath = folderPath + L"*.*";
	CString csStart = CorrectForWorkingFolder(StartFolder);
	CString csEnd = CorrectForWorkingFolder(EndFolder);

	BOOL bWorking = finder.FindFile(searchPath);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// Ignore . and .. directories
		if (finder.IsDots())
			continue;

		// we are only interested in folders at this point
		if (finder.IsDirectory())
		{
			const CString csPath = finder.GetFilePath();
			if (csPath < csStart)
			{
				continue;
			}

			CString csFile = CHelper::GetDataName(csPath);
			csFile.MakeLower();

			// labeled folders contain the target labeled files
			// and are not part of the album paths
			if (csFile != L"labeled")
			{
				if (ContainsAnAlbum(csPath))
				{
					shared_ptr<int> pNull = shared_ptr<int>(new int(0));
					m_keyFolders.add(csPath, pNull);
				}

				// recurse into the folder
				ListSubFolders(csPath);

				// after returning from the recursion, check to see
				// if we are done by reaching the end folder
				if (csPath == csEnd)
				{
					bWorking = false;
				}
			}
		}
	}

	finder.Close();
} // ListSubFolders

/////////////////////////////////////////////////////////////////////////////
// count the images within the labeled folders
void CPhotoPrinterDoc::CountLabeledImages()
{
	UINT value = 0;
	for (auto& node : m_keyFolders.Items)
	{
		CFileFind finder;
		CString csPath = node.first;

		CString csLabeled = csPath + L"\\Labeled\\*.jpg";
		BOOL bWorking = finder.FindFile(csLabeled);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			// Ignore . and .. directories
			if (finder.IsDots())
				continue;

			// Check if it's a directory
			if (finder.IsDirectory())
				continue;

			value++;
		}
	}

	Images = value;

} // CountLabeledImages

/////////////////////////////////////////////////////////////////////////////
CString CPhotoPrinterDoc::GetXPComment(Image* pImage)
{
	if (!pImage)
		return CString();

	const PROPID XPCommentTag = 0x9C9C; // PropertyTagXPComment

	UINT size = pImage->GetPropertyItemSize(XPCommentTag);
	if (size == 0)
		return CString(); // No XPComment metadata

	std::vector<BYTE> buffer(size);
	Gdiplus::PropertyItem* pItem = reinterpret_cast<Gdiplus::PropertyItem*>(buffer.data());

	if (pImage->GetPropertyItem(XPCommentTag, size, pItem) != Gdiplus::Ok)
		return CString();

	// XPComment is UTF-16LE, null-terminated
	const WCHAR* pwsz = reinterpret_cast<const WCHAR*>(pItem->value);

	// Length in WCHARs = byte length / 2
	UINT wcharCount = pItem->length / sizeof(WCHAR);

	// Construct CString directly
	return CString(pwsz, wcharCount);

} // GetXPComment

/////////////////////////////////////////////////////////////////////////////
// count the queried images
void CPhotoPrinterDoc::CountQueriedImages(vector<CString>& arrImages)
{
	UINT value = 0;
	CString csStartFolder = CorrectForWorkingFolder(StartFolder);
	CString csEndFolder = CorrectForWorkingFolder(EndFolder);

	for (auto& node : arrImages)
	{
		CString csImage = node;
		const CString csFile = CHelper::GetFileName(csImage);

		CString csFolder = CHelper::GetFolder(csImage);
		csFolder.TrimRight(L"\\");
		CString csAlbum = CHelper::GetFolder(csFolder);
		csAlbum.TrimRight(L"\\");

		if (csAlbum < csStartFolder)
		{
			continue;
		}
		else if (csAlbum > csEndFolder)
		{
			break;
		}
		else
		{
			value++;
		}
	}

	Images = value;

} // CountQueriedImages

/////////////////////////////////////////////////////////////////////////////
// cache the images within the labeled folders
// cache the queried images using the new inverted index
void CPhotoPrinterDoc::CacheQueriedImages()
{
	m_queryResults.clear();

	CString csQuery = Query;
	if (csQuery.IsEmpty())
		return;

	CString csStartFolder = CorrectForWorkingFolder(StartFolder);
	CString csEndFolder = CorrectForWorkingFolder(EndFolder);

	// ===============================================================
	// Detect OR operator
	// ===============================================================
	bool hasOr = (csQuery.Find(L'|') != -1);

	// This will hold the final absolute paths for either OR or AND logic
	std::vector<CString> arrImages;

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
	CountQueriedImages(arrImages);

	theApp.OnIdle(0);
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	// launch the progress dialog
	CThumbnailDialog dlg;
	dlg.Parent = pFrame;
	dlg.CreateDlg();
	dlg.ShowWindow(SW_SHOW);

	int nImage = 0;
	CString csCurrentAlbum;

	// a terminator string insures the last album is processed
	arrImages.push_back(L"Terminator");

	dlg.TotalImages = (int)Images;
	dlg.SetWindowText(L"Caching Queried Images");
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

		if (csImage != L"Terminator" && csAlbum < csStartFolder)
		{
			continue;
		}
		else if (csImage != L"Terminator" && csAlbum > csEndFolder)
		{
			CString csYear = CHelper::GetFolder(csAlbum);
			csYear.TrimRight(L"\\");
			if (csYear != csEndFolder)
			{
				CString csContainer = CHelper::GetFolder(csYear);
				csContainer.TrimRight(L"\\");
				if (csContainer != csEndFolder)
				{
					break;
				}
			}
		}

		if (csCurrentAlbum.IsEmpty())
		{
			csCurrentAlbum = csAlbum;
			arrAlbumImages.push_back(csImage);
		}
		else
		{
			// we are at the end or album has changed
			if (csImage == L"Terminator" || csAlbum != csCurrentAlbum)
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

		dlg.CurrentImage = nImage++;

		if (dlg.Cancel)
			break;

		pFrame->Wait(1);
	}

	dlg.DestroyWindow();

} // CacheQueriedImages

/////////////////////////////////////////////////////////////////////////////
// cache the images within the labeled folders
void CPhotoPrinterDoc::CacheLabeledImages()
{
	CString csQuery = Query;
	if (csQuery.IsEmpty())
	{
		CountLabeledImages();
	}
	else
	{
		CacheQueriedImages();
		return;
	}

	theApp.OnIdle(0);
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	// launch the progress dialog
	CThumbnailDialog dlg;
	dlg.Parent = pFrame;
	dlg.CreateDlg();
	dlg.ShowWindow(SW_SHOW);
	dlg.SetWindowText(L"Caching Images");

	dlg.TotalImages = (int)Images;

	int nImage = 0;
	UINT uiInclude = 0;
	bool bAbort = false;

	for (auto& node : m_keyFolders.Items)
	{
		CFileFind finder;
		CString csPath = node.first;
		bool bAlbum = false;

		CString csLabeled = csPath + L"\\Labeled\\*.jpg";
		BOOL bWorking = finder.FindFile(csLabeled);

		// a collection of image names and the cached image
		// for the current album
		shared_ptr<MAP_IMAGES> pImages =
			shared_ptr<MAP_IMAGES>(new MAP_IMAGES);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			// Ignore . and .. directories
			if (finder.IsDots())
				continue;

			// Check if it's a directory
			if (finder.IsDirectory())
				continue;

			// let the user cancel out
			if (dlg.Cancel)
			{
				bAbort = true;
				break;
			}

			// update the progress dialog's status
			dlg.CurrentImage = nImage++;

			CString csError;
			const CString csImage = finder.GetFilePath();
			const CString csFile = CHelper::GetFileName(csImage);

			shared_ptr<Image> pImage =
				CHelper::LoadImageFromFile(csImage, csError);

			CString csData = CHelper::GetDataName(csImage);
			pImages->add(csData, pImage);
			bAlbum = true;

			// wait one millisecond while letting normal 
			// window messaging to run
			pFrame->Wait(1);
		}

		if (bAbort)
		{
			break;
		}

		if (bAlbum)
		{
			m_mapAlbums.add(csPath, pImages);
		}

		// wait ten milliseconds while letting normal 
		// window messaging to run
		pFrame->Wait(10);
	}

	// done with the progress dialog
	dlg.DestroyWindow();

} // CacheLabeledImages


/////////////////////////////////////////////////////////////////////////////
// collect the image pages and allocate the rectangles
// used to contain each image on the page
void CPhotoPrinterDoc::GeneratePages()
{
	m_arrPages.clear();

	// set the current page number to account for title
	// and table of contents
	UINT nCountTOC = TableOfContentsPages;
	Page = nCountTOC + 1;

	for (auto& album : m_mapAlbums.Items)
	{
		CString csFolder = album.first;
		shared_ptr<MAP_IMAGES> pImages = album.second;
		int nImages = pImages->Count;
		int nFolderImage = 0;
		while (nImages > 0)
		{
			// add a page to the document
			const UINT nPage = Page + 1;
			Page = nPage;
			Pages = nPage;
			CRect rect = MarginRectangle;
			shared_ptr<CPage> pPage =
				shared_ptr<CPage>(new CPage(nPage, csFolder, rect));
			m_arrPages.append(pPage);
			int nLoopImage = 0;

			switch (nImages)
			{
				case 1: // whole page portrait image
				{
					for (auto& image : pImages->Items)
					{
						if (nLoopImage++ < nFolderImage)
						{
							continue;
						}

						CString csImage = image.first;
						pPage->AddAnImage(csImage, rect);
						nImages--;
					}
					break;
				}
				case 2: // two landscape image page
				{
					int nRect = 0;
					int nOffset = rect.Height() / 2;
					for (auto& image : pImages->Items)
					{
						if (nLoopImage++ < nFolderImage)
						{
							continue;
						}

						CString csImage = image.first;
						switch (nRect)
						{
							case 0 :
							{
								CRect temp = rect;
								temp.bottom -= nOffset;
								pPage->AddAnImage(csImage, temp);
								break;
							}
							case 1 :
							{
								CRect temp = rect;
								temp.top += nOffset;
								pPage->AddAnImage(csImage, temp);
								break;
							}
						}
						nRect++;
						nImages--;
					}
					break;
				}
				case 3:
				{
					int nRect = 0;
					int nOffsetVertical = rect.Height() / 2;
					int nOffsetHorizontal = rect.Width() / 2;
					for (auto& image : pImages->Items)
					{
						if (nLoopImage++ < nFolderImage)
						{
							continue;
						}

						CString csImage = image.first;
						switch (nRect)
						{
							case 0 :
							{
								CRect temp = rect;
								temp.bottom -= nOffsetVertical;
								temp.right -= nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								break;
							}
							case 1 :
							{
								CRect temp = rect;
								temp.bottom -= nOffsetVertical;
								temp.left += nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								break;
							}
							case 2 :
							{
								CRect temp = rect;
								temp.top += nOffsetVertical;
								pPage->AddAnImage(csImage, temp);
								break;
							}
						}
						nRect++;
						nImages--;
					}
					break;
				}
				case 4:
				{
					int nRect = 0;
					int nOffsetVertical = rect.Height() / 2;
					int nOffsetHorizontal = rect.Width() / 2;
					for (auto& image : pImages->Items)
					{
						if (nLoopImage++ < nFolderImage)
						{
							continue;
						}

						CString csImage = image.first;
						switch (nRect)
						{
							case 0 :
							{
								CRect temp = rect;
								temp.bottom -= nOffsetVertical;
								temp.right -= nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								break;
							}
							case 1 :
							{
								CRect temp = rect;
								temp.bottom -= nOffsetVertical;
								temp.left += nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								break;
							}
							case 2:
							{
								CRect temp = rect;
								temp.top += nOffsetVertical;
								temp.right -= nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								break;
							}
							case 3:
							{
								CRect temp = rect;
								temp.top += nOffsetVertical;
								temp.left += nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								break;
							}
						}
						nRect++;
						nImages--;
					}
					break;
				}
				default:
				{
					int nRect = 0;
					int nOffsetVertical = rect.Height() / 3;
					int nOffsetHorizontal = rect.Width() / 2;
					bool bDone = false;
					for (auto& image : pImages->Items)
					{
						if (nLoopImage++ < nFolderImage)
						{
							continue;
						}

						CString csImage = image.first;
						switch (nRect)
						{
							case 0:
							{
								CRect temp = rect;
								temp.bottom -= 2 * nOffsetVertical;
								temp.right -= nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								nImages--;
								break;
							}
							case 1:
							{
								CRect temp = rect;
								temp.bottom -= 2 * nOffsetVertical;
								temp.left += nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								nImages--;
								break;
							}
							case 2:
							{
								CRect temp = rect;
								temp.top += nOffsetVertical;
								temp.bottom -= nOffsetVertical;
								temp.right -= nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								nImages--;
								break;
							}
							case 3:
							{
								CRect temp = rect;
								temp.top += nOffsetVertical;
								temp.bottom -= nOffsetVertical;
								temp.left += nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								nImages--;
								break;
							}
							case 4:
							{
								CRect temp = rect;
								temp.top += 2 * nOffsetVertical;
								temp.right -= nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								nImages--;
								break;
							}
							case 5:
							{
								CRect temp = rect;
								temp.top += 2 * nOffsetVertical;
								temp.left += nOffsetHorizontal;
								pPage->AddAnImage(csImage, temp);
								nImages--;
								break;
							}
							default :
							{
								bDone = true;
							}
						}

						if (bDone)
						{
							break;
						}
						else
						{
							nRect++;
							nFolderImage++;
						}
					}
				}
			}
		}
	}
} // GeneratePages

/////////////////////////////////////////////////////////////////////////////
// get a vector corresponding to the document's table of contents
vector<pair<CString, int>>& CPhotoPrinterDoc::GetAlbumTableOfContents()
{
	m_arrTOC.clear();
	pair<CString, int> item;

	// account for the title page
	item.first = L"Title Page";
	item.second = 1;
	m_arrTOC.push_back(item);

	// account for the title page
	item.first = L"Table of Contents";
	item.second = 2;
	m_arrTOC.push_back(item);

	CString csCurrentAlbum;
	for (auto& node : m_arrPages.Items)
	{
		CString csFolder = node->Folder;
		CString csAlbum = CHelper::GetDataName( csFolder );
		if (csAlbum != csCurrentAlbum)
		{
			csCurrentAlbum = csAlbum;
			int nPage = node->Page;
			item.first = csAlbum;
			item.second = nPage;
			m_arrTOC.push_back(item);
		}
	}

	return m_arrTOC;

} // GetAlbumTableOfContents

/////////////////////////////////////////////////////////////////////////////
shared_ptr<Image> CPhotoPrinterDoc::FindImage
(
	CString csFolder, CString csImage
)
{
	shared_ptr<Image> value;
	shared_ptr<MAP_IMAGES> pImages = m_mapAlbums.find( csFolder );
	if (pImages != nullptr)
	{
		value = pImages->find(csImage);
	}
	return value;
} // FindImage

/////////////////////////////////////////////////////////////////////////////
bool CPhotoPrinterDoc::LoadSearchIndex()
{
	CString baseFolder = WorkingFolder;
	CString indexPath = baseFolder + L"\\PhotoIndex.phix";

	// Try to load existing index
	CPhotoIndexBuilder::LoadedIndex existingIndex;
	bool bHasExistingIndex =
		CPhotoIndexBuilder::LoadBinaryIndex(indexPath, existingIndex);

	// If no existing index → FULL REBUILD using rebuild session
	if (!bHasExistingIndex)
	{
		// Use the rebuild session for full rebuild
		CPhotoIndexRebuildSession session;
		if (!session.Initialize(baseFolder))
		{
			CString errorMessage = session.GetLastError();
			AfxMessageBox(L"Index rebuild failed:\n" + errorMessage);
			return false;
		}

		theApp.OnIdle(0);
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

		// launch the progress dialog
		CThumbnailDialog dlg;
		dlg.Parent = pFrame;
		dlg.CreateDlg();
		dlg.ShowWindow(SW_SHOW);
		dlg.TotalImages = session.GetTotalCount();
		dlg.SetWindowText(L"Building missing index");

		while (!session.IsDone())
		{
			session.Step();
			int nProcessed = session.GetProcessedCount();
			if (nProcessed % 100 == 0)
			{
				dlg.CurrentImage = nProcessed;
				if (dlg.Cancel)
					break;

				pFrame->Wait(1);
			}
		}

		dlg.DestroyWindow();

		// Save final index
		const auto& result = session.GetResult();
		CPhotoIndexBuilder::SaveBinaryIndexFromComponents
		(
			result.imageTable,
			result.invertedIndex,
			indexPath
		);

		//return CPhotoIndexBuilder::LoadBinaryIndex(indexPath, existingIndex);
	}

	// Update the index first
	auto result = CPhotoIndexBuilder::UpdateIndex(baseFolder, indexPath);
	if (!result.success)
	{
		AfxMessageBox(L"Index update failed:\n" + result.errorMessage);
		return false;
	}

	// Load the updated index
	m_loadedIndex = CPhotoIndexBuilder::LoadedIndex();
	if (!CPhotoIndexBuilder::LoadBinaryIndex(indexPath, m_loadedIndex))
	{
		AfxMessageBox(L"Failed to load updated index.");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// Cache chronological images using the new index
/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterDoc::CacheChronologicalImages()
{
	m_queryResults.clear();

	// MUST be absolute paths
	CString csStartFolder = CHelper::ToAbsolute(CorrectForWorkingFolder(StartFolder));
	CString csEndFolder = CHelper::ToAbsolute(CorrectForWorkingFolder(EndFolder));

	// ===============================================================
	// 1. Load ALL images from the new index
	// ===============================================================
	std::vector<CString> arrImages;

	for (size_t id = 0; id < m_loadedIndex.images.size(); id++)
	{
		// paths can be store as relative paths
		CString abs = CHelper::ToAbsolute(m_loadedIndex.images[id].path);

		// if an image has been deleted, the index keeps it in the table
		// of images to insure its deletion does not affect other IDs
		// that come after the image, so we have to test to see 
		// if the image still exists
		if (::PathFileExists(abs))
		{
			arrImages.push_back(abs);
		}
	}

	// Sort chronologically
	std::sort(arrImages.begin(), arrImages.end(),
		[](const CString& a, const CString& b)
	{
		return a.CompareNoCase(b) < 0;
	});

	if (arrImages.empty())
		return;

	// ===============================================================
	// PRE-SCAN: Count how many images fall inside the Start/End range
	// ===============================================================
	UINT filteredCount = 0;

	for (auto& node : arrImages)
	{
		CString csImage = node;

		CString csFolder = CHelper::GetFolder(csImage);
		csFolder.TrimRight(L"\\");
		CString csAlbum = CHelper::GetFolder(csFolder);
		csAlbum.TrimRight(L"\\");

		if (csAlbum < csStartFolder)
			continue;

		if (csAlbum > csEndFolder)
		{
			CString csYear = CHelper::GetFolder(csAlbum);
			csYear.TrimRight(L"\\");

			if (csYear != csEndFolder)
			{
				CString csContainer = CHelper::GetFolder(csYear);
				csContainer.TrimRight(L"\\");

				if (csContainer != csEndFolder)
					break;
			}
		}

		filteredCount++;
	}

	// This is the correct number of images for the dialog
	Images = filteredCount;

	theApp.OnIdle(0);
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	CThumbnailDialog dlg;
	dlg.Parent = pFrame;
	dlg.CreateDlg();
	dlg.ShowWindow(SW_SHOW);

	dlg.TotalImages = (int)Images;
	dlg.SetWindowText(L"Caching Images");

	// ===============================================================
	// 2. Album building (original logic with Terminator)
	// ===============================================================
	arrImages.push_back(L"Terminator");

	m_mapAlbums.clear();
	m_keyFolders.clear();

	CString csCurrentAlbum;
	std::vector<CString> arrAlbumImages;

	int nImage = 0;

	for (auto& node : arrImages)
	{
		CString csImage = node;

		CString csFolder = CHelper::GetFolder(csImage);
		csFolder.TrimRight(L"\\");
		CString csAlbum = CHelper::GetFolder(csFolder);
		csAlbum.TrimRight(L"\\");

		// StartFolder / EndFolder logic (original)
		if (csImage != L"Terminator" && csAlbum < csStartFolder)
		{
			continue;
		}
		else if (csImage != L"Terminator" && csAlbum > csEndFolder)
		{
			CString csYear = CHelper::GetFolder(csAlbum);
			csYear.TrimRight(L"\\");
			if (csYear != csEndFolder)
			{
				CString csContainer = CHelper::GetFolder(csYear);
				csContainer.TrimRight(L"\\");
				if (csContainer != csEndFolder)
				{
					break;
				}
			}
		}

		if (csCurrentAlbum.IsEmpty())
		{
			csCurrentAlbum = csAlbum;
			arrAlbumImages.push_back(csImage);
		}
		else
		{
			//
			// RANGE CHECK (this is the block we replace)
			//
			if (csAlbum > csEndFolder)
			{
				if (csImage == L"Terminator")
				{
					// allow terminator to flush
				}
				else
				{
					CString csYear = CHelper::GetFolder(csAlbum);
					csYear.TrimRight(L"\\");

					if (csYear != csEndFolder)
					{
						CString csContainer = CHelper::GetFolder(csYear);
						csContainer.TrimRight(L"\\");

						if (csContainer != csEndFolder)
						{
							break;   // safe to break only after terminator
						}
					}
				}
			}

			//
			// ALBUM GROUPING (this block must remain EXACTLY as before)
			//
			if (csImage == L"Terminator" || csAlbum != csCurrentAlbum)
			{
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

				csCurrentAlbum = csAlbum;
				arrAlbumImages.clear();
				arrAlbumImages.push_back(csImage);
			}
			else
			{
				arrAlbumImages.push_back(csImage);
			}
		}

		dlg.CurrentImage = nImage++;

		if (dlg.Cancel)
			break;

		pFrame->Wait(1);
	}

	// After the for (auto& node : arrImages) loop:

	if (!csCurrentAlbum.IsEmpty() && !arrAlbumImages.empty())
	{
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
	}

	dlg.DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// initialize the document
void CPhotoPrinterDoc::InitDocument()
{
	Clear();

} // InitDocument

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterDoc::OnFileBuildPages()
{
	Clear();

	LoadSearchIndex();

	if (!Query.IsEmpty())
	{
		CacheQueriedImages();
	}
	else
	{
		CacheChronologicalImages();
	}

	const UINT uiImages = Images;
	if (uiImages > 0)
	{
		GeneratePages();

		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		pFrame->Wait(1000);

		// update the properties panel
		CPropertiesWnd* pProperties = pFrame->PropertiesPane;
		//pProperties->UpdatePropertiesFromDocument(this);
		pProperties->UpdateTableOfContents();

		// update the view
		CPhotoPrinterView* pView = PhotoPrinterView;
		pView->SetupScrollBars();
		pView->Invalidate();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterDoc::OnUpdateFileBuildPages(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);

	// verify the start and end folders are in a common folder
	CString csStartFolder = CorrectForWorkingFolder(StartFolder);
	CString csEndFolder = CorrectForWorkingFolder(EndFolder);

	if (!::PathFileExists(csStartFolder))
	{
		return;
	}
	if (!::PathFileExists(csEndFolder))
	{
		return;
	}

	// see if their parent folders are the same
	CString csStart = CHelper::GetFolder(csStartFolder);
	CString csEnd = CHelper::GetFolder(csEndFolder);

	// alternatively check to see if they are in the working folder
	CString csWorkingFolder = WorkingFolder;
	bool bStart = csStartFolder.Find(csWorkingFolder, 0) != -1;
	bool bEnd = csEndFolder.Find(csWorkingFolder, 0) != -1;
	bool bBuild = bStart && bEnd;

	// test for success in either approach
	if (bBuild || csEnd == csStart)
	{
		if (!csStart.IsEmpty() && !csEnd.IsEmpty())
		{
			pCmdUI->Enable();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoPrinterDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	BOOL value = FALSE;
	if (!CBaseDoc::OnOpenDocument(lpszPathName))
		return value;

	if (Open(lpszPathName))
	{
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		CPropertiesWnd* pProperties = pFrame->PropertiesPane;
		pProperties->UpdatePropertiesFromDocument( this );
		pProperties->PropList->ResetOriginalValues();
		InitDocument();
		CPhotoPrinterView* pView = PhotoPrinterView;
		pView->Invalidate();
		value = TRUE;
	}

	return value;
} // OnOpenDocument

/////////////////////////////////////////////////////////////////////////////
// collection of page numbers to be exported
CKeyedCollection<UINT, UINT>& CPhotoPrinterDoc::GetExportPageNumbers()
{
	m_keyExportPages.clear();

	CString csPages = ExportPages;
	csPages.Trim();
	CString csToken;
	int nStart = 0;
	bool bDone = false;
	do
	{
		csToken = csPages.Tokenize(L",", nStart);
		csToken.Trim();
		bDone = csToken.IsEmpty();
		if (!bDone)
		{
			int nBegin = 0;
			CString csBegin = csToken.Tokenize(L"-", nBegin);
			csBegin.Trim();
			UINT nPage1 = (int)_tstol(csBegin);
			UINT nPage2 = 0;
			if (csBegin == csToken)
			{
				if (!m_keyExportPages.Exists[nPage1])
				{
					shared_ptr<UINT> pRight = shared_ptr<UINT>(new UINT(0));
					m_keyExportPages.add(nPage1, pRight);
				}
			}
			else
			{
				CString csEnd = csToken.Tokenize(L"-", nBegin);
				csEnd.Trim();
				nPage2 = (int)_tstol(csEnd);
				if (nPage1 < nPage2)
				{
					for (UINT nPage = nPage1; nPage <= nPage2; nPage++)
					{
						if (!m_keyExportPages.Exists[nPage])
						{
							shared_ptr<UINT> pRight = shared_ptr<UINT>(new UINT(0));
							m_keyExportPages.add(nPage, pRight);
						}
					}
				}
			}
		}

	} while (!bDone);

	// if empty, all pages are implied
	if (m_keyExportPages.Count == 0)
	{
		UINT nPages = Pages;
		for (UINT nPage = 1; nPage <= nPages; nPage++)
		{
			if (!m_keyExportPages.Exists[nPage])
			{
				shared_ptr<UINT> pRight = shared_ptr<UINT>(new UINT(0));
				m_keyExportPages.add(nPage, pRight);
			}
		}
	}

	return m_keyExportPages;
} // GetExportPageNumbers

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterDoc::OnCloseDocument()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPropertiesWnd* pProperties = pFrame->PropertiesPane;
	Title = L"";
	Subtitle = L"";
	Publisher = L"";
	ISBN = L"";
	Description = L"";
	Copyright = L"";
	StartFolder = L"";
	EndFolder = L"";
	Query = L"";
	ExportFolder = L"";
	pProperties->UpdatePropertiesFromDocument(this);
	Clear();
	pProperties->UpdateTableOfContents();

	// Prevent MFC from doing a second save with empty contents
	SetModifiedFlag(FALSE);

	CBaseDoc::OnCloseDocument();
}

/////////////////////////////////////////////////////////////////////////////

