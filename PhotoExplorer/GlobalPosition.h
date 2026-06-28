/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// convert raw Exif GPS data into a formatted string
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
