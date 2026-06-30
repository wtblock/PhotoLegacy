/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"

/////////////////////////////////////////////////////////////////////////////
// CPage
//
// Represents a single page in a PhotoPrinter document. Each page contains
// a boundary rectangle (which varies for even/odd pages), the folder name
// associated with the images on that page, and a collection of image
// rectangles describing where each photograph will be placed.
//
// Purpose:
//   • Provide a lightweight container for all layout information needed to
//     render a single page of the photo book.
//   • Store the page number, page geometry, folder name, and the set of
//     image rectangles assigned to that page.
//   • Allow PhotoPrinterView and the PDF export system to render pages
//     consistently and device‑independently.
//
// Why this class exists:
//   PhotoPrinter builds pages dynamically based on the number of images in
//   each album folder. The layout rules (1‑image, 2‑image, 3‑image, 4‑image,
//   or 6‑image templates) are computed in CPhotoPrinterDoc, but the results
//   must be stored in a simple, reusable structure. CPage provides that
//   structure and isolates page geometry from rendering logic.
//
// Responsibilities:
//   • Store the page number (even pages on the left, odd pages on the right).
//   • Store the page boundary rectangle (in logical units).
//   • Store the folder name associated with the images on the page.
//   • Store a keyed collection of image rectangles (image name → CRect).
//   • Provide AddAnImage() to attach an image and its layout rectangle.
//   • Provide RenderImageRectangles() for debugging or preview rendering.
//
// Layout Rules (computed by CPhotoPrinterDoc):
//   • 1 image  → 1 portrait rectangle (7.3" × 10.0")
//   • 2 images → 2 landscape rectangles (7.3" × 5.0")
//   • 3 images → 2 portrait + 1 landscape
//   • 4 images → 4 portrait rectangles
//   • >4 images → 6 rectangles (3.65" × 3.33")
//   • Images may be rotated to match their containing rectangle shape.
//   • Rectangles are positioned relative to page margins and gutter.
//
// Interaction with other components:
//   • CPhotoPrinterDoc creates and populates CPage objects during
//     GeneratePages().
//   • CPhotoPrinterView uses CPage to draw images, margins, and page
//     geometry on screen.
//   • PDF export uses CPage rectangles to place images at high DPI.
//   • The Properties pane uses page numbers to navigate the document.
//
// This class is the fundamental building block of PhotoPrinter’s page
// layout system, encapsulating all geometry and placement information
// needed to render a complete page.
/////////////////////////////////////////////////////////////////////////////
class CPage
{
// protected data
protected:
	// the page number
	UINT m_nPage;

	// page boundary rectangle varies depending on the page
	// number being an even or odd number. Even pages are
	// on the left and odd pages are on the right of the
	// printed document.
	CRect m_Rect;

	// folder name in the format of "path\YYYY.MM.DD Title of Event"
	// where the year, month and day allow the albums to be 
	// sorted by the date of the event.
	CString m_csFolder;

	// A collection of image rectangles for this page where
	// the application fits as many images on the page as
	// possible. The containing rectangles for the images are
	// described as portrait or landscape where portrait is
	// defined as narrower width than height and landscape
	// is defined as wider width that height (same as the
	// images themselves are defined). 
	// 
	// If the shape of the image is not the same as its 
	// containing rectangle, it is rotated such that its 
	// bottom is nearest to the outer margin.
	// 
	// The number of rectangles depends on the number of images
	// in the current folder that are remaining to be printed:
	// 1 - single portrait rectangle (7.3" x 10.0")
	// 2 - two landscape rectangles (7.3" x 5.0")
	// 3 - two portrait rectangles and a single landscape
	//		(3.65" x 5.0") and (7.3" x 5.0")
	// 4 - four portrait rectangles (3.65" x 5.0")
	// More than 4 images will create pages with six rectangles (3.65" x 3.33")
	//
	// the key is the image name associated with the given rectangle the
	// image will be drawn in
	CKeyedCollection<CString, CRect> m_arrImages;

// public properties
public:
	// current page
	UINT GetPage()
	{
		return m_nPage;
	}
	// current page
	void SetPage(UINT value)
	{
		m_nPage = value;
	}
	// current page
	__declspec( property( get = GetPage, put = SetPage ) )
		UINT Page;

	// number of images on the page
	UINT GetImageCount()
	{
		return (UINT)m_arrImages.Count;
	}
	// number of images on the page
	__declspec( property( get = GetImageCount) )
		UINT ImageCount;

	// page boundary rectangle varies depending on the page
	// number being an even or odd number. Even pages are
	// on the left and odd pages are on the right of the
	// printed document.
	CRect GetRect()
	{
		return m_Rect;
	}
	// page boundary rectangle varies depending on the page
	// number being an even or odd number. Even pages are
	// on the left and odd pages are on the right of the
	// printed document.
	void SetRect(CRect value)
	{
		m_Rect = value;
	}
	// page boundary rectangle varies depending on the page
	// number being an even or odd number. Even pages are
	// on the left and odd pages are on the right of the
	// printed document.
	__declspec( property( get = GetRect, put = SetRect ) )
		CRect Rect;

	// folder name in the format of "YYYY.MM.DD Title of Event"
	// where the year, month and day allow the albums to be 
	// sorted by the date of the event.
	CString GetFolder()
	{
		return m_csFolder;
	}
	// folder name in the format of "YYYY.MM.DD Title of Event"
	// where the year, month and day allow the albums to be 
	// sorted by the date of the event.
	void SetFolder(CString value)
	{
		m_csFolder = value;
	}
	// folder name in the format of "YYYY.MM.DD Title of Event"
	// where the year, month and day allow the albums to be 
	// sorted by the date of the event.
	__declspec(property(get = GetFolder, put = SetFolder))
		CString Folder;

	// image names and rectangles
	CKeyedCollection<CString, CRect>& GetImages()
	{
		return m_arrImages;
	}
	// image names and rectangles
	__declspec(property(get = GetImages))
		CKeyedCollection<CString, CRect>& Images;

// protected methods
protected:

// public methods
public:
	// add an image to the page
	void AddAnImage(CString csImage, CRect rect )
	{
		shared_ptr<CRect> pRect = shared_ptr<CRect>(new CRect( &rect ));
		m_arrImages.add(csImage, pRect);
	}

	// render the image rectangles
	void RenderImageRectangles(CDC* pDC)
	{
		for (auto& image : m_arrImages.Items)
		{
			shared_ptr<CRect> pRect = image.second;
			pDC->Rectangle(pRect.get());
		}
	}

// protected overrides
protected:

// public overrides
public:

// public constructor/destructor
public:
	CPage()
	{
		Page = 0;
		m_Rect.SetRectEmpty();
	}
	CPage( UINT nPage, CString csFolder, CRect rect )
	{
		Page = nPage;
		Folder = csFolder;
		Rect = rect;
	}
	~CPage()
	{

	}
};

/////////////////////////////////////////////////////////////////////////////
