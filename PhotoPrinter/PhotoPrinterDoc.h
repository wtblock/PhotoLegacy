/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "BaseDoc.h"
#include "Page.h"
#include "SmartArray.h"
#include "KeyedCollection.h"
#include "CHelper.h"
#include "PhotoPrinter.h"
#include "PhotoIndexBuilder.h"
#include <gdiplus.h>

using namespace Gdiplus;

/////////////////////////////////////////////////////////////////////////////
class CPhotoPrinterView;

/////////////////////////////////////////////////////////////////////////////
// CPhotoPrinterDoc
//
// Document class for PhotoPrinter. This is the central data model that
// represents an entire multi‑page photo book, including:
//
//   • Loaded search index (image table + inverted index)
//   • Queried or labeled images grouped into albums
//   • Page objects containing image rectangles and layout information
//   • Table of contents entries
//   • Document metadata (title, subtitle, publisher, ISBN, copyright)
//   • Export settings (folder, DPI, quality, selected pages)
//   • Page geometry (margins, gutter, inside/outside margins)
//   • Working folder and start/end folder range
//
// Purpose:
//   PhotoPrinterView handles rendering, but CPhotoPrinterDoc is responsible
//   for *building* the book: discovering albums, loading images, grouping
//   them, laying them out into pages, and exposing all geometry and metadata
//   needed for device‑independent rendering.
//
// Why this class exists:
//   MFC’s CDocument provides no concept of pages, albums, margins, or
//   multi‑page layout. PhotoPrinter requires a rich document model capable of:
//
//       • Loading and querying the photo index
//       • Organizing images into albums
//       • Generating page layouts (1‑image, 2‑image, 3‑image templates, etc.)
//       • Computing margin rectangles based on even/odd pages
//       • Supporting title page and multi‑page table of contents
//       • Providing metadata for PDF export and high‑DPI rendering
//
// Responsibilities:
//   • Load the binary search index (LoadedIndex)
//   • Build album list from StartFolder → EndFolder
//   • Cache labeled or queried images into MAP_ALBUM
//   • Count images and compute total page count
//   • Generate page objects (CPage) with assigned rectangles
//   • Provide TOC entries (album → starting page)
//   • Provide all page geometry in inches and logical units
//   • Provide export settings and selected page numbers
//   • Save and load .pp project files using XML
//
// Rendering:
//   • CPhotoPrinterView calls GetCurrentPage(), MarginRectangle,
//     HeightOfPage, WidthOfPage, and other geometry properties.
//   • The document never draws anything itself; it only supplies data.
//
// This class is the backbone of PhotoPrinter’s book‑building pipeline.
/////////////////////////////////////////////////////////////////////////////
class CPhotoPrinterDoc : public CBaseDoc
{
// public definitions
public:
	typedef CKeyedCollection<CString, Image> MAP_IMAGES;
	typedef CKeyedCollection<CString, MAP_IMAGES> MAP_ALBUM;
	typedef CKeyedCollection<CString, CString> MAP_INDEX;

// protected data
protected:
	// New: loaded search index (image table + inverted index)
	CPhotoIndexBuilder::LoadedIndex m_loadedIndex;

	//// collection of image names and their corresponding comments
	//MAP_INDEX m_mapIndex;
	std::vector<CString> m_queryResults;

	// collection of albums (folders) where each album is a collection
	// of bitmaps
	MAP_ALBUM m_mapAlbums;

	// sorted folders 
	CKeyedCollection< CString, int > m_keyFolders;
	
	// collection of pages containing image names and rectangles
	CSmartArray<CPage> m_arrPages;

	// table of contents lines for the entire document where
	// the string is the album title and the int is the page
	// number where the album begins. Return value is the
	// number of lines.
	vector<pair<CString, int>> m_arrTOC;

	// collection of page numbers to be exported
	CKeyedCollection<UINT, UINT> m_keyExportPages;

	// number of pages in the document
	UINT m_nPages;

	// current page
	UINT m_nPage;

	// number of images in the document
	UINT m_nImages;

	// height of document in inches
	double m_dHeight;

	// height of page in inches
	double m_dHeightOfPage;

	// width of page in inches
	double m_dWidthOfPage;

	// document export folder
	CString m_csExportFolder;

