/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "PhotoPrinter.h"
#include "PhotoPrinterDoc.h"
#include "PhotoPrinterView.h"
#include "MainFrm.h"
#include "ThumbnailDialog.h"
#include "ImagePlus.h"
#include "PdfWriter.h"

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CPhotoPrinterView, CBaseView)

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPhotoPrinterView, CBaseView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CBaseView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CBaseView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CPhotoPrinterView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_SCROLL_TOP, &CPhotoPrinterView::OnScrollTop)
	ON_COMMAND(ID_SCROLL_BOTTOM, &CPhotoPrinterView::OnScrollBottom)
	ON_COMMAND(ID_SCROLL_PAGEUP, &CPhotoPrinterView::OnScrollPageup)
	ON_COMMAND(ID_SCROLL_PAGEDOWN, &CPhotoPrinterView::OnScrollPagedown)
	ON_COMMAND(ID_SCROLL_LINEUP, &CPhotoPrinterView::OnScrollLineup)
	ON_COMMAND(ID_SCROLL_LINEDOWN, &CPhotoPrinterView::OnScrollLinedown)
	ON_COMMAND(ID_FILE_EXPORTPAGES, &CPhotoPrinterView::OnFileExportPages)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXPORTPAGES, &CPhotoPrinterView::OnUpdateFileExportPages)
	ON_COMMAND(ID_FILE_PDF, &CPhotoPrinterView::OnFilePdf)
	ON_UPDATE_COMMAND_UI(ID_FILE_PDF, &CPhotoPrinterView::OnUpdateFilePdf)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CPhotoPrinterView::CPhotoPrinterView() noexcept
{
	// TODO: add construction code here

}

