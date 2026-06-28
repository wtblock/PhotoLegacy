/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "framework.h"
#include "CHelper.h"
#include "KeyedCollection.h"

/////////////////////////////////////////////////////////////////////////////
// copies fully qualified source folder to destination folder. 
// the source can be limited to an optional set of filenames
// and subfolders by default are included in the copy
int CHelper::ShellCopy
(
	// fully qualified source folder name 
	LPCTSTR pcszSource,
	// fully qualified destination folder name 
	LPCTSTR pcszDestination,
	// semicolon separated filename list (may use wild cards) 
	LPCTSTR pcszFiles,
	// true if moving instead of copying
	bool bMove,
	// title for progress dialog  
	LPCTSTR pcszTitle,
	// copy sub-folders 
	bool bSubfolders,
	// rename the file if it already exists in the destination folder
	bool bRenameOnCollision
)
{
	CString csSource(pcszSource);
	csSource.TrimRight(_T("\\"));
	csSource.MakeLower();
	CString csDestination(pcszDestination);
	csDestination.TrimRight(_T("\\"));
	csDestination.MakeLower();

	// make sure the folders are not the same
	if (csSource == csDestination)
	{
		return 0;
	}

	// if a list of files is given, the buffer becomes semicolon separated
	// list of fully qualified paths
	CString csBuffer;
	if (pcszFiles)
	{
		const CString csFiles(pcszFiles);
		const CString csDelim = _T(";");
		int nStart = 0;
		CString csToken = csFiles.Tokenize(csDelim, nStart);
		while (!csToken.IsEmpty())
		{
			CString csSrcFile(csSource);
			csSrcFile += _T("\\");
			csSrcFile += csToken;
			csBuffer += csSrcFile;
			csBuffer += _T(";");
			csToken = csFiles.Tokenize(csDelim, nStart);
		}
	}
	else // buffer is the source folder
	{
		csBuffer = csSource + _T(";");
	}

	// allow room for double null terminator
	// trailing semicolon will be replaced with null
	int nFrom, nFroms = csBuffer.GetLength() + 1;
	// room for two nulls
	int nTos = csDestination.GetLength() + 2;

	// allocate the from buffer
	vector<TCHAR> arrFrom(nFroms, 0);
	//TCHAR* pFrom = new TCHAR[ nFroms ];
	TCHAR* pFrom = &arrFrom[0];
	_tcscpy(pFrom, csBuffer); // will copy the null

	// replace semicolons with nulls
	for (nFrom = 0; nFrom < nFroms; nFrom++)
	{
		if (pFrom[nFrom] == ';')
		{
			pFrom[nFrom] = 0;
		}
	}

	// allocate the to buffer
	vector<TCHAR> arrTo(nTos, 0);
	//TCHAR* pTo = new TCHAR[ nTos ];
	TCHAR* pTo = &arrTo[0];
	_tcscpy(pTo, csDestination); // will copy the null

	// write second null
	pTo[nTos - 1] = 0;

	SHFILEOPSTRUCT sfo;
	::FillMemory(&sfo, sizeof sfo, 0);

	const UINT nNoRecursion =
		bSubfolders ? 0 : FOF_NORECURSION;
	const UINT nRenameOnCollision =
		bRenameOnCollision ? FOF_RENAMEONCOLLISION : 0;

	sfo.lpszProgressTitle = pcszTitle;
	sfo.pFrom = pFrom;
	sfo.pTo = pTo;
	sfo.wFunc = bMove ? FO_MOVE : FO_COPY;
	// Respond with "Yes to All" for any dialog box that is displayed. 
	sfo.fFlags =
		FOF_NOCONFIRMATION |
		// Do not confirm the creation of a new directory if the 
		// operation requires one to be created. 
		FOF_NOCONFIRMMKDIR |
		// do not handle subfolders
		nNoRecursion |
		// Display a progress dialog box but do 
		// not show the file names. 
		FOF_SIMPLEPROGRESS |
		// rename the file in the destination folder if there
		// is already a file in the folder with the same name
		nRenameOnCollision;

	const int nResult = SHFileOperation(&sfo);

	//delete pFrom;
	//delete pTo;

	return nResult;
} // ShellCopy

