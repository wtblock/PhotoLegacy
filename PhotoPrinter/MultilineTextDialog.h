/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "resource.h"
#include "CHelper.h"

/////////////////////////////////////////////////////////////////////////////
// CMultilineTextDialog
//
// Modal dialog used to edit multi‑line text fields within the property grid.
// The standard CMFCPropertyGridProperty only supports single‑line editing,
// so PhotoPrinter uses this dialog to provide a comfortable, resizable,
// multi‑line editor for fields such as “Description” and “Query”.
//
// Purpose:
//   • Provide a dedicated multi‑line text editor invoked from
//     CPropertyGridMultilineText.
//   • Allow users to enter long descriptive text with proper line breaks,
//     scrolling, resizing, and TAB insertion.
//   • Convert between the property grid’s internal newline format and the
//     edit control’s CR/LF requirements.
//
// Why this class exists:
//   The MFC property grid cannot display multi‑line edit controls inline.
//   PhotoPrinter requires a clean, user‑friendly way to edit long text
//   metadata. This dialog solves that problem by presenting a full‑size
//   edit box with:
//       – Resizable layout
//       – Optional maximum length
//       – Custom title
//       – Correct newline handling
//       – TAB key insertion (instead of focus navigation)
//
// Responsibilities:
//   • Display and manage a multi‑line edit control (IDC_EDIT_COMMENT_TEXT).
//   • Convert newline tokens between “\n”, “\r\n”, and escaped “\\n” forms.
//   • Track and return the edited text via GetValue().
//   • Position the dialog near the property grid item that launched it.
//   • Support dynamic resizing of the edit box and OK/Cancel buttons.
//   • Allow TAB characters to be inserted directly into the text.
//
// Interaction with other components:
//   • Launched by CPropertyGridMultilineText::OnClickButton().
//   • Returns edited text to the property grid, which then updates
//     CPhotoPrinterDoc through the normal property‑change pipeline.
//   • Uses CHelper for string manipulation where needed.
//
// Wizard‑generated portions:
//   • Base dialog class (CDialog), DDX/DDV mapping, message map.
//
// Application‑specific additions:
//   • Newline conversion logic (SetValue / GetValue).
//   • TAB insertion override (PreTranslateMessage).
//   • Dynamic layout in OnSize().
//   • Title and position customization.
//   • Maximum length enforcement.
//
// This dialog provides a polished, user‑friendly multi‑line editor that
// integrates seamlessly with the property grid and greatly improves the
// metadata editing experience in PhotoPrinter.
/////////////////////////////////////////////////////////////////////////////
class CMultilineTextDialog : public CDialog
{
	DECLARE_DYNAMIC( CMultilineTextDialog )

public:
	// standard constructor
	CMultilineTextDialog( CWnd* pParent = NULL );
	virtual ~CMultilineTextDialog();

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
		CString csDelim = _T( "\n" );
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
	( 
		LPCTSTR lpszValue, 
		LPCTSTR pcszLineToken = _T( "\r\n" )
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
	}

	// set the dialog title
	void SetTitle( LPCTSTR pcszValue )
	{
		m_csTitle = pcszValue;
	}
	// set the dialog title
	__declspec( property( put = SetTitle ) )
		CString Title;

	// set the upper left corner of the dialog
	void SetUpperLeftCorner( CPoint value )
	{
		m_ptUpperLeft = value;
	}
	// set the upper left corner of the dialog
	__declspec( property( put = SetUpperLeftCorner ) )
		CPoint UpperLeftCorner;

	enum
	{
		IDD = IDD_DIALOG_COMMENT_TEXT
	};

protected:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

protected:
	CString m_csValue;
	int m_nMaxLength;
	CString m_csTitle;
	CPoint m_ptUpperLeft;

public:
	afx_msg void OnSize( UINT nType, int cx, int cy );
	virtual BOOL PreTranslateMessage( MSG* );
};
