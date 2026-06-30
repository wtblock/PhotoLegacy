/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CThumbnailDialog
//
// Modal (but UI‑friendly) progress dialog originally designed for Photo
// Explorer to show thumbnail‑generation progress. PhotoPrinter reuses this
// dialog as a general progress indicator during long operations such as
// caching labeled images, caching queried images, and building pages.
//
// Purpose:
//   • Display progress for operations that may take several seconds or
//     minutes, especially when processing thousands of images.
//   • Provide both a graphical progress bar and a textual status line.
//   • Allow the user to cancel the operation cleanly.
//   • Keep the UI responsive by cooperating with ThreadHelp::UpdateUI().
//
// Why this class exists:
//   Standard MFC progress dialogs (CProgressCtrl inside a dialog) do not
//   provide cancellation, parent‑window positioning, or dynamic status text.
//   PhotoPrinter requires a lightweight, reusable dialog that can be shown
//   non‑modally and updated frequently during long loops.
//
// Responsibilities:
//   • Create and position the dialog relative to the main window.
//   • Display progress using a percentage bar and “X of Y Images” text.
//   • Track cancellation state (Cancel property).
//   • Allow the caller to update progress via CurrentImage and TotalImages.
//   • Provide a fallback mechanism to locate the main window if the parent
//     is not available (IdentifyAppMainWindowProc).
//   • Maintain UI responsiveness by calling ThreadHelp::UpdateUI().
//
// Interaction with other components:
//   • Used heavily by CPhotoPrinterDoc during:
//       – CacheLabeledImages()
//       – CacheQueriedImages()
//       – CacheChronologicalImages()
//   • The main frame’s Wait() method is often used alongside this dialog
//     to keep the UI responsive.
//   • The dialog is updated inside tight loops, so it must be lightweight
//     and safe to call frequently.
//
// Wizard‑generated portions:
//   • Base class (CDialogEx), DDX/DDV mapping, message map.
//
// Application‑specific additions:
//   • Parent‑window discovery logic.
//   • Dynamic progress text formatting.
//   • Cancel flag and handler.
//   • Integration with ThreadHelp for UI pumping.
//   • Automatic initialization of progress range and status text.
//
// This dialog provides a simple, reliable progress indicator that enhances
// the user experience during long-running operations in PhotoPrinter and
// Photo Explorer.
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

	// display the progress in text form
	CEdit m_editStatus;

	// indicates the user has cancelled the dialog
	bool m_bCancel;

	// total number of thumbnail images being processed
	int m_nTotalImages;

	// the current image being processed
	int m_nCurrentImage;

	// type of objects (image, page, etc.)
	CString m_csObjects;

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

	// type of objects (image, page, etc.)
	CString GetObjects()
	{
		return m_csObjects;
	}
	// type of objects (image, page, etc.)
	void SetObjects(CString value)
	{
		if (value.IsEmpty())
		{
			value = L"Images";
		}
		m_csObjects = value;
	}
	// type of objects (image, page, etc.)
	__declspec(property(get = GetObjects, put = SetObjects))
		CString Objects;

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
		CString csObjects = Objects;
		m_ProgressCtrl.SetPos( nProgress );
		CString csText;
		csText.Format(L"%4d of %4d %s.", value, nMax, csObjects);
		m_editStatus.SetWindowText(csText);
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
