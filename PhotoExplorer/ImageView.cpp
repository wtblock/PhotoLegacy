/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "PhotoExplorer.h"
#include "CHelper.h"
#include "MainFrm.h"
#include "ShellListView.h"
#include "PhotoExplorerDoc.h"
#include "ImageView.h"
#include <gdiplus.h>

using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CImageView, CView)

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CImageView, CView)
	ON_COMMAND( ID_FILE_PRINT, &CView::OnFilePrint )
	ON_COMMAND( ID_FILE_PRINT_DIRECT, &CView::OnFilePrint )
	ON_COMMAND( ID_FILE_PRINT_PREVIEW, &CImageView::OnFilePrintPreview )
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CImageView::CImageView()
{
	CWinApp* pApp = AfxGetApp();
	CString csSec( L"Settings" );
	Label = 0 != pApp->GetProfileIntW( csSec, L"Label", 0 );
	Export = false;

} // CImageView

/////////////////////////////////////////////////////////////////////////////
CImageView::~CImageView()
{
}

/////////////////////////////////////////////////////////////////////////////
bool CImageView::GetAlbumTitle()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumTitle;
}

/////////////////////////////////////////////////////////////////////////////
bool CImageView::GetAlbumLocation()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumLocation;
}

/////////////////////////////////////////////////////////////////////////////
bool CImageView::GetAlbumComment()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumComment;
}

/////////////////////////////////////////////////////////////////////////////
bool CImageView::GetAlbumDate()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumDate;
}

/////////////////////////////////////////////////////////////////////////////
bool CImageView::GetAlbumArtist()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumArtist;
}

/////////////////////////////////////////////////////////////////////////////
bool CImageView::GetAlbumCopyright()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumCopyright;
}

/////////////////////////////////////////////////////////////////////////////
bool CImageView::GetAlbumSoftware()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumSoftware;
}

/////////////////////////////////////////////////////////////////////////////
bool CImageView::GetAlbumKeywords()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	return pDoc->AlbumKeywords;
}

/////////////////////////////////////////////////////////////////////////////
// load an image from the selected image and apply the orientation property
unique_ptr<Image> CImageView::LoadAndAdjustImage()
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	const CString csImagePath = pDoc->ImagePath;
	const WCHAR* pFile = (LPCTSTR)csImagePath;
	unique_ptr<Image> value = make_unique<Image>( pFile );
	
	// Check if the image is valid 
	if ( value->GetLastStatus() != Ok ) 
	{
		return nullptr; 
	}

	CExifRotation* pRotator = pDoc->Rotator;
	pRotator->ApplyOrientation( value.get() );

	return value;
} // LoadAndAdjustImage

