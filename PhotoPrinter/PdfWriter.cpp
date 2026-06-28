/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 by W. T. Block, All Rights Reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "PdfWriter.h"
#include <vector>
#include <string>

/////////////////////////////////////////////////////////////////////////////
CPdfWriter::CPdfWriter()
{
	m_bOpen = false;
	m_dpi = 400;

	m_objPages = 0; // always 1
	m_objCatalog = 0; // always 2
	m_objInfo = 0; // object #3
	m_nextObj = 0; // next free object number
	m_objOutlines = 0;

	m_offsets.push_back(0); // index 0 unused
}

/////////////////////////////////////////////////////////////////////////////
CPdfWriter::~CPdfWriter()
{
	if (m_bOpen)
		End();
}

/////////////////////////////////////////////////////////////////////////////
void CPdfWriter::WriteStr(const char* s)
{
	m_file.Write(s, (UINT)strlen(s));
}

/////////////////////////////////////////////////////////////////////////////
void CPdfWriter::WriteFmt(const char* fmt, ...)
{
	char buf[512];
	va_list args;
	va_start(args, fmt);
	int len = _vsnprintf_s(buf, _TRUNCATE, fmt, args);
	va_end(args);
	if (len > 0)
		m_file.Write(buf, (UINT)len);
}

/////////////////////////////////////////////////////////////////////////////
int CPdfWriter::NewObject()
{
	return m_nextObj++;
}

/////////////////////////////////////////////////////////////////////////////
void CPdfWriter::MarkOffset(int objNum)
{
	if ((int)m_offsets.size() <= objNum)
		m_offsets.resize((size_t)objNum + 1, 0);
	m_offsets[objNum] = m_file.GetPosition();
}

/////////////////////////////////////////////////////////////////////////////
void CPdfWriter::SetMetadata
(
	const CString& title,
	const CString& author,
	const CString& subject,
	const CString& keywords
)
{
	m_title = title;
	m_author = author;
	m_subject = subject;
	m_keywords = keywords;
	m_producer = L"PhotoPrinter PDF Engine";

	// Build PDF date string: D:YYYYMMDDHHmmSS
	SYSTEMTIME st;
	GetLocalTime(&st);

	m_creationDate.Format(
		L"D:%04d%02d%02d%02d%02d%02d",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);

	m_modDate = m_creationDate;
}

