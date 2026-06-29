/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ATLComTime.h"
#include "KeyedCollection.h"
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CDate
//
// Utility class for parsing, normalizing, validating, and formatting dates
// used in photo metadata (EXIF DateTimeOriginal, XPComment, etc.).
//
// Supports the canonical EXIF format:
//     "YYYY:MM:DD HH:MM:SS"
//
// Stores each component (year, month, day, hour, minute, second) separately,
// provides COleDateTime conversion, and exposes property-style accessors.
//
// Also includes:
//   • Month-name lookup ("jan" → 1, "feb" → 2, …)
//   • Graceful handling of malformed date strings
//   • Validation via COleDateTime::GetStatus()
//   • Preservation of original input string
/////////////////////////////////////////////////////////////////////////////
class CDate
{
	// protected definition
protected:
	/////////////////////////////////////////////////////////////////////////////
	// MONTH_LOOKUP
	//
	// Simple struct used by m_MonthLookup to map month-name prefixes
	// ("jan", "feb", "mar") to numeric month values (1..12).
	/////////////////////////////////////////////////////////////////////////////
	typedef struct tagMonthLookup
	{
		CString m_csMonth;
		int m_nMonth;

	} MONTH_LOOKUP;

	/////////////////////////////////////////////////////////////////////////////
	// TOKEN_NAME
	//
	// Enumeration used when parsing EXIF-style date strings into
	// individual components (year, month, day, hour, minute, second).
	//
	// The order matches the EXIF format exactly:
	//     YYYY : MM : DD   HH : MM : SS
	/////////////////////////////////////////////////////////////////////////////
	typedef enum
	{
		tnYear = 0,
		tnMonth = tnYear + 1,
		tnDay = tnMonth + 1,
		tnHour = tnDay + 1,
		tnMinute = tnHour + 1,
		tnSecond = tnMinute + 1,
	} TOKEN_NAME;

// protected data
protected:
	/////////////////////////////////////////////////////////////////////////////
	// m_csDate
	//
	// Cached formatted date string ("YYYY:MM:DD HH:MM:SS") produced by GetDate().
	/////////////////////////////////////////////////////////////////////////////
	CString m_csDate;

	/////////////////////////////////////////////////////////////////////////////
	// m_csDateTaken
	//
	// Raw date string from metadata (EXIF DateTimeOriginal or XPComment).
	// Stored exactly as provided before parsing.
	/////////////////////////////////////////////////////////////////////////////
	CString m_csDateTaken;

	/////////////////////////////////////////////////////////////////////////////
	// m_nYear, m_nMonth, m_nDay, m_nHour, m_nMinute, m_nSecond
	//
	// Individual date/time components extracted from m_csDateTaken.
	// Stored as integers for easy validation and manipulation.
	/////////////////////////////////////////////////////////////////////////////
	
	// 4 digit year
	int m_nYear;

	// month as a number (1..12)
	int m_nMonth;

	// day of the month (0..31)
	int m_nDay;

	// hour of the day (0..23)
	int m_nHour;

	// minute of the hour (0..59)
	int m_nMinute;

	// second of the minute (0..59)
	int m_nSecond;

	/////////////////////////////////////////////////////////////////////////////
	// m_bOkay
	//
	// Indicates whether the stored date/time components form a valid date.
	// Determined using COleDateTime::GetStatus().
	/////////////////////////////////////////////////////////////////////////////
	bool m_bOkay;

	/////////////////////////////////////////////////////////////////////////////
	// m_MonthLookup
	//
	// Lookup table mapping month-name prefixes ("jan", "feb", "mar") to
	// numeric month values (1..12). Used by GetMonthOfTheYear().
	/////////////////////////////////////////////////////////////////////////////
	CKeyedCollection<CString, int> m_MonthLookup;

// public properties
public:
	/////////////////////////////////////////////////////////////////////////////
	// GetDate
	//
	// Returns the formatted date/time string ("YYYY:MM:DD HH:MM:SS").
	// Uses COleDateTime for validation and formatting.
	// Updates m_csDate when valid.
	/////////////////////////////////////////////////////////////////////////////
	inline CString GetDate()
	{
		CString value;
		COleDateTime oDT = DateAndTime;
		COleDateTime::DateTimeStatus eStatus = oDT.GetStatus();
		bool bOkay = COleDateTime::DateTimeStatus::valid == eStatus;

		// if the status is good, format into a string
		if ( bOkay )
		{
			value = oDT.Format( _T( "%Y:%m:%d %H:%M:%S" ) );
			m_csDate = value;
		}

		return m_csDate;
	}
	/////////////////////////////////////////////////////////////////////////////
	// SetDate
	//
	// Parses a formatted date/time string using COleDateTime::ParseDateTime.
	// If valid, updates all components and marks the date as Okay.
	/////////////////////////////////////////////////////////////////////////////
	inline void SetDate( CString value )
	{
		COleDateTime oDT;
		if ( oDT.ParseDateTime( value ) )
		{
			DateAndTime = oDT;
			Okay = true;
			m_csDate = value;
		}
	}
	// date and time formatted as a string
	__declspec( property( get = GetDate, put = SetDate ) )
		CString Date;

