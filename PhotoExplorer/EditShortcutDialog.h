/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include "CHelper.h"

/////////////////////////////////////////////////////////////////////////////
class CEditShortcutDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CEditShortcutDialog)

// protected data
protected:
	CRichEditCtrl m_EditControl;

	// unique collection of short cuts
	CKeyedCollection<CString, CString> m_mapShortcuts;

	CString m_csValue;
	int m_nMaxLength;
	CString m_csTitle;
	CPoint m_ptUpperLeft;

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_SHORTCUT_DIALOG };
#endif

// public properties
public:
	// limit the size of text that can be entered
	void SetMaxLength( int nMax )
	{
		m_nMaxLength = nMax;
	}
	// limit the size of text that can be entered
	__declspec( property( put = SetMaxLength ) )
		int MaxLength;

	// number of lines
	int GetNumberOfLines()
	{
		int nLines = 0;
		CString csBuffer = m_csValue;
		CString csDelim = _T( "\r\n" );
		int nLen = csBuffer.GetLength();
		TCHAR* pBuf = csBuffer.GetBuffer( nLen );
		TCHAR* pToken = _tcstok( pBuf, csDelim );
		while ( pToken )
		{
			nLines++;
			pToken = _tcstok( NULL, csDelim );
		}
		csBuffer.ReleaseBuffer();
		return nLines;
	}
	// number of lines
	__declspec( property( get = GetNumberOfLines ) )
		int NumberOfLines;

	// value of the text entered
	CString GetValue( LPCTSTR pcszLineToken = _T( "\n" ) )
	{
		CString csBuffer = m_csValue;
		CString csDelim = _T( "\r\n" );
		int nLen = csBuffer.GetLength();
		TCHAR* pBuf = csBuffer.GetBuffer( nLen );
		TCHAR* pToken = _tcstok( pBuf, csDelim );
		m_csValue.Empty();
		while ( pToken )
		{
			CString csLine = pToken;
			m_csValue += csLine;
			pToken = _tcstok( NULL, csDelim );
			if ( pToken != NULL )
			{
				m_csValue += pcszLineToken;
			}
		}
		csBuffer.ReleaseBuffer();
		return m_csValue;
	}

	// value of the text entered
	void SetValue
	( LPCTSTR lpszValue, LPCTSTR pcszLineToken = _T( "\r\n" )
	)
	{
		CString csBuffer = lpszValue;
		int nLen = csBuffer.GetLength();
		// parse sections separated by line feeds and append
		// to the value string separated by carriage return 
		// and line feed combination: \r\n (requirement of
		// the edit control)
		CString csDelim = pcszLineToken;
		if ( csDelim == _T( "\\n" ) )
		{
			int nPos = csBuffer.Find( csDelim );
			while ( nPos != -1 )
			{
				csBuffer.SetAt( nPos++, '\r' );
				csBuffer.SetAt( nPos++, '\n' );
				nPos = csBuffer.Find( csDelim, nPos );
			}
			m_csValue = csBuffer;
		} else
		{
			TCHAR* pBuf = csBuffer.GetBuffer( nLen );
			TCHAR* pToken = _tcstok( pBuf, csDelim );
			m_csValue.Empty();
			while ( pToken )
			{
				CString csLine = pToken;
				m_csValue += csLine;
				m_csValue += _T( "\r\n" );
				pToken = _tcstok( NULL, csDelim );
			}
			csBuffer.ReleaseBuffer();
		}

		UpdateData( FALSE );
	}

	// the dialog title
	CString GetTitle()
	{
		return m_csTitle;
	}
	// the dialog title
	void SetTitle( LPCTSTR pcszValue )
	{
		m_csTitle = pcszValue;
	}
	// the dialog title
	__declspec( property( get = GetTitle, put = SetTitle ) )
		CString Title;

	// set the upper left corner of the dialog
	void SetUpperLeftCorner( CPoint value )
	{
		m_ptUpperLeft = value;
	}
	// set the upper left corner of the dialog
	__declspec( property( put = SetUpperLeftCorner ) )
		CPoint UpperLeftCorner;

	// unique collection of short cuts
	CKeyedCollection<CString, CString>& GetShortcuts()
	{
		return m_mapShortcuts;
	}
	// unique collection of short cuts
	__declspec( property( get = GetShortcuts ) )
		CKeyedCollection<CString, CString>& Shortcuts;

// protected methods
protected:
	CString MapData();

	DECLARE_MESSAGE_MAP()

// public methods
public:

// protected overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);

// public overrides
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage( MSG* pMsg );

// public constructor/destructor
public:
	CEditShortcutDialog(CWnd* pParent = nullptr);
	virtual ~CEditShortcutDialog();

// public message handlers
public:
	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg void OnOk();
};

/////////////////////////////////////////////////////////////////////////////