/////////////////////////////////////////////////////////////////////////////
int CHelper::ShellDelete
(
	LPCTSTR pcszSource, // fully qualified source folder name 
	// semicolon separated filename list (may use wild cards) 
	LPCTSTR pcszFiles,
	// title for progress dialog  
	LPCTSTR pcszTitle,
	bool bSubfolders // copy sub-folders 
)
{
	CString csSource(pcszSource);
	if (!::PathFileExists(csSource))
	{
		return 0;
	}
	csSource.TrimRight(_T("\\"));
	csSource.MakeLower();

	// if a list of files is given, the buffer becomes semicolon separated
	// list of fully qualified paths
	CString csBuffer;
	if (pcszFiles)
	{
		const CString csFiles(pcszFiles);
		const CString csDelim = _T(";");
		int nStart = 0;
		CString csToken = csFiles.Tokenize(csDelim, nStart);
		while (!csToken.IsEmpty())
		{
			CString csSrcFile(csSource);
			csSrcFile += _T("\\");
			csSrcFile += csToken;
			csBuffer += csSrcFile;
			csBuffer += _T(";");
			csToken = csFiles.Tokenize(csDelim, nStart);
		}
	}
	else // buffer is the source folder
	{
		csBuffer = csSource + _T(";");
	}

	// allow room for double null terminator
	// trailing semicolon will be replaced with null
	int nFrom, nFroms = csBuffer.GetLength() + 2;

	// allocate the from buffer
	vector<TCHAR> arrFrom(nFroms, 0);
	//TCHAR* pFrom = new TCHAR[ nFroms ];
	TCHAR* pFrom = &arrFrom[0];
	_tcscpy(pFrom, csBuffer); // will copy the null

	// replace semicolons with nulls
	for (nFrom = 0; nFrom < nFroms; nFrom++)
	{
		if (pFrom[nFrom] == ';')
		{
			pFrom[nFrom] = 0;
		}
	}

	SHFILEOPSTRUCT sfo;
	::FillMemory(&sfo, sizeof sfo, 0);

	const UINT nNoRecursion = bSubfolders ? 0 : FOF_NORECURSION;

	sfo.lpszProgressTitle = pcszTitle;
	sfo.pFrom = pFrom;
	sfo.pTo = 0;
	sfo.wFunc = FO_DELETE;
	// Respond with "Yes to All" for any dialog box 
	// that is displayed. 
	sfo.fFlags = FOF_NOCONFIRMATION |
		// copy deleted items to recycle bin
		FOF_ALLOWUNDO |
		nNoRecursion |
		// Display a progress dialog box but do not 
		// show the file names. 
		FOF_SIMPLEPROGRESS;

	const int nResult = SHFileOperation(&sfo);

	return nResult;
} // ShellDelete

/////////////////////////////////////////////////////////////////////////////
CString CHelper::FormatDate(CString csDate)
{
	CKeyedCollection<CString, int> months = CHelper::Months();

	CString value;
	int nStart = 0;
	vector<CString> arrTokens;
	CString csToken = csDate.Tokenize(L", ", nStart);
	while (!csToken.IsEmpty())
	{
		arrTokens.push_back(csToken);
		csToken = csDate.Tokenize(L", ", nStart);
	}

	int nYear = 0;
	int nDay = 1;
	int nMonth = 1;

	for (auto& node : arrTokens)
	{
		node.MakeLower();
		if (months.Exists[node.Left(3)])
		{
			nMonth = *months.find(node.Left(3));
			continue;
		}
		int nValue = _tstoi(node);

		// test for invalid year or day
		if (nValue == 0)
		{
			break;
		}
		if (nValue > 31)
		{
			nYear = nValue;

		}
		else
		{
			nDay = nValue;
		}
	}

	if (nYear != 0)
	{
		value.Format(L"%04d:%02d:%02d 00:00:00", nYear, nMonth, nDay);
	}

	return value;
} // FormatDate

