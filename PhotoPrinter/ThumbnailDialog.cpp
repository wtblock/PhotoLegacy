/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "Resource.h"
#include "ThumbnailDialog.h"
#include "ThreadHelp.h"
#include "afxdialogex.h"

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CThumbnailDialog, CDialogEx)

/////////////////////////////////////////////////////////////////////////////
CThumbnailDialog::CThumbnailDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_THUMBNAIL_DIALOG, pParent)
{
	Cancel = false;
	TotalImages = 1;
	Objects = L"Images";
	m_nCurrentImage = 0;
	Parent = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
CThumbnailDialog::~CThumbnailDialog()
{

}

/////////////////////////////////////////////////////////////////////////////
void CThumbnailDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_ProgressCtrl);
	DDX_Control(pDX, IDC_EDIT1, m_editStatus);
}

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CThumbnailDialog, CDialogEx)
    ON_BN_CLICKED( IDCANCEL, &CThumbnailDialog::OnCancel )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CThumbnailDialog::IdentifyAppMainWindowProc
( 
	HWND hWnd, LPARAM lParam
)
{
	CWnd** ppWnd = (CWnd**)lParam;
	WINDOWINFO info;
	info.cbSize = sizeof( WINDOWINFO );
	::GetWindowInfo( hWnd, &info );
	if ( info.dwStyle & WS_EX_APPWINDOW )
	{
		*ppWnd = CWnd::FromHandle( hWnd );
		return FALSE;
	}
	return TRUE;
} // IdentifyAppMainWindowProc

/////////////////////////////////////////////////////////////////////////////
void CThumbnailDialog::CreateDlg()
{
	CWnd* pWnd = AfxGetMainWnd();
	if ( pWnd == 0 )
	{
		pWnd = m_pParent;
		if ( pWnd == 0 )
		{
			EnumThreadWindows
			( GetCurrentThreadId(),
				IdentifyAppMainWindowProc,
				(LPARAM)&pWnd
			);
			if ( pWnd == 0 )
			{
				return;
			}
		}
	}

	CWnd* effectiveParent = m_pParent;
	if ( !effectiveParent )
	{
		effectiveParent = pWnd;
	}
	Create( IDD, effectiveParent );

	CenterWindow( effectiveParent );

	SetBackgroundColor( RGB( 128, 128, 128 ) );

	BringWindowToTop();

	ShowWindow( SW_SHOW );

	Cancel = false;
	TotalImages = 1;
	CurrentImage = 0;

	m_ProgressCtrl.SetRange( 0, 100 );
	m_ProgressCtrl.SetPos( 0 );

	ThreadHelp::UpdateUI();
} // CreateDlg

/////////////////////////////////////////////////////////////////////////////
void CThumbnailDialog::OnCancel()
{
    Cancel = true;

    CDialogEx::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////
BOOL CThumbnailDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
} // OnInitDialog

/////////////////////////////////////////////////////////////////////////////
