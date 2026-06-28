#include "pch.h"
#include "MapDialog.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(CMapHtmlView, CHtmlView)

IMPLEMENT_DYNAMIC(CMapDialog, CDialogEx)

BEGIN_MESSAGE_MAP(CMapDialog, CDialogEx)
	ON_WM_SIZE()
END_MESSAGE_MAP()

CMapDialog::CMapDialog(CString url, CWnd* pParent)
	: CDialogEx(IDD_MAP_DIALOG, pParent), m_url(url), m_pHtmlView(nullptr), m_pBrowserFrame(nullptr)
{}

BOOL CMapDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_pBrowserFrame = GetDlgItem(IDC_BROWSER_FRAME);
	CRect rect;
	m_pBrowserFrame->GetWindowRect(&rect);
	ScreenToClient(&rect);

	m_pHtmlView = (CMapHtmlView*)RUNTIME_CLASS(CMapHtmlView)->CreateObject();
	m_pHtmlView->Create(nullptr, nullptr, WS_VISIBLE | WS_CHILD, rect, this, AFX_IDW_PANE_FIRST, nullptr);
	m_pHtmlView->Navigate2(m_url.GetString(), 0, nullptr);	return TRUE;
}

void CMapDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_pHtmlView && m_pBrowserFrame)
	{
		CRect rect;
		m_pBrowserFrame->GetWindowRect(&rect);
		ScreenToClient(&rect);
		m_pHtmlView->MoveWindow(rect);
	}
}