/////////////////////////////////////////////////////////////////////////////
unique_ptr<Image> CImageView::AddTextToImage()
{
	CPhotoExplorerDoc* pDoc = GetDocument();

	// load the image and adjust its orientation
	unique_ptr<Image> pImage = LoadAndAdjustImage();
	if ( pImage == nullptr )
	{
		return pImage;
	}

	UINT uiWidth = pImage->GetWidth();
	UINT uiHeight = pImage->GetHeight();
	
	float fontSizeRatio = 0.02f; 
	UINT uiFont = static_cast<UINT>( uiWidth * fontSizeRatio );
	if ( uiHeight > uiWidth )
	{
		uiFont = static_cast<UINT>( uiHeight * fontSizeRatio );
	}


#ifdef _DEBUG
#undef new
#endif
	// this code does not compile with DEBUG_NEW enabled
	unique_ptr<Bitmap> dummyBitmap =
		unique_ptr<Bitmap>( new Gdiplus::Bitmap( 1, 1 ) );

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

	// Create a GDI+ Graphics object for text measurement
	unique_ptr<Graphics> graphics = 
		unique_ptr<Graphics>( Graphics::FromImage( dummyBitmap.get() ));
	Gdiplus::Font font( L"Arial Unicode MS", REAL( uiFont ));
	RectF textRect( 0, 0, (REAL)uiWidth, 0 );
	StringFormat format;
	format.SetAlignment( Gdiplus::StringAlignmentNear );

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CShellListView* pListView = (CShellListView*)pFrame->ListView;
	CImageProperties* pImageProps = pListView->ImageProperties;
	CKeyedCollection<CString, CImageProperty>* pProperties = 
		pImageProps->Properties;

	const CString csTitleKey = L"Label|1. Title";
	const CString csLocationKey = L"Label|2. Location";
	const CString csCommentKey = L"Label|3. Comment";
	const CString csDateKey = L"Label|4. Date taken";

	const CString csArtistKey = L"User|2. Artist";
	const CString csCopyrightKey = L"User|3. Copyright";
	const CString csSoftwareKey = L"User|4. Software";
	const CString csKeywordsKey = L"User|5. Keywords";

	// Calculate the height required for the text
	RectF boundingBox( 0, 0, (REAL)uiWidth, 0 );

	FontFamily fontFamily( L"Arial Unicode MS" );
	CString csTitle, csComment, csLocation, csDateTaken, csDateLabel;

	// if the shift key is down, then overwrite existing values with
	// album values and not just empty values
	const bool bShift = CHelper::ShiftKeyDown();

	// get the space required by the image title taking
	// into account the substitution of the album title
	bool bTitle = false;
	RectF rectTitle;
	if ( pProperties->Exists[ csTitleKey ] )
	{
		shared_ptr<CImageProperty> pProp = 
			pProperties->find( csTitleKey );
		csTitle =
			pDoc->ResolveShortcuts( pProp->UserInterfaceValue );
		if ( AlbumTitle && ( bShift || csTitle.IsEmpty()))
		{
			csTitle = pDoc->Title;
		}
		bTitle = csTitle.GetLength() > 0;
		if ( bTitle )
		{
			graphics->MeasureString
			(
				csTitle, -1, &font, textRect, &format, &rectTitle
			);
			boundingBox.Height += rectTitle.Height;
		}
	}

	// get the space required by the image comment taking
	// into account the substitution of the album comment
	bool bComment = false;
	RectF rectComment;
	if ( pProperties->Exists[ csCommentKey ] )
	{
		shared_ptr<CImageProperty> pProp = 
			pProperties->find( csCommentKey );
		csComment = 
			pDoc->ResolveShortcuts( pProp->UserInterfaceValue );
		if ( AlbumComment && ( bShift || csComment.IsEmpty() ) )
		{
			csComment = pDoc->Comment;
		}
		bComment = csComment.GetLength() > 0;
		if ( bComment )
		{
			graphics->MeasureString
			(
				csComment, -1, &font, textRect, &format, &rectComment
			);
			boundingBox.Height += rectComment.Height;
		}
	}

	// get the space required by the image location taking
	// into account the substitution of the album location
	bool bLocation = false;
	RectF rectLocation;
	if ( pProperties->Exists[ csLocationKey ] )
	{
		shared_ptr<CImageProperty> pProp = 
			pProperties->find( csLocationKey );
		csLocation =
			pDoc->ResolveShortcuts( pProp->UserInterfaceValue );
		if ( AlbumLocation && ( bShift || csLocation.IsEmpty() ) )
		{
			csLocation = pDoc->Location;
		}
		bLocation = csLocation.GetLength() > 0;
		if ( bLocation )
		{
			graphics->MeasureString
			(
				csLocation, -1, &font, textRect, &format, &rectLocation
			);
		}
	}

	// get the space required by the image date taking
	// into account the substitution of the album date
	bool bDate = false;
	RectF rectDate;
	if ( pProperties->Exists[ csDateKey ] )
	{
		shared_ptr<CImageProperty> pProp = 
			pProperties->find( csDateKey );
		csDateTaken =
			pDoc->ResolveShortcuts( pProp->UserInterfaceValue );
		if ( AlbumDate && ( bShift || csDateTaken.IsEmpty() ) )
		{
			csDateTaken = pDoc->Date;
		}
		CString csResolve = pDoc->ResolveShortcuts( csDateTaken );
		csDateLabel = LabelDate[ csResolve ];
		bDate = csDateLabel.GetLength() > 0;
		if ( bDate )
		{
			graphics->MeasureString
			(
				csDateLabel, -1, &font, textRect, &format, &rectDate
			);
		}
	}

	// continue if any of the above were found
	bool bOkay = bTitle || bComment || bLocation || bDate;

	if ( !bOkay )
	{
		return pImage;
	}

	// the location and date are drawn on the same line
	// if there is enough room
	if ( rectLocation.Width + rectDate.Width > uiWidth )
	{
		boundingBox.Height += ( rectLocation.Height + rectDate.Height );

	} else 
	{
		if ( rectLocation.Height > rectDate.Height )
		{
			boundingBox.Height += rectLocation.Height;

		} else
		{
			boundingBox.Height += rectDate.Height;
		}
	}

	// border width in pixels
	int nBorder = 20;

	// Add some padding
	int additionalHeight = static_cast<int>( boundingBox.Height ); 

#ifdef _DEBUG
#undef new
#endif

	// this code does not compile with DEBUG_NEW enabled
	unique_ptr<Image> pNewImage =
		unique_ptr<Image>
		( 
			new Bitmap
			( 
				uiWidth + 2 * nBorder, 
				uiHeight + additionalHeight + 2 * nBorder
			) 
		);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

	// Create a new image with extended height
	unique_ptr<Graphics> newGraphics = 
		unique_ptr<Graphics>( Graphics::FromImage( pNewImage.get() ));

	// Set the background color for the new area
	SolidBrush backgroundBrush( Gdiplus::Color( 255, 255, 255 ) ); 
	
	// White background rectangle 
	RectF backgroundRect
	( 
		0, 0, (REAL)pNewImage->GetWidth(), (REAL)pNewImage->GetHeight() 
	);
	newGraphics->FillRectangle( &backgroundBrush, backgroundRect );

	// Draw the original image onto the new image
	newGraphics->DrawImage( pImage.get(), nBorder, nBorder, uiWidth, uiHeight );

	// Draw the text in the extended area
	SolidBrush brush( Color( 0, 0, 255 ) ); // blue text

	// if the title is enabled, draw it in the title rectangle
	if ( bTitle )
	{
		RectF newTextRect
		( 
			(REAL)nBorder, (REAL)uiHeight + 2 * nBorder, 
			(REAL)uiWidth, (REAL)rectTitle.Height 
		);
		newGraphics->DrawString( csTitle, -1, &font, newTextRect, &format, &brush );
	}

	// if the comment is enabled, draw it in the comment rectangle
	if ( bComment )
	{
		RectF newTextRect
		( 
			(REAL)nBorder, (REAL)uiHeight + 2 * nBorder + rectTitle.Height, 
			(REAL)uiWidth, (REAL)rectComment.Height 
		);
		newGraphics->DrawString( csComment, -1, &font, newTextRect, &format, &brush );
	}

	if ( bLocation )
	{
		RectF newTextRect
		( 
			(REAL)nBorder, 
			(REAL)uiHeight + 2 * nBorder + rectComment.Height + rectTitle.Height, 
			(REAL)uiWidth, rectLocation.Height 
		);
		newGraphics->DrawString( csLocation, -1, &font, newTextRect, &format, &brush );
	}

	if ( bDate )
	{
		RectF newTextRect
		( 
			(REAL)uiWidth - rectDate.Width, 
			(REAL)uiHeight + 2 * nBorder + rectComment.Height + rectTitle.Height, 
			(REAL)uiWidth, rectDate.Height 
		);
		if ( rectLocation.Width + rectDate.Width > uiWidth )
		{
			newTextRect.Y += rectLocation.Height;
		}
		Gdiplus::StringFormat rightAlignFormat;
		rightAlignFormat.SetAlignment( Gdiplus::StringAlignmentFar ); 
		newGraphics->DrawString( csDateLabel, -1, &font, newTextRect, &format, &brush );
	}

	const bool bExport = Export;
	if ( bShift || bExport )
	{
		// Preserve all metadata
		const UINT uiPropertyCount = pImage->GetPropertyCount();
		unique_ptr<PROPID> propIDs = unique_ptr<PROPID>( new PROPID[ uiPropertyCount ] );
		pImage->GetPropertyIdList( uiPropertyCount, propIDs.get() );

		bool bPropTitle = false;
		bool bPropLocation = false;
		bool bPropComment = false;
		bool bPropDate = false;
		bool bPropArtist = false;
		bool bPropCopyright = false;
		bool bPropSoftware = false;
		bool bPropKeywords = false;

		// loop through the metadata properties of the original image and 
		// copy them to the new image
		for ( UINT i = 0; i < uiPropertyCount; ++i )
		{
			UINT size = pImage->GetPropertyItemSize( propIDs.get()[ i ] );
			Gdiplus::PropertyItem* pItem = (Gdiplus::PropertyItem*)malloc( size );
			pImage->GetPropertyItem( propIDs.get()[ i ], size, pItem );

			// test for orientation item and if it exists, apply the rotation
			// to the new image
			const ULONG ulId = pItem->id;
			switch ( ulId )
			{
				case PropertyTagOrientation:
				{
					// the image rotation has already been applied, so just
					// set the rotation to none
					USHORT* pValue = reinterpret_cast<USHORT*>( pItem->value );
					pValue[ 0 ] = 1;
					pNewImage->SetPropertyItem( pItem );
					break;
				}
				case PropertyTagImageDescription:
				{
					pNewImage->SetPropertyItem( pItem );
					ULONG ulLength = pItem->length;
					// the title property is ASCII
					CStringA csItem;
					void* pBuffer = csItem.GetBufferSetLength( ulLength );
					::CopyMemory( pBuffer, pItem->value, ulLength );
					csItem.ReleaseBuffer();

					bPropTitle = !csItem.IsEmpty();

					// if the shift key is down and album titles are
					// enabled allow the property to be overwridden
					if ( bShift && AlbumTitle )
					{
						bPropTitle = false;
					}
					break;
				}
				case PropertyTagXPSubject:
				{
					pNewImage->SetPropertyItem( pItem );
					ULONG ulLength = pItem->length;
					// the location property is UNICODE
					CString csItem;
					void* pBuffer = csItem.GetBufferSetLength( ulLength );
					::CopyMemory( pBuffer, pItem->value, ulLength );
					csItem.ReleaseBuffer();

					bPropLocation = !csItem.IsEmpty();

					// if the shift key is down and album locations are
					// enabled allow the property to be overwridden
					if ( bShift && AlbumLocation )
					{
						bPropLocation = false;
					}
					break;
				}
				case PropertyTagXPComment:
				{
					pNewImage->SetPropertyItem( pItem );
					ULONG ulLength = pItem->length;
					// the comment property is UNICODE
					CString csItem;
					void* pBuffer = csItem.GetBufferSetLength( ulLength );
					::CopyMemory( pBuffer, pItem->value, ulLength );
					csItem.ReleaseBuffer();

					bPropComment = !csItem.IsEmpty();

					// if the shift key is down and album comments are
					// enabled allow the property to be overwridden
					if ( bShift && AlbumComment )
					{
						bPropComment = false;
					}
					break;
				}
				case PropertyTagExifDTOrig:
				{
					pNewImage->SetPropertyItem( pItem );
					ULONG ulLength = pItem->length;
					// the date property is ASCII
					CString csItem;
					void* pBuffer = csItem.GetBufferSetLength( ulLength );
					::CopyMemory( pBuffer, pItem->value, ulLength );
					csItem.ReleaseBuffer();

					bPropDate = !csItem.IsEmpty();

					// if the shift key is down and album dates are
					// enabled allow the property to be overwridden
					if ( bShift && AlbumDate )
					{
						bPropDate = false;
					}
					break;
				}
				case PropertyTagArtist:
				{
					pNewImage->SetPropertyItem( pItem );
					ULONG ulLength = pItem->length;
					// the artist property is ASCII
					CString csItem;
					void* pBuffer = csItem.GetBufferSetLength( ulLength );
					::CopyMemory( pBuffer, pItem->value, ulLength );
					csItem.ReleaseBuffer();

					bPropArtist = !csItem.IsEmpty();

					// if the shift key is down and album artists are
					// enabled allow the property to be overwridden
					if ( bShift && AlbumArtist )
					{
						bPropArtist = false;
					}
					break;
				}
				case PropertyTagCopyright:
				{
					pNewImage->SetPropertyItem( pItem );
					ULONG ulLength = pItem->length;
					// the copyright property is ASCII
					CString csItem;
					void* pBuffer = csItem.GetBufferSetLength( ulLength );
					::CopyMemory( pBuffer, pItem->value, ulLength );
					csItem.ReleaseBuffer();

					bPropCopyright = !csItem.IsEmpty();

					// if the shift key is down and album copyrights are
					// enabled allow the property to be overwridden
					if ( bShift && AlbumCopyright )
					{
						bPropCopyright = false;
					}
					break;
				}
				case PropertyTagSoftwareUsed:
				{
					pNewImage->SetPropertyItem( pItem );
					ULONG ulLength = pItem->length;
					// the software property is ASCII
					CString csItem;
					void* pBuffer = csItem.GetBufferSetLength( ulLength );
					::CopyMemory( pBuffer, pItem->value, ulLength );
					csItem.ReleaseBuffer();

					bPropSoftware = !csItem.IsEmpty();

					// if the shift key is down and album software is
					// enabled allow the property to be overwridden
					if ( bShift && AlbumSoftware )
					{
						bPropSoftware = false;
					}
					break;
				}
				case PropertyTagXPKeywords:
				{
					pNewImage->SetPropertyItem( pItem );
					ULONG ulLength = pItem->length;
					// the keywords property is UNICODE
					CString csItem;
					void* pBuffer = csItem.GetBufferSetLength( ulLength );
					::CopyMemory( pBuffer, pItem->value, ulLength );
					csItem.ReleaseBuffer();

					bPropKeywords = !csItem.IsEmpty();

					// if the shift key is down and album keywords are
					// enabled allow the property to be overwridden
					if ( bShift && AlbumKeywords )
					{
						bPropKeywords = false;
					}
					break;
				}
				default:
				{
					pNewImage->SetPropertyItem( pItem );
				}
			}

			free( pItem );
		}

		// add the album title to the exported image if missing from original
		pDoc->WriteAlbumToImage( csTitleKey, csTitle, bPropTitle, pNewImage );

		// add the album location to the exported image if missing from original
		pDoc->WriteAlbumToImage( csLocationKey, csLocation, bPropLocation, pNewImage );

		// add the album comment to the exported image if missing from original
		pDoc->WriteAlbumToImage( csCommentKey, csComment, bPropComment, pNewImage );

		// add the album date taken to the exported image if missing from original
		pDoc->WriteAlbumToImage( csDateKey, csDateTaken, bPropDate, pNewImage );

		CString csArtist = pDoc->Artist;
		CString csCopyright = pDoc->Copyright;
		CString csSoftware = pDoc->Software;
		CString csKeywords = pDoc->Keywords;

		// add the album artist to the exported image if missing from original
		pDoc->WriteAlbumToImage( csArtistKey, csArtist, bPropArtist, pNewImage );

		// add the album copyright to the exported image if missing from original
		pDoc->WriteAlbumToImage( csCopyrightKey, csCopyright, bPropCopyright, pNewImage );

		// add the album software to the exported image if missing from original
		pDoc->WriteAlbumToImage( csSoftwareKey, csSoftware, bPropSoftware, pNewImage );

		// add the album keywords to the exported image if missing from original
		pDoc->WriteAlbumToImage( csKeywordsKey, csKeywords, bPropKeywords, pNewImage );
	}

	return pNewImage;

} // AddTextToImage