	// document export pages: blank for all, 
	// example: 5-9, 11, 15 mean pages 5 through 9 and pages 11 and 15
	CString m_csExportPages;

	// Dots Per Inch when exporting the document
	UINT m_nExportDPI;

	// Image quality in percent between 1 and 100
	// document title
	UINT m_nExportQuality;

	CString m_csTitle;

	// document subtitle
	CString m_csSubtitle;

	// document publisher
	CString m_csPublisher;

	// document ISBN
	CString m_csISBN;

	// document copyright
	CString m_csCopyright;

	// document description
	CString m_csDescription;

	// starting folder
	CString m_csStartFolder;

	// ending folder
	CString m_csEndFolder;

	// query
	CString m_csQuery;

	// working folder
	CString m_csWorkingFolder;

// public properties
public:
	// number of albums in the document
	UINT GetAlbumCount()
	{
		UINT value = m_mapAlbums.Count;
		return value;
	}
	// number of albums in the document
	__declspec( property( get = GetAlbumCount))
		UINT AlbumCount;

	// number of pages in the table of contents
	UINT GetTableOfContentsPages()
	{
		// take into account the Title Page and Table of Contents lines that are at
		// the beginning of the first page of the table of contents.
		UINT nAlbums = 2;

		// add the actual number of albums which represents lines int th
		// table of contents
		nAlbums += AlbumCount;

		// place 55 lines on each page
		UINT value = nAlbums / 55;

		// if there is a fraction of a page, add one more page to account for it.
		UINT nMod = nAlbums % 55;
		if (nMod > 0)
		{
			value++;
		}
		return value;
	}
	// number of pages in the table of contents
	__declspec( property( get = GetTableOfContentsPages))
		UINT TableOfContentsPages;

	// number of pages in the document
	virtual UINT GetPages()
	{
		return m_nPages;
	}
	// number of pages in the document
	void SetPages(UINT value)
	{
		m_nPages = value;
	}
	// number of pages in the document
	__declspec( property( get = GetPages, put = SetPages ) )
		UINT Pages;

	// current page number of the entire document
	// including the overhead pages of title and
	// table of contents.
	virtual UINT GetPage()
	{
		return m_nPage;
	}
	// current page number of the entire document
	// including the overhead pages of title and
	// table of contents.
	void SetPage(UINT value)
	{
		m_nPage = value;
	}
	// current page number of the entire document
	// including the overhead pages of title and
	// table of contents.
	__declspec( property( get = GetPage, put = SetPage ) )
		UINT Page;

	// current page object where page objects contain
	// the rendered images and the indexing is zero
	// based compared to document pages that are one based 
	// and include overhead pages for the title page and 
	// the table of contents page.
	shared_ptr<CPage> GetCurrentPage()
	{
		shared_ptr<CPage> value;

		// this is a one based page number: 1..n
		UINT nPage = Page;
		UINT nPagesTOC = TableOfContentsPages;
		UINT nOverhead = nPagesTOC + 1;
		if (nPage > nOverhead)
		{
			// this is a zero based index accounting for 
			// overhead pages, so we need to decrement
			// by overhead + 1 to make zero based
			nPage -= (nOverhead + 1);
			const int nPages = m_arrPages.Count;
			value = m_arrPages.get((long)nPage);
		}
		return value;
	}
	// current page object where page objects contain
	// the rendered images and the indexing is zero
	// based compared to document pages that are one based 
	// and include overhead pages for the title page and 
	// the table of contents page.
	__declspec( property( get = GetCurrentPage) )
		shared_ptr<CPage> CurrentPage;

	// height of page in inches
	virtual double GetHeightOfPage()
	{
		return m_dHeightOfPage;
	}
	// height of page in inches
	void SetHeightOfPage(double value)
	{
		m_dHeightOfPage = value;
	}
	// height of page in inches
	__declspec(property(get = GetHeightOfPage, put = SetHeightOfPage))
		double HeightOfPage;

	// width of page in inches
	virtual double GetWidthOfPage()
	{
		return m_dWidthOfPage;
	}
	// width of page in inches
	void SetWidthOfPage(double value)
	{
		m_dWidthOfPage = value;
	}
	// width of page in inches
	__declspec(property(get = GetWidthOfPage, put = SetWidthOfPage))
		double WidthOfPage;

