/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "PhotoPrinter.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "PhotoPrinterDoc.h"
#include "PhotoPrinterView.h"
#include "AboutVersions.h"
#include "Splash.h"

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPhotoPrinterApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CPhotoPrinterApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CPhotoPrinterApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CPhotoPrinterApp::CPhotoPrinterApp() noexcept
{
	m_bHiColorIcons = TRUE;
	m_nAppLook = 0;

	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;

	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("PhotoPrinter.AppID.NoVersion"));

}

/////////////////////////////////////////////////////////////////////////////
CPhotoPrinterApp theApp;

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoPrinterApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	// create a command line instance
	m_pCommandLine = unique_ptr<CCommandLine>(new CCommandLine);

	// Parse command line for standard shell commands, DDE, file open
	ParseCommandLine(*m_pCommandLine);

	WorkingFolder = m_pCommandLine->WorkingFolder;
	if (WorkingFolder.IsEmpty())
	{
		CString csWorkingFolder(char(0), MAX_PATH);
		DWORD dwLen = ::GetCurrentDirectory(MAX_PATH, csWorkingFolder.GetBuffer());
		csWorkingFolder.ReleaseBuffer();
		WorkingFolder = csWorkingFolder;
	}
	else
	{
		::SetCurrentDirectory(WorkingFolder);
	}

	CString csTest = WorkingFolder;

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
	SetRegistryKey(_T("WTBLOCK.ORG"));
	LoadStdProfileSettings(16);  // Load standard INI file options (including MRU)

	GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);

	CSplash ProgressDlg(CWnd::GetDesktopWindow());
	CWnd* pTopMostWnd = nullptr;
	pTopMostWnd = new CWnd;
	pTopMostWnd->Attach(HWND_TOPMOST);

	ProgressDlg.CreateDlg();
	ProgressDlg.SetWindowPos
	(
		pTopMostWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE
	);
	ThreadHelp::UpdateUI();

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams
	(
		AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams
	);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate
	(
		IDR_PhotoPrinterTYPE,
		RUNTIME_CLASS(CPhotoPrinterDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CPhotoPrinterView)
	);
	if (!pDocTemplate)
	{
		// Shutdown GDI+
		GdiplusShutdown(m_gdiplusToken);

		return FALSE;
	}

	AddDocTemplate(pDocTemplate);

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
		GdiplusShutdown(m_gdiplusToken);

		return FALSE;
	}

	// do not allow new file here
	if (m_pCommandLine->m_nShellCommand == CCommandLineInfo::FileNew)
	{
		m_pCommandLine->m_nShellCommand = CCommandLineInfo::FileNothing;
	}

	// Application was launched with /Embedding or /Automation switch.
	// Run application as automation server.
	if (m_pCommandLine->m_bRunEmbedded || m_pCommandLine->m_bRunAutomated)
	{
		// Don't show the main window
		return TRUE;
	}

	// Dispatch commands specified on the command line.  Will return FALSE if
	// application was launched with /RegServer, /Register, /Unregserver or 
	// /Unregister.
	if (!ProcessShellCommand(*m_pCommandLine))
		return FALSE;

	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	pMainFrame->Wait(2000);

	return TRUE;
} // InitInstance

/////////////////////////////////////////////////////////////////////////////
int CPhotoPrinterApp::ExitInstance()
{
	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	// Uninitialize COM
	CoUninitialize();

	// Shutdown GDI+
	GdiplusShutdown( m_gdiplusToken );

	return CWinAppEx::ExitInstance();
} // ExitInstance

/////////////////////////////////////////////////////////////////////////////
BOOL CPhotoPrinterApp::OnIdle(LONG lCount)
{
	BOOL bMore = CWinAppEx::OnIdle(lCount);
	if (!bMore)
	{
		((CMainFrame*)m_pMainWnd)->TransferQuitMessages();
	}
	return bMore;
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterApp::OnFileOpen()
{
	CString strFilePath;
	CFileDialog fileDlg
	(
		TRUE, L"pp", NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("Photo Printer Files (*.pp)|*.pp|All Files (*.*)|*.*||")
	);

	if (fileDlg.DoModal() == IDOK)
	{
		strFilePath = fileDlg.GetPathName();
		OpenDocumentFile(strFilePath);
	}
} // OnFileOpen

/////////////////////////////////////////////////////////////////////////////
// App command to run the dialog
void CPhotoPrinterApp::OnAppAbout()
{
	CAboutVersions aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterApp::LoadCustomState()
{
}

/////////////////////////////////////////////////////////////////////////////
void CPhotoPrinterApp::SaveCustomState()
{
}

/////////////////////////////////////////////////////////////////////////////