	// 4 digit year
	inline int GetYear()
	{
		return m_nYear;
	}
	// 4 digit year
	inline void SetYear( int value )
	{
		m_nYear = value;
	}
	// 4 digit year
	__declspec( property( get = GetYear, put = SetYear ) )
		int Year;

	// month of the year as a number (1..12)
	inline int GetMonth()
	{
		return m_nMonth;
	}
	// month of the year as a number (1..12)
	inline void SetMonth( int value )
	{
		m_nMonth = value;
	}
	// month of the year as a number (1..12)
	__declspec( property( get = GetMonth, put = SetMonth ) )
		int Month;

	// day of the month (0..31)
	inline int GetDay()
	{
		return m_nDay;
	}
	// day of the month (0..31)
	inline void SetDay( int value )
	{
		m_nDay = value;
	}
	// day of the month (0..31)
	__declspec( property( get = GetDay, put = SetDay ) )
		int Day;

	// hour of the day (0..23)
	inline int GetHour()
	{
		return m_nHour;
	}
	// hour of the day (0..23)
	inline void SetHour( int value )
	{
		m_nHour = value;
	}
	// hour of the day (0..23)
	__declspec( property( get = GetHour, put = SetHour ) )
		int Hour;

	// minute of the hour (0..59)
	inline int GetMinute()
	{
		return m_nMinute;
	}
	// minute of the hour (0..59)
	inline void SetMinute( int value )
	{
		m_nMinute = value;
	}
	// minute of the hour (0..59)
	__declspec( property( get = GetMinute, put = SetMinute ) )
		int Minute;

	// second of the minute (0..59)
	inline int GetSecond()
	{
		return m_nSecond;
	}
	// second of the minute (0..59)
	inline void SetSecond( int value )
	{
		m_nSecond = value;
	}
	// second of the minute (0..59)
	__declspec( property( get = GetSecond, put = SetSecond ) )
		int Second;

	// boolean indicator that all is well
	inline bool GetOkay()
	{
		COleDateTime oDT( Year, Month, Day, Hour, Minute, Second );
		COleDateTime::DateTimeStatus eStatus = oDT.GetStatus();
		Okay = COleDateTime::DateTimeStatus::valid == eStatus;
		return m_bOkay;
	}
	// boolean indicator that all is well
	inline void SetOkay( bool value )
	{
		m_bOkay = value;
	}
	// boolean indicator that all is well
	__declspec( property( get = GetOkay, put = SetOkay ) )
		bool Okay;

	/////////////////////////////////////////////////////////////////////////////
	// GetDateAndTime
	//
	// Constructs a COleDateTime from the stored components.
	// Updates Okay based on COleDateTime::GetStatus().
	/////////////////////////////////////////////////////////////////////////////
	inline COleDateTime GetDateAndTime()
	{
		COleDateTime value( Year, Month, Day, Hour, Minute, Second );
		COleDateTime::DateTimeStatus eStatus = value.GetStatus();
		Okay = COleDateTime::DateTimeStatus::valid == eStatus;
		return value;
	}
	/////////////////////////////////////////////////////////////////////////////
	// SetDateAndTime
	//
	// Assigns all components from a COleDateTime if it is valid.
	// Marks the date as Okay.
	/////////////////////////////////////////////////////////////////////////////
	inline void SetDateAndTime( COleDateTime value )
	{
		COleDateTime::DateTimeStatus eStatus = value.GetStatus();
		bool bOkay = COleDateTime::DateTimeStatus::valid == eStatus;
		if ( bOkay )
		{
			Year = value.GetYear();
			Month = value.GetMonth();
			Day = value.GetDay();
			Hour = value.GetHour();
			Minute = value.GetMinute();
			Second = value.GetSecond();
			Okay = bOkay;
		}
	}
	// date and time property
	__declspec( property( get = GetDateAndTime, put = SetDateAndTime ) )
		COleDateTime DateAndTime;