	// top of page in inches
	double GetTopOfPage()
	{
		double value = 0;
		const UINT nPage = Page;
		const double dHeight = HeightOfPage;
		value = dHeight * (nPage - 1);
		return value;
	}
	// top of page in inches
	__declspec(property(get = GetTopOfPage))
		double TopOfPage;

	// bottom of page in inches
	double GetBottomOfPage()
	{
		double value = TopOfPage;
		const double dHeight = HeightOfPage;
		value += dHeight;
		return value;
	}
	// bottom of page in inches
	__declspec(property(get = GetBottomOfPage))
		double BottomOfPage;

	// height of document in inches
	virtual double GetHeight()
	{
		m_dHeight = HeightOfPage * Pages;
		return m_dHeight;
	}
	// height of document in inches
	void SetHeight(double value)
	{
		m_dHeight = value;
	}
	// height of document in inches
	__declspec( property( get = GetHeight, put = SetHeight ) )
		double Height;

	// width of document in inches
	virtual double GetWidth()
	{
		return WidthOfPage;
	}
	// width of document in inches
	void SetWidth(double value)
	{
		WidthOfPage = value;
	}
	// width of document in inches
	__declspec(property(get = GetWidth, put = SetWidth))
		double Width;

	// top margin in inches
	double GetTopMargin()
	{
		return 0.5;
	}
	// top margin in inches
	__declspec( property( get = GetTopMargin ) )
		double TopMargin;

	// bottom margin in inches
	double GetBottomMargin()
	{
		return 0.5;
	}
	// bottom margin in inches
	__declspec( property( get = GetBottomMargin ) )
		double BottomMargin;

	// inside margin in inches
	double GetInsideMargin()
	{
		return 0.6;
	}
	// inside margin in inches
	__declspec( property( get = GetInsideMargin ) )
		double InsideMargin;

	// outside margin in inches
	double GetOutsideMargin()
	{
		return 0.4;
	}
	// outside margin in inches
	__declspec( property( get = GetOutsideMargin ) )
		double OutsideMargin;

	// gutter in inches
	double GetGutter()
	{
		return 0.2;
	}
	// gutter in inches
	__declspec( property( get = GetGutter ) )
		double Gutter;

	// left margin of the current page
	double GetLeftMargin()
	{
		double value = InsideMargin + Gutter;
		const UINT nPage = Page;
		const bool bEven = CHelper::GetEven(nPage);
		if (bEven)
		{
			value = OutsideMargin;
		}
		return value;
	}
	// left margin of the current page
	__declspec(property(get = GetLeftMargin))
		double LeftMargin;

	// right margin of the current page
	double GetRightMargin()
	{
		double value = InsideMargin + Gutter;
		const UINT nPage = Page;
		const bool bOdd = CHelper::GetOdd(nPage);
		if (bOdd)
		{
			value = OutsideMargin;
		}
		return value;
	}
	// right margin of the current page
	__declspec(property(get = GetRightMargin))
		double RightMargin;

	// The margin rectangle varies depending on the page
	// number being an even or odd number. Even pages are
	// on the left side of the printed book and odd pages
	// are on the right side of the printed book
	CRect GetMarginRectangle()
	{
		const double dTopOfPage = TopOfPage;
		const double dBottomOfPage = BottomOfPage;
		const double dRightOfPage = Width;

		const double dTopMargin = TopMargin;
		const double dBottomMargin = BottomMargin;
		const double dLeftMargin = LeftMargin;
		const double dRightMargin = RightMargin;

		const int nTop = InchesToLogical(dTopOfPage + dTopMargin);
		const int nBottom = InchesToLogical(dBottomOfPage - dBottomMargin);
		const int nLeft = InchesToLogical(dLeftMargin);
		const int nRight = InchesToLogical(dRightOfPage - dRightMargin);

		CRect value(nLeft, nTop, nRight, nBottom);

		return value;
	} 
	// The margin rectangle varies depending on the page
	// number being an even or odd number. Even pages are
	// on the left side of the printed book and odd pages
	// are on the right side of the printed book
	__declspec(property(get = GetMarginRectangle))
		CRect MarginRectangle;

