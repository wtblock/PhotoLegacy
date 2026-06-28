/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
class CThumbnailDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CThumbnailDialog)

	DECLARE_MESSAGE_MAP()
	
// protected data
protected:
	// Dialog Data
	enum
	{
		IDD = IDD_THUMBNAIL_DIALOG
	};
	// the progress control informs the user of the 
	// progress of thumbnail creation
	CProgressCtrl m_ProgressCtrl;

	// indicates the user has cancelled the dialog
	bool m_bCancel;

	// total number of thumbnail images being processed
	int m_nTotalImages;

	// the current image being processed
	int m_nCurrentImage;

	// the parent window of this dialog
	CWnd* m_pParent;

// public properties
public:
	// indicates the user has cancelled the dialog
	bool GetCancel()
	{
		return m_bCancel;
	} 
	// indicates the user has cancelled the dialog
	void SetCancel( bool value )
	{
		m_bCancel = value;
	} 
	// indicates the user has cancelled the dialog
	__declspec( property( get = GetCancel, put = SetCancel))
		bool Cancel;

	// total number of thumbnail images being processed
	int GetTotalImages()
	{
		return m_nTotalImages;
	} 
	// total number of thumbnail images being processed
	void SetTotalImages( int value )
	{
		m_nTotalImages = value;
	} 
	// total number of thumbnail images being processed
	__declspec( property( get = GetTotalImages, put = SetTotalImages))
		int TotalImages;

	// the current image being processed
	int GetCurrentImage()
	{
		return m_nCurrentImage;
	} 
	// the current image being processed
	void SetCurrentImage( int value )
	{
		m_nCurrentImage = value;
		int nMax = TotalImages;
		int nProgress = 0;
		if ( TotalImages > 0 )
		{
			nProgress = int( ( float( value ) / float( nMax ) ) * 100.0f );
		}
		m_ProgressCtrl.SetPos( nProgress );
	} 
	// the current image being processed
	__declspec( property( get = GetCurrentImage, put = SetCurrentImage))
		int CurrentImage;

	// parent window
	inline CWnd* GetParent()
	{
		return m_pParent;
	}
	// parent window
	inline void SetParent( CWnd* value )
	{
		m_pParent = value;
	}
	// parent window
	__declspec( property( get = GetParent, put = SetParent ) )
		CWnd* Parent;

// protected methods
protected:
	static BOOL CALLBACK IdentifyAppMainWindowProc
	( 
		HWND hWnd, LPARAM lParam
	);

// public methods
public:
	void CreateDlg();
	afx_msg void OnCancel();

// protected overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// public overrides
public:
	virtual BOOL OnInitDialog();

// public constructor/destructor
public:
	CThumbnailDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CThumbnailDialog();

};

/////////////////////////////////////////////////////////////////////////////
