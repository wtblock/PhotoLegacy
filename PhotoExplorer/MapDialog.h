#pragma once
#include <afxdialogex.h>
#include <afxhtml.h>  // For CHtmlView

class CMapHtmlView : public CHtmlView
{
public:
	CMapHtmlView() {}
	virtual ~CMapHtmlView() {}

	DECLARE_DYNCREATE(CMapHtmlView)
};

class CMapDialog : public CDialogEx
{
public:
	CMapDialog(CString url, CWnd* pParent = nullptr);

protected:
	DECLARE_DYNCREATE(CMapDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

private:
	CString m_url;
	CMapHtmlView* m_pHtmlView;
	CWnd* m_pBrowserFrame;
};