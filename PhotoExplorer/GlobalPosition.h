/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CGlobalPosition
//
// Utility class that converts raw EXIF GPS coordinate data into a formatted,
// human-readable string. EXIF GPS metadata stores latitude and longitude as
// three rational values (degrees, minutes, seconds), each represented by a
// numerator/denominator pair. This class extracts those values and produces
// a clean textual representation suitable for display in Photo Explorer.
//
// Purpose:
//   • Interpret EXIF GPS coordinate data stored in rational form.
//   • Convert degrees/minutes/seconds into a readable string such as:
//         "29° 31' 45.23\""
//   • Provide a simple interface for storing raw GPS data and retrieving
//     formatted output.
//
// Why this class exists:
//   Digital cameras and phones store GPS coordinates in EXIF metadata using
//   rational numbers. Without conversion, the raw values are difficult to
//   interpret and unsuitable for display. CGlobalPosition centralizes this
//   conversion logic so Photo Explorer can present meaningful location
//   information alongside each photograph.
//
// Responsibilities:
//   • Store the raw EXIF GPS data (RawData property).
//     - Six ULONG values representing:
//         degreesNumerator, degreesDenominator,
//         minutesNumerator, minutesDenominator,
//         secondsNumerator, secondsDenominator
//   • Convert the raw rational values into floating-point degrees,
//     minutes, and seconds.
//   • Produce a formatted output string (Output property).
//
// Interaction with other components:
//   • Used by metadata extraction routines when loading JPEG EXIF data.
//   • The formatted GPS string is displayed in the Properties pane and
//     may be used for labeling or indexing images.
//   • Works alongside other EXIF utilities such as CExifRotation.
//
// Notes:
//   • This class does not perform geolocation lookup (e.g., converting
//     coordinates into street addresses). It only formats the raw EXIF
//     values.
//   • The conversion logic is implemented in GetOutput(), which interprets
//     the rational values and constructs the final string.
//
// This class provides a clean, reliable way to interpret EXIF GPS metadata
// and ensures that location information is presented in a readable form
// throughout Photo Explorer.
/////////////////////////////////////////////////////////////////////////////
class CGlobalPosition
{
// protected data
protected:
	// GPS data is stored as three rational (numerator/denominator) pairs
	// for degrees, minutes, and seconds for a total of six values
	vector<ULONG> m_arrRawData;

	// output string formatted as: "degrees minutes seconds"
	CString m_csOutput;

// public properties
public:
	// GPS data is stored as three rational (numerator/denominator) pairs
	// for degrees, minutes, and seconds for a total of six values
	inline vector<ULONG> GetRawData()
	{
		return m_arrRawData;
	}
	// GPS data is stored as three rational (numerator/denominator) pairs
	// for degrees, minutes, and seconds for a total of six values
	inline void SetRawData( vector<ULONG> value )
	{
		m_arrRawData = value;
	}
	// GPS data is stored as three rational (numerator/denominator) pairs
	// for degrees, minutes, and seconds for a total of six values
	__declspec( property( get = GetRawData, put = SetRawData ))
		vector<ULONG> RawData;

	// output string formatted as: "degrees minutes seconds"
	CString GetOutput();
	// output string formatted as: "degrees minutes seconds"
	inline void SetOutput( CString value )
	{
		m_csOutput = value;
	}
	// output string formatted as: "degrees minutes seconds"
	__declspec( property( get = GetOutput, put = SetOutput ))
		CString Output;

// public methods
public:

// protected overrides
protected:

// public overrides
public:

// public constructor/destructor
public:
	CGlobalPosition()
	{
	}
	~CGlobalPosition()
	{
	}
};

/////////////////////////////////////////////////////////////////////////////
