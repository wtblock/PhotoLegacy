/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoExplorer.h"
#include "ShortcutsDialog.h"
#include "afxdialogex.h"
#include <Commctrl.h> // Include the header for LPNMLVDISPINFO

/////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CShortcutsDialog, CDialogEx)

/////////////////////////////////////////////////////////////////////////////
CShortcutsDialog::CShortcutsDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SHORTCUTS_DIALOG, pParent)
{

}

/////////////////////////////////////////////////////////////////////////////
CShortcutsDialog::~CShortcutsDialog()
{
}

/////////////////////////////////////////////////////////////////////////////
void CShortcutsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LIST_CONTROL, m_listCtrl );
}

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CShortcutsDialog, CDialogEx)
    ON_BN_CLICKED( IDOK, &CShortcutsDialog::OnOk )
	ON_BN_CLICKED( IDCANCEL, &CShortcutsDialog::OnCancel )
	ON_NOTIFY
	( 
		LVN_ENDLABELEDIT, IDC_LIST_CONTROL, 
		&CShortcutsDialog::OnLvnEndlabeleditListCtrl 
	)
	ON_NOTIFY( NM_DBLCLK, IDC_LIST_CONTROL, &CShortcutsDialog::OnDblclkListControl )
	ON_MESSAGE( WM_USER_END_LABEL_EDIT, &CShortcutsDialog::OnEndLabelEdit )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
void CShortcutsDialog::OnOk()
{
	// Save changes to the registry before closing
	SaveShortcutsToRegistry();
	CDialogEx::OnOK();
} // OnOk

/////////////////////////////////////////////////////////////////////////////
void CShortcutsDialog::OnCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
} // OnCancel

/////////////////////////////////////////////////////////////////////////////
BOOL CShortcutsDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_listCtrl.ModifyStyle( 0, LVS_REPORT | LVS_EDITLABELS ); // Enable in-place editing
	m_listCtrl.SetExtendedStyle( LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

	m_listCtrl.InsertColumn( 0, _T( "Shortcut" ), LVCFMT_LEFT, 150 ); // Set width to 150
	m_listCtrl.InsertColumn( 1, _T( "Full Name" ), LVCFMT_LEFT, 250 ); // Set width to 250

	LoadShortcutsFromRegistry();

	int nItem = m_listCtrl.InsertItem( 0, _T( "@sister" ) );
	m_listCtrl.SetItemText( nItem, 1, _T( "Alice Smith" ) );
	nItem = m_listCtrl.InsertItem( 1, _T( "@brother" ) );
	m_listCtrl.SetItemText( nItem, 1, _T( "John Doe" ) );

	int blankRow = m_listCtrl.InsertItem( m_listCtrl.GetItemCount(), _T( "" ) );
	m_listCtrl.SetItemText( blankRow, 1, _T( "" ) );

	// Ensure the new blank line is visible
	m_listCtrl.EnsureVisible( blankRow, FALSE );

	m_listCtrl.SetColumnWidth( 0, LVSCW_AUTOSIZE_USEHEADER ); 
	m_listCtrl.SetColumnWidth( 1, LVSCW_AUTOSIZE_USEHEADER );

	return TRUE;
} // OnInitDialog

/////////////////////////////////////////////////////////////////////////////
void CShortcutsDialog::LoadShortcutsFromRegistry()
{
	CString regKey = RegistryBaseKey + _T( "\\Shortcuts" );
	HKEY hKey;
	if 
	( 
		RegOpenKeyEx
		( 
			HKEY_CURRENT_USER, regKey, 0, KEY_READ, &hKey 
		) == ERROR_SUCCESS 
	)
	{
		DWORD index = 0;
		TCHAR shortcut[ 256 ];
		TCHAR fullName[ 256 ];
		DWORD shortcutSize;
		DWORD fullNameSize;

		while ( true )
		{
			shortcutSize = sizeof( shortcut ) / sizeof( TCHAR );
			fullNameSize = sizeof( fullName ) / sizeof( TCHAR );
			DWORD result = RegEnumValue
			( 
				hKey, index, shortcut, &shortcutSize, nullptr, nullptr, 
				(LPBYTE)fullName, &fullNameSize 
			);
			if ( result == ERROR_NO_MORE_ITEMS )
				break;
			if ( result == ERROR_SUCCESS )
			{

				int nItem = m_listCtrl.InsertItem( index, shortcut );
				m_listCtrl.SetItemText( nItem, 1, fullName );
			}
			index++;
		}

		RegCloseKey( hKey );
	}
} // LoadShortcutsFromRegistry

