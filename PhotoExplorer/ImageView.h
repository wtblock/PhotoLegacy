/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <memory>
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
class CPhotoExplorerDoc;

/////////////////////////////////////////////////////////////////////////////
// CImageView
//
// The primary image-display view for Photo Explorer. This class is responsible
// for rendering the currently selected image, applying album and metadata
// labels, handling printing and print preview, and exporting labeled images.
// It integrates tightly with the document (CPhotoExplorerDoc) and the metadata
// system to present a fully annotated preview of each photograph.
//
// Purpose:
//   • Display the selected image in the main view window.
//   • Apply metadata labels (title, location, date, artist, copyright,
//     keywords, etc.) when enabled.
//   • Support exporting images with labels applied.
//   • Provide printing and print-preview functionality.
//   • Load and adjust images for EXIF rotation.
//   • Provide album-level fallback values when image metadata is blank.
//
// Why this class exists:
//   Photo Explorer is built around visual inspection of photographs. The view
//   must show the image exactly as the user expects — rotated correctly,
//   annotated with metadata, and ready for printing or export. CImageView
//   centralizes all rendering logic so the UI remains consistent across
//   preview, print, and export operations.
//
// Responsibilities:
//   • Load images from disk and apply EXIF rotation (LoadAndAdjustImage).
//   • Render metadata labels directly onto the image (AddTextToImage).
//   • Export labeled images to user-selected locations (ExportImageWithLabels).
//   • Provide printing support (OnPreparePrinting, OnPrint, etc.).
//   • Respect user settings for label visibility (Label property).
//   • Respect album-level fallback rules:
//       – AlbumTitle
//       – AlbumLocation
//       – AlbumComment
//       – AlbumDate
//       – AlbumArtist
//       – AlbumCopyright
//       – AlbumSoftware
//       – AlbumKeywords
//   • Convert EXIF date strings into human-readable label dates
//     (GetLabelDate → “Monday January 12, 2025”).
//   • Create directories when exporting (CreatePath).
//
// Interaction with other components:
//   • CPhotoExplorerDoc — provides the current image and metadata.
//   • CImageProperties — supplies metadata values and album fallbacks.
//   • MainFrm — provides access to application-wide settings.
//   • PropertiesWnd — displays metadata in the side panel.
//   • GDI+ — used for image loading, rotation, and drawing.
//   • MFC printing framework — used for print preview and printing.
//
// Key Features:
//   • Label overlay system for titles, locations, dates, and comments.
//   • Album fallback logic ensures consistent labeling even when images
//     lack metadata.
//   • EXIF rotation correction ensures images display upright.
//   • Export pipeline produces labeled JPEGs for albums, slideshows,
//     or external sharing.
//   • Print pipeline integrates with MFC’s printing architecture.
//   • User preference persistence via WriteProfileInt (“Settings/Label”).
//
// Internal Structure:
//   • m_bLabel — whether labels are shown in the preview.
//   • m_bExport — whether the current operation is an export with labels.
//   • GetLabelDate — converts EXIF date strings into friendly text.
//   • LoadAndAdjustImage — loads and rotates the image.
//   • AddTextToImage — draws metadata labels onto the image.
//   • ExportImageWithLabels — writes labeled images to disk.
//   • PrintImage — renders the image for printing.
//   • Standard MFC overrides for drawing, printing, and initialization.
//
// This class provides the visual foundation of Photo Explorer, ensuring that
// images are displayed, labeled, printed, and exported with complete fidelity
// to the metadata and album structure defined by the user.
/////////////////////////////////////////////////////////////////////////////
class CImageView : public CView
{
// protected data
protected:
	// if true, the preview image displays the labels
	bool m_bLabel;

	// if true, the has requesting an export with labels
	bool m_bExport;

// public properties
public:
	// get a pointer to the document
	CPhotoExplorerDoc* GetDocument();
	// get a pointer to the document
	__declspec( property( get = GetDocument ) )
		CPhotoExplorerDoc* Document;

	// if true, the has requesting an export with labels
	inline bool GetExport()
	{
		return m_bExport;
	}
	// if true, the has requesting an export with labels
	inline void SetExport( bool value )
	{
		m_bExport = value;
	}
	// if true, the has requesting an export with labels
	__declspec( property( get = GetExport, put = SetExport ) )
		bool Export;

	// if true, the preview image displays the labels
	inline bool GetLabel()
	{
		return m_bLabel;
	}
	// if true, the preview image displays the labels
	inline void SetLabel( bool value )
	{
		AfxGetApp()->WriteProfileInt
		(
			L"Settings", L"Label", value ? 1 : 0
		);
		m_bLabel = value;
	}
	// if true, the preview image displays the labels
	__declspec( property( get = GetLabel, put = SetLabel ) )
		bool Label;

	// if true, the title is replaced with the album title
	// if the image title is blank
	bool GetAlbumTitle();
	// if true, the title is replaced with the album title
	// if the image title is blank
	__declspec( property( get = GetAlbumTitle ) )
		bool AlbumTitle;

