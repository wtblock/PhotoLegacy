/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "resource.h"
#include "CHelper.h"

/////////////////////////////////////////////////////////////////////////////
// CMultilineTextDialog
//
// Modal dialog used for editing multi‑line text fields within Photo Explorer.
// This dialog provides a controlled environment for entering long comments,
// descriptions, or other metadata fields that exceed the capabilities of
// single‑line edit controls. It supports configurable maximum length,
// customizable dialog titles, and flexible parsing of line breaks.
//
// Purpose:
//   • Provide a dedicated multi‑line text editor for EXIF fields such as
//     comments, descriptions, and user notes.
//   • Normalize line endings between UI controls and stored metadata.
//   • Allow callers to specify maximum text length to prevent oversized
//     metadata fields.
//   • Support custom dialog titles and positioning.
//
// Why this class exists:
//   Many EXIF fields (e.g., XPComment, ImageDescription, UserComment) can
//   contain multiple lines of text. Editing these fields inside a property
//   grid or single‑line control is impractical. CMultilineTextDialog offers
//   a clean, modal interface that ensures proper formatting, consistent
//   line‑break handling, and safe text retrieval.
//
// Responsibilities:
//   • Store and manage the multi‑line text value (m_csValue).
//   • Convert between UI line breaks (“\r\n”) and metadata line breaks (“\n”
//     or custom tokens).
//   • Count the number of lines entered (GetNumberOfLines).
//   • Enforce maximum length limits (MaxLength).
//   • Allow callers to set dialog title and screen position.
//   • Provide standard MFC dialog initialization and data exchange.
//
// Interaction with other components:
//   • PropertyGridMultilineText — invokes this dialog when editing multi‑line
//     metadata fields.
//   • CImageProperties — stores and retrieves the resulting text for EXIF tags.
//   • CHelper — assists with parsing and formatting when
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
