/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// COutputList window

class COutputList : public CListBox
{
// Construction
public:
	COutputList() noexcept;

// Implementation
public:
	virtual ~COutputList();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
};

class COutputWnd : public CDockablePane
{
// public properties
public:
	// write text to the progress tab
	void SetProgressText( CString value )
	{
		if ( value != m_csProgress )
		{
			m_csProgress = value;
			m_wndOutputProgress.AddString( value );
			ShowProgressOutput();
			ScrollToBottom( m_wndOutputProgress );
		}
	}
	// write text to the progress tab
	__declspec( property( put = SetProgressText ) )
		CString ProgressText;

	// write text to the warnings tab
	void SetWarningsText( CString value )
	{
		if ( value != m_csWarning )
		{
			m_csWarning = value;
			m_wndOutputWarnings.AddString( value );
			ShowWarningOutput();
			ScrollToBottom( m_wndOutputWarnings );
		}
	}
	// write text to the warnings tab
	__declspec( property( put = SetWarningsText ) )
		CString WarningsText;

	// write text to the errors tab
	void SetErrorsText( CString value )
	{
		if ( value != m_csError )
		{
			m_csError = value;
			m_wndOutputErrors.AddString( value );
			ShowErrorOutput();
			ScrollToBottom( m_wndOutputErrors );
		}
	}
	// write text to the errors tab
	__declspec( property( put = SetErrorsText ) )
		CString ErrorsText;

// public methods
public:
	void ScrollToBottom( COutputList& listBox )
	{
		int count = listBox.GetCount();
		if ( count > 0 )
		{
			listBox.SetTopIndex( count - 1 );
		}
	}

	// show the error output
	inline void ShowErrorOutput()
	{
		ShowOutputTab( m_wndOutputErrors );
	}
	// show the warning output
	inline void ShowWarningOutput()
	{
		ShowOutputTab( m_wndOutputWarnings );
	}
	// show the progress output
	inline void ShowProgressOutput()
	{
		ShowOutputTab( m_wndOutputProgress );
	}

	// clear the error output
	inline void ClearErrorOutput()
	{
		m_wndOutputErrors.ResetContent();
	}
	// clear the warning output
	inline void ClearWarningOutput()
	{
		m_wndOutputWarnings.ResetContent();
	}
	// clear the progress output
	inline void ClearProgressOutput()
	{
		m_wndOutputProgress.ResetContent();
	}

// Construction
public:
	COutputWnd() noexcept;

	void UpdateFonts();

// Attributes
protected:
	CMFCTabCtrl	m_wndTabs;
	COutputList m_wndOutputProgress;
	COutputList m_wndOutputWarnings;
	COutputList m_wndOutputErrors;
	CString m_csProgress;
	CString m_csError;
	CString m_csWarning;
	CFont* m_pFont;

protected:
	// show one of the output tabs
	void ShowOutputTab( COutputList& refWnd );

	void FillOutputWindow();
	void FillWarningsWindow();
	void FillErrorsWindow();

	void AdjustHorzScroll(CListBox& wndListBox);

// Implementation
public:
	virtual ~COutputWnd();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

