/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ShellListView.h"
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
class CPhotoExplorerView : public CShellListView
{
protected:
	// create from serialization only
	CPhotoExplorerView() noexcept;
	DECLARE_DYNCREATE(CPhotoExplorerView)

// protected data
protected:
	int m_nCurrentView;
	vector<CString> m_arrCut;
	vector<CString> m_arrCopy;

// public properties
public:
	// the list control's current view
	int GetCurrentView()
	{
		return m_nCurrentView;
	}
	void SetCurrentView( int value )
	{
		m_nCurrentView = value;
	}
	// Get the pointer for the list control.
	__declspec( property( get = GetCurrentView, put = SetCurrentView ) )
		int CurrentView;

// Attributes
public:
	CPhotoExplorerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();

protected:
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CPhotoExplorerView();
	BOOL PreTranslateMessage( MSG* pMsg );
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnViewLargeicon();
	afx_msg void OnUpdateViewLargeicon( CCmdUI* pCmdUI );
	afx_msg void OnViewList();
	afx_msg void OnUpdateViewList( CCmdUI* pCmdUI );
	afx_msg void OnViewSmallicon();
	afx_msg void OnUpdateViewSmallicon( CCmdUI* pCmdUI );
	afx_msg void OnViewDetails();
	afx_msg void OnUpdateViewDetails( CCmdUI* pCmdUI );

    afx_msg void OnEditCut();
    afx_msg void OnEditCopy();
    afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditCut( CCmdUI* pCmdUI );
	afx_msg void OnUpdateEditCopy( CCmdUI* pCmdUI );
	afx_msg void OnUpdateEditPaste( CCmdUI* pCmdUI );
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateEditDelete( CCmdUI* pCmdUI );
    afx_msg void OnEditSelectAll();
    afx_msg void OnUpdateEditSelectAll( CCmdUI* pCmdUI );
    afx_msg void OnBingMap();
    afx_msg void OnUpdateBingMap(CCmdUI* pCmdUI);
    afx_msg void OnGoogleMap();
    afx_msg void OnUpdateGoogleMap(CCmdUI* pCmdUI);
    afx_msg void OnPropertiesLabel();
    afx_msg void OnUpdatePropertiesLabel(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////
#ifndef _DEBUG  // debug version in PhotoExplorerView.cpp
inline CPhotoExplorerDoc* CPhotoExplorerView::GetDocument() const
   { return reinterpret_cast<CPhotoExplorerDoc*>(m_pDocument); }
#endif

/////////////////////////////////////////////////////////////////////////////
