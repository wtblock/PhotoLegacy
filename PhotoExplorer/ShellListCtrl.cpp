/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PhotoExplorer.h"
#include "ShellListCtrl.h"
#include "ShellListView.h"
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP( CShellListCtrl, CListCtrl )
	ON_MESSAGE( WM_MFC_INITCTRL, OnInitControl )
    ON_NOTIFY_REFLECT( LVN_ITEMCHANGED, &CShellListCtrl::OnLvnItemchanged )
	ON_NOTIFY_REFLECT( NM_CUSTOMDRAW, &CShellListCtrl::OnNMCustomdraw )
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
void CShellListCtrl::SelectAllItems()
{
	// Get the total number of items in the list control
	int itemCount = GetItemCount();

	// Iterate through all items and select them
	for ( int i = 0; i < itemCount; ++i )
	{
		SetItemState( i, LVIS_SELECTED, LVIS_SELECTED );
	}
} // SelectAllItems

/////////////////////////////////////////////////////////////////////////////
void CShellListCtrl::DeselectAllItems()
{
	// Get the total number of items in the list control
	int itemCount = GetItemCount();

	// Iterate through all items and clear the selection
	for (int i = 0; i < itemCount; ++i)
	{
		SetItemState(i, 0, LVIS_SELECTED);
	}
} // DeselectAllItems

/////////////////////////////////////////////////////////////////////////////
LRESULT CShellListCtrl::OnInitControl( WPARAM wParam, LPARAM lParam )
{
	return 0;
} // OnInitControl

/////////////////////////////////////////////////////////////////////////////
void CShellListCtrl::OnNMCustomdraw( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>( pNMHDR );
	*pResult = CDRF_DODEFAULT;

	if ( pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;

	} else if ( pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		int nIndex = static_cast<int>( pLVCD->nmcd.dwItemSpec );
		if ( GetItemState( nIndex, LVIS_SELECTED ) & LVIS_SELECTED )
		{
			// Custom drawing for thumbnails using GDI+
			CDC* pDC = CDC::FromHandle( pLVCD->nmcd.hdc );
			Gdiplus::Graphics graphics( pDC->GetSafeHdc() );

			CRect rcItem;
			GetItemRect( nIndex, &rcItem, LVIR_BOUNDS );

			// Define the semi-transparent color
			// Alpha, R, G, B (128 = 50% transparency)
			Gdiplus::Color semiTransparentBlue( 128, 173, 216, 230 ); 

			// Create a solid brush with the semi-transparent color
			Gdiplus::SolidBrush brush( semiTransparentBlue );

			// Draw the semi-transparent rectangle over the thumbnail
			graphics.FillRectangle
			( 
				&brush, rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height() 
			);

			// Ensure text and other elements are drawn correctly
			*pResult = CDRF_NEWFONT;
		}
	}
} // OnNMCustomdraw

/////////////////////////////////////////////////////////////////////////////
BOOL CShellListCtrl::OnEraseBkgnd( CDC* pDC )
{
	// Set the background color to black
	CRect rect;
	GetClientRect( &rect );

	Gdiplus::Graphics graphics( pDC->GetSafeHdc() );
	Gdiplus::SolidBrush brush( Gdiplus::Color( 255, 0, 0, 0 ) ); // Solid black color
	graphics.FillRectangle( &brush, rect.left, rect.top, rect.Width(), rect.Height() );

	return TRUE; // Return TRUE to indicate background is erased
} // OnEraseBkgnd

/////////////////////////////////////////////////////////////////////////////
void CShellListCtrl::OnLvnItemchanged( NMHDR* pNMHDR, LRESULT* pResult )
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>( pNMHDR );
	CShellListView* pParent = (CShellListView*)GetParent();

	// Check if the item state has changed
	if ( pNMLV->uChanged & LVIF_STATE )
	{
		if ( pNMLV->uNewState & LVIS_SELECTED )
		{
			// Item was selected
			// Assuming column 0 contains the path
			CString strPath = GetItemText( pNMLV->iItem, 0 ); 
			pParent->AddSelectedImage = strPath;

		} else if 
		( 
			!( pNMLV->uNewState & LVIS_SELECTED ) && 
			( pNMLV->uOldState & LVIS_SELECTED ) 
		)
		{
			// Item was deselected
			// Assuming column 0 contains the path
			CString strPath = GetItemText( pNMLV->iItem, 0 ); 
			pParent->RemoveSelectedImage = strPath;
		}

		// Enumerate selected items
		POSITION pos = GetFirstSelectedItemPosition();
		while ( pos )
		{
			int nItem = GetNextSelectedItem( pos );

			// Do something with the selected item index (nItem)
			// For example, retrieve the item text:
			CString strFileName = GetItemText( nItem, 0 );

			// record the filename into multiple selection
			pParent->AddSelectedImage = strFileName;
		}

		Invalidate();
	}

	*pResult = 0;

	//// Ensure the item changed
	//if 
	//( 
	//	( pNMLV->uChanged & LVIF_STATE ) &&
	//	( pNMLV->uNewState & LVIS_SELECTED ) &&
	//	!( pNMLV->uOldState & LVIS_SELECTED ) 
	//)
	//{
	//	// Check if more than one item is selected
	//	int nSelectedCount = GetSelectedCount();
	//	if ( nSelectedCount > 1 )
	//	{

	//		Invalidate();

	//	} else
	//	{
	//		// Get the index of the selected item
	//		int nItem = pNMLV->iItem;

	//		// Retrieve the file name from the item
	//		// assuming column 0 has the file name
	//		CString strFileName = GetItemText( nItem, 0 );
	//		pParent->SelectedImage = strFileName;
	//	}
	//} else
	//{
	//	int nSelectedCount = GetSelectedCount();
	//	if ( nSelectedCount == 0 )
	//	{
	//		pParent->SelectedImage = L"";
	//		Invalidate();
	//	}
	//}

	//*pResult = 0;
} // OnLvnItemchanged

/////////////////////////////////////////////////////////////////////////////