	/////////////////////////////////////////////////////////////////////////////
	// GetDateTaken
	// 
	// The date and time when the original image data was generated.
	// For a digital still camera, this is the date and time the picture 
	// was taken or recorded. The format is "YYYY:MM:DD HH:MM:SS" with time 
	// shown in 24-hour format, and the date and time separated by one blank 
	// character (hex 20).
	inline CString GetDateTaken()
	{
		return m_csDateTaken;
	}
	/////////////////////////////////////////////////////////////////////////////
	// SetDateTaken
	//
	// Parses EXIF-style date strings:
	//
	//     "YYYY:MM:DD HH:MM:SS"
	//
	// Tokenizes using delimiters ":" and " ".
	// Ensures exactly six tokens (pads with "00" if needed).
	// Converts tokens to integers and assigns Year..Second.
	//
	// Updates Okay based on COleDateTime validation.
	/////////////////////////////////////////////////////////////////////////////
	void SetDateTaken( CString csDate )
	{
		// reset the m_Date to undefined state
		Year = -1;
		Month = -1;
		Day = -1;
		Hour = 0;
		Minute = 0;
		Second = 0;
		bool value = Okay;

		// parse the date into a vector of string tokens
		const CString csDelim( _T( ": " ) );
		int nStart = 0;
		vector<CString> tokens;

		do
		{
			const CString csToken =
				csDate.Tokenize( csDelim, nStart ).MakeLower();
			if ( csToken.IsEmpty() )
			{
				break;
			}

			tokens.push_back( csToken );

		}
		while ( true );

		// there should be six tokens in the proper format of
		// "YYYY:MM:DD HH:MM:SS"
		const size_t tTokens = tokens.size();
		const CString csVal( L"00" );
		if ( tTokens != 6 )
		{	
			if ( tTokens == 0 )
			{
				return;
			}
			tokens.resize( 6, csVal );
		}

		// populate the date and time members with the values
		// in the vector
		TOKEN_NAME eToken = tnYear;
		int nToken = 0;

		for ( CString csToken : tokens )
		{
			int nValue = _tstol( csToken );

			switch ( eToken )
			{
				case tnYear:
				{
					Year = nValue;
					break;
				}
				case tnMonth:
				{
					Month = nValue;
					break;
				}
				case tnDay:
				{
					Day = nValue;
					break;
				}
				case tnHour:
				{
					Hour = nValue;
					break;
				}
				case tnMinute:
				{
					Minute = nValue;
					break;
				}
				case tnSecond:
				{
					Second = nValue;
					break;
				}
			}

			nToken++;
			eToken = (TOKEN_NAME)nToken;
		}

		// this will be true if all of the values define a proper date and time
		value = Okay;

	} 
	/////////////////////////////////////////////////////////////////////////////
	// The date and time when the original image data was generated.
	// For a digital still camera, this is the date and time the picture 
	// was taken or recorded. The format is "YYYY:MM:DD HH:MM:SS" with time 
	// shown in 24-hour format, and the date and time separated by one blank 
	// character (hex 20).
	/////////////////////////////////////////////////////////////////////////////
	__declspec( property( get = GetDateTaken, put = SetDateTaken ) )
		CString DateTaken;

// public methods
public:
	/////////////////////////////////////////////////////////////////////////////
	// GetMonthOfTheYear
	//
	// Returns the numeric month (1..12) given a month name such as
	// "January", "Feb", "mar", etc.
	//
	// Uses only the first three characters (lowercased) as the key.
	// Returns 0 if the month name is not recognized.
	/////////////////////////////////////////////////////////////////////////////
	int GetMonthOfTheYear( CString month )
	{
		int value = 0;

		// the key is the first three characters in lower case
		const CString csKey = month.Left( 3 ).MakeLower();

		// if the key exists in the cross reference, then lookup the
		// month of the year (1..12)
		if ( m_MonthLookup.Exists[ csKey ] )
		{
			value = *m_MonthLookup.find( csKey );
		}

		return value;
	}

	/////////////////////////////////////////////////////////////////////////////
	// CDate constructor
	//
	// Initializes all date/time components to an undefined state and builds
	// the month-name lookup table ("jan" → 1, ..., "dec" → 12).
	//
	// Sets Okay = false until a valid date is parsed or assigned.
	/////////////////////////////////////////////////////////////////////////////
	CDate()
	{
		Year = -1;
		Month = -1;
		Day = -1;
		Hour = 0;
		Minute = 0;
		Second = 0;

		// create a lookup table for months
		LPCTSTR months[] =
		{
			_T( "jan" ),
			_T( "feb" ),
			_T( "mar" ),
			_T( "apr" ),
			_T( "may" ),
			_T( "jun" ),
			_T( "jul" ),
			_T( "aug" ),
			_T( "sep" ),
			_T( "oct" ),
			_T( "nov" ),
			_T( "dec" ),
		};
		const int nMonths = _countof( months );
		for ( int nMonth = 0; nMonth < nMonths; nMonth++ )
		{
			m_MonthLookup.add
			(
				months[ nMonth ],
				shared_ptr<int>( new int( nMonth + 1 ) )
			);
		}

		Okay = false;
	}
};