	// if true, the location is replaced with the album location
	// if the image location is blank
	bool GetAlbumLocation();
	// if true, the location is replaced with the album location
	// if the image location is blank
	__declspec( property( get = GetAlbumLocation ) )
		bool AlbumLocation;

	// if true, the comment is replaced with the album comment
	// if the image comment is blank
	inline bool GetAlbumComment();
	// if true, the comment is replaced with the album comment
	// if the image comment is blank
	__declspec( property( get = GetAlbumComment ) )
		bool AlbumComment;

	// if true, the date is replaced with the album date
	// if the image date is blank
	inline bool GetAlbumDate();
	// if true, the date is replaced with the album date
	// if the image date is blank
	__declspec( property( get = GetAlbumDate ) )
		bool AlbumDate;

	// if true, the artist is replaced with the album artist
	// if the image artist is blank
	inline bool GetAlbumArtist();
	// if true, the artist is replaced with the album artist
	// if the image artist is blank
	__declspec( property( get = GetAlbumArtist ) )
		bool AlbumArtist;

	// if true, the Copyright is replaced with the album Copyright
	// if the image Copyright is blank
	inline bool GetAlbumCopyright();
	// if true, the Copyright is replaced with the album Copyright
	// if the image Copyright is blank
	__declspec( property( get = GetAlbumCopyright ) )
		bool AlbumCopyright;

	// if true, the Software is replaced with the album Software
	// if the image Software is blank
	inline bool GetAlbumSoftware();
	// if true, the Software is replaced with the album Software
	// if the image Software is blank
	__declspec( property( get = GetAlbumSoftware ) )
		bool AlbumSoftware;

	// if true, the Keywords is replaced with the album Keywords
	// if the image Keywords is blank
	inline bool GetAlbumKeywords();
	// if true, the Keywords is replaced with the album Keywords
	// if the image Keywords is blank
	__declspec( property( get = GetAlbumKeywords ) )
		bool AlbumKeywords;

	// get a user friendly label date from the property date
	// in the form of "day_of_week month day, year"
	CString GetLabelDate( CString csPropertyDate )
	{
		CString value;
		int nStart = 0;
		vector<int> arrValues;
		int nIndex = 0;
		CString csToken = csPropertyDate.Tokenize( L": ", nStart );
		while ( !csToken.IsEmpty())
		{
			const int nValue = (int)_tstol( csToken );
			arrValues.push_back( nValue );
			csToken = csPropertyDate.Tokenize( L": ", nStart );
			nIndex++;
		}

		if ( nIndex == 0 )
		{
			return value;
		}

		for ( ; nIndex < 6; nIndex++ )
		{
			switch ( nIndex )
			{
				case 0: arrValues.push_back( 1900 ); break;
				case 1: arrValues.push_back( 1 ); break;
				case 2: arrValues.push_back( 1 ); break;
				case 3: arrValues.push_back( 0 ); break;
				case 4: arrValues.push_back( 0 ); break;
				case 5: arrValues.push_back( 0 ); break;
			}
		}
		
		if ( 6 == arrValues.size())
		{
			COleDateTime oDT
			( 
				arrValues[ 0 ], arrValues[ 1 ], arrValues[ 2 ],
				arrValues[ 3 ], arrValues[ 4 ], arrValues[ 5 ]
			);
			COleDateTime::DateTimeStatus eStatus = oDT.GetStatus();
			if ( eStatus == COleDateTime::valid )
			{
				value = oDT.Format( L"%A %B %d, %Y" );
			}
		}

		return value;
	}
	// get a user friendly label date from the property date
	// in the form of "day_of_week month day, year"
	__declspec( property( get = GetLabelDate ) )
		CString LabelDate[];

// protected methods
protected:
	// load the image and adjust for rotation
	unique_ptr<Image> LoadAndAdjustImage();

	// load the image and apply labels
	unique_ptr<Image> AddTextToImage();

	// create the given pathname
	bool CreatePath( LPCTSTR pszPath )
	{
		if ( ERROR_SUCCESS == SHCreateDirectoryEx( NULL, pszPath, NULL ) )
		{
			return true;
		}

		return false;
	} // CreatePath

	// public methods
public:
	// export the image with labels
	void ExportImageWithLabels();
    void PrintImage( CDC* pDC, CPrintInfo* pInfo );

// protected overrides
protected:
	virtual void OnInitialUpdate(); // called first time after construct

// public overrides
public:
	virtual BOOL OnPreparePrinting( CPrintInfo* pInfo );
	virtual void OnBeginPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnPrint( CDC* pDC, CPrintInfo* pInfo );
	virtual void OnEndPrinting( CDC* pDC, CPrintInfo* pInfo );
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnDraw( CDC* pDC );  // overridden to draw this view
	virtual ~CImageView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// public constructor/destructor
public:
	CImageView();
	DECLARE_DYNCREATE( CImageView )

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

public:

    afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg void OnFilePrintPreview();
};

/////////////////////////////////////////////////////////////////////////////
#ifndef _DEBUG  // debug version in ImageView.cpp
inline CPhotoExplorerDoc* CImageView::GetDocument()
   { return reinterpret_cast<CPhotoExplorerDoc*>(m_pDocument); }
#endif

/////////////////////////////////////////////////////////////////////////////
