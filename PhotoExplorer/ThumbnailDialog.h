/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CThumbnailDialog
//
// Modal dialog used to display progress during thumbnail generation in
// Photo Explorer. This dialog provides a simple, responsive interface that
// informs the user of the number of images processed, the current progress
// percentage, and whether the operation has been cancelled. It is typically
// invoked when loading a folder containing many images.
//
// Purpose:
//   • Present a progress bar while thumbnails are being generated.
//   • Allow the user to cancel the thumbnail-generation process.
//   • Track total images, current image index, and compute progress percent.
//   • Provide a lightweight modal dialog that does not block UI updates.
//
// Why this dialog exists:
//   Thumbnail generation can be time-consuming for large folders. Users need
//   clear feedback that the application is working and the ability to cancel
//   the operation if desired. CThumbnailDialog provides a simple, focused UI
//   that integrates cleanly with the document’s thumbnail-generation logic.
//
// Responsibilities:
//   • Maintain progress state:
//       – m_nTotalImages (total images)
//       – m_nCurrentImage (current index)
//       – m_bCancel (user cancellation flag)
//   • Update the progress bar as images are processed.
//   • Provide Cancel handling via OnCancel.
//   • Support modeless creation via CreateDlg() when needed.
//   • Identify the application’s main window using IdentifyAppMainWindowProc.
//
// Interaction with other components:
//   • CPhotoExplorerDoc — drives thumbnail generation and updates progress.
//   • CMainFrame — may host or parent the dialog.
//   • COutputWnd — receives progress messages during thumbnail creation.
//   • GDI+ — used indirectly through thumbnail generation in the document.
//
// Key Features:
//   • Real-time progress updates using SetCurrentImage.
//   • Automatic percentage calculation based on total images.
//   • User cancellation support via m_bCancel.
//   • Modeless or modal display depending on workflow.
//   • Clean integration with the document’s thumbnail-generation loop.
//
// Internal Structure:
//   • m_ProgressCtrl — progress bar control.
//   • m_bCancel — indicates whether the user cancelled the operation.
//   • m_nTotalImages — total number of thumbnails to generate.
//   • m_nCurrentImage — current image index; updates progress bar.
//   • m_pParent — parent window for dialog positioning.
//   • IdentifyAppMainWindowProc — helper for locating main window handle.
//   • OnInitDialog — initializes progress bar and dialog state.
//   • OnCancel — sets cancellation flag and closes dialog.
//
// This dialog provides a clear, responsive progress indicator for thumbnail
// generation, improving user experience when loading large image folders.
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