/////////////////////////////////////////////////////////////////////////////
CPhotoPrinterView::~CPhotoPrinterView()
{
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoPrinterView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= WS_VSCROLL | WS_HSCROLL; // Ensure scrollbars are enabled
	cs.dwExStyle &= ~WS_EX_COMPOSITED;   // Disable modern composited rendering
	cs.dwExStyle |= WS_EX_CLIENTEDGE;    // Force classic scrollbar appearance

	return CBaseView::PreCreateWindow(cs);
} // PreCreateWindow

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::RenderMargins
(
	CDC* pDC, double dLeftOfView, double dTopOfView, 
	double dRightOfView, double dBottomOfView
)
{
	CPhotoPrinterDoc* pDoc = GetDocument();
	const int nPage = pDoc->Page;

	// save the entry state
	const int nDC = pDC->SaveDC();

	const int nLeftOfView = InchesToLogical(dLeftOfView);
	const int nTopOfView = InchesToLogical(dTopOfView);
	const int nRightOfView = InchesToLogical(dRightOfView);
	const int nBottomOfView = InchesToLogical(dBottomOfView);
	pDC->SetWindowOrg(nLeftOfView, nTopOfView);

	int nMode = pDC->SetBkMode(TRANSPARENT);
	const double dLineWidth = 0.02;
	const int nLineWidth = InchesToLogical(dLineWidth);
	CPen penBorder(PS_SOLID, nLineWidth, RGB( 0, 0, 0 ));
	CPen penRect(PS_DOT, 0, RGB( 128, 0, 0 ));
	CPen* pPen = pDC->SelectObject(&penBorder);

	CRect rect = pDoc->MarginRectangle;
	if (nPage == 1)
	{
		pDC->Rectangle(&rect);
	}

	double dBottomOfPage = pDoc->HeightOfPage * nPage;
	int nBottomOfPage = InchesToLogical(dBottomOfPage);

	const bool bPrinting = Printing;
	const bool bExporting = Exporting;
	if (!bPrinting && !bExporting)
	{
		pDC->MoveTo(nLeftOfView, nBottomOfPage);
		pDC->LineTo(nRightOfView, nBottomOfPage);
	}

	//shared_ptr<CPage> page = pDoc->CurrentPage;
	//if (page != nullptr)
	//{
	//	page->RenderImageRectangles(pDC);
	//}

	// restore the entry state
	pDC->RestoreDC(nDC);

} // RenderMargins

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::RenderHeader
(
	CDC* pDC, double dLeftOfView, double dTopOfView,
	double dRightOfView, double dBottomOfView
)
{
	CPhotoPrinterDoc* pDoc = GetDocument();
	const UINT nPage = pDoc->Page;
	const bool bEven = CHelper::GetEven(nPage);

	// no header on the overhead pages
	UINT nPagesTOC = pDoc->TableOfContentsPages;
	UINT nOverhead = nPagesTOC + 1;
	if (nPage <= nOverhead)
	{
		return;
	}

	// save the entry state
	const int nDC = pDC->SaveDC();

	const int nLeftOfView = InchesToLogical(dLeftOfView);
	const int nTopOfView = InchesToLogical(dTopOfView);
	pDC->SetWindowOrg(nLeftOfView, nTopOfView);
	const int nTopMargin = InchesToLogical(pDoc->TopMargin);

	shared_ptr<CPage> page = pDoc->CurrentPage;
	if (page == nullptr)
	{
		return;
	}
	const CString csFolder = page->Folder;
	const CString csAlbum = CHelper::GetDataName(csFolder);
	const CString csTitle = pDoc->Title;

	const int nMap = pDoc->Map;
	const int nPoint12 = int(nMap * 12.0 / 72.0);

	CFont fontText;
	BuildFont(L"Arial", true, false, nPoint12, false, fontText);

	int nMode = pDC->SetBkMode(TRANSPARENT);
	CFont* pFont = pDC->SelectObject(&fontText);
	COLORREF rgbOld = pDC->SetTextColor(RGB( 0, 0, 188 ));

	const UINT uiFormatLeft = 
		DT_LEFT | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP;
	const UINT uiFormatRight = 
		DT_RIGHT | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP;
	
	// the text rectangle located in the top margin
	CRect rect = pDoc->MarginRectangle;
	CRect rectText = rect;
	rectText.bottom = rectText.top;
	rectText.top -= nPoint12;

	if (bEven)
	{
		pDC->DrawText(csTitle, &rectText, uiFormatLeft);
		pDC->DrawText(csAlbum, &rectText, uiFormatRight);
	}
	else
	{
		pDC->DrawText(csTitle, &rectText, uiFormatRight);
		pDC->DrawText(csAlbum, &rectText, uiFormatLeft);
	}

	// restore the entry state
	pDC->RestoreDC(nDC);
} // RenderHeader

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::RenderFooter
(
	CDC* pDC, double dLeftOfView, double dTopOfView,
	double dRightOfView, double dBottomOfView
)
{
	CPhotoPrinterDoc* pDoc = GetDocument();
	const UINT nPage = pDoc->Page;
	const UINT nPages = pDoc->Pages;
	const bool bEven = CHelper::GetEven(nPage);

	// no footer on the overhead pages
	UINT nPagesTOC = pDoc->TableOfContentsPages;
	UINT nOverhead = nPagesTOC + 1;
	if (nPage <= nOverhead)
	{
		return;
	}

	// save the entry state
	const int nDC = pDC->SaveDC();

	const int nLeftOfView = InchesToLogical(dLeftOfView);
	const int nTopOfView = InchesToLogical(dTopOfView);
	pDC->SetWindowOrg(nLeftOfView, nTopOfView);
	const int nTopMargin = InchesToLogical(pDoc->TopMargin);

	shared_ptr<CPage> page = pDoc->CurrentPage;
	if (page == nullptr)
	{
		return;
	}
	CString csFooter;
	csFooter.Format(L"Page %d of %d", nPage, nPages);

	const int nMap = pDoc->Map;
	const int nPoint12 = int(nMap * 12.0 / 72.0);

	CFont fontText;
	BuildFont(L"Arial", true, false, nPoint12, false, fontText);

	int nMode = pDC->SetBkMode(TRANSPARENT);
	CFont* pFont = pDC->SelectObject(&fontText);
	COLORREF rgbOld = pDC->SetTextColor(RGB( 0, 0, 188 ));

	const UINT uiFormat = 
		DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP;
	
	// the text rectangle located in the top margin
	CRect rect = pDoc->MarginRectangle;
	CRect rectText = rect;
	rectText.top = rectText.bottom;
	rectText.bottom += nPoint12;

	pDC->DrawText(csFooter, &rectText, uiFormat);

	// restore the entry state
	pDC->RestoreDC(nDC);
} // RenderFooter

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::RenderTitlePage
(
	CDC* pDC, double dLeftOfView, double dTopOfView, 
	double dRightOfView, double dBottomOfView
)
{
	CPhotoPrinterDoc* pDoc = GetDocument();
	const int nPage = pDoc->Page;
	if (nPage != 1)
	{
		return;
	}

	// save the entry state
	const int nDC = pDC->SaveDC();

	const int nMap = pDoc->Map;

	const int nLeftOfView = InchesToLogical(dLeftOfView);
	const int nTopOfView = InchesToLogical(dTopOfView);
	pDC->SetWindowOrg(nLeftOfView, nTopOfView);
	const int nTopMargin = InchesToLogical(pDoc->TopMargin);

	const CString csTitle = pDoc->Title;
	const CString csSubtitle = pDoc->Subtitle;
	const CString csPublisher = pDoc->Publisher;
	const CString csISBN = pDoc->ISBN;
	const CString csCopyright = pDoc->Copyright;
	const CString csDescription = pDoc->Description;


	const int nPoint24 = int(nMap * 24.0 / 72.0);
	const int nPoint12 = nPoint24 / 2;

	CFont fontTitle, fontText;
	BuildFont(L"Arial", true, false, nPoint24, false, fontTitle);
	BuildFont(L"Arial", true, false, nPoint12, false, fontText);

	int nMode = pDC->SetBkMode(TRANSPARENT);
	CFont* pFont = pDC->SelectObject(&fontTitle);
	COLORREF rgbOld = pDC->SetTextColor(RGB( 0, 0, 0 ));

	CRect rect = pDoc->MarginRectangle;
	CPoint ptCenter = rect.CenterPoint();
	const UINT uiFormat = 
		DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP;

	CRect rectTitle = rect;
	rectTitle.top += InchesToLogical(1.5);
	pDC->DrawText(csTitle, &rectTitle, uiFormat);

	pDC->SelectObject(&fontText);

	CRect rectSubtitle = rectTitle;
	rectSubtitle.top += InchesToLogical(0.5);
	pDC->DrawText(csSubtitle, &rectSubtitle, uiFormat);

	CRect rectPublish = rectSubtitle;
	rectPublish.top += InchesToLogical(1.0);
	CString csPublish;
	csPublish.Format
	(
		L"%s\n%s\n%s", 
		(LPCTSTR)csPublisher, (LPCTSTR)csISBN, (LPCTSTR)csCopyright
	);
	pDC->DrawText(csPublish, &rectPublish, uiFormat);

	CRect rectDesc = rect;
	rectDesc.top = ptCenter.y;
	pDC->DrawText(csDescription, &rectDesc, uiFormat);

	// restore the entry state
	pDC->RestoreDC(nDC);
} // RenderTitlePage

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::RenderTableOfContentsPage
(
	CDC* pDC, double dLeftOfView, double dTopOfView,
	double dRightOfView, double dBottomOfView
)
{
	CPhotoPrinterDoc* pDoc = GetDocument();
	UINT nPagesTOC = pDoc->TableOfContentsPages;
	UINT nOverhead = nPagesTOC + 1;
	const UINT nPage = pDoc->Page;
	if (nPage == 1 || nPage > nOverhead )
	{
		return;
	}

	// save the entry state
	const int nDC = pDC->SaveDC();

	const int nMap = pDoc->Map;

	const int nLeftOfView = InchesToLogical(dLeftOfView);
	const int nTopOfView = InchesToLogical(dTopOfView);

	pDC->SetWindowOrg(nLeftOfView, nTopOfView);
	const int nTopMargin = InchesToLogical(pDoc->TopMargin);

	const int nPoint24 = int(nMap * 24.0 / 72.0);
	const int nPoint12 = nPoint24 / 2;

	CFont fontTitle, fontText;
	BuildFont(L"Arial", true, false, nPoint24, false, fontTitle);
	BuildFont(L"Arial", true, false, nPoint12, false, fontText);

	CPen penRect(PS_DOT, 0, RGB( 128, 0, 0 ));
	pDC->SelectObject(&penRect);

	int nMode = pDC->SetBkMode(OPAQUE);
	CFont* pFont = pDC->SelectObject(&fontTitle);
	COLORREF rgbOld = pDC->SetTextColor(RGB(0, 0, 0));

	CRect rect = pDoc->MarginRectangle;

	const UINT uiLeft =
		DT_LEFT | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP;
	const UINT uiRight =
		DT_RIGHT | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP;

	CRect rectLine = rect;
	rectLine.top += InchesToLogical(0.25);
	rectLine.bottom = rectLine.top + nPoint24;

	//pDC->Rectangle(&rectLine);
	pDC->DrawText(L"Table of Contents", &rectLine, uiLeft);

	rectLine.top += nPoint24;
	rectLine.bottom = rectLine.top + nPoint12;

	pDC->SelectObject(&fontText);

	vector<pair<CString, int>>& arrItems = pDoc->AlbumTableOfContents;
	UINT nFirstLine = (nPage - 2) * 55;
	UINT nLastLine = nFirstLine + 55;

	const bool bPDF = PDF;

	UINT nLine = 0;
	for (auto& node : arrItems)
	{
		if (nLine >= nFirstLine && nLine < nLastLine)
		{
			const CString csLabel = node.first;
			const int nPage = node.second;
			CString csPage;
			csPage.Format(L"%d", nPage);

			int nHeight = rectLine.Height();
			CRect rectDots = rectLine;
			rectDots.DeflateRect(0, nHeight / 3);
			pDC->MoveTo(rectDots.left, rectDots.bottom);
			pDC->LineTo(rectDots.right, rectDots.bottom);
			pDC->DrawText(csLabel, &rectLine, uiLeft);
			pDC->DrawText(csPage, &rectLine, uiRight);

			rectLine.top += nPoint12;
			rectLine.bottom = rectLine.top + nPoint12;
		}
		nLine++;
	}

	// restore the entry state
	pDC->RestoreDC(nDC);

} // RenderTableOfContentsPage

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::DrawImage
(
	CDC* pDC, shared_ptr<Image>& pImage, const CRect* pRect
)
{
	if (!pImage || !pDC) 
		return;

	// Cast shared_ptr<Image> to shared_ptr<Bitmap>
	shared_ptr<Bitmap> pBitmap = static_pointer_cast<Bitmap>(pImage);

	if (!pBitmap) 
		return;

	// save the entry state
	const int nDC = pDC->SaveDC();

	// Get original image dimensions
	int imgWidth = pBitmap->GetWidth();
	int imgHeight = pBitmap->GetHeight();
	CSize size = pRect->Size();
	const float fImageAspect =
		CHelper::GetAspectRatio(imgWidth, imgHeight);

	int nLeft = pRect->left;
	int nTop = pRect->top;

	int drawWidth = size.cx;
	int drawHeight = static_cast<int>(drawWidth / fImageAspect);

	if (drawHeight > size.cy) 
	{
		drawHeight = size.cy;
		drawWidth = static_cast<int>(drawHeight * fImageAspect);
		nLeft += (size.cx - drawWidth) / 2;
	}
	else
	{
		nTop += (size.cy - drawHeight) / 2;
	}

	// GDI+ does not correctly honor the GDI mapping mode so the
	// CImagePlus class is my work around for that. By honoring
	// the GDI mapping mode, the same code can display images
	// on the screen, the print preview, and printed output without
	// being concerned about the resolution of these devices.
	CImagePlus GDI(pBitmap);
	CPoint ptDest(nLeft, nTop);
	CSize sizeDest(drawWidth, drawHeight);
	CRect rectDest(ptDest, sizeDest);
	GDI.Draw(pDC, rectDest);

} // DrawImage

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::RenderImagePage
(
	CDC* pDC, double dLeftOfView, double dTopOfView,
	double dRightOfView, double dBottomOfView
)
{
	CPhotoPrinterDoc* pDoc = GetDocument();
	const UINT nPage = pDoc->Page;
	const UINT nPages = pDoc->Pages;
	const bool bEven = CHelper::GetEven(nPage);
	const double dPageHeight = pDoc->HeightOfPage;
	const int nPageHeight = InchesToLogical(dPageHeight);
	UINT nPagesTOC = pDoc->TableOfContentsPages;
	UINT nOverhead = nPagesTOC + 1;

	// no images on the overhead pages
	if (nPage <= nOverhead)
	{
		return;
	}

	// save the entry state
	const int nDC = pDC->SaveDC();

	const int nLeftOfView = InchesToLogical(dLeftOfView);
	const int nTopOfView = InchesToLogical(dTopOfView);
	pDC->SetWindowOrg(nLeftOfView, nTopOfView);

	shared_ptr<CPage> page = pDoc->CurrentPage;
	if (page != nullptr)
	{
		CString csFolder = page->Folder;
		CKeyedCollection<CString, CRect>& mapImages = page->Images;
		for (auto& node : mapImages.Items)
		{
			const CString csImage = node.first;
			shared_ptr<CRect> pRect = node.second;
			double dTop = LogicalToInches(pRect->top);
			double dBottom = LogicalToInches(pRect->bottom);
			if (dBottomOfView < dTop) // entire view is above rectangle
			{
				break; // we are done
			}
			else if (dTopOfView > dBottom) // entire view is below rectangle
			{
				continue;
			}

			shared_ptr<Image> pImage = pDoc->FindImage(csFolder, csImage);
			if (pImage)
			{
				DrawImage(pDC, pImage, pRect.get());
			}
		}
	}

	// restore the entry state
	pDC->RestoreDC(nDC);

} // RenderImagePage