/////////////////////////////////////////////////////////////////////////////
BOOL CImageView::OnEraseBkgnd( CDC* pDC )
{
	// Set the background color to black
	CRect rect;
	GetClientRect( &rect );

	Gdiplus::Graphics graphics( pDC->GetSafeHdc() );

	// Solid black color
	Gdiplus::SolidBrush brush( Gdiplus::Color( 255, 0, 0, 0 ) );
	graphics.FillRectangle
	(
		&brush, rect.left, rect.top, rect.Width(), rect.Height()
	);

	// Return TRUE to indicate background is erased
	return TRUE;
} // OnEraseBkgnd

/////////////////////////////////////////////////////////////////////////////
void CImageView::OnDraw( CDC* pDC )
{
	CPhotoExplorerDoc* pDoc = Document;
	const int nSelectedCount = pDoc->SelectedCount;
	CString csMessage;
	csMessage.Format( L"The number of selected images: %d", nSelectedCount );

	Graphics graphics( pDC->GetSafeHdc() ); 

	if ( nSelectedCount != 1 )
	{
		Gdiplus::Font font( L"Arial", 24 );
		Gdiplus::RectF layoutRect;
		graphics.GetVisibleClipBounds( &layoutRect );
		Gdiplus::StringFormat format;
		format.SetAlignment( Gdiplus::StringAlignmentCenter );
		format.SetLineAlignment( Gdiplus::StringAlignmentCenter );
		Gdiplus::SolidBrush brush( Gdiplus::Color( 255, 255, 255, 255 ) ); // Black color

		graphics.DrawString( csMessage, -1, &font, layoutRect, &format, &brush );
		return;
	}

	CRect rect;
	GetWindowRect( &rect );
	
	//// initialize the screen to white
	//graphics.Clear( Gdiplus::Color( 255, 255, 255 ) );

	// are we labeling the image?
	const bool bLabel = Label;

	// load the image and adjust its orientation
	unique_ptr<Image> pImage;
	if ( bLabel )
	{
		// add label information to the image preview
		pImage = AddTextToImage();

	} else
	{
		// load and rotate the image based on orientation property
		pImage = LoadAndAdjustImage();
	}

	// if we opened the image
	if ( pImage )
	{
		Graphics graphics( pDC->GetSafeHdc() );

		// Calculate the aspect ratio of the image
		const UINT uiImageWidth = pImage->GetWidth();
		const UINT uiImageHeight = pImage->GetHeight();
		CSize sizeImage( uiImageWidth, uiImageHeight );
		CRect rectImage( CPoint( 0, 0 ), sizeImage );

		// get the drawing rectangle that accounts for rotation
		// and aspect ratio
		CRect rectDraw = 
			CHelper::GetDrawingRectangle( rect, rectImage );

		// draw the image with the new dimensions
		graphics.DrawImage
		( 
			pImage.get(), rectDraw.left, rectDraw.top, 
			rectDraw.Width(), rectDraw.Height() 
		);

		const bool bExport = Export;
		if ( bExport )
		{
			Export = false;

			CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
			CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
			CShellListView* pListView = (CShellListView*)pFrame->ListView;
			CImageProperties* pImageProperties = pListView->ImageProperties;

			CString csPath = pDoc->ImagePath;
			if ( !::PathFileExists( csPath ) )
			{
				return;
			}

			CString csExt = CHelper::GetExtension( csPath );
			CString csData = CHelper::GetDataName( csPath );
			pImageProperties->Extension->FileExtension = csExt;
			CLSID classID = pImageProperties->Extension->ClassID;

			const CString csFolder = CHelper::GetFolder( csPath ) + L"Labeled";
			if ( !::PathFileExists( csFolder ) )
			{
				if ( !CreatePath( csFolder ) )
				{
					return;
				}
			}

			// create a new path from the pieces
			CString csLabelPath;
			csLabelPath.Format( L"%s\\%s", (LPCTSTR)csFolder, (LPCTSTR)csData );

			// save the image to the corrected folder
			Gdiplus::Status status = pImage->Save( csLabelPath, &classID );
			if ( status != Gdiplus::Ok )
			{
				pDoc->ErrorsText = L"Error creating labeled image:";
				pDoc->ErrorsText = csLabelPath;
			}
		}
	}

} // OnDraw

