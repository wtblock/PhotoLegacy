/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "PhotoExplorer.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "PhotoExplorerDoc.h"
#include "PhotoExplorerView.h"
#include "ImageView.h"
#include "AboutVersions.h"
#include "Splash.h"
#include "CHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPhotoExplorerApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CPhotoExplorerApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CPhotoExplorerApp::OnFileOpen)
	ON_COMMAND( ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CPhotoExplorerApp::CPhotoExplorerApp() noexcept
{
	m_bHiColorIcons = TRUE;

	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("PhotoExplorer.AppID.NoVersion"));

	m_bHiColorIcons = TRUE;
	m_nAppLook = ID_VIEW_APPLOOK_WINDOWS_7;

	m_gdiplusToken = 0;
}

/////////////////////////////////////////////////////////////////////////////
CPhotoExplorerApp theApp;

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoExplorerApp::InitInstance()
{
	const bool bRegistration = CHelper::RegistrationCommandLine();

	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	// Initialize COM
	HRESULT hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
	if ( FAILED( hr ) )
	{
		AfxMessageBox( _T( "Failed to initialize COM library" ) );
		return FALSE; // Terminate application if COM initialization fails
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	// AfxInitRichEdit2() is required to use RichEdit control
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("PhotoExplorer"));
	LoadStdProfileSettings(16);  // Load standard INI file options (including MRU)

	GdiplusStartup( &m_gdiplusToken, &m_gdiplusStartupInput, NULL );

	CSplash ProgressDlg( CWnd::GetDesktopWindow() );
	CWnd* pTopMostWnd = nullptr;
	pTopMostWnd = new CWnd;
	pTopMostWnd->Attach( HWND_TOPMOST );

	ProgressDlg.CreateDlg();
	ProgressDlg.SetWindowPos
	( 
		pTopMostWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE
	);
	ThreadHelp::UpdateUI();

	InitContextMenuManager();
	InitShellManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_PhotoExplorerTYPE,
		RUNTIME_CLASS(CPhotoExplorerDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CPhotoExplorerView));
	if (!pDocTemplate)
	{
		// Shutdown GDI+
		GdiplusShutdown( m_gdiplusToken );

		return FALSE;
	}

	AddDocTemplate(pDocTemplate);

	ThreadHelp::UpdateUI();

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	m_pMainWnd = pMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pTopMostWnd;
		pTopMostWnd = nullptr;
		ProgressDlg.DestroyWindow();
		delete pMainFrame;

		// Shutdown GDI+
		GdiplusShutdown( m_gdiplusToken );

		return FALSE;
	}

	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
	{
		delete pTopMostWnd;
		pTopMostWnd = nullptr;
		ProgressDlg.DestroyWindow();

		// Shutdown GDI+
		GdiplusShutdown( m_gdiplusToken );

		return FALSE;
	}

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	pTopMostWnd->Detach();
	delete pTopMostWnd;
	pTopMostWnd = nullptr;
	ProgressDlg.DestroyWindow();
	pMainFrame->Invalidate();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
int CPhotoExplorerApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	// Uninitialize COM
	CoUninitialize();

	// Shutdown GDI+
	GdiplusShutdown( m_gdiplusToken );

	return CWinAppEx::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// App command to run the dialog
void CPhotoExplorerApp::OnAppAbout()
{
	CAboutVersions aboutDlg;
	aboutDlg.DoModal();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoExplorerApp::OnIdle( LONG lCount )
{
	BOOL bMore = CWinAppEx::OnIdle( lCount );
	if ( !bMore )
	{
		( (CMainFrame*)m_pMainWnd )->TransferQuitMessages();
	}
	return bMore;
} // OnIdle

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerApp::LoadCustomState()
{
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerApp::SaveCustomState()
{
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoExplorerApp::OnFileOpen()
{
	const CString csFilter =
		_T( "Photo Explorer Album (*.album)|*.album|" )
		_T( "All Files (*.*)|*.*||" );

	CFileDialog dlg
	( TRUE, 0, 0, OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY,
		csFilter
	);

	if ( IDOK == dlg.DoModal() )
	{
		CString csPathName = dlg.GetPathName();

		OpenDocumentFile( csPathName );
	}

} // OnFileOpen

/////////////////////////////////////////////////////////////////////////////