/////////////////////////////////////////////////////////////////////////////
bool CPdfWriter::Begin(const CString& pdfPath, int dpi)
{
	if (!m_file.Open(pdfPath,
		CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate | CFile::shareExclusive))
		return false;

	m_bOpen = true;
	m_dpi = dpi > 0 ? dpi : 72;

	m_offsets.clear();
	m_offsets.push_back(0);
	m_pageObjs.clear();

	m_nextObj = 1;

	// PDF header
	WriteStr("%PDF-1.4\n");
	WriteStr("%\xE2\xE3\xCF\xD3\n");

	// Reserve object numbers ONCE
	m_objPages = NewObject();   // 1
	m_objCatalog = NewObject();   // 2
	m_objInfo = NewObject();   // 3
	m_objOutlines = NewObject();

	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CPdfWriter::AddPageFromJpeg
(
	const CString& jpegPath, int pixelWidth, int pixelHeight
)
{
	if (!m_bOpen)
		return false;

	// Read JPEG
	CFile jpegFile;
	if (!jpegFile.Open(jpegPath, CFile::modeRead | CFile::shareDenyWrite))
		return false;

	DWORD jpegSize = (DWORD)jpegFile.GetLength();
	std::vector<BYTE> jpegData(jpegSize);
	if (jpegSize > 0)
		jpegFile.Read(jpegData.data(), jpegSize);
	jpegFile.Close();

	// Convert pixels → points
	double ptWidth = (double(pixelWidth) / m_dpi) * 72.0;
	double ptHeight = (double(pixelHeight) / m_dpi) * 72.0;

	// Image XObject
	int imgObj = NewObject();
	MarkOffset(imgObj);
	WriteFmt("%d 0 obj\n", imgObj);
	WriteStr("<< /Type /XObject\n");
	WriteStr("/Subtype /Image\n");
	WriteFmt("/Width %d\n", pixelWidth);
	WriteFmt("/Height %d\n", pixelHeight);
	WriteStr("/ColorSpace /DeviceRGB\n");
	WriteStr("/BitsPerComponent 8\n");
	WriteStr("/Filter /DCTDecode\n");
	WriteFmt("/Length %u\n", jpegSize);
	WriteStr(">>\nstream\n");
	if (jpegSize > 0)
		m_file.Write(jpegData.data(), jpegSize);
	WriteStr("\nendstream\nendobj\n");

	// Content stream
	char contentBuf[256];
	int contentLen = _snprintf_s(
		contentBuf, _TRUNCATE,
		"q\n%.2f 0 0 %.2f 0 0 cm\n/Im0 Do\nQ\n",
		ptWidth, ptHeight);

	int contentsObj = NewObject();
	MarkOffset(contentsObj);
	WriteFmt("%d 0 obj\n", contentsObj);
	WriteFmt("<< /Length %d >>\n", contentLen);
	WriteStr("stream\n");
	m_file.Write(contentBuf, contentLen);
	WriteStr("\nendstream\nendobj\n");

	// Page object
	int pageObj = NewObject();
	MarkOffset(pageObj);
	WriteFmt("%d 0 obj\n", pageObj);
	WriteStr("<< /Type /Page\n");
	WriteFmt("/Parent %d 0 R\n", m_objPages);
	WriteFmt("/MediaBox [0 0 %.2f %.2f]\n", ptWidth, ptHeight);
	WriteStr("/Resources << /XObject << /Im0 ");
	WriteFmt("%d 0 R >> >>\n", imgObj);
	WriteFmt("/Contents %d 0 R\n", contentsObj);
	WriteStr(">>\nendobj\n");

	m_pageObjs.push_back(pageObj);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CPdfWriter::AddPageFromJpegMemory
(
	const BYTE* data, size_t size,
	int pixelWidth, int pixelHeight
)
{
	if (!m_bOpen || data == nullptr || size == 0)
		return false;

	// Convert pixels → points
	double ptWidth = (double(pixelWidth) / m_dpi) * 72.0;
	double ptHeight = (double(pixelHeight) / m_dpi) * 72.0;

	// 1) Image XObject
	int imgObj = NewObject();
	MarkOffset(imgObj);
	WriteFmt("%d 0 obj\n", imgObj);
	WriteStr("<< /Type /XObject\n");
	WriteStr("/Subtype /Image\n");
	WriteFmt("/Width %d\n", pixelWidth);
	WriteFmt("/Height %d\n", pixelHeight);
	WriteStr("/ColorSpace /DeviceRGB\n");
	WriteStr("/BitsPerComponent 8\n");
	WriteStr("/Filter /DCTDecode\n");
	WriteFmt("/Length %u\n", (UINT)size);
	WriteStr(">>\nstream\n");
	m_file.Write(data, (UINT)size);
	WriteStr("\nendstream\nendobj\n");

	// 2) Content stream
	char contentBuf[256];
	int contentLen = _snprintf_s(
		contentBuf, _TRUNCATE,
		"q\n%.2f 0 0 %.2f 0 0 cm\n/Im0 Do\nQ\n",
		ptWidth, ptHeight);

	int contentsObj = NewObject();
	MarkOffset(contentsObj);
	WriteFmt("%d 0 obj\n", contentsObj);
	WriteFmt("<< /Length %d >>\n", contentLen);
	WriteStr("stream\n");
	m_file.Write(contentBuf, contentLen);
	WriteStr("\nendstream\nendobj\n");

	// 3) Page object
	int pageObj = NewObject();
	MarkOffset(pageObj);
	WriteFmt("%d 0 obj\n", pageObj);
	WriteStr("<< /Type /Page\n");
	WriteFmt("/Parent %d 0 R\n", m_objPages);
	WriteFmt("/MediaBox [0 0 %.2f %.2f]\n", ptWidth, ptHeight);
	WriteStr("/Resources << /XObject << /Im0 ");
	WriteFmt("%d 0 R >> >>\n", imgObj);
	WriteFmt("/Contents %d 0 R\n", contentsObj);
	WriteStr(">>\nendobj\n");

	m_pageObjs.push_back(pageObj);
	return true;
}

/////////////////////////////////////////////////////////////////////////////
bool CPdfWriter::End()
{
	if (!m_bOpen)
		return false;

	// Pages tree
	MarkOffset(m_objPages);
	WriteFmt("%d 0 obj\n", m_objPages);
	WriteStr("<< /Type /Pages\n/Kids [");
	for (int obj : m_pageObjs)
		WriteFmt("%d 0 R ", obj);
	WriteStr("]\n");
	WriteFmt("/Count %d\n", (int)m_pageObjs.size());
	WriteStr(">>\nendobj\n");

	// Info dictionary
	MarkOffset(m_objInfo);
	WriteFmt("%d 0 obj\n", m_objInfo);
	WriteStr("<<\n");

	CStringA tTitle = Utf8FromCString(m_title);
	CStringA tAuthor = Utf8FromCString(m_author);
	CStringA tSubject = Utf8FromCString(m_subject);
	CStringA tKeys = Utf8FromCString(m_keywords);
	CStringA tProd = Utf8FromCString(m_producer);
	CStringA tCDate = Utf8FromCString(m_creationDate);
	CStringA tMDate = Utf8FromCString(m_modDate);

	if (!tTitle.IsEmpty())
		WriteFmt("/Title (%s)\n", (LPCSTR)tTitle);
	if (!tAuthor.IsEmpty())
		WriteFmt("/Author (%s)\n", (LPCSTR)tAuthor);
	if (!tSubject.IsEmpty())
		WriteFmt("/Subject (%s)\n", (LPCSTR)tSubject);
	if (!tKeys.IsEmpty())
		WriteFmt("/Keywords (%s)\n", (LPCSTR)tKeys);

	WriteFmt("/Producer (%s)\n", (LPCSTR)tProd);
	WriteFmt("/CreationDate (%s)\n", (LPCSTR)tCDate);
	WriteFmt("/ModDate (%s)\n", (LPCSTR)tMDate);

	WriteStr(">>\nendobj\n");

	// ---- Outline items (flat list: one bookmark per page) ----
	std::vector<int> outlineObjs;

	for (size_t i = 0; i < m_bookmarks.size(); ++i)
	{
		const PdfBookmark& bm = m_bookmarks[i];

		if (bm.pageIndex < 0 || bm.pageIndex >= (int)m_pageObjs.size())
			continue;

		int pageObj = m_pageObjs[bm.pageIndex];

		int itemObj = NewObject();
		outlineObjs.push_back(itemObj);

		MarkOffset(itemObj);
		WriteFmt("%d 0 obj\n", itemObj);
		WriteStr("<<\n");

		CStringA tTitle = Utf8FromCString(bm.title);
		WriteFmt("/Title (%s)\n", (LPCSTR)tTitle);

		WriteFmt("/Parent %d 0 R\n", m_objOutlines);
		WriteFmt("/Dest [%d 0 R /Fit]\n", pageObj);

		// Prev: we already know the previous item’s object number
		if (outlineObjs.size() > 1)
			WriteFmt("/Prev %d 0 R\n", outlineObjs[outlineObjs.size() - 2]);

		// Next: the next outline item will use the current m_nextObj
		if (i + 1 < m_bookmarks.size())
		{
			int nextObj = m_nextObj;   // next object number to be assigned
			WriteFmt("/Next %d 0 R\n", nextObj);
		}

		WriteStr(">>\nendobj\n");
	}

	// ---- Outlines dictionary ----
	MarkOffset(m_objOutlines);
	WriteFmt("%d 0 obj\n", m_objOutlines);
	WriteStr("<< /Type /Outlines\n");

	if (!outlineObjs.empty())
	{
		WriteFmt("/First %d 0 R\n", outlineObjs.front());
		WriteFmt("/Last %d 0 R\n", outlineObjs.back());
	}

	WriteStr(">>\nendobj\n");

	// Catalog (object 2)
	MarkOffset(m_objCatalog);
	WriteFmt("%d 0 obj\n", m_objCatalog);
	WriteStr("<< /Type /Catalog\n");
	WriteFmt("/Pages %d 0 R\n", m_objPages);
	if (!m_bookmarks.empty())
		WriteFmt("/Outlines %d 0 R\n", m_objOutlines);
	WriteStr(">>\nendobj\n");

	// xref
	LONGLONG xrefPos = m_file.GetPosition();
	int objCount = m_nextObj;

	WriteFmt("xref\n0 %d\n", objCount);
	WriteStr("0000000000 65535 f \n");
	for (int i = 1; i < objCount; ++i)
		WriteFmt("%010lld 00000 n \n", m_offsets[i]);

	// single trailer with Root + Info
	WriteStr("trailer\n<<\n");
	WriteFmt("/Size %d\n", objCount);
	WriteFmt("/Root %d 0 R\n", m_objCatalog);
	WriteFmt("/Info %d 0 R\n", m_objInfo);
	WriteStr(">>\nstartxref\n");
	WriteFmt("%lld\n", xrefPos);
	WriteStr("%%EOF\n");

	m_file.Close();
	m_bOpen = false;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CPdfWriter::AddBookmark(const CString& title, int pageIndex)
{
	PdfBookmark bm;
	bm.title = title;
	bm.pageIndex = pageIndex;
	m_bookmarks.push_back(bm);
}

/////////////////////////////////////////////////////////////////////////////