/////////////////////////////////////////////////////////////////////////////
BOOL CImageView::OnPreparePrinting( CPrintInfo* pInfo )
{
	// Default preparation
	BOOL value = DoPreparePrinting( pInfo );
	return value;
} // OnPreparePrinting

/////////////////////////////////////////////////////////////////////////////
void CImageView::OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo )
{
	// let the print dialog know
	pInfo->SetMinPage( 1 );
	pInfo->SetMaxPage( 1 );

	CPhotoExplorerDoc* pDoc = GetDocument();
	pDoc->StopMonitorThread();
} // OnBeginPrinting

/////////////////////////////////////////////////////////////////////////////
// A dedicated function to handle the printing logic
void CImageView::PrintImage( CDC* pDC, CPrintInfo* pInfo )
{
	// are we labeling the image?
	const bool bLabel = Label;

	// load the image and adjust its orientation
	unique_ptr<Image> pImage;
	if ( bLabel )
	{
		// add label information to the image preview
		pImage = AddTextToImage();

	} else
	{
		// load and rotate the image based on orientation property
		pImage = LoadAndAdjustImage();
	}

	// The printing logic we developed earlier
	HBITMAP hBitmap = CHelper::CreateHBITMAPFromGdiplusImage( pImage.get() );
	if ( hBitmap )
	{
		CDC memDC;
		memDC.CreateCompatibleDC( pDC );

		CBitmap bitmap;
		bitmap.Attach( hBitmap );
		CBitmap* pOldBitmap = memDC.SelectObject( &bitmap );

		BITMAP bm;
		bitmap.GetBitmap( &bm );

		int nPrintableWidth = pDC->GetDeviceCaps( HORZRES );
		int nPrintableHeight = pDC->GetDeviceCaps( VERTRES );

		double imageAspectRatio = static_cast<double>( bm.bmWidth ) / bm.bmHeight;
		double pageAspectRatio = static_cast<double>( nPrintableWidth ) / nPrintableHeight;

		int nPrintWidth, nPrintHeight;

		if ( imageAspectRatio > pageAspectRatio )
		{
			nPrintWidth = nPrintableWidth;
			nPrintHeight = static_cast<int>( nPrintableWidth / imageAspectRatio );
		
		} else
		{
			nPrintHeight = nPrintableHeight;
			nPrintWidth = static_cast<int>( nPrintableHeight * imageAspectRatio );
		}

		int nPhysicalOffsetX = pDC->GetDeviceCaps( PHYSICALOFFSETX );
		int nPhysicalOffsetY = pDC->GetDeviceCaps( PHYSICALOFFSETY );

		int nOffsetX = nPhysicalOffsetX + ( nPrintableWidth - nPrintWidth ) / 2;
		int nOffsetY = nPhysicalOffsetY + ( nPrintableHeight - nPrintHeight ) / 2;

		pDC->StretchBlt( nOffsetX, nOffsetY, nPrintWidth, nPrintHeight, &memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY );

		memDC.SelectObject( pOldBitmap );
		bitmap.Detach();
		::DeleteObject( hBitmap );
	}
	
} // PrintImage