	// table of contents lines for the entire document where
	// the string is the album title and the int is the page
	// number where the album begins. Return value is the
	// number of lines.
	vector<pair<CString, int>>& GetAlbumTableOfContents();
	// table of contents lines for the entire document where
	// the string is the album title and the int is the page
	// number where the album begins. Return value is the
	// number of lines.
	__declspec(property(get = GetAlbumTableOfContents))
		vector<pair<CString, int>>& AlbumTableOfContents;
	
	// document export folder
	CString GetExportFolder()
	{
		return m_csExportFolder;
	}
	// document export folder
	void SetExportFolder(CString value)
	{
		if (!value.IsEmpty())
		{
			if (value.Right(1) != L"\\")
			{
				value += L"\\";
			}
		}
		m_csExportFolder = value;
	}
	// document export folder
	__declspec(property(get = GetExportFolder, put = SetExportFolder))
		CString ExportFolder;

	// document export pages: blank for all, 
	// example: 5-9, 11, 15 mean pages 5 through 9 and pages 11 and 15
	CString GetExportPages()
	{
		return m_csExportPages;
	}
	// document export pages: blank for all, 
	// example: 5-9, 11, 15 mean pages 5 through 9 and pages 11 and 15
	void SetExportPages(CString value)
	{
		m_csExportPages = value;
	}
	// document export pages: blank for all, 
	// example: 5-9, 11, 15 mean pages 5 through 9 and pages 11 and 15
	__declspec(property(get = GetExportPages, put = SetExportPages))
		CString ExportPages;

	// collection of page numbers to be exported
	CKeyedCollection<UINT, UINT>& GetExportPageNumbers();
	// collection of page numbers to be exported
	__declspec(property(get = GetExportPageNumbers))
		CKeyedCollection<UINT, UINT>& ExportPageNumbers;

	// Dots Per Inch when exporting the document
	UINT GetExportDPI()
	{
		return m_nExportDPI;
	}
	// Dots Per Inch when exporting the document
	void SetExportDPI(UINT value)
	{
		m_nExportDPI = value;
	}
	// Dots Per Inch when exporting the document
	__declspec(property(get = GetExportDPI, put = SetExportDPI))
		UINT ExportDPI;

	// Image quality in percent between 1 and 100
	UINT GetExportQuality()
	{
		return m_nExportQuality;
	}
	// Image quality in percent between 1 and 100
	void SetExportQuality(UINT value)
	{
		if (value < 1)
		{
			value = 75;
		}
		else if (value > 100)
		{
			value = 100;
		}
		m_nExportQuality = value;
	}
	// Image quality in percent between 1 and 100
	__declspec(property(get = GetExportQuality, put = SetExportQuality))
		UINT ExportQuality;

	// document title
	CString GetTitle()
	{
		return m_csTitle;
	}
	// document title
	void SetTitle(CString value)
	{
		m_csTitle = value;
	}
	// document title
	__declspec(property(get = GetTitle, put = SetTitle))
		CString Title;

	// document subtitle
	CString GetSubtitle()
	{
		return m_csSubtitle;
	}
	// document subtitle
	void SetSubtitle(CString value)
	{
		m_csSubtitle = value;
	}
	// document subtitle
	__declspec(property(get = GetSubtitle, put = SetSubtitle))
		CString Subtitle;

	// document publisher
	CString GetPublisher()
	{
		return m_csPublisher;
	}
	// document publisher
	void SetPublisher(CString value)
	{
		m_csPublisher = value;
	}
	// document publisher
	__declspec(property(get = GetPublisher, put = SetPublisher))
		CString Publisher;

	// document ISBN
	CString GetISBN()
	{
		return m_csISBN;
	}
	// document ISBN
	void SetISBN(CString value)
	{
		m_csISBN = value;
	}
	// document ISBN
	__declspec(property(get = GetISBN, put = SetISBN))
		CString ISBN;

	// document copyright
	CString GetCopyright()
	{
		return m_csCopyright;
	}
	// document copyright
	void SetCopyright(CString value)
	{
		m_csCopyright = value;
	}
	// document copyright
	__declspec(property(get = GetCopyright, put = SetCopyright))
		CString Copyright;

	// document description
	CString GetDescription()
	{
		return m_csDescription;
	}
	// document description
	void SetDescription(CString value)
	{
		m_csDescription = value;
	}
	// document description
	__declspec(property(get = GetDescription, put = SetDescription))
		CString Description;