/////////////////////////////////////////////////////////////////////////////
void CShortcutsDialog::SaveShortcutsToRegistry()
{
	CString regKey = RegistryBaseKey + _T( "\\Shortcuts" );
	m_mapShortcuts.clear();

	HKEY hKey;
	if 
		( 
			RegCreateKeyEx
			( 
				HKEY_CURRENT_USER, regKey, 0, nullptr, 0, KEY_WRITE, 
				nullptr, &hKey, nullptr 
			) == ERROR_SUCCESS 
		)
	{
		for ( int i = 0; i < m_listCtrl.GetItemCount(); i++ )
		{
			CString shortcut = m_listCtrl.GetItemText( i, 0 );
			if ( shortcut.Left( 1 ) != L"@" )
			{
				CString csTemp( L"@" );
				csTemp += shortcut;
				shortcut = csTemp;
			}
			CString fullName = m_listCtrl.GetItemText( i, 1 );
			if ( m_mapShortcuts.Exists[ shortcut ] )
			{
				continue;
			}
			m_mapShortcuts.add
			( 
				shortcut, shared_ptr<CString>( new CString( fullName ) )
			);
			RegSetValueEx
			( 
				hKey, shortcut, 0, REG_SZ, (const BYTE*)fullName.GetBuffer(), 
				( fullName.GetLength() + 1 ) * sizeof( TCHAR ) 
			);
		}

		RegCloseKey( hKey );
	}
} // SaveShortcutsToRegistry

/////////////////////////////////////////////////////////////////////////////
void CShortcutsDialog::OnLvnEndlabeleditListCtrl( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMLVDISPINFOA pDispInfo = reinterpret_cast<LPNMLVDISPINFOA>( pNMHDR );
	int rowIndex = pDispInfo->item.iItem;

	if ( pDispInfo->item.pszText != nullptr )
	{
		CString newText = CString( pDispInfo->item.pszText );
		m_listCtrl.SetItemText( rowIndex, pDispInfo->item.iSubItem, newText );
	}

	if ( rowIndex == m_listCtrl.GetItemCount() - 1 )
	{
		CString shortcut = m_listCtrl.GetItemText( rowIndex, 0 );
		CString fullName = m_listCtrl.GetItemText( rowIndex, 1 );

		if ( !shortcut.IsEmpty() || !fullName.IsEmpty() )
		{
			int blankRow = m_listCtrl.InsertItem( m_listCtrl.GetItemCount(), _T( "" ) );
			m_listCtrl.SetItemText( blankRow, 1, _T( "" ) );

			m_listCtrl.EnsureVisible( blankRow, FALSE );
		}
	}

	*pResult = 0;
}

/////////////////////////////////////////////////////////////////////////////
void CShortcutsDialog::OnDblclkListControl( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>( pNMHDR );
	if ( pNMIA->iItem != -1 && pNMIA->iSubItem != -1 )
	{
		CRect rect; m_listCtrl.GetSubItemRect
		( 
			pNMIA->iItem, pNMIA->iSubItem, LVIR_LABEL, rect 
		); 
		CListEdit* pEdit = 
			new CListEdit
			( 
				pNMIA->iItem, pNMIA->iSubItem, 
				m_listCtrl.GetItemText( pNMIA->iItem, pNMIA->iSubItem ) 
			); 
		pEdit->Create
		( 
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, rect, this, 1 
		); 
		pEdit->SetFocus(); 
		pEdit->SetSel( 0, -1 );
	}
	*pResult = 0;
} // OnDblclkListControl

/////////////////////////////////////////////////////////////////////////////
LRESULT CShortcutsDialog::OnEndLabelEdit( WPARAM wParam, LPARAM lParam )
{
	int iItem = (int)wParam;
	CString* pstr = (CString*)lParam;
	m_listCtrl.SetItemText( iItem, m_listCtrl.GetNextItem( -1, LVNI_SELECTED ), *pstr );

	delete pstr;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
