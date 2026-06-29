/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <type_traits>
#include "comutil.h"
#include "Date.h"
#include <vector>
#include <gdiplus.h>
#include <memory>
#include <string>
#include <atlstr.h>

using namespace std;
using namespace Gdiplus;

class CHelper
{
public:
	// PI
	static double GetPI()
	{
		return 3.1415926535897932384626433832795;
	}
	// PI
	__declspec(property(get = GetPI))
		double PI;

	// degrees from radians
	static double GetDegrees(double dRadians)
	{
		return 180.0 / GetPI() * dRadians;
	}
	// degrees from radians
	__declspec(property(get = GetDegrees))
		double Degrees[];

	// radians from degrees
	static double GetRadians(double dDegrees)
	{
		return GetPI() / 180.0 * dDegrees;
	}
	// radians from degrees
	__declspec(property(get = GetRadians))
		double Radians[];
	/*
	/////////////////////////////////////////////////////////////////////////////
	 parsing command lines has some rules that user's may find difficult to
	 follow, for example:

			SetDateTaken "c:\temp\camera roll\" 1980 9 6

	 will fail because the double quote (") preceded by a backslash will
	 generate a literal double quote. This will cause the rest of the
	 arguments to be included in the second argument as a single string.
	 The double quotes around the pathname are required because the path
	 has white space (spaces or tabs) in it which would ordinarily break
	 the string up into multiple arguments.

	 The correct way to enter these arguments is:

			SetDateTaken "c:\temp\camera roll\\" 1980 9 6

	 where the double backslash will generate a single backslash to
	 prevent the generation of a literal double quote.

	 This method addresses that specific scenario because in my mind
	 the rules seem to be too obtuse and likely to be overlooked by
	 the average user (I have made that mistake myself).
	*/
	static vector<CString> CorrectedCommandLine(int argc, TCHAR* argv[])
	{
		// create a vector of command line arguments, which is done
		// to allow for handling of common command line argument
		// errors
		vector<CString> value;

		// copy the original arguments
		for (int arg = 0; arg < argc; arg++)
		{
			value.push_back(argv[arg]);
		}

		// a common error when passing in pathnames is to end
		// the pathname with a single backslash and a double quote
		// which will result in all of the arguments being lumped
		// into a single string
		size_t nArgs = value.size();

		// the first argument is the executable path and the 
		// second argument will be the parameters passed
		// to the executable
		if (nArgs == 2)
		{
			CString arg1 = value[1];
			const int nPos = arg1.Find(_T("\""));
			if (nPos != -1)
			{
				// replace the double quote with a backslash
				// because of a common escape sequence error of
				// placing a double quote after a single 
				// backslash in a pathname
				arg1.SetAt(nPos, '\\');

				// parse arg1 to create the new arguments
				value.resize(1);
				value.push_back(arg1.Left(nPos + 1));

				// start parsing after the first argument
				int nStart = nPos + 1;

				// parse the remainder of the command line with whitespace
				// delimiters
				do
				{
					const CString csToken = arg1.Tokenize(_T(" \t"), nStart);
					if (csToken.IsEmpty())
					{
						break;
					}

					value.push_back(csToken);

				} while (true);

				// recalculate the number of arguments
				nArgs = value.size();
			}
		}

		// return the arguments as a vector of strings which may be the 
		// original values or a corrected set of values.
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// return true if the given value is an even number
	template <class T> static inline bool GetEven(T input)
	{
		const bool value = (input % 2 == 0);
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// return true if the given value is an odd number
	template <class T> static inline bool GetOdd(T input)
	{
		const bool value = (input % 2 != 0);
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	template <class T> static inline void Message(LPCTSTR text, T number)
	{
		CString csMessage;
		csMessage.Format(_T("%s: %d\n"), text, number);
		TRACE(csMessage);
	}

	/////////////////////////////////////////////////////////////////////////////
	static inline void ErrorMessage(LPCTSTR file, int line)
	{
		CString csMessage;
		csMessage.Format(_T("Exception in: %s\non line: %d\n"), file, line);
		TRACE(csMessage);
	}

	/////////////////////////////////////////////////////////////////////////////
	// parse the filename from a pathname
	static inline CString GetFileName(LPCTSTR pcszPath)
	{
		CString csPath(pcszPath);
		TCHAR* pBuf = csPath.GetBuffer(csPath.GetLength() + 1);
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szFile[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];

		_tsplitpath(pBuf, szDrive, szDir, szFile, szExt);
		csPath.ReleaseBuffer();
		return szFile;
	}

	/////////////////////////////////////////////////////////////////////////////
	// parse the extension from a pathname
	static inline CString GetExtension(LPCTSTR pcszPath)
	{
		CString csPath(pcszPath);
		TCHAR* pBuf = csPath.GetBuffer(csPath.GetLength() + 1);
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szFile[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];

		_tsplitpath(pBuf, szDrive, szDir, szFile, szExt);
		csPath.ReleaseBuffer();
		return szExt;
	}

	/////////////////////////////////////////////////////////////////////////////
	// parse the directory from a pathname
	static inline CString GetDirectory(LPCTSTR pcszPath)
	{
		CString csPath(pcszPath);
		TCHAR* pBuf = csPath.GetBuffer(csPath.GetLength() + 1);
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szFile[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];

		_tsplitpath(pBuf, szDrive, szDir, szFile, szExt);
		csPath.ReleaseBuffer();
		return szDir;
	}

	/////////////////////////////////////////////////////////////////////////////
	// parse the drive from a pathname
	static inline CString GetDrive(LPCTSTR pcszPath)
	{
		CString csPath(pcszPath);
		TCHAR* pBuf = csPath.GetBuffer(csPath.GetLength() + 1);
		TCHAR szDrive[_MAX_DRIVE];
		TCHAR szDir[_MAX_DIR];
		TCHAR szFile[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];

		_tsplitpath(pBuf, szDrive, szDir, szFile, szExt);
		csPath.ReleaseBuffer();
		return szDrive;
	}

	/////////////////////////////////////////////////////////////////////////////
	// parse folder from a pathname (drive and directory)
	static inline CString GetFolder(LPCTSTR pcszPath)
	{
		CString csDrive = GetDrive(pcszPath);
		CString csDir = GetDirectory(pcszPath);
		return csDrive + csDir;
	}

	/////////////////////////////////////////////////////////////////////////////
	// parse data name from a pathname (filename and extension)
	static inline CString GetDataName(LPCTSTR pcszPath)
	{
		CString csFile = GetFileName(pcszPath);
		CString csExt = GetExtension(pcszPath);
		return csFile + csExt;
	}

	/////////////////////////////////////////////////////////////////////////////
	// returns the application's current directory
	static inline CString GetCurrentDirectory()
	{
		TCHAR pBuffer[_MAX_PATH];
		DWORD len = ::GetCurrentDirectory(_MAX_PATH, pBuffer);

		CString value;

		if (len > 0)
		{
			value = pBuffer;

			if (value.Right(1) != _T("\\"))
				value += _T("\\");
		}

		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// This function creates a file system folder whose fully qualified 
	// path is given by pszPath. If one or more of the intermediate 
	// folders do not exist, they will be created as well. 
	// returns true if the path is created or already exists
	static inline bool CreatePath(LPCTSTR pszPath)
	{
		if (ERROR_SUCCESS == SHCreateDirectoryEx(NULL, pszPath, NULL))
		{
			return true;
		}

		return false;
	} // CreatePath

	/////////////////////////////////////////////////////////////////////////////
	// compare two reals and determine if they are nearly equal 
	// (within the given error range)
	template <class T> static inline bool NearlyEqual
	(
		T value1, T value2, T error = T(0.0001)
	)
	{
		const T diff = (T)fabs((double)value1 - (double)value2);
		return diff < error;
	}

	/////////////////////////////////////////////////////////////////////////////
	// floating point less than will return true if value1 is 
	// less than value2
	template <class T> static inline bool LessThan
	(
		T value1, T value2, T error = T(0.00001)
	)
	{	// check for nearly equal first
		const bool bEqual = NearlyEqual(value1, value2, error);
		if (bEqual)
		{
			return false;
		}
		// if not nearly equal do the normal compare
		if (value1 < value2)
		{
			return true;
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////
	// floating point less than or equal will return true if value1 is 
	// less than or equal to value2
	template <class T> static inline bool LessThanOrEqual
	(
		T value1, T value2, T error = T(0.00001)
	)
	{	// check for nearly equal first
		const bool bEqual = NearlyEqual(value1, value2, error);
		if (bEqual)
		{
			return true;
		}
		// if not nearly equal do the normal compare
		if (value1 <= value2)
		{
			return true;
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////
	// floating point greater than or equal will return true if value1 is 
	// greater than or equal to value2
	template <class T> static inline bool GreaterThanOrEqual
	(
		T value1, T value2, T error = T(0.00001)
	)
	{	// check for nearly equal first
		const bool bEqual = NearlyEqual(value1, value2, error);
		if (bEqual)
		{
			return true;
		}
		// if not nearly equal do the normal compare
		if (value1 >= value2)
		{
			return true;
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////
	// floating point greater than will return true if value1 is greater than 
	// value2
	template <class T> static inline bool GreaterThan
	(
		T value1, T value2, T error = T(0.00001)
	)
	{	// check for nearly equal first
		const bool bEqual = NearlyEqual(value1, value2, error);
		if (bEqual)
		{
			return false;
		}
		// if not nearly equal do the normal compare
		if (value1 > value2)
		{
			return true;
		}
		return false;
	}

	/////////////////////////////////////////////////////////////////////////////
	// Round value to nearest multiple of nearest (ie. if nearest is 0.1
	// round to the nearest tenth or if nearest is 5, round to the nearest
	// multiple of five).  If second parameter is not supplied, value is 
	// rounded to the nearest whole number 
	template <class T> static inline T RoundToNearest
	(
		T value, T nearest = T(1)
	)
	{
		if (nearest == 0)
		{
			nearest = 1;
		}
		nearest = fabs(nearest); // positive number
		value /= nearest;
		value = value < 0 ? ceil(value - T(0.5)) : floor(value + T(0.5));
		value *= nearest;
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// method to calculate the maximum value of the array
	// taking into account the given null value
	template <class T> static T Maximum
	(
		T null, // value's empty or missing value
		T defaultValue, // default to return if there are no non-null values
		const vector<T>& arrValues // values to operate on
	)
	{
		T result = null;
		bool bSet = false;

		for each (T value in arrValues)
		{
			if (NearlyEqual(value, null))
			{
				continue;
			}
			// update the result
			if (bSet)
			{
				result = max(result, value);
			}
			else // never assigned a value
			{
				result = value;
				bSet = true;
			}
		}
		if (!bSet)
		{
			result = defaultValue;
		}
		return result;
	}

	/////////////////////////////////////////////////////////////////////////////
	// method to calculate the minimum value of the array
	// taking into account the given null value
	template <class T> static T Minimum
	(
		T null, // value's empty or missing value
		T defaultValue, // default to return if there are no non-null values
		const vector<T>& arrValues // values to operate on
	)
	{
		T result = null;
		bool bSet = false;

		for each (T value in arrValues)
		{
			if (NearlyEqual(value, null))
			{
				continue;
			}
			// update the result
			if (bSet)
			{
				result = min(result, value);
			}
			else // never assigned a value
			{
				result = value;
				bSet = true;
			}
		}
		if (!bSet)
		{
			result = defaultValue;
		}
		return result;
	}

	/////////////////////////////////////////////////////////////////////////////
	// method to calculate the average value of the array
	// taking into account the given null value
	template <class T> static T Average
	(
		T null, // values's empty or missing value
		T defaultValue, // default to return if there are no non-null values
		const vector<T>& arrValues // values to operate on
	)
	{
		T result = null;
		bool bSet = false;
		T count = 0;
		const int nValues = arrValues.size();

		for each (T value in arrValues)
		{
			if (NearlyEqual(value, null))
			{
				continue;
			}
			// update the result
			if (bSet)
			{
				result += value;
			}
			else // never assigned a value
			{
				result = value;
				bSet = true;
			}
			count++;
		}
		if (!bSet)
		{
			result = defaultValue;
		}
		else
		{
			result /= count;
		}
		return result;
	}

	/////////////////////////////////////////////////////////////////////////////
	// get the number of days in a month given the year and month number (1 to 12)
	static inline int GetNumberOfDays(int nYear, int nMonth)
	{
		int value = 31;
		switch (nMonth)
		{
		case 2: // feb
		{
			value = 28;
			const int nMod = nYear % 4;
			if (nMod == 0)
			{
				value++;
			}
			break;
		}
		case 4: // apr
		case 6: // jun
		case 9: // sep
		case 11: // nov
		{
			value = 30;
			break;
		}
		}
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// get temperature in degrees fahrenheit from Celsius value and an empty value
	inline static float GetFahrenheit(float fCelsius, float fEmpty)
	{
		// return value
		float value = fCelsius;

		if (!NearlyEqual(value, fEmpty))
		{
			value *= 1.8f;
			value += 32.0f;
		}

		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// convert a variant input into given type and return true if successful
	template <class T> static inline bool Convert
	(
		_variant_t input, // input variant
		VARENUM vt, // output variant being requested 
		T& value // return value
	)
	{
		value = 0;
		bool bValue = false;

		if (SUCCEEDED(::VariantChangeType(&input, &input, 0, vt)))
		{
			bValue = true;
		}
		else // we are done
		{
			return bValue;
		}

		// convert the numeric value to the data type given
		switch (vt)
		{
		case VT_I1: value = (T)input.cVal; break;
		case VT_UI1: value = (T)input.bVal; break;
		case VT_I2: value = (T)input.iVal; break;
		case VT_UI2: value = (T)input.uiVal; break;
		case VT_I4: value = (T)input.lVal; break;
		case VT_UI4: value = (T)input.ulVal; break;
		case VT_I8: value = (T)input.llVal; break;
		case VT_UI8: value = (T)input.ullVal; break;
		case VT_R4: value = (T)input.fltVal; break;
		case VT_DATE: // dates are doubles
		case VT_R8: value = (T)input.dblVal; break;
		case VT_BSTR: value = (T)input.bstrVal; break;
		default: bValue = false;
		}

		return bValue;
	}

	/////////////////////////////////////////////////////////////////////////////
	// safe array (one dimensional) size in elements
	static int GetOneDimensionalElementCount(const VARIANT& data)
	{
		long nUBound, nLBound;
		::SafeArrayGetLBound(data.parray, 1, &nLBound);
		::SafeArrayGetUBound(data.parray, 1, &nUBound);
		return (nUBound + 1 - nLBound);
	}

	/////////////////////////////////////////////////////////////////////////////
	// safe array (one dimensional) size in bytes
	static int GetOneDimensionalSize(const VARIANT& data)
	{
		long nUBound, nLBound;
		UINT uSize = ::SafeArrayGetElemsize(data.parray);
		::SafeArrayGetLBound(data.parray, 1, &nLBound);
		::SafeArrayGetUBound(data.parray, 1, &nUBound);
		return (nUBound + 1 - nLBound) * uSize;
	}

	/////////////////////////////////////////////////////////////////////////////
	// copy string variant array into a vector of CStrings
	bool CopyVariantStrings(VARIANT& Source, vector<CString>& Dest)
	{
		Dest.clear();

		// expecting array of BSTR keys
		bool bOK = Source.vt == (VT_ARRAY | VT_BSTR);
		if (!bOK)
		{
			return false;
		}

		// number of source strings
		const long lStrings = GetOneDimensionalElementCount(Source);
		if (lStrings == 0)
		{
			return false;
		}

		COleSafeArray sa(Source);
		BSTR* pData = 0;
		sa.AccessData((void**)&pData);
		for (long lString = 0; lString < lStrings; lString++)
		{
			const CString csValue(pData[lString]);
			Dest.push_back(csValue);
		}

		sa.UnaccessData();

		return true;
	} // CopyVariantStrings

	/////////////////////////////////////////////////////////////////////////////
	// copy string vector into a safe array
	bool CopyStringVector(vector<CString>& Source, VARIANT* Dest)
	{
		::VariantInit(Dest);
		const size_t lValues = Source.size();
		if (lValues == 0)
		{
			return false;
		}

		// stuff the names into an array of BSTRs
		vector<BSTR> arrBSTR(lValues);

		// initialize VARIANTs and copy in the strings
		for (size_t lValue = 0; lValue < lValues; lValue++)
		{
			arrBSTR[lValue] = Source[lValue].AllocSysString();
		}

		// create a 1 dimensional safe array of VARIANTs and initialize
		COleSafeArray sa;
		BSTR* pData = &arrBSTR[0];
		sa.CreateOneDim(VT_BSTR, DWORD(lValues), pData);

		// copy out the result
		const HRESULT hr = ::VariantCopy(Dest, &sa);
		const bool bOK = SUCCEEDED(hr) != FALSE;

		return bOK;
	} // CopyStringVector

	/////////////////////////////////////////////////////////////////////////////
	// copy a variant array to a vector
	template <class T>
	static inline bool CopyVariant
	(
		VARIANT& Source, // source safe array
		VARENUM eVar, // type of variant expected
		vector<T>& Dest // vector is returned
	)
	{
		Dest.clear();
		bool bOK = Source.vt == (VT_ARRAY | eVar);
		if (!bOK)
		{
			return false;
		}

		// number of source values
		const long lValues =
			GetOneDimensionalElementCount(Source);
		if (lValues == 0)
		{
			return false;
		}

		COleSafeArray sa(Source);
		T* pData = 0;
		sa.AccessData((void**)&pData);
		// performance enhancement
		int bytes = GetOneDimensionalSize(Source);
		Dest.resize(lValues);
		memcpy(&Dest[0], pData, bytes);
		sa.UnaccessData();
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////
	// copy vector into a safe array
	template <class T>
	static bool CopyVector
	(
		vector<T>& Source, // source vector
		VARENUM eVar, // destination type corresponding to source type
		VARIANT* Dest // returned safe array
	)
	{
		::VariantInit(Dest);
		const long lValues = Source.size();
		if (lValues == 0)
		{
			return false;
		}

		// create a 1 dimensional safe array of given type and initialize
		COleSafeArray sa;
		T* pData = &Source[0];
		sa.CreateOneDim(eVar, lValues, pData);

		// copy out the result
		::VariantCopy(Dest, &sa);
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////
	// validate given string is numeric
	bool ValidateNumeric(LPCTSTR value)
	{
		bool bValue = false;
		TCHAR* stop;

		// the following will handle the case of "100.0 m" by returning 100
		// because it scans the string from left to right returning the stop
		// pointer at the first non-numeric character
		const double dValue = _tcstod(value, &stop);

		// if zero is returned, then it is possible that the value was
		// actually zero or the value is a non-numeric string
		if (NearlyEqual(dValue, 0.0, 0.000001))
		{
			// zero is a number too, but zero is also the 
			// result of a non-numeric value
			_variant_t var = value;
			if (SUCCEEDED(::VariantChangeType(&var, &var, 0, VT_R8)))
			{
				bValue = true;
			}
		}
		else
		{
			bValue = true;
		}

		return bValue;
	} // ValidateNumeric

	//////////////////////////////////////////////////////////////////////////////
	// A general function to delete a pointer
	template <class T>
	static inline void SAFE_DELETE(T*& pT)
	{
		if (pT == NULL)
		{
			return;
		}

		delete pT;
		pT = NULL;
	}

	//////////////////////////////////////////////////////////////////////////////
	// function to delete a pointer array
	template <class T>
	static inline void SAFE_DELETE_ARRAY(T*& pT)
	{
		if (pT == NULL)
		{
			return;
		}

		delete[] pT;
		pT = NULL;
	}

	//////////////////////////////////////////////////////////////////////////////
	// convert boolean into VARIANT_BOOL
	static inline VARIANT_BOOL VariantBool(bool value)
	{
		const VARIANT_BOOL bValue =
			value != false ? VARIANT_TRUE : VARIANT_FALSE;
		return bValue;
	}

	//////////////////////////////////////////////////////////////////////////////
	// convert VARIANT_BOOL into boolean
	static inline bool VariantBool(VARIANT_BOOL value)
	{
		const bool bValue =
			value != VARIANT_FALSE ? true : false;
		return bValue;
	}

	/////////////////////////////////////////////////////////////////////////////
	// generate GUID string like the following example: 
	//		{6612CAF8-FFA1-49CD-B6E6-11208660E918}
	static inline CString MakeGUID()
	{
		GUID guid;
		::CoCreateGuid(&guid);
		const int nLen = 39; // make room for null character at the end
		OLECHAR str[nLen];
		::StringFromGUID2(guid, str, nLen);
		CString csGuid(str);
		csGuid.MakeLower();
		return csGuid;
	};

	//////////////////////////////////////////////////////////////////////////////
	// generate GUID from string in the format of: 
	//	{c200e360-38c5-11ce-ae62-08002b2b79ef}
	// which includes the braces by COM convention
	static inline bool MakeGUID(CString& input, GUID& guid)
	{
		const int nLen = 39; // allow for terminating null
		HRESULT hr = ::CLSIDFromString(input, &guid);
		bool value = hr == S_OK;
		return value;
	}

	//////////////////////////////////////////////////////////////////////////////
	// generate string in the format of: {c200e360-38c5-11ce-ae62-08002b2b79ef}
	// which includes the braces by COM convention
	static inline CString MakeStringGUID(GUID& guid)
	{
		const int nLen = 39; // allow for terminating null
		OLECHAR str[nLen];
		::StringFromGUID2(guid, str, nLen);
		CString csGuid(str);
		csGuid.MakeLower();
		return csGuid;
	}

	//////////////////////////////////////////////////////////////////////////////
	// generate string in the format of: {c200e360-38c5-11ce-ae62-08002b2b79ef}
	// which includes the braces by COM convention
	static inline CString CreateGUID()
	{
		GUID guid;
		::CoCreateGuid(&guid);
		CString value = MakeStringGUID(guid);
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// given an input void pointer to some data and the length of the data 
	// in bytes, return a Comma Separated Value (CSV) string of the data
	template <class T> static CString ArrayToCSV
	(
		void* value, // array of value of type T
		ULONG bytes, // the length of the array in bytes
		T Type, // let the template know the type
		ULONG ulMax = 50 // the maximum number of values to process
	)
	{
		CString csValue, csValues;
		T* pValues = reinterpret_cast<T*>(value);
		const ULONG ulValues = bytes / sizeof(T);
		bool bDone = false;
		for (ULONG ulValue = 0; ulValue < ulValues; ulValue++)
		{

			if (is_same<T, char>::value)
			{
				csValue = CHAR(pValues[ulValue]);

			}
			else
			{
				csValue.Format(_T("%d, "), pValues[ulValue]);
			}
			if (ulValue >= ulMax)
			{
				csValue += _T("...");
				bDone = true;
			}
			csValues += csValue;
			if (bDone)
			{
				break;
			}
		}

		// remove trailing comma
		csValues.TrimRight(_T(", "));
		return csValues;
	}

	/////////////////////////////////////////////////////////////////////////////
	// given an input void pointer to some data and the length of the data 
	// in bytes, return a vector of 
	template <class T> static bool ArrayToVector
	(
		void* pValue, // array of value of type T
		ULONG bytes, // the length of the array in bytes
		T Type, // let the template know the type
		vector<T>& arrValues
	)
	{
		bool value = false;
		arrValues.clear();

		T* pValues = reinterpret_cast<T*>(pValue);
		const ULONG ulValues = bytes / sizeof(T);
		bool bDone = false;
		for (ULONG ulValue = 0; ulValue < ulValues; ulValue++)
		{

			if (is_same<T, char>::value)
			{
				T val = T(pValues[ulValue]);
				arrValues.push_back(val);

			}
			else
			{
				T val = pValues[ulValue];
				arrValues.push_back(val);
			}
			value = true;
		}

		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// convert a unicode CString to a character buffer
	static inline void CStringToASCII
	(
		const CString& unicodeString, char* asciiBuffer, int bufferSize
	)
	{
		// Convert the CString to a CStringA (ASCII)
		CStringA asciiString(unicodeString);

		const int nLen = asciiString.GetLength();

		// Copy the ASCII string to the provided buffer
		strncpy_s(asciiBuffer, bufferSize, asciiString, _TRUNCATE);
	}

	/////////////////////////////////////////////////////////////////////////////
	// calculate the aspect ratio given a width and height
	// a value of zero indicates a failure
	static inline float GetAspectRatio(UINT uiWidth, UINT uiHeight)
	{
		float value = 0.0f;
		if (uiHeight == 0)
		{
			return value;
		}
		value = static_cast<float>(uiWidth) / uiHeight;
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// given a rectangle of the destination and the rectangle of the source,
	// calculate the drawing rectangle that accounts for aspect ratio
	static inline CRect GetDrawingRectangle
	(
		CRect& rectWindow, CRect& rectImage
	)
	{
		// Calculate the aspect ratio of the image
		const UINT uiImageWidth = rectImage.Width();
		const UINT uiImageHeight = rectImage.Height();
		float imageAspect =
			CHelper::GetAspectRatio(uiImageWidth, uiImageHeight);

		// calculate the aspect ratio of the window
		const UINT uiWindowWidth = rectWindow.Width();
		const UINT uiWindowHeight = rectWindow.Height();
		float windowAspect =
			CHelper::GetAspectRatio(uiWindowWidth, uiWindowHeight);

		// modify the dimensions to correctly display the image
		int drawWidth, drawHeight;
		if (windowAspect > imageAspect)
		{
			drawHeight = uiWindowHeight;
			drawWidth = static_cast<int>(drawHeight * imageAspect);

		}
		else
		{
			drawWidth = uiWindowWidth;
			drawHeight = static_cast<int>(drawWidth / imageAspect);
		}

		// Center the image within the window
		int offsetX = (uiWindowWidth - drawWidth) / 2;
		int offsetY = (uiWindowHeight - drawHeight) / 2;
		CPoint pt(offsetX, offsetY);
		CSize sz(drawWidth, drawHeight);

		CRect value(pt, sz);

		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// a replacement method to: 
	//		unique_ptr<Bitmap> pBitmap(Bitmap::FromFile((LPCTSTR)csPath));
	// which leaves the file open even after the bitmap pointer has been freed.
	static unique_ptr<Bitmap> LoadBitmapFromFile
	(
		const CString& csPath, CString& csError
	)
	{
		// Open the file and read it into a memory buffer
		CFile file;
		if (!file.Open(csPath, CFile::modeRead | CFile::shareDenyWrite))
		{
			csError.Format(L"Failed to open image file: %s", csPath);
			return nullptr;
		}

		ULONGLONG fileSize = file.GetLength();
		auto pFileData = std::make_unique<BYTE[]>((size_t)fileSize);
		file.Read(pFileData.get(), static_cast<UINT>(fileSize));
		file.Close();

		// Create a memory stream from the file data
		IStream* pStream = nullptr;
		HRESULT hr = ::CreateStreamOnHGlobal(nullptr, TRUE, &pStream);
		if (hr != S_OK)
		{
			csError.Format(L"Failed to create the stream: %s");
			return nullptr;
		}

		pStream->Write(pFileData.get(), static_cast<ULONG>(fileSize), nullptr);
		LARGE_INTEGER liZero = {};
		pStream->Seek(liZero, STREAM_SEEK_SET, nullptr);

		// Load the image from the memory stream
		std::unique_ptr<Bitmap> pBitmap(new Bitmap(pStream));
		pStream->Release(); // Release the stream

		if (!pBitmap || pBitmap->GetLastStatus() != Ok)
		{
			csError.Format
			(L"Failed to load image from memory: %s", csPath);
			return nullptr;
		}

		return pBitmap;
	} // LoadBitmapFromFile

	/////////////////////////////////////////////////////////////////////////////
	// a replacement method to: 
	//		unique_ptr<Image> pImage(Image::FromFile((LPCTSTR)csPath));
	// which leaves the file open even after the image pointer has been freed.
	static unique_ptr<Image> LoadImageFromFile
	(
		const CString& csPath, CString& csError
	)
	{
		// Open the file and read it into a memory buffer
		CFile file;
		if (!file.Open(csPath, CFile::modeRead | CFile::shareDenyWrite))
		{
			csError.Format(L"Failed to open: %s", csPath);
			return nullptr;
		}

		ULONGLONG fileSize = file.GetLength();
		auto pFileData = std::make_unique<BYTE[]>((size_t)fileSize);
		file.Read(pFileData.get(), static_cast<UINT>(fileSize));
		file.Close();

		// Create a memory stream from the file data
		IStream* pStream = nullptr;
		HRESULT hr = ::CreateStreamOnHGlobal(nullptr, TRUE, &pStream);
		if (hr != S_OK)
		{
			csError.Format
			(L"Failed to create the stream: %s", csPath);
			return nullptr;
		}

		pStream->Write(pFileData.get(), static_cast<ULONG>(fileSize), nullptr);
		LARGE_INTEGER liZero = {};
		pStream->Seek(liZero, STREAM_SEEK_SET, nullptr);

		// Load the image from the memory stream
		std::unique_ptr<Image> pImage(new Image(pStream));
		pStream->Release(); // Release the stream

		if (!pImage || pImage->GetLastStatus() != Ok)
		{
			csError.Format
			(L"Failed to load image from memory: %s", csPath);
			return nullptr;
		}

		return pImage;
	} // LoadImageFromFile

	/////////////////////////////////////////////////////////////////////////////
	// copies fully qualified source folder to destination folder. 
	// the source can be limited to an optional set of filenames
	// and sub-folders by default are included in the copy
	static int ShellCopy
	(
		// fully qualified source folder name 
		LPCTSTR pcszSource,
		// fully qualified destination folder name 
		LPCTSTR pcszDestination,
		// semicolon separated filename list (may use wild cards) 
		LPCTSTR pcszFiles = 0,
		// true if moving instead of copying
		bool bMove = false,
		// title for progress dialog  
		LPCTSTR pcszTitle = _T("File copy progress"),
		// copy sub-folders 
		bool bSubfolders = true,
		// rename the file if it already exists in the destination folder
		bool bRenameOnCollision = true
	);

	/////////////////////////////////////////////////////////////////////////////
	// deletes fully qualified source folder 
	// the source can be limited to an optional set of filenames
	// and sub-folders by default are included in the copy
	static int ShellDelete
	(LPCTSTR pcszSource, // fully qualified source folder name 
		// semicolon separated filename list (may use wild cards) 
		LPCTSTR pcszFiles = 0,
		// title for progress dialog  
		LPCTSTR pcszTitle = _T("Deleting Folders/Files"),
		bool bSubfolders = true // copy sub-folders 
	);

	/////////////////////////////////////////////////////////////////////////////
// get well known folder, where nID can be one of the following:
//		CSIDL_PERSONAL--My Documents 
//		CSIDL_APPDATA--Application Data
//		CSIDL_COMMON_APPDATA--All Users\Application Data 
//		CSIDL_COMMON_DOCUMENTS--All Users\Documents 
//		CSIDL_PROGRAM_FILES--C:\Program Files
//		CSIDL_PROGRAM_FILES_COMMON--C:\Program Files\Common
//		CSIDL_DESKTOP
//		CSIDL_MYPICTURES
	static CString GetWellKnownFolder(int nID)
	{
		CString value;

		LPITEMIDLIST pidl = NULL;
		LPMALLOC pshmalloc;
		int csidl = nID;
		TCHAR pszPath[_MAX_PATH];
		BOOL bSuccessful = FALSE;

		// ask the shell for the folder location
		if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, csidl, &pidl)))
		{
			bSuccessful = SHGetPathFromIDList(pidl, pszPath);
			if (bSuccessful)
			{
				value = pszPath;
			}
		}

		// release all resources
		if (pidl && SUCCEEDED(SHGetMalloc(&pshmalloc)))
		{
			pshmalloc->Free(pidl);
			pshmalloc->Release();
		}

		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// remove Unicode "smart quotes" and replace with dumb quotes
	static void ReplaceCurlyQuotes(CString& text)
	{
		// UTF-8 curly quotes encoded as hex sequences
		const CString openCurlyQuote = L"\xE2\x80\x9C";  // “
		const CString closeCurlyQuote = L"\xE2\x80\x9D"; // ”
		const CString openCurlySingleQuote = L"\xE2\x80\x98"; // ‘
		const CString closeCurlySingleQuote = L"\xE2\x80\x99"; // ’

		// Straight quotes
		const CString straightDoubleQuote = L"\"";
		const CString straightSingleQuote = L"'";

		// Replace curly double quotes
		text.Replace(openCurlyQuote, straightDoubleQuote);
		text.Replace(closeCurlyQuote, straightDoubleQuote);

		// Replace curly single quotes
		text.Replace(openCurlySingleQuote, straightSingleQuote);
		text.Replace(closeCurlySingleQuote, straightSingleQuote);
	}

	/*
		/////////////////////////////////////////////////////////////////////////////
		// overcomes the problems with CStdioFile::ReadString when reading Unicode
		// text file
		static CString ReadUTF8File( const CString& filePath )
		{
			CString value;
			CStdioFile file;
			if ( file.Open( filePath, CFile::modeRead | CFile::typeBinary ) )
			{
				ULONGLONG fileLength = file.GetLength();
				vector<BYTE> buffer( static_cast<size_t>( fileLength ) );
				file.Read( buffer.data(), static_cast<UINT>( fileLength ) );
				file.Close();

				// Convert the binary data to a UTF-16 string
				int utf16Length = MultiByteToWideChar
				(
					CP_UTF8, 0, (LPCCH)buffer.data(),
					static_cast<int>( fileLength ), NULL, 0
				);
				vector<WCHAR> utf16Buffer( utf16Length );
				MultiByteToWideChar
				(
					CP_UTF8, 0, (LPCCH)buffer.data(),
					static_cast<int>( fileLength ), utf16Buffer.data(), utf16Length
				);

				CString strContent( utf16Buffer.data(), utf16Length );
				value = strContent;
			}
			return value;
		}

		/////////////////////////////////////////////////////////////////////////////
		static inline vector<CString> ReadTextOld( CString csPath )
		{
			vector<CString> value;
			CString csText = ReadUTF8File( csPath );

			// separate concurrant line feeds so that blank lines will not be removed
			csText.Replace( L"\n\n", L"\n \n" );

			int nStart = 0;
			CString csToken = csText.Tokenize( L"\n", nStart );
			while ( !csToken.IsEmpty())
			{
				value.push_back( csToken );
				csToken = csText.Tokenize( L"\n", nStart );
			}
			return value;
		}
	*/

	/////////////////////////////////////////////////////////////////////////////
		// Reads text files in various encodings and returns a vector of strings
		// for each line of text read
		// Parameter csCCS allowed values are: 
		//		UNICODE, 
		//		UTF-8, and
		//		UTF-16LE
		// If no value is specified for csCCS, ReadText uses ANSI encoding.	
	static inline vector<CString> ReadText
	(
		CString csPath, CString csCCS = L"UTF-8"
	)
	{
		vector<CString> value;

		FILE* pFile = nullptr;
		CString csMode;
		csMode.Format(L"rt, ccs=%s", (LPCTSTR)csCCS);
		errno_t e = ::_wfopen_s(&pFile, csPath, csMode);
		if (e == 0)
		{
			CStdioFile file(pFile);
			CString csLine;
			while (file.ReadString(csLine))
			{
				value.push_back(csLine);
			}

			if (pFile != 0)
			{
				::fclose(pFile);
			}
		}

		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// returns a vector of all words separated by spaces (default) in the given
	// line of text where puncuation is treated as part of the word
	static vector<CString> Words(CString csLine, CString csDelim = L" ")
	{
		vector<CString> value;
		int nStart = 0;
		do
		{
			CString csToken = csLine.Tokenize(csDelim, nStart);
			if (csToken.IsEmpty())
			{
				break;
			}
			value.push_back(csToken);
		} while (true);
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// a collection of month abbriviations along with their corresponding integer
	// values
	static CKeyedCollection<CString, int> Months()
	{
		CKeyedCollection<CString, int> months;
		months.add(L"jan", shared_ptr<int>(new int(1)));
		months.add(L"feb", shared_ptr<int>(new int(2)));
		months.add(L"mar", shared_ptr<int>(new int(3)));
		months.add(L"apr", shared_ptr<int>(new int(4)));
		months.add(L"may", shared_ptr<int>(new int(5)));
		months.add(L"jun", shared_ptr<int>(new int(6)));
		months.add(L"jul", shared_ptr<int>(new int(7)));
		months.add(L"aug", shared_ptr<int>(new int(8)));
		months.add(L"sep", shared_ptr<int>(new int(9)));
		months.add(L"oct", shared_ptr<int>(new int(10)));
		months.add(L"nov", shared_ptr<int>(new int(11)));
		months.add(L"dec", shared_ptr<int>(new int(12)));

		return months;
	}

	/////////////////////////////////////////////////////////////////////////////
	// given the name of a month, return its integer value (1..12) or zero if
	// not found
	static int Month(CString csMonth)
	{
		CKeyedCollection<CString, int> months = Months();
		csMonth.MakeLower();
		int value = 0;
		CString csAbbriviation = csMonth.Left(3);
		if (months.Exists[csAbbriviation])
		{
			value = *months.find(csAbbriviation);
		}
		return value;
	}

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
	static CString EndDate(CString csLine);

	/////////////////////////////////////////////////////////////////////////////
	static CString FormatDate(CString csDate);

	/////////////////////////////////////////////////////////////////////////////
	static CString GdiPlusStatusToText(Gdiplus::Status code)
	{
		CString csError(L"Unknown");

		switch (code)
		{
		case Gdiplus::GenericError: csError = L"Generic Error"; break;
		case Gdiplus::InvalidParameter: csError = L"Invalid Parameter"; break;
		case Gdiplus::OutOfMemory: csError = L"Out Of Memory"; break;
		case Gdiplus::ObjectBusy: csError = L"Object Busy"; break;
		case Gdiplus::InsufficientBuffer: csError = L"Insufficien tBuffer"; break;
		case Gdiplus::NotImplemented: csError = L"Not Implemented"; break;
		case Gdiplus::Win32Error: csError = L"Win32 Error"; break;
		case Gdiplus::WrongState: csError = L"Wrong State"; break;
		case Gdiplus::Aborted: csError = L"Aborted"; break;
		case Gdiplus::FileNotFound: csError = L"File Not Found"; break;
		case Gdiplus::ValueOverflow: csError = L"Value Overflow"; break;
		case Gdiplus::AccessDenied: csError = L"Access Denied"; break;
		case Gdiplus::UnknownImageFormat: csError = L"Unknown Image Format"; break;
		case Gdiplus::FontFamilyNotFound: csError = L"Font Family Not Found"; break;
		case Gdiplus::FontStyleNotFound: csError = L"Font Style Not Found"; break;
		case Gdiplus::NotTrueTypeFont: csError = L"Not True Type Font"; break;
		case Gdiplus::UnsupportedGdiplusVersion: csError = L"Unsupported Gdiplus Version"; break;
		case Gdiplus::GdiplusNotInitialized: csError = L"Gdiplus Not Initialized"; break;
		case Gdiplus::PropertyNotFound: csError = L"Property Not Found"; break;
		case Gdiplus::PropertyNotSupported: csError = L"Property Not Supported"; break;
		}

		return csError;
	}

	/////////////////////////////////////////////////////////////////////////////
	static bool AppendStringToTextFile(CString csFile, CString csInput)
	{
		bool value = false;
		CStdioFile file;
		if
			(
				file.Open
				(
					csFile,
					CFile::modeWrite | CFile::modeNoTruncate |
					CFile::modeCreate | CFile::typeText
				)
				)
		{
			// Move to the end of the file
			file.SeekToEnd();

			// Write the text to the file
			file.WriteString(csInput);

			file.Close();

			value = true;
		}

		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// returns true if a registration command line is passed in:
	//		/ or - followed by:
	//			regserver, 
	//			register, 
	//			unregserver, or 
	//			unregister
	static bool RegistrationCommandLine()
	{
		bool bRegistration = false;
		for (int i = 1; i < __argc; i++)
		{
			LPCTSTR pszParam = __targv[i];
			BOOL bFlag = FALSE;
			BOOL bLast = ((i + 1) == __argc);
			if (pszParam[0] == '-' || pszParam[0] == '/')
			{
				// remove flag specifier
				bFlag = TRUE;
				++pszParam;
			}
			CString csParam = pszParam;
			csParam.MakeLower();
			if (csParam == _T("regserver") ||
				csParam == _T("unregserver") ||
				csParam == _T("register") ||
				csParam == _T("unregister"))
			{
				bRegistration = true;
				break;
			}
		}
		return bRegistration;
	}

	/////////////////////////////////////////////////////////////////////////////
	// return the path where the executable is running
	static inline CString GetExecutablePath()
	{
		CString value;
		TCHAR szFilePath[MAX_PATH];
		if (GetModuleFileName(NULL, szFilePath, MAX_PATH))
		{
			value = CString(szFilePath);
		}
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	static inline CString LastError(LPCTSTR pcscFile = 0, int nLine = -1)
	{
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();

		::FormatMessage
		(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0, NULL
		);

		// put in useful form
		CString csMessage = (LPCTSTR)lpMsgBuf;
		::LocalFree(lpMsgBuf);

		csMessage.Trim();
		CString csOutput;
		if (pcscFile == 0)
		{
			csOutput.Format(_T("%s\n"), csMessage);

		}
		else if (nLine == -1)
		{
			csOutput.Format
			(
				_T("%s error in file: %s\n"), csMessage, pcscFile
			);
		}
		else
		{
			csOutput.Format
			(
				_T("%s error in file: %s at line: %06d\n"),
				csMessage, pcscFile, nLine
			);
		}
		return csMessage;
	}

	/////////////////////////////////////////////////////////////////////////////
	static bool RenameFolder(const CString& oldFolderPath, const CString& newFolderPath)
	{
		SHFILEOPSTRUCT fileOp = { 0 };

		// Both oldFolderPath and newFolderPath must be double null-terminated
		TCHAR oldPath[MAX_PATH];
		TCHAR newPath[MAX_PATH];
		_tcscpy_s(oldPath, MAX_PATH, oldFolderPath);
		_tcscpy_s(newPath, MAX_PATH, newFolderPath);

		oldPath[_tcslen(oldPath) + 1] = 0; // Double null-terminate
		newPath[_tcslen(newPath) + 1] = 0; // Double null-terminate

		fileOp.wFunc = FO_RENAME;
		fileOp.pFrom = oldPath;
		fileOp.pTo = newPath;
		fileOp.fFlags = FOF_NOCONFIRMATION | FOF_SILENT; // Optional flags

		int result = SHFileOperation(&fileOp);

		// Check for errors
		if (result != 0)
		{
			// Handle the error
			return false;
		}
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////
	// create a handle to a bitmap from a GDI+ image
	static HBITMAP CreateHBITMAPFromGdiplusImage(Gdiplus::Image* pImage)
	{
		if (!pImage)
		{
			return nullptr;
		}

		int width = pImage->GetWidth();
		int height = pImage->GetHeight();

		// Create a new Gdiplus::Bitmap
		Bitmap bitmap(width, height, PixelFormat32bppARGB);
		Graphics graphics(&bitmap);
		graphics.DrawImage(pImage, 0, 0, width, height);

		HBITMAP hBitmap = nullptr;
		bitmap.GetHBITMAP(Color(255, 255, 255), &hBitmap);
		return hBitmap;
	} // CreateHBITMAPFromGdiplusImage

	/////////////////////////////////////////////////////////////////////////////
	// given an image pointer and an ASCII property ID, return the property value
	static CString GetStringProperty(shared_ptr<Bitmap> pBitmap, PROPID id)
	{
		CString value;

		// get the size of the date property
		const UINT uiSize = pBitmap->GetPropertyItemSize(id);

		// if the property exists, it will have a non-zero size 
		if (uiSize > 0)
		{
			// using a smart pointer which will release itself
			// when it goes out of context
			unique_ptr<Gdiplus::PropertyItem> pItem =
				unique_ptr<Gdiplus::PropertyItem>
				(
					(PropertyItem*)malloc(uiSize)
					);

			// Get the property item.
			pBitmap->GetPropertyItem(id, uiSize, pItem.get());

			// the property should be ASCII
			if (pItem->type == PropertyTagTypeASCII)
			{
				value = (LPCSTR)pItem->value;
			}
		}

		return value;
	} // GetStringProperty

	/////////////////////////////////////////////////////////////////////////////
	// replace reserved characters (not allowed in file names) with underscore
	static CString ReplaceReservedCharacters(CString csInput)
	{
		CString value = csInput;
		vector<wchar_t> reserved =
		{
			'<',
			'>',
			':',
			'\"',
			'/',
			'\\',
			'|',
			'?',
			'*',
		};

		for (auto& node : reserved)
		{
			value.Replace(node, '_');
		}
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// creates a version of a pathname when there is a duplication where
	// the version is appended to the original name inside of open curly brackets
	static CString NewPathVersion(CString csInput)
	{
		CString value;
		int nVersion = 2;
		int nStart = 0;
		CString csNoVersion = csInput.Tokenize(L"{", nStart);
		CString csVersion = csInput.Tokenize(L"}", nStart);
		if (!csVersion.IsEmpty())
		{
			nVersion = _tstoi(csVersion);
		}

		value = csNoVersion;
		while (::PathFileExists(value))
		{
			value.Format
			(
				L"%s {%d}", csNoVersion, nVersion++
			);
		}

		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// Get desk top window rectangle based on the 
	// current screen metrics accounting for multiple monitors
	static inline void GetDeskTopWindowRectangle(CRect& rect)
	{	// min / max tracking dimensions account for multiple monitors
		const int cx = ::GetSystemMetrics(SM_CXMAXTRACK);
		const int cy = ::GetSystemMetrics(SM_CYMAXTRACK);

		rect.left = 0;
		rect.top = 0;
		rect.right = cx;
		rect.bottom = cy;
	}

	/////////////////////////////////////////////////////////////////////////////
	// force the shift key to be up
	static inline void SetShiftKeyUp()
	{
		const short nShiftKeyState = ::GetKeyState(VK_SHIFT);
		const bool bShiftKeyDown = (nShiftKeyState < 0);
		if (bShiftKeyDown)
		{
			vector<BYTE> arrBytes(256, 0);
			const BOOL bOK = GetKeyboardState(&arrBytes[0]);
			if (bOK)
			{
				arrBytes[VK_SHIFT] = 0;
				SetKeyboardState(&arrBytes[0]);
			}
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// returns true if the shift key is down
	static inline bool ShiftKeyDown()
	{
		const short nShiftKeyState = ::GetKeyState(VK_SHIFT);
		const bool value = (nShiftKeyState < 0);
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// returns true if the control key is down
	static inline bool ControlKeyDown()
	{
		const short nControlKeyState = ::GetKeyState(VK_CONTROL);
		const bool value = (nControlKeyState < 0);
		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// ToVector
	//
	// Converts an MFC CArray<T,T> into a modern std::vector<T>.
	//
	// Purpose:
	//   • Modernize legacy code that still uses CArray
	//   • Improve debugger visibility (vector shows contents directly)
	//   • Enable range‑based for loops and STL algorithms
	//   • Provide safe, value‑based semantics instead of raw pointer storage
	//
	// Notes:
	//   • Performs a shallow copy of each element
	//   • Preserves order
	//   • Reserves capacity up front for efficiency
	/////////////////////////////////////////////////////////////////////////////
	template<typename T>
	static std::vector<T> ToVector(const CArray<T, T>& arr)
	{
		std::vector<T> vec;
		vec.reserve(arr.GetSize());

		for (int i = 0; i < arr.GetSize(); i++)
			vec.push_back(arr[i]);

		return vec;
	}

	/////////////////////////////////////////////////////////////////////////////
	// GetRootFolder
	//
	// Returns the application's current working directory, normalized to always
	// end with a trailing backslash. Used by ToRelative() and ToAbsolute() to
	// compute portable paths for image indexing.
	//
	// Example:
	//   C:\Photos\Family\  (always ends with '\')
	/////////////////////////////////////////////////////////////////////////////
	static CString GetRootFolder();

	/////////////////////////////////////////////////////////////////////////////
	// NormalizeSlashes
	//
	// Converts all forward slashes ('/') to backslashes ('\') and removes
	// duplicate slashes. Ensures consistent Windows‑style path formatting.
	//
	// Example:
	//   "C:/Photos//Family" → "C:\Photos\Family"
	/////////////////////////////////////////////////////////////////////////////
	static CString NormalizeSlashes(const CString& path);

	/////////////////////////////////////////////////////////////////////////////
	// ToRelative
	//
	// Converts an absolute path into a portable relative path based on the
	// application's root folder.
	//
	// Example:
	//   Root: "C:\Photos\"
	//   Abs:  "C:\Photos\Family\IMG_0001.jpg"
	//   Rel:  ".\Family\IMG_0001.jpg"
	//
	// Used heavily by PhotoIndexBuilder to ensure indexes are portable across
	// machines and drive letters.
	/////////////////////////////////////////////////////////////////////////////
	static CString ToRelative(const CString& absPath);


	/////////////////////////////////////////////////////////////////////////////
	// ToAbsolute
	//
	// Converts a relative path (".\Family\IMG_0001.jpg") back into an absolute
	// path using the application's root folder.
	//
	// If the path is already absolute, it is returned unchanged.
	/////////////////////////////////////////////////////////////////////////////
	static CString ToAbsolute(const CString& relPath);

	/////////////////////////////////////////////////////////////////////////////
	// PathExists
	//
	// Returns TRUE if the given file or folder exists on disk.
	// Wraps ::GetFileAttributes() with safe error handling.
	/////////////////////////////////////////////////////////////////////////////
	static BOOL PathExists(const CString& path);

	/////////////////////////////////////////////////////////////////////////////
	// GetFileTimestampAndSize
	//
	// Retrieves the file's last‑write timestamp (as a 64‑bit FILETIME) and
	// the file size (also 64‑bit).
	//
	// Purpose:
	//   • Detect modified images in PhotoIndexBuilder::UpdateIndex()
	//   • Support incremental index updates without full rebuilds
	//
	// Returns TRUE on success, FALSE if the file cannot be accessed.
	/////////////////////////////////////////////////////////////////////////////
	static BOOL GetFileTimestampAndSize
	(
		const CString& path,
		uint64_t& timestamp,
		uint64_t& size
	);

	/////////////////////////////////////////////////////////////////////////////
	// Tokenize
	//
	// Splits a comment string into individual tokens (words).
	// Handles Unicode whitespace, punctuation, and multi‑language input.
	//
	// Used by:
	//   • PhotoIndexBuilder
	//   • PhotoIndexRebuildSession
	//   • QueryPhotoIndex
	//
	// Produces clean, searchable tokens for inverted index construction.
	/////////////////////////////////////////////////////////////////////////////
	static void Tokenize(const CString& comment, std::vector<CString>& tokens);

	/////////////////////////////////////////////////////////////////////////////
	// NormalizeToken
	//
	// Cleans up a single token by:
	//   • Lowercasing
	//   • Removing punctuation
	//   • Normalizing apostrophes
	//   • Removing zero‑width characters
	//
	// Ensures consistent search behavior across languages and input sources.
	/////////////////////////////////////////////////////////////////////////////
	static void NormalizeToken(CString& token);

	/////////////////////////////////////////////////////////////////////////////
	// StripPunctuation
	//
	// Removes leading/trailing punctuation from a string while preserving
	// internal alphanumeric content.
	//
	// Example:
	//   "hello," → "hello"
	//   "(family)" → "family"
	//   "O'Connor" → "O'Connor"   (apostrophes preserved)
	/////////////////////////////////////////////////////////////////////////////
	static CString StripPunctuation(const CString& s);

	/////////////////////////////////////////////////////////////////////////////
	// GetXPComment
	//
	// Reads the XPComment EXIF tag (0x9C9C) from a GDI+ Image object.
	// XPComment is stored as UTF‑16LE and may contain multi‑language text.
	//
	// Returns:
	//   • Cleaned UTF‑16 CString
	//   • Empty string if tag is missing or unreadable
	//
	// Used by PhotoIndexBuilder and PhotoIndexRebuildSession to extract
	// searchable metadata from images.
	/////////////////////////////////////////////////////////////////////////////
	static CString GetXPComment(Gdiplus::Image* pImage);

	/////////////////////////////////////////////////////////////////////////////
	// GetXPCommentFromFile
	//
	// Convenience wrapper that loads an image from disk and extracts its
	// XPComment tag using GetXPComment().
	//
	// Returns empty string if the image cannot be loaded or the tag is missing.
	/////////////////////////////////////////////////////////////////////////////
	static CString GetXPCommentFromFile(const CString& absPath);

	/////////////////////////////////////////////////////////////////////////////
	CHelper()
	{
	}

	/////////////////////////////////////////////////////////////////////////////
	~CHelper()
	{
	}
};

