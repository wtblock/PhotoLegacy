/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include "CHelper.h"

/////////////////////////////////////////////////////////////////////////////
// CEditShortcutDialog
//
// Dialog used to edit and manage text-expansion shortcuts for metadata
// entry. Shortcuts allow the user to type compact tokens (e.g., "@bill")
// which automatically expand into full, canonical names such as
// "William Theodore 'Bill' Block III". This ensures consistent naming
// across thousands of photographs without requiring repetitive typing.
//
// Purpose:
//   • Provide a dedicated editor for defining and modifying shortcut
//     expansions used throughout Photo Explorer and Photo Printer.
//   • Allow multi-line editing of shortcut definitions using a rich edit
//     control.
//   • Store shortcuts in a keyed collection for fast lookup and expansion.
//   • Support custom dialog positioning, maximum text length, and dynamic
//     resizing.
//
// Why this class exists:
//   Metadata consistency is critical when organizing large family photo
//   archives. Without shortcuts, long formal names would be error-prone
//   and inconsistently typed. This dialog provides a simple, intuitive
//   interface for defining shortcuts that the metadata editor can expand
//   automatically whenever the user types “@shortcut” in any text field.
//
// Responsibilities:
//   • Display and manage a CRichEditCtrl for editing shortcut definitions.
//   • Maintain a unique collection of shortcuts (key → expansion).
//   • Convert between internal newline formats and CRichEditCtrl’s CR/LF
//     requirements (SetValue / GetValue).
//   • Support dialog customization:
//       – Title (Title property)
//       – Maximum length (MaxLength property)
//       – Initial position (UpperLeftCorner property)
//   • Provide multi-line parsing and reconstruction of shortcut text.
//   • Handle TAB insertion, resizing, and OK/Cancel behavior.
//
// Interaction with other components:
//   • Shortcut definitions are consumed by metadata-editing controls
//     throughout Photo Explorer and Photo Printer.
//   • The dialog is typically launched from a “Manage Shortcuts” command
//     in the Properties pane or metadata editor.
//   • The resulting shortcut map is stored in the application and used
//     whenever the user types “@name” in any metadata field.
//
// Wizard-generated portions:
//   • Base dialog class (CDialogEx), DDX/DDV mapping, message map.
//   • Basic dialog creation and initialization.
//
// Application-specific additions:
//   • Rich edit control for multi-line shortcut editing.
//   • Shortcut collection using CKeyedCollection.
//   • Newline conversion logic for CRichEditCtrl compatibility.
//   • TAB insertion override in PreTranslateMessage.
//   • Dynamic resizing of edit control and buttons.
//   • Custom dialog positioning and title handling.
//
// This dialog provides a clean, efficient way to maintain naming shortcuts,
// ensuring consistent metadata across all albums and greatly speeding up
// the process of labeling photographs.
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