/////////////////////////////////////////////////////////////////////////////
void CImageView::OnPrint( CDC* pDC, CPrintInfo* pInfo )
{
	PrintImage( pDC, pInfo );

} // OnPrint

/////////////////////////////////////////////////////////////////////////////
void CImageView::OnFilePrintPreview()
{
	AFXPrintPreview( this );

} // OnFilePrintPreview

/////////////////////////////////////////////////////////////////////////////
void CImageView::OnEndPrinting( CDC* pDC, CPrintInfo* pInfo )
{
	CPhotoExplorerDoc* pDoc = GetDocument();
	pDoc->StartMonitorThread();
}

/////////////////////////////////////////////////////////////////////////////
void CImageView::ExportImageWithLabels()
{
	Export = true;
	Invalidate();

} // ExportImageWithLabels

/////////////////////////////////////////////////////////////////////////////
BOOL CImageView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
void CImageView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

}

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG

/////////////////////////////////////////////////////////////////////////////
void CImageView::AssertValid() const
{
	CView::AssertValid();
}

/////////////////////////////////////////////////////////////////////////////
void CImageView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

/////////////////////////////////////////////////////////////////////////////
CPhotoExplorerDoc* CImageView::GetDocument() // non-debug version is inline
{
	ASSERT( m_pDocument->IsKindOf( RUNTIME_CLASS( CPhotoExplorerDoc ) ) );
	return (CPhotoExplorerDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