/////////////////////////////////////////////////////////////////////////////
// render the page or view
void CPhotoPrinterView::render
(
	CDC* pDC,
	double dLeftOfView,
	double dTopOfView,
	double dRightOfView,
	double dBottomOfView
)
{
	CBaseView::render(pDC, dLeftOfView, dTopOfView, dRightOfView, dBottomOfView );
	CPhotoPrinterDoc* pDoc = GetDocument();

	const UINT nPages = pDoc->Pages;
	UINT nSavedPage = pDoc->Page;
	const double dPageHeight = pDoc->HeightOfPage;
	double dTopOfPage = 0;


	// loop through all pages in the document and draw those which 
	// overlap our view
	for (UINT nPage = 0; nPage < nPages; nPage++)
	{
		// physical pages begin with page one
		pDoc->Page = nPage + 1;

		dTopOfPage = dPageHeight * nPage;
		const double dBottomOfPage = dTopOfPage + dPageHeight;
		if (dBottomOfView < dTopOfPage) // entire view is above page
		{
			break; // we are done
		}
		else if (dTopOfView > dBottomOfPage) // entire view is below page
		{
			continue;
		}

		RenderMargins
		(
			pDC, dLeftOfView, dTopOfView, dRightOfView, dBottomOfView
		);
		RenderHeader
		(
			pDC, dLeftOfView, dTopOfView, dRightOfView, dBottomOfView
		);
		RenderFooter
		(
			pDC, dLeftOfView, dTopOfView, dRightOfView, dBottomOfView
		);
		RenderTitlePage
		(
			pDC, dLeftOfView, dTopOfView, dRightOfView, dBottomOfView
		);
		RenderTableOfContentsPage
		(
			pDC, dLeftOfView, dTopOfView, dRightOfView, dBottomOfView
		);
		RenderImagePage
		(
			pDC, dLeftOfView, dTopOfView, dRightOfView, dBottomOfView
		);
	}

	CMainFrame* pMainFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame)
	{
		pMainFrame->PropertiesPane->UpdatePropertiesFromDocument(static_cast<CPhotoPrinterDoc*>(pDoc));
	}

	pDoc->Page = nSavedPage;
} // render