/////////////////////////////////////////////////////////////////////////////
// if the line of text ends in a date, return a string representing that
// date in the form of:
//		YYYY.MM.DD		if all three are present
//		YYYY.MM			if only year and month are present
//		YYYY			if only the year is present
//		empty			if none are present
// Date endings can by like this:
//		2024			year only
//		April 2024		month and year
//		April 18, 2024	Month, day, and year
//
CString CHelper::EndDate(CString csLine)
{
	CString value;
	int nYear = 0, nMonth = 0, nDay = 0;
	vector<CString> arrWords = Words(csLine);
	int nWords = (int)arrWords.size();
	if (nWords > 1)
	{
		int nIndex = nWords - 1;
		CString csYear = arrWords[nIndex--].TrimRight(L".");
		nYear = _tstoi(csYear);
		if (nYear > 1000)
		{
			value += csYear;
			if (nIndex >= 0)
			{
				CString csDay = arrWords[nIndex--].TrimRight(L".,");
				nDay = _tstoi(csDay);
				if (nDay == 0)
				{
					nMonth = Month(csDay);
					if (nMonth != 0)
					{
						value.Format(L"%4d.%02d", nYear, nMonth);
						return value;

					}
					else
					{
						return value;
					}
				}
				else // nDay != 0
				{
					if (nIndex >= 0)
					{
						CString csMonth = arrWords[nIndex--];
						nMonth = Month(csMonth);
						if (nMonth != 0)
						{
							value.Format(L"%4d.%02d.%02d", nYear, nMonth, nDay);
						}
					}
				}
			}
		}
	}

	return value;
}

/////////////////////////////////////////////////////////////////////////////
CString CHelper::GetRootFolder()
{
	CString root = GetCurrentDirectory();

	return root;
}

/////////////////////////////////////////////////////////////////////////////
CString CHelper::NormalizeSlashes(const CString& path)
{
	CString s(path);
	s.Replace(L"/", L"\\");
	while (s.Replace(L"\\\\", L"\\")) {}  // collapse double slashes
	return s;
}

/////////////////////////////////////////////////////////////////////////////
CString CHelper::ToRelative(const CString& absPath)
{
	CString root = GetCurrentDirectory();      // always ends with "\"
	CString normAbs = NormalizeSlashes(absPath);

	// If the absolute path begins with the root, convert to .\ form
	if (normAbs.Left(root.GetLength()).CompareNoCase(root) == 0)
		return L".\\" + normAbs.Mid(root.GetLength());

	// Otherwise return the normalized absolute path unchanged
	return normAbs;
}

/////////////////////////////////////////////////////////////////////////////
CString CHelper::ToAbsolute(const CString& relPath)
{
	CString norm = NormalizeSlashes(relPath);

	// If it begins with .\ then prepend the root folder
	if (norm.Left(2) == L".\\")
		return GetCurrentDirectory() + norm.Mid(2);

	// If it already looks absolute (C:\ or \\server\share), return as-is
	if (norm.GetLength() > 1 && norm[1] == L':')
		return norm;

	if (norm.Left(2) == L"\\\\")
		return norm;

	// Otherwise treat it as relative to the root
	return GetCurrentDirectory() + norm;
}

