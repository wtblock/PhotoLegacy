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
