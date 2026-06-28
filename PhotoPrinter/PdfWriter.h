/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>

class CPdfWriter
{
public:
	CPdfWriter();
	~CPdfWriter();

	bool Begin(const CString& pdfPath, int dpi);
	bool AddPageFromJpeg(const CString& jpegPath, int pixelWidth, int pixelHeight);
	bool AddPageFromJpegMemory(const BYTE* data, size_t size,
		int pixelWidth, int pixelHeight);
	bool End();
	void SetMetadata
	(
		const CString& title,
		const CString& author,
		const CString& subject,
		const CString& keywords
	);

	static CStringA Utf8FromCString(const CString& s)
	{
		if (s.IsEmpty())
			return CStringA();

		int len = ::WideCharToMultiByte
		(
			CP_UTF8, 0,
			s, -1,
			nullptr, 0,
			nullptr, nullptr
		);

		if (len <= 1)
			return CStringA();

		CStringA out;
		LPSTR pBuf = out.GetBuffer(len - 1);
		::WideCharToMultiByte
		(
			CP_UTF8, 0,
			s, -1,
			pBuf, len - 1,
			nullptr, nullptr
		);
		out.ReleaseBuffer(len - 1);

		return out;
	}
	void AddBookmark(const CString& title, int pageIndex);

private:
	CFile m_file;
	bool  m_bOpen;
	int   m_dpi;

	int m_objPages; // always 1
	int m_objCatalog; // always 2
	int m_objInfo; // object #3
	int m_nextObj; // next free object number
	int m_objOutlines;

	std::vector<LONGLONG> m_offsets;   // byte offsets for xref
	std::vector<int>      m_pageObjs;  // list of page object numbers

	CString m_title;
	CString m_author;
	CString m_subject;
	CString m_keywords;
	CString m_producer;
	CString m_creationDate;
	CString m_modDate;

	void WriteStr(const char* s);
	void WriteFmt(const char* fmt, ...);
	void MarkOffset(int objNum);
	int  NewObject();

	struct PdfBookmark
	{
		CString title;
		int pageIndex;
	};
	std::vector<PdfBookmark> m_bookmarks;
};
