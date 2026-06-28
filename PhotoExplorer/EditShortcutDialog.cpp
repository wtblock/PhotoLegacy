/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoExplorer.h"
#include "MainFrm.h"
#include "PhotoExplorerDoc.h"
#include "EditShortcutDialog.h"
#include "afxdialogex.h"

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CEditShortcutDialog, CDialogEx)

/////////////////////////////////////////////////////////////////////////////
CEditShortcutDialog::CEditShortcutDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EDIT_SHORTCUT_DIALOG, pParent)
	, m_csValue( _T( "" ) )
{
	MaxLength = 0;
}

/////////////////////////////////////////////////////////////////////////////
CEditShortcutDialog::~CEditShortcutDialog()
{
}

/////////////////////////////////////////////////////////////////////////////
void CEditShortcutDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_SHORTCUTS, m_csValue );
}

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CEditShortcutDialog, CDialogEx)
	ON_WM_SIZE()
	ON_BN_CLICKED( IDOK, &CEditShortcutDialog::OnOk )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
CString CEditShortcutDialog::MapData()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;

	CString value = pDoc->MapShortcuts( m_csValue, Shortcuts );

	return value;
} // MapData

/////////////////////////////////////////////////////////////////////////////
void CEditShortcutDialog::OnOk()
{
	UpdateData();
	m_csValue = MapData();
	AfxGetApp()->WriteProfileString
	( 
		L"Settings", L"Shortcuts", m_csValue 
	);
	CDialogEx::OnOK();
} // OnOk

/////////////////////////////////////////////////////////////////////////////
BOOL CEditShortcutDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CWnd* pWnd = AfxGetMainWnd();
	CenterWindow( pWnd );

	CString csValue = AfxGetApp()->GetProfileString
	( 
		L"Settings", L"Shortcuts" 
	);

	if ( -1 == csValue.FindOneOf( L"\\n" ))
	{
		csValue.Replace( L"\r\n", L"\\n" );
		csValue.Replace( L"\n", L"\\n" );
	}

	SetValue( csValue, L"\\n" );

	CEdit* pEdit = (CEdit*)GetDlgItem( IDC_EDIT_SHORTCUTS );

	if ( m_nMaxLength > 0 )
	{
		pEdit->SetLimitText( m_nMaxLength );
	}

	if ( !Title.IsEmpty() )
	{
		SetWindowText( Title );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
void CEditShortcutDialog::OnSize( UINT nType, int cx, int cy )
{
	CDialogEx::OnSize( nType, cx, cy );

	CDialog::OnSize( nType, cx, cy );
	if ( m_hWnd == 0 )
		return;
	CEdit* pEdit = (CEdit*)GetDlgItem( IDC_EDIT_SHORTCUTS );
	if ( pEdit == 0 )
		return;
	pEdit->MoveWindow( 15, 15, cx - 30, cy - 50 );
	CWnd* pOK = GetDlgItem( IDOK );
	CRect rect;
	pOK->GetClientRect( &rect );
	pOK->MoveWindow( 15, cy - 30, rect.Width(), rect.Height() );

	CWnd* pCancel = GetDlgItem( IDCANCEL );
	pCancel->GetClientRect( &rect );
	pCancel->MoveWindow
	( 
		cx - 15 - rect.Width(), cy - 30, rect.Width(), rect.Height() 
	);
} // OnSize

/////////////////////////////////////////////////////////////////////////////
BOOL CEditShortcutDialog::PreTranslateMessage( MSG* pMsg )
{
	if ( ( pMsg->message == WM_KEYDOWN ) && ( pMsg->wParam == VK_TAB ) )
	{
		CEdit* pEdit = (CEdit*)GetDlgItem( IDC_EDIT_SHORTCUTS );
		if ( pEdit )
		{

			// get the char index of the caret position
			int nPos = LOWORD( pEdit->CharFromPos( pEdit->GetCaretPos() ) );

			// select zero chars
			pEdit->SetSel( nPos, nPos );

			// then replace that selection with a TAB
			pEdit->ReplaceSel( L"\t", TRUE );

			// no need to do a msg translation, so quit. 
			// that way no further processing gets done
			return TRUE;
		}
	}

	//just let other messages to work normally
	return CDialogEx::PreTranslateMessage( pMsg );
}

/////////////////////////////////////////////////////////////////////////////