/////////////////////////////////////////////////////////////////////////////
bool EncodeBitmapToJpegMemory
(
	Gdiplus::Bitmap* bmp,
	std::vector<BYTE>& outBuffer,
	ULONG quality = 90
)
{
	if (!bmp)
		return false;

	CLSID clsidEncoder;
	UINT numEncoders = 0, size = 0;

	Gdiplus::GetImageEncodersSize(&numEncoders, &size);
	if (size == 0)
		return false;

	std::vector<BYTE> buffer(size);
	Gdiplus::ImageCodecInfo* pInfo =
		reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());

	Gdiplus::GetImageEncoders(numEncoders, size, pInfo);

	for (UINT i = 0; i < numEncoders; i++)
	{
		if (wcscmp(pInfo[i].MimeType, L"image/jpeg") == 0)
		{
			clsidEncoder = pInfo[i].Clsid;
			break;
		}
	}

	// JPEG quality parameter
	Gdiplus::EncoderParameters params;
	params.Count = 1;
	params.Parameter[0].Guid = Gdiplus::EncoderQuality;
	params.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
	params.Parameter[0].NumberOfValues = 1;
	params.Parameter[0].Value = &quality;

	// Create memory stream
	IStream* pStream = nullptr;
	CreateStreamOnHGlobal(NULL, TRUE, &pStream);

	// Encode into stream
	bmp->Save(pStream, &clsidEncoder, &params);

	// Extract bytes
	STATSTG stat;
	pStream->Stat(&stat, STATFLAG_NONAME);

	outBuffer.resize((size_t)stat.cbSize.QuadPart);

	LARGE_INTEGER zero = {};
	pStream->Seek(zero, STREAM_SEEK_SET, NULL);

	ULONG bytesRead = 0;
	pStream->Read(outBuffer.data(), (ULONG)outBuffer.size(), &bytesRead);

	pStream->Release();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::ExportDocument()
{
	CPhotoPrinterDoc* pDoc = GetDocument();
	double dLeftOfView = 0;
	double dTopOfView = 0;
	double dRightOfView = pDoc->WidthOfPage;
	double dBottomOfView = pDoc->HeightOfPage;

	CString csFolder = pDoc->CorrectForWorkingFolder(pDoc->ExportFolder);
	if (csFolder.IsEmpty())
	{
		return;
	}

	csFolder += pDoc->Title;
	csFolder += L"\\";
	if (!::PathFileExists(csFolder))
	{
		if (!CHelper::CreatePath(csFolder))
		{
			return;
		}
	}

	UINT nSavedPage = pDoc->Page;
	const double dPageHeight = pDoc->HeightOfPage;
	double dTopOfPage = 0;

	const int dpi = (int)pDoc->ExportDPI;
	const int width = static_cast<int>(dRightOfView * dpi);
	const int height = static_cast<int>(dBottomOfView * dpi);

	// the quality of the images in percent
	ULONG ulQuality = pDoc->ExportQuality;

	// the view's PDF property is set if the method was called as 
	// a result of the user clicking on the PDF toolbar button.
	const bool bExportPDF = PDF;
	CPdfWriter pdf;
	if (bExportPDF)
	{
		CString csTitle = pDoc->Title;
		CString csPDF = csFolder + csTitle + L".pdf";
		if (!pdf.Begin(csPDF, dpi))
		{
			CString csMessage;
			csMessage.Format(L"Failed to create PDF:\n%s", (LPCTSTR)csPDF);
			AfxMessageBox(csMessage);
			return;
		}

		CString csKeywords;
		CString csQuery = pDoc->Query;
		int nStart = 0;
		CString csToken = csQuery.Tokenize(L" |", nStart);
		while (!csToken.IsEmpty())
		{
			csKeywords += csToken + L"; ";
			csToken = csQuery.Tokenize(L" |", nStart);
		}

		csToken.Format(L"DPI:%03d; ", dpi);
		csKeywords += csToken;
		csToken.Format(L"Quality:%03d%%; ", ulQuality);
		csKeywords += csToken;

		pdf.SetMetadata
		(
			pDoc->Title, // Title
			pDoc->Publisher, // Author
			pDoc->Description, // Subject
			csKeywords // Keywords
		);

	}

	// tell the renderer what is going on
	Exporting = true;

	// prepare the progress dialog
	theApp.OnIdle( 0 );
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	// launch the progress dialog
	CThumbnailDialog dlg;
	dlg.Parent = pFrame;
	dlg.CreateDlg();
	dlg.ShowWindow( SW_SHOW );
	CString csDialogTitle(L"Rendering Book Page Images");
	if (bExportPDF)
	{
		csDialogTitle = L"Rendering PDF Pages";
	}
	dlg.SetWindowText(csDialogTitle);
	dlg.Objects = L"Pages";

	// collection of page numbers to be exported
	CKeyedCollection<UINT, UINT> keyPages = pDoc->ExportPageNumbers;
	const UINT nPages = (UINT)keyPages.Count;

	dlg.TotalImages = nPages;

	bool bAbort = false;
	int nPdfPage = 0;

	// collection of one based page numbers
	for (auto& node : keyPages.Items)
	{
		// convert to zero based page number
		UINT nPage = node.first - 1;

		// let the user cancel out
		if (dlg.Cancel)
		{
			bAbort = true;
			break;
		}

		// update the progress dialog's status
		dlg.CurrentImage = nPage;

		// physical pages begin with page one
		pDoc->Page = nPage + 1;

		dTopOfPage = dPageHeight * nPage;
		const double dBottomOfPage = dTopOfPage + dPageHeight;

		// Create high-DPI bitmap
		Gdiplus::Bitmap bitmap(width, height, PixelFormat32bppARGB);
		bitmap.SetResolution(REAL(dpi), REAL(dpi));

		Graphics graphics(&bitmap);
		graphics.Clear(Color::White);

		// Create GDI-compatible CDC from HDC
		HDC hDC = graphics.GetHDC();
		CDC dc;
		dc.Attach(hDC);
		CDC* pDC = &dc;

		int nLogicalWidth = 0, nLogicalHeight = 0;
		SetImageDC(pDC, dpi, width, height, nLogicalWidth, nLogicalHeight);

		// Call your rendering method with inch-based bounds
		render(pDC, dLeftOfView, dTopOfPage, dRightOfView, dBottomOfPage);

		dc.Detach();
		graphics.ReleaseHDC(hDC);

		// Save to JPEG
		CLSID clsidEncoder;
		CImagePlus::GetEncoderClsid(L"image/jpeg", &clsidEncoder);

		if (bExportPDF)
		{
			std::vector<BYTE> jpegBuffer;
			EncodeBitmapToJpegMemory(&bitmap, jpegBuffer, ulQuality);

			pdf.AddPageFromJpegMemory
			(
				jpegBuffer.data(),
				jpegBuffer.size(),
				width,
				height
			);

			CString csPage;
			csPage.Format(L"Page %03d", nPage + 1);

			pdf.AddBookmark(csPage, nPdfPage);

			nPdfPage++;
		}
		else
		{
			CString fileName;
			fileName.Format(L"page_%03d.jpg", nPage + 1);

			CString csJPEG = csFolder + fileName;

			// JPEG quality parameter
			Gdiplus::EncoderParameters params;
			params.Count = 1;
			params.Parameter[0].Guid = Gdiplus::EncoderQuality;
			params.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
			params.Parameter[0].NumberOfValues = 1;
			params.Parameter[0].Value = &ulQuality;

			bitmap.Save(csJPEG, &clsidEncoder, &params);
		}

		// wait ten milliseconds while letting normal 
		// window messaging to run
		pFrame->Wait(10);
	}

	// done with the progress dialog
	dlg.DestroyWindow();

	pDoc->Page = nSavedPage;

	Exporting = false;
	if (bExportPDF)
	{
		pdf.End();
	}

} // ExportDocument

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnDraw(CDC* pDC)
{
	CPhotoPrinterDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CBaseView::OnDraw(pDC);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnInitialUpdate()
{
	CBaseView::OnInitialUpdate();

	// Force classic scrollbar style
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_FRAMECHANGED);

	// calculate the total size of this view
	CPhotoPrinterDoc* pDoc = GetDocument();
	const double dHeight = pDoc->Height;
	const double dWidth = pDoc->WidthOfPage;
	const int nHeight = InchesToLogical(dHeight);
	const int nWidth = InchesToLogical(dWidth);
	
	// update the table of contents in the property panel
	CMainFrame* pMainFrame = static_cast<CMainFrame*>(AfxGetMainWnd());
	CPropertiesWnd* pProps = pMainFrame->PropertiesPane;
	pProps->UpdateTableOfContents();

	CSize sizeTotal(nWidth, nHeight);
	SetScrollSizes(MM_HIENGLISH, sizeTotal);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoPrinterView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CBaseView::OnBeginPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	CBaseView::OnEndPrinting(pDC, pInfo);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	// Convert screen coordinates to client coordinates
	ScreenToClient(&point);

	// Get the dimensions of the scrollbars
	//SCROLLBARINFO sbiHorz = { sizeof(SCROLLBARINFO) };
	SCROLLBARINFO sbiVert = { sizeof(SCROLLBARINFO) };

	//GetScrollBarInfo(OBJID_HSCROLL, &sbiHorz);
	GetScrollBarInfo(OBJID_VSCROLL, &sbiVert);

	//CRect rectHorzScroll = sbiHorz.rcScrollBar;
	CRect rectVertScroll = sbiVert.rcScrollBar;
	ScreenToClient(&rectVertScroll);

	// Check if the click is inside a scrollbar
	if 
	(
		/*rectHorzScroll.PtInRect(point) ||*/ 
		rectVertScroll.PtInRect(point)
	)
	{
		CMenu menu;
		menu.LoadMenu(IDR_SCROLLBAR_MENU);
		CMenu* pSubMenu = menu.GetSubMenu(0);
		if (pSubMenu)
		{
			ClientToScreen(&point);
			pSubMenu->TrackPopupMenu
			(
				TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
				point.x, point.y, this
			);
		}
	}
	else
	{
		ClientToScreen(&point);
		theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
	}
}

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::AssertValid() const
{
	CBaseView::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::Dump(CDumpContext& dc) const
{
	CBaseView::Dump(dc);
}

/////////////////////////////////////////////////////////////////////////////
CPhotoPrinterDoc* CPhotoPrinterView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPhotoPrinterDoc)));
	return (CPhotoPrinterDoc*)m_pDocument;
}