	// starting folder
	CString GetStartFolder()
	{
		return m_csStartFolder;
	}
	// starting folder
	void SetStartFolder(CString value)
	{
		m_csStartFolder = value;
	}
	// starting folder
	__declspec(property(get = GetStartFolder, put = SetStartFolder))
		CString StartFolder;

	// ending folder
	CString GetEndFolder()
	{
		return m_csEndFolder;
	}
	// ending folder
	void SetEndFolder(CString value)
	{
		m_csEndFolder = value;
	}
	// ending folder
	__declspec(property(get = GetEndFolder, put = SetEndFolder))
		CString EndFolder;

	// query
	CString GetQuery()
	{
		return m_csQuery;
	}
	// query
	void SetQuery(CString value)
	{
		m_csQuery = value;
	}
	// query
	__declspec(property(get = GetQuery, put = SetQuery))
		CString Query;

	// the view associated with this document
	CPhotoPrinterView* GetPhotoPrinterView();
	// the view associated with this document
	__declspec(property(get = GetPhotoPrinterView))
		CPhotoPrinterView* PhotoPrinterView;

	// number of images in the document
	virtual UINT GetImages()
	{
		return m_nImages;
	}
	// number of images in the document
	void SetImages(UINT value)
	{
		m_nImages = value;
	}
	// number of images in the document
	__declspec(property(get = GetImages, put = SetImages))
		UINT Images;

	// optional working folder 
	inline CString GetWorkingFolder()
	{
		CPhotoPrinterApp* pApp = (CPhotoPrinterApp*)AfxGetApp();
		m_csWorkingFolder = pApp->WorkingFolder;
		return m_csWorkingFolder;
	}
	// optional working folder 
	__declspec(property(get = GetWorkingFolder))
		CString WorkingFolder;

// protected methods
protected:
	// clear the document data
	void Clear()
	{
		m_mapAlbums.clear();
		m_keyFolders.clear();
		m_arrPages.clear();
		m_nPages = 2;
		Height = HeightOfPage;
	}

	// does the given path contain an album
	bool ContainsAnAlbum(CString folderPath);

	// find all folders contained in a given folder that are in the
	// interested range
	void ListSubFolders(CString folderPath);

	// count the images within the labeled folders
	void CountLabeledImages();

	// count the queried images
	void CountQueriedImages(vector<CString>& arrQuery);

	// cache the images within the labeled folders
	void CacheLabeledImages();

	// cache the queried images
	void CacheQueriedImages();

	void CacheChronologicalImages();

		// collect the image pages and allocate the rectangles
	// used to contain each image on the page
	void GeneratePages();

	// return the XP comment metadata from the given image
	CString GetXPComment(Image* pImage);

// public methods
public:
	shared_ptr<Image> FindImage(CString csFolder, CString csImage);

	bool LoadSearchIndex();

	// initialize the document
	void InitDocument();

	// correct for working folder
	inline CString CorrectForWorkingFolder(CString csInput)
	{
		CString value;
		if (csInput.Left(1) == L".")
		{
			csInput.TrimLeft(L".");
			CString csWork = WorkingFolder;
			value.Format(L"%s%s", csWork, csInput);
		}
		else
		{
			value = csInput;
		}
		return value;
	}

// protected overrides
protected:

// public overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL SaveModified();
	virtual BOOL OnSaveDocument(CString& csPath);
	// Save the document data to a file
	// lpszPathName = path name where to save document file
	// if lpszPathName is NULL then the user will be prompted (SaveAs)
	// note: lpszPathName can be different than 'm_strPathName'
	// if 'bReplace' is TRUE will change file name if successful (SaveAs)
	// if 'bReplace' is FALSE will not change path name (SaveCopyAs)
	virtual BOOL DoSave(CString& csPath, BOOL bReplace = TRUE);
	// open a data file
	virtual bool Open
	(
		LPCTSTR szFilename, // name of data file to open
		bool bRead = true, // do an initial read after open
		LPCTSTR pcszFileID = _T("F1")
	);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// public constructor/destructor
public:
	CPhotoPrinterDoc() noexcept;
	DECLARE_DYNCREATE(CPhotoPrinterDoc)
	virtual ~CPhotoPrinterDoc();

	BOOL PromptForFileName(CString& strFilePath);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
public:

    afx_msg void OnFileBuildPages();
    afx_msg void OnUpdateFileBuildPages(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////
