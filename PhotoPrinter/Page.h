/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"

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