/////////////////////////////////////////////////////////////////////////////
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// the scrollbar context menu handler for ID_SCROLL_TOP
void CPhotoPrinterView::OnScrollTop()
{
	CScrollBar* pVert = GetScrollBarCtrl(SB_VERT);

	CBaseView::HeightScroll(SB_TOP, 0, pVert);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnScrollBottom()
{
	CScrollBar* pVert = GetScrollBarCtrl(SB_VERT);

	CBaseView::HeightScroll( SB_BOTTOM, 0, pVert );
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnScrollPageup()
{
	CScrollBar* pVert = GetScrollBarCtrl(SB_VERT);

	CBaseView::HeightScroll(SB_PAGEUP, 0, pVert);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnScrollPagedown()
{
	CScrollBar* pVert = GetScrollBarCtrl(SB_VERT);

	CBaseView::HeightScroll(SB_PAGEDOWN, 0, pVert);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnScrollLineup()
{
	CScrollBar* pVert = GetScrollBarCtrl(SB_VERT);

	CBaseView::HeightScroll(SB_LINEUP, 0, pVert);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnScrollLinedown()
{
	CScrollBar* pVert = GetScrollBarCtrl(SB_VERT);

	CBaseView::HeightScroll(SB_LINEDOWN, 0, pVert);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnFileExportPages()
{
	PDF = false;
	ExportDocument();

} // OnFileExportPages

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnUpdateFileExportPages(CCmdUI* pCmdUI)
{
	CPhotoPrinterDoc* pDoc = GetDocument();
	CString csStart = pDoc->CorrectForWorkingFolder(pDoc->StartFolder);
	CString csEnd = pDoc->CorrectForWorkingFolder(pDoc->EndFolder);
	CString csExport = pDoc->CorrectForWorkingFolder(pDoc->ExportFolder);

	pCmdUI->Enable(FALSE);

	if (!::PathFileExists(csStart))
	{
		return;
	}
	if (!::PathFileExists(csEnd))
	{
		return;
	}
	if (!::PathFileExists(csExport))
	{
		return;
	}

	pCmdUI->Enable();
} // OnUpdateFileExportPages

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnFilePdf()
{
	PDF = true;
	ExportDocument();

} // OnFilePdf

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterView::OnUpdateFilePdf(CCmdUI* pCmdUI)
{
	OnUpdateFileExportPages(pCmdUI);

} // OnUpdateFilePdf

/////////////////////////////////////////////////////////////////////////////
