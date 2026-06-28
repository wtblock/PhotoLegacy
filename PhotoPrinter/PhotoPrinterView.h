/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "BaseView.h"

/////////////////////////////////////////////////////////////////////////////
class CPhotoPrinterView : public CBaseView
{
// protected data
protected:

// public properties
public:

// protected methods
protected:

// public methods
public:
	CPhotoPrinterDoc* GetDocument() const;

	void RenderMargins
	( 
		CDC* pDC, double dLeftOfView, double dTopOfView, 
		double dRightOfView, double dBottomOfView
	);

	void RenderHeader
	( 
		CDC* pDC, double dLeftOfView, double dTopOfView, 
		double dRightOfView, double dBottomOfView
	);

	void RenderFooter
	( 
		CDC* pDC, double dLeftOfView, double dTopOfView, 
		double dRightOfView, double dBottomOfView
	);

	void RenderTitlePage
	( 
		CDC* pDC, double dLeftOfView, double dTopOfView, 
		double dRightOfView, double dBottomOfView
	);

	void RenderTableOfContentsPage
	( 
		CDC* pDC, double dLeftOfView, double dTopOfView, 
		double dRightOfView, double dBottomOfView
	);

	void DrawImage
	(
		CDC* pDC, shared_ptr<Image>& pImage, const CRect* pRect
	);

	void RenderImagePage
	( 
		CDC* pDC, double dLeftOfView, double dTopOfView, 
		double dRightOfView, double dBottomOfView
	);

	void ExportDocument();

// protected overrides
protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// public overrides
public:
	// render the page or view
	virtual void render
	(
		CDC* pDC, 
		double dLeftOfView, 
		double dTopOfView, 
		double dRightOfView,
		double dBottomOfView
	);
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// public constructor/destructor
public:
	CPhotoPrinterView() noexcept;
	virtual ~CPhotoPrinterView();
	DECLARE_DYNCREATE(CPhotoPrinterView)

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnScrollTop();
	afx_msg void OnScrollBottom();
	afx_msg void OnScrollPageup();
	afx_msg void OnScrollPagedown();
	afx_msg void OnScrollLineup();
	afx_msg void OnScrollLinedown();
	afx_msg void OnFileExportPages();
	afx_msg void OnUpdateFileExportPages(CCmdUI* pCmdUI);
    afx_msg void OnFilePdf();
    afx_msg void OnUpdateFilePdf(CCmdUI* pCmdUI);
};

/////////////////////////////////////////////////////////////////////////////
#ifndef _DEBUG
// debug version in PhotoPrinterView.cpp
inline CPhotoPrinterDoc* CPhotoPrinterView::GetDocument() const
{
	return reinterpret_cast<CPhotoPrinterDoc*>(m_pDocument);
}
#endif

/////////////////////////////////////////////////////////////////////////////