/////////////////////////////////////////////////////////////////////////////
BOOL CHelper::PathExists(const CString& path)
{
	DWORD attr = GetFileAttributes(path);
	return (attr != INVALID_FILE_ATTRIBUTES);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CHelper::GetFileTimestampAndSize
(
	const CString& path,
	uint64_t& timestamp,
	uint64_t& size
)
{
	WIN32_FILE_ATTRIBUTE_DATA data;
	if (!GetFileAttributesEx(path, GetFileExInfoStandard, &data))
		return FALSE;

	ULARGE_INTEGER li;
	li.LowPart = data.ftLastWriteTime.dwLowDateTime;
	li.HighPart = data.ftLastWriteTime.dwHighDateTime;
	timestamp = li.QuadPart;

	ULARGE_INTEGER sz;
	sz.LowPart = data.nFileSizeLow;
	sz.HighPart = data.nFileSizeHigh;
	size = sz.QuadPart;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
CString CHelper::StripPunctuation(const CString& s)
{
	CString out;
	for (int i = 0; i < s.GetLength(); i++)
	{
		WCHAR c = s[i];
		if (iswalnum(c) || c == L' ')
			out += c;
	}
	return out;
}

/////////////////////////////////////////////////////////////////////////////
void CHelper::NormalizeToken(CString& s)
{
	s.MakeLower();
	s.Trim();

	int len = s.GetLength();
	if (len == 0)
		return;

	// 1) Possessive handling: trailing 's or trailing '
	len = s.GetLength();
	if (len >= 2 && s[len - 2] == L'\'' && s[len - 1] == L's')
	{
		s = s.Left(len - 2);          // block's → block
	}
	else if (len >= 1 && s[len - 1] == L'\'')
	{
		s = s.Left(len - 1);          // grimes' → grimes
	}

	// 2) Leading apostrophe (nickname quotes): 'bob' → bob
	len = s.GetLength();
	if (len >= 1 && s[0] == L'\'')
	{
		s = s.Mid(1);
	}

	// 3) Strip other leading/trailing punctuation, but keep internal apostrophes
	while (!s.IsEmpty())
	{
		WCHAR c = s[0];
		if (iswalnum(c) || c == L'\'')
			break;
		s.Delete(0);                  // drop leading .,,"() etc.
	}

	while (!s.IsEmpty())
	{
		int last = s.GetLength() - 1;
		WCHAR c = s[last];
		if (iswalnum(c) || c == L'\'')
			break;
		s.Delete(last);               // drop trailing .,,"() etc.
	}

	s.Trim();
}

/////////////////////////////////////////////////////////////////////////////
void CHelper::Tokenize(const CString& comment, std::vector<CString>& tokens)
{
	CString clean = comment;
	clean.MakeLower();

	// space, hyphen, period, semicolon, and colon are separators
	const CString csDelim = _T(" -,.;:");

	int nStart = 0;
	CString tok = clean.Tokenize(csDelim, nStart);

	while (!tok.IsEmpty())
	{
		tok.Trim();
		if (!tok.IsEmpty())
			tokens.push_back(tok);

		tok = clean.Tokenize(csDelim, nStart);
	}
}

/////////////////////////////////////////////////////////////////////////////
// read the XP comment (tab 0x9C9C) from an image
CString CHelper::GetXPComment(Gdiplus::Image* pImage)
{
	if (!pImage)
		return CString();

	const PROPID XPCommentTag = 0x9C9C;

	UINT size = pImage->GetPropertyItemSize(XPCommentTag);
	if (size == 0)
		return CString();

	std::vector<BYTE> buffer(size);
	Gdiplus::PropertyItem* pItem =
		reinterpret_cast<Gdiplus::PropertyItem*>(buffer.data());

	if (pImage->GetPropertyItem(XPCommentTag, size, pItem) != Gdiplus::Ok)
		return CString();

	const WCHAR* pwsz = reinterpret_cast<const WCHAR*>(pItem->value);
	UINT wcharCount = pItem->length / sizeof(WCHAR);

	return CString(pwsz, wcharCount);
}

/////////////////////////////////////////////////////////////////////////////
CString CHelper::GetXPCommentFromFile(const CString& absPath)
{
	CString csError;
	std::shared_ptr<Gdiplus::Image> pImage =
		CHelper::LoadImageFromFile(absPath, csError);

	if (!pImage)
		return CString();

	return GetXPComment(pImage.get());
}

/////////////////////////////////////////////////////////////////////////////
