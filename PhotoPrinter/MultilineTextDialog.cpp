/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "MultilineTextDialog.h"

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC( CMultilineTextDialog, CDialog )

/////////////////////////////////////////////////////////////////////////////
CMultilineTextDialog::CMultilineTextDialog( CWnd* pParent /*=NULL*/ )
	: CDialog( CMultilineTextDialog::IDD, pParent )
{
	MaxLength = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMultilineTextDialog::~CMultilineTextDialog()
{
}

/////////////////////////////////////////////////////////////////////////////
void CMultilineTextDialog::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Text( pDX, IDC_EDIT_COMMENT_TEXT, m_csValue );
}

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP( CMultilineTextDialog, CDialog )
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
BOOL CMultilineTextDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWnd* pWnd = AfxGetMainWnd();
	CenterWindow( pWnd );

	CEdit* pEdit = (CEdit*)GetDlgItem( IDC_EDIT_COMMENT_TEXT );

	if ( m_nMaxLength > 0 )
	{
		pEdit->SetLimitText( m_nMaxLength );
	}

	if ( !m_csTitle.IsEmpty() )
	{
		SetWindowText( m_csTitle );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
} // OnInitDialog

/////////////////////////////////////////////////////////////////////////////

void CMultilineTextDialog::OnSize( UINT nType, int cx, int cy )
{
	CDialog::OnSize( nType, cx, cy );
	if ( m_hWnd == 0 )
		return;
	CEdit* pEdit = (CEdit*)GetDlgItem( IDC_EDIT_COMMENT_TEXT );
	if ( pEdit == 0 )
		return;
	pEdit->MoveWindow( 15, 15, cx - 30, cy - 50 );
	CWnd* pOK = GetDlgItem( IDOK );
	CRect rect;
	pOK->GetClientRect( &rect );
	pOK->MoveWindow( 15, cy - 30, rect.Width(), rect.Height() );

	CWnd* pCancel = GetDlgItem( IDCANCEL );
	pCancel->GetClientRect( &rect );
	pCancel->MoveWindow( cx - 15 - rect.Width(), cy - 30, rect.Width(), rect.Height() );

	// TODO: Add your message handler code here
}

/////////////////////////////////////////////////////////////////////////////
BOOL CMultilineTextDialog::PreTranslateMessage( MSG* pMsg )
{
	if ( ( pMsg->message == WM_KEYDOWN ) && ( pMsg->wParam == VK_TAB ) )
	{
		CEdit* pEdit = (CEdit*)GetDlgItem( IDC_EDIT_COMMENT_TEXT );
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

	//just let other massages to work normally
	return CDialog::PreTranslateMessage( pMsg );
}

/////////////////////////////////////////////////////////////////////////////
