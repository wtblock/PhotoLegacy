/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// CPdfWriter
//
// Minimal, self‑contained PDF 1.4 writer used by PhotoPrinter to export
// high‑DPI page images. This class constructs valid PDF files entirely
// through manual object creation, cross‑reference tracking, and dictionary
// assembly—without relying on external PDF libraries.
//
// Purpose:
//   • Export each rendered PhotoPrinter page as a JPEG‑based PDF page.
//   • Provide predictable, library‑free PDF generation suitable for
//     long‑term maintenance and debugging.
//   • Support metadata (Title, Author, Subject, Keywords).
//   • Support PDF bookmarks (Outlines) for album navigation.
//   • Produce compact, standards‑compliant PDF 1.4 output.
//
// Why this class exists:
//   PDF libraries are large, complex, and often introduce heavy dependencies.
//   PhotoPrinter needs only a small subset of PDF functionality:
//       – A catalog
//       – A pages tree
//       – Page objects containing JPEG XObjects
//       – Content streams that draw the image
//       – Metadata (Info dictionary)
//       – Optional bookmarks
//       – A correct xref table and trailer
//
//   CPdfWriter implements exactly these components—no more, no less—making
//   the PDF export pipeline fast, deterministic, and easy to debug.
//
// Responsibilities:
//   • Open and manage the output file.
//   • Assign object numbers and track byte offsets for xref generation.
//   • Write PDF dictionaries, streams, and structural objects.
//   • Convert JPEG pixel dimensions → PDF point dimensions using DPI.
//   • Embed JPEG data as /XObject streams using /DCTDecode.
//   • Generate content streams that draw the image at the correct size.
//   • Build the /Pages tree and /Catalog.
//   • Write metadata into the /Info dictionary.
//   • Build a flat bookmark list under /Outlines.
//   • Write the xref table and trailer.
//
// Key Concepts:
//   • Object numbers begin at 1; object 0 is the free object.
//   • m_offsets[objNum] stores the byte offset of each object.
//   • m_pageObjs stores the object numbers of all page objects.
//   • JPEGs are embedded directly using /Filter /DCTDecode.
//   • Coordinates are in PDF points (1/72 inch).
//   • DPI determines scaling from pixels → points.
//   • Bookmarks reference page objects via /Dest [pageObj 0 R /Fit].
//
// Usage:
//   CPdfWriter pdf;
//   pdf.Begin(path, dpi);
//   pdf.SetMetadata(...);
//   pdf.AddPageFromJpegMemory(...);  // or AddPageFromJpeg()
//   pdf.AddBookmark("Album Name", pageIndex);
//   pdf.End();
//
// This class provides a compact, reliable PDF export engine tailored
// specifically for PhotoPrinter’s high‑DPI page rendering pipeline.
/////////////////////////////////////////////////////////////////////////////
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
