/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "VariantVector.h"
#include "CHelper.h"
#include <float.h>

/////////////////////////////////////////////////////////////////////////////
// Default constructor
//
// Initializes the vector to an empty state with:
//   • Null = −9999
//   • Type = VT_EMPTY
//   • 1 column, 1 element per column
//   • Conversion parameters reset
//   • Statistics reset
/////////////////////////////////////////////////////////////////////////////
CVariantVector::CVariantVector(void)
{
	Null = -9999;
	Type = VT_EMPTY;
	Columns = 1;
	ColumnElements = 1;

	Multiplier = 1;
	Additive = 0;

	FirstLive = Null;
	LastLive = Null;
	Minimum = Null;
	Maximum = Null;
	Average = Null;

	LevelFirstLive = -1;
	LevelLastLive = -1;
	LevelMinimum = -1;
	LevelMaximum = -1;
}

/////////////////////////////////////////////////////////////////////////////
CVariantVector::CVariantVector
(	double dNull, 
	VARENUM eType, 
	ULONG ulColumns/* = 1*/, 
	ULONG ulColumnElements/* = 1*/,
	ULONG ulRows/* = 0*/,
	LPCTSTR pcszUnits/* = _T( "" )*/
)
{
	Create( dNull, eType, ulColumns, ulColumnElements, ulRows, pcszUnits );
}

/////////////////////////////////////////////////////////////////////////////
CVariantVector::~CVariantVector(void)
{
}

/////////////////////////////////////////////////////////////////////////////
// Create
//
// Resets the vector and initializes type, dimensions, units, and null value.
// If ulRows > 0, preallocates the buffer and fills with nulls.
/////////////////////////////////////////////////////////////////////////////
void CVariantVector::Create
(	double dNull, // the empty value
	VARENUM eType, // variant type
	ULONG ulColumns/* = 1*/, // matrix columns
	ULONG ulColumnElements/* = 1*/, // elements per column
	ULONG ulRows/* = 0*/, // number of rows (if set will fill with nulls)
	LPCTSTR pcszUnits/* = _T( "" )*/ // units
)
{
	clear();
	setEmptyValue( dNull );
	setType( eType );
	setColumns( ulColumns );
	setColumnElements( ulColumnElements );
	setUnits( pcszUnits );

	Multiplier = 1;
	Additive = 0;

	// if the number of rows are given, preallocate with 
	// null values
	if ( ulRows != 0 )
	{
		const ULONG ulRowElements = getRowElements();
		const ULONG ulElements = ulRows * ulRowElements;
		resize( ulElements );
	}
} // Create

/////////////////////////////////////////////////////////////////////////////
// count
//
// Returns the number of stored elements based on the active VARIANT type.
// Each type uses its own internal std::vector<T>.
/////////////////////////////////////////////////////////////////////////////
ULONG CVariantVector::count()
{
	const VARENUM nType = getType();
	switch ( nType )
	{
		case VT_I1: return (ULONG)m_vectorChars.size();
		case VT_UI1: return (ULONG)m_vectorBytes.size();
		case VT_I2: return (ULONG)m_vectorShorts.size();
		case VT_UI2: return (ULONG)m_vectorUShorts.size();
		case VT_I4: return (ULONG)m_vectorLongs.size();
		case VT_UI4: return (ULONG)m_vectorULongs.size();
		case VT_I8: return (ULONG)m_vectorLongLongs.size();
		case VT_UI8: return (ULONG)m_vectorULongLongs.size();
		case VT_R4: return (ULONG)m_vectorFloats.size();
		case VT_R8: return (ULONG)m_vectorDoubles.size();
		default: return 0;
	}
} // count

/////////////////////////////////////////////////////////////////////////////
// assignment operator
CVariantVector& CVariantVector::operator=( CVariantVector& src )
{
	m_dEmptyValue = src.m_dEmptyValue;
	m_vtType = src.m_vtType;
	m_ulColumns = src.m_ulColumns;
	m_ulColumnElements = src.m_ulColumnElements;
	m_csUnits = src.m_csUnits;
	m_vectorChars = src.m_vectorChars;
	m_vectorBytes = src.m_vectorBytes;
	m_vectorShorts = src.m_vectorShorts;
	m_vectorUShorts = src.m_vectorUShorts;
	m_vectorLongs = src.m_vectorLongs;
	m_vectorULongs = src.m_vectorULongs;
	m_vectorLongLongs = src.m_vectorLongLongs;
	m_vectorULongLongs = src.m_vectorULongLongs;
	m_vectorFloats = src.m_vectorFloats;
	m_vectorDoubles = src.m_vectorDoubles;
	return *this;
} // operator=

/////////////////////////////////////////////////////////////////////////////
// clear the data
void CVariantVector::clear()
{
	const VARENUM nType = getType();
	switch ( nType )
	{
		case VT_I1: m_vectorChars.clear(); break;
		case VT_UI1 : m_vectorBytes.clear(); break;
		case VT_I2 : m_vectorShorts.clear(); break;
		case VT_UI2 : m_vectorUShorts.clear(); break;
		case VT_I4 : m_vectorLongs.clear(); break;
		case VT_UI4 : m_vectorULongs.clear(); break;
		case VT_I8 : m_vectorLongLongs.clear(); break;
		case VT_UI8 : m_vectorULongLongs.clear(); break;
		case VT_R4 : m_vectorFloats.clear(); break;
		case VT_R8 : m_vectorDoubles.clear(); break;
	}
} // clear
	
/////////////////////////////////////////////////////////////////////////////
// set data from a char* pointer
bool CVariantVector::setData( VARENUM nType, ULONG ulValues, char* pData )
{
	setType( nType );
	resize( ulValues );
	// if the resize above failed, return a false
	if ( count() == 0 )
	{
		setType();
		return false;
	}
	const ULONG ulBytes = getElementSizeInBytes() * ulValues;
	void* pDest = getData();
	::CopyMemory( pDest, pData, ulBytes );
	return true;
} // setData
	
/////////////////////////////////////////////////////////////////////////////
// get a void pointer to the data and optionally to 
// a specific index of the data
void* CVariantVector::getData( ULONG ulIndex/* = 0*/ )
{
	void* pData = 0;
	const ULONG ulValues = count();
	if ( ulValues == 0 )
	{
		return pData;
	}
	const VARENUM nType = getType();
	switch ( nType )
	{
		case VT_I1: pData = &m_vectorChars[ ulIndex ]; break;
		case VT_UI1 : pData = &m_vectorBytes[ ulIndex ]; break;
		case VT_I2 : pData = &m_vectorShorts[ ulIndex ]; break;
		case VT_UI2 : pData = &m_vectorUShorts[ ulIndex ]; break;
		case VT_I4 : pData = &m_vectorLongs[ ulIndex ]; break;
		case VT_UI4 : pData = &m_vectorULongs[ ulIndex ]; break;
		case VT_I8 : pData = &m_vectorLongLongs[ ulIndex ]; break;
		case VT_UI8 : pData = &m_vectorULongLongs[ ulIndex ]; break;
		case VT_R4 : pData = &m_vectorFloats[ ulIndex ]; break;
		case VT_R8 : pData = &m_vectorDoubles[ ulIndex ]; break;
	}
	return pData;
} // getData

/////////////////////////////////////////////////////////////////////////////
// get a void pointer to the data at a given row, column, and element
// where the data is potentially formatted as a matrix which can 
// be an array of arrays. for a simple two dimensional array, the
// column will be zero and the element is the column.
void* CVariantVector::getData
(	ULONG ulRow, // row of the table
	ULONG ulColumn, // matrix column
	ULONG ulElement/* = 0*/ // element of the column
)
{
	void* pData = 0;
	const ULONG ulRows = getRows();
	if ( ulRow >= ulRows )
	{
		return pData;
	}
	const ULONG ulColumns = getColumns();
	if ( ulColumn >= ulColumns )
	{
		return pData;
	}
	const ULONG ulIndex = getIndex( ulRow, ulColumn, ulElement );
	pData = getData( ulIndex );
	return pData;
} // getData
	
/////////////////////////////////////////////////////////////////////////////
// resize the data to the given number of elements. if the size
// is extended, the new levels are filled with null values.
// if the size is reduced, data will be deleted at the end.
// if bProtectContents is true, the existing data will remain
// unchanged except when the size is reduced.
// if bProtectContents is false, the entire content will be
// filled with null values.
void CVariantVector::resize
(	// new size of the of the vector in values
	ULONG values, 
	// protect existing content when true or null the values when false
	bool bProtectContents/* = false*/ 
)
{
	const VARENUM nType = getType();
	const double dNull = getEmptyValue();
	// if not protecting existing content, clear the content out
	if ( !bProtectContents )
	{
		clear();
	}
	switch ( nType )
	{
		case VT_I1: m_vectorChars.resize( values, (CHAR)dNull ); break;
		case VT_UI1 : m_vectorBytes.resize( values, (UCHAR)dNull ); break;
		case VT_I2 : m_vectorShorts.resize( values, (SHORT)dNull ); break;
		case VT_UI2 : m_vectorUShorts.resize( values, (USHORT)dNull ); break;
		case VT_I4 : m_vectorLongs.resize( values, (LONG)dNull ); break;
		case VT_UI4 : m_vectorULongs.resize( values, (ULONG)dNull ); break;
		case VT_I8 : m_vectorLongLongs.resize( values, (LONGLONG)dNull ); break;
		case VT_UI8 : m_vectorULongLongs.resize( values, (ULONGLONG)dNull ); break;
		case VT_R4 : m_vectorFloats.resize( values, (FLOAT)dNull ); break;
		case VT_R8 : m_vectorDoubles.resize( values, (DOUBLE)dNull ); break;
	}
} // resize
	
/////////////////////////////////////////////////////////////////////////////
// data element size in bytes (0 for unsupported type)
short CVariantVector::getElementSizeInBytes( VARENUM nType )
{
	switch ( nType )
	{
		case VT_I1: return sizeof( CHAR );
		case VT_UI1 : return sizeof( BYTE );
		case VT_I2 : return sizeof( SHORT );
		case VT_UI2 : return sizeof( WORD );
		case VT_I4 : return sizeof( LONG );
		case VT_UI4 : return sizeof( ULONG );
		case VT_I8 : return sizeof( LONGLONG );
		case VT_UI8 : return sizeof( ULONGLONG );
		case VT_R4 : return sizeof( FLOAT );
		case VT_R8 : return sizeof( DOUBLE );
		default : return 0;
	}
} // getElementSizeInBytes

/////////////////////////////////////////////////////////////////////////////
// Convert
//
// Applies a linear transformation to all non‑null values:
//     newValue = oldValue * dMult + dAdd
//
// Skips null values and replaces non‑finite results with Null.
/////////////////////////////////////////////////////////////////////////////
bool CVariantVector::Convert( DOUBLE dMult, DOUBLE dAdd )
{	// if the multiplier is 1 and the add is zero, do nothing
	if ( CHelper::NearlyEqual( dMult, 1.0, 0.0000001 ) )
	{
		if ( CHelper::NearlyEqual( dAdd, 0.0, 0.0000001 ) )
		{
			return true;
		}
	}
	const ULONG ulValues = count();
	const VARENUM varEnum = getType();
	if ( varEnum == VT_EMPTY || ulValues == 0 )
	{
		return false;
	}
	const double dNull = getEmptyValue();
	for ( ULONG ulValue = 0; ulValue < ulValues; ulValue++ )
	{
		const double dInput = getDouble( ulValue );
		if ( CHelper::NearlyEqual( dInput, dNull ) )
		{
			continue;
		}
		const double dValue = dInput * dMult + dAdd;
		if ( !::_finite( dValue ) )
		{
			setDouble( ulValue, dNull );
		} else
		{
			setDouble( ulValue, dValue );
		}
	}
	return true;
} // Convert

/////////////////////////////////////////////////////////////////////////////
// find the minimum and maximum values
bool CVariantVector::FindMinMax( DOUBLE& dMinimum, DOUBLE& dMaximum )
{
	dMinimum = DBL_MAX;
	dMaximum = DBL_MIN;
	const ULONG ulValues = count();
	const VARENUM varEnum = getType();
	if ( varEnum == VT_EMPTY || ulValues == 0 )
	{
		return false;
	}
	const double dNull = getEmptyValue();
	ULONG ulCount = 0;
	for ( ULONG ulValue = 0; ulValue < ulValues; ulValue++ )
	{
		COleVariant var = getValue( ulValue );
		var.ChangeType( VT_R8 );
		const double dValue = var.dblVal;
		if ( !::_finite( dValue ) )
		{
			continue;
		}
		if ( CHelper::NearlyEqual( dValue, dNull ) )
		{
			continue;
		}
		ulCount++;
		dMinimum = min( dMinimum, dValue );
		dMaximum = max( dMaximum, dValue );
	}
	return ulCount != 0;
} // FindMinMax

/////////////////////////////////////////////////////////////////////////////
// CalculateStatistics
//
// Computes FirstLive, LastLive, Minimum, Maximum, Average in a single pass.
// Skips null values, non‑finite values, and sentinel −9999 values.
/////////////////////////////////////////////////////////////////////////////
bool CVariantVector::CalculateStatistics()
{
	bool value = false;

	// do a quick sanity check before getting started
	const ULONG ulValues = Elements;
	const VARENUM varEnum = Type;
	if ( varEnum == VT_EMPTY || ulValues == 0 )
	{
		return value;
	}

	// setup the statistical values
	const double dNull = Null;
	double dMinimum = DBL_MAX;
	ULONG ulMinimum = -9999;
	double dMaximum = DBL_MIN;
	ULONG ulMaximum = -9999;
	double dTotal = dNull;
	double dFirstLive = dNull;
	ULONG ulFirstLive = -9999;
	double dLastLive = dNull;
	ULONG ulLastLive = -9999;
	bool bFirst = false;

	ULONG ulCount = 0;
	for ( ULONG ulValue = 0; ulValue < ulValues; ulValue++ )
	{
		// get the value as a double
		const double dValue = getDouble( ulValue );

		// check for not a number
		if ( !::_finite( dValue ) )
		{
			continue;
		}

		// check for a null value
		if ( CHelper::NearlyEqual( dValue, dNull, 0.001 ) )
		{
			continue;
		}

		// check for a -9999 value
		if ( CHelper::NearlyEqual( dValue, -9999.0, 0.001 ) )
		{
			continue;
		}

		// increment the value counter
		ulCount++;

		// if first live has not been found, assign it now
		if ( bFirst == false )
		{
			dFirstLive = dValue;
			ulFirstLive = ulValue;
			bFirst = true;
		}

		// do the other calculations
		if ( dValue < dMinimum )
		{
			dMinimum = dValue;
			ulMinimum = ulValue;
		}

		if ( dValue > dMaximum )
		{
			dMaximum = dValue;
			ulMaximum = ulValue;
		}

		dTotal += dValue;
		dLastLive = dValue;
		ulLastLive = ulValue;
	}

	// we are good if there are any values
	value = ulCount != 0;

	// if we were successful, record the statistics
	if ( value == true )
	{
		FirstLive = dFirstLive;
		LastLive = dLastLive;
		Average = dTotal / ulCount;
		Minimum = dMinimum;
		Maximum = dMaximum;

		LevelFirstLive = ulFirstLive;
		LevelLastLive = ulLastLive;
		LevelMinimum = ulMinimum;
		LevelMaximum = ulMaximum;
	}

	return value;
} // CalculateStatistics

/////////////////////////////////////////////////////////////////////////////
// changeType
//
// Reinterprets the underlying buffer as a different VARIANT type.
// Byte‑preserving: the total byte count must match exactly.
// Caller must ensure the reinterpretation is meaningful.
/////////////////////////////////////////////////////////////////////////////
bool CVariantVector::changeType( VARENUM eNew )
{
	bool value = true;
	const VARTYPE eType = getType();
	if ( eType == eNew )
	{
		return value;
	}

	// number of bytes in the variant vector currently
	const ULONG ulBytesOld = getNumberOfBytes();

	void* pOld = getData();

	// change to the new type
	setType( eNew );

	// number of bytes in the variant vector after type change
	const ULONG ulBytesNew = getNumberOfBytes();

	// the buffers need to be the same size
	value = ulBytesOld == ulBytesNew;

	// copy the old buffer to the new buffer if the buffers are the same 
	// size
	if ( value == true )
	{
		void* pNew = getData();
		CopyMemory( pNew, pOld, ulBytesNew );
	}

	return value;
} // changeType
	
/////////////////////////////////////////////////////////////////////////////
// set a value at a given index
void CVariantVector::setValue( ULONG ulIndex, COleVariant var )
{	// range check?
	const ULONG ulValues = count();
	if ( ulValues <= ulIndex )
	{
		return; // out of range
	}
	// type compatibility?
	const VARENUM nType = getType();
	if ( var.vt != nType )
	{
		const short nSize = getElementSizeInBytes( (VARENUM)var.vt );
		if ( nSize == 0 )
		{
			return; // not supported
		}
		var.ChangeType( nType );
	}
	// assign the variant based on type
	switch ( nType )
	{
		case VT_I1: m_vectorChars[ ulIndex ] = var.cVal; break;
		case VT_UI1 : m_vectorBytes[ ulIndex ] = var.bVal; break;
		case VT_I2 : m_vectorShorts[ ulIndex ] = var.iVal; break;
		case VT_UI2 : m_vectorUShorts[ ulIndex ] = var.uiVal; break; 
		case VT_I4 : m_vectorLongs[ ulIndex ] = var.lVal; break;
		case VT_UI4 : m_vectorULongs[ ulIndex ] = var.ulVal; break; 
		case VT_I8 : m_vectorLongLongs[ ulIndex ] = var.llVal; break;
		case VT_UI8 : m_vectorULongLongs[ ulIndex ] = var.ullVal; break;
		case VT_R4 : m_vectorFloats[ ulIndex ] = var.fltVal; break;
		case VT_R8 : m_vectorDoubles[ ulIndex ] = var.dblVal; break;
	}
} // setValue

/////////////////////////////////////////////////////////////////////////////
// get a value at a given index
COleVariant CVariantVector::getValue( ULONG ulIndex )
{
	COleVariant var;
	VARIANT variant;
	::VariantInit( &variant );
	const ULONG ulValues = count();
	if ( ulValues <= ulIndex )
	{
		return var;
	}
	const VARENUM nType = getType();
	variant.vt = nType;
	switch ( nType )
	{
		case VT_I1: // not handled by COleVariant directly
		{
			variant.ulVal = m_vectorChars[ ulIndex ];
			var = variant;
			break;
		}
		case VT_UI1 : var = m_vectorBytes[ ulIndex ]; break;
		case VT_I2 : var = m_vectorShorts[ ulIndex ]; break;
		case VT_UI2 : // not handled by COleVariant directly
		{
			variant.ulVal = m_vectorUShorts[ ulIndex ]; 
			var = variant;
			break;
		}
		case VT_I4 : var = m_vectorLongs[ ulIndex ]; break;
		case VT_UI4 : // not handled by COleVariant directly
		{
			variant.ulVal = m_vectorULongs[ ulIndex ]; 
			var = variant;
			break;
		}
		case VT_I8 : var = m_vectorLongLongs[ ulIndex ]; break;
		case VT_UI8 : var = m_vectorULongLongs[ ulIndex ]; break;
		case VT_R4 : 
		{
			const float value = m_vectorFloats[ ulIndex ]; 
			if ( !::_finite( value ))
			{
				var = (float)getEmptyValue();
				setValue( ulIndex, var );
			
			} else
			{
				var = value;
			}
			break;
		}
		case VT_R8 : 
		{
			const double value = m_vectorDoubles[ ulIndex ]; 
			if ( !::_finite( value ))
			{
				var = getEmptyValue();
				setValue( ulIndex, var );
			
			} else
			{
				var = value;
			}
			break;
		}
	}
	return var;
} // getValue 

/////////////////////////////////////////////////////////////////////////////
// get a double value at a given index and return null value if unsuccessful
double CVariantVector::getDouble( ULONG ulIndex )
{
	double dValue = getEmptyValue();
	const ULONG ulValues = count();
	if ( ulValues <= ulIndex )
	{
		return dValue;
	}
	const VARENUM nType = getType();
	switch ( nType )
	{
		case VT_I1: dValue = m_vectorChars[ ulIndex ]; break;
		case VT_UI1: dValue = m_vectorBytes[ ulIndex ]; break;
		case VT_I2: dValue = m_vectorShorts[ ulIndex ]; break;
		case VT_UI2: dValue = m_vectorUShorts[ ulIndex ]; break;
		case VT_I4: dValue = m_vectorLongs[ ulIndex ]; break;
		case VT_UI4: dValue = m_vectorULongs[ ulIndex ]; break;
		case VT_I8: dValue = (double)m_vectorLongLongs[ ulIndex ]; break;
		case VT_UI8: dValue = (double)m_vectorULongLongs[ ulIndex ]; break;
		case VT_R4: dValue = m_vectorFloats[ ulIndex ]; break;
		case VT_R8: dValue = m_vectorDoubles[ ulIndex ]; break;
	}
	// validate the value
	if ( !::_finite( dValue ) )
	{
		dValue = getEmptyValue();
	}
	return dValue;
} // getDouble 

/////////////////////////////////////////////////////////////////////////////
// set a double value at a given index and return false if unsuccessful
bool CVariantVector::setDouble( ULONG ulIndex, DOUBLE dValue )
{
	const ULONG ulValues = count();
	if ( ulValues <= ulIndex )
	{
		return false;
	}
	const VARENUM nType = getType();
	switch ( nType )
	{
		case VT_I1: m_vectorChars[ ulIndex ] = (CHAR)dValue; return true;
		case VT_UI1: m_vectorBytes[ ulIndex ] = (BYTE)dValue; return true;
		case VT_I2: m_vectorShorts[ ulIndex ] = (SHORT)dValue; return true;
		case VT_UI2: m_vectorUShorts[ ulIndex ] = (USHORT)dValue; return true;
		case VT_I4: m_vectorLongs[ ulIndex ] = (LONG)dValue; return true;
		case VT_UI4: m_vectorULongs[ ulIndex ] = (ULONG)dValue; return true;
		case VT_I8: m_vectorLongLongs[ ulIndex ] = (LONGLONG)dValue; return true;
		case VT_UI8: m_vectorULongLongs[ ulIndex ] = (ULONGLONG)dValue; return true;
		case VT_R4: m_vectorFloats[ ulIndex ] = (FLOAT)dValue; return true;
		case VT_R8: m_vectorDoubles[ ulIndex ] = dValue; return true;
	}
	return false;
} // setDouble 

/////////////////////////////////////////////////////////////////////////////
// append a double value and return the count
ULONG CVariantVector::appendDouble( DOUBLE dValue )
{
	const VARENUM nType = getType();
	switch ( nType )
	{
		case VT_I1: m_vectorChars.push_back( (CHAR)dValue ); break;
		case VT_UI1: m_vectorBytes.push_back( (BYTE)dValue ); break;
		case VT_I2: m_vectorShorts.push_back( (SHORT)dValue ); break;
		case VT_UI2: m_vectorUShorts.push_back( (USHORT)dValue ); break;
		case VT_I4: m_vectorLongs.push_back( (LONG)dValue ); break;
		case VT_UI4: m_vectorULongs.push_back( (ULONG)dValue ); break;
		case VT_I8: m_vectorLongLongs.push_back( (LONGLONG)dValue ); break;
		case VT_UI8: m_vectorULongLongs.push_back( (ULONGLONG)dValue ); break;
		case VT_R4: m_vectorFloats.push_back( (FLOAT)dValue ); break;
		case VT_R8: m_vectorDoubles.push_back( dValue ); break;
	}
	const ULONG ulValues = count();
	return ulValues;
} // appendDouble 

/////////////////////////////////////////////////////////////////////////////
// set a double array into the vector and return false if unsuccessful
bool CVariantVector::setDoubles( vector<DOUBLE>& arrValues )
{
	ULONG ulValues = count();
	const ULONG ulDoubles = (ULONG)arrValues.size();
	if ( ulValues != ulDoubles )
	{
		resize( ulDoubles );
		ulValues = count();
	}
	const VARENUM nType = getType();
	switch ( nType )
	{
		case VT_I1:
			for ( ULONG ulIndex = 0; ulIndex < ulValues; ulIndex++ )
			{
				m_vectorChars[ ulIndex ] = (CHAR)arrValues[ ulIndex ];
			}
			return true;
		case VT_UI1:
			for ( ULONG ulIndex = 0; ulIndex < ulValues; ulIndex++ )
			{
				m_vectorBytes[ ulIndex ] = (BYTE)arrValues[ ulIndex ];
			}
			return true;
		case VT_I2:
			for ( ULONG ulIndex = 0; ulIndex < ulValues; ulIndex++ )
			{
				m_vectorShorts[ ulIndex ] = (SHORT)arrValues[ ulIndex ];
			}
			return true;
		case VT_UI2:
			for ( ULONG ulIndex = 0; ulIndex < ulValues; ulIndex++ )
			{
				m_vectorUShorts[ ulIndex ] = (USHORT)arrValues[ ulIndex ];
			}
			return true;
		case VT_I4:
			for ( ULONG ulIndex = 0; ulIndex < ulValues; ulIndex++ )
			{
				m_vectorLongs[ ulIndex ] = (LONG)arrValues[ ulIndex ];
			}
			return true;
		case VT_UI4:
			for ( ULONG ulIndex = 0; ulIndex < ulValues; ulIndex++ )
			{
				m_vectorULongs[ ulIndex ] = (ULONG)arrValues[ ulIndex ];
			}
			return true;
		case VT_I8:
			for ( ULONG ulIndex = 0; ulIndex < ulValues; ulIndex++ )
			{
				m_vectorLongLongs[ ulIndex ] = (LONGLONG)arrValues[ ulIndex ];
			}
			return true;
		case VT_UI8:
			for ( ULONG ulIndex = 0; ulIndex < ulValues; ulIndex++ )
			{
				m_vectorULongLongs[ ulIndex ] = (ULONGLONG)arrValues[ ulIndex ];
			}
			return true;
		case VT_R4:
			for ( ULONG ulIndex = 0; ulIndex < ulValues; ulIndex++ )
			{
				m_vectorFloats[ ulIndex ] = (FLOAT)arrValues[ ulIndex ];
			}
			return true;
		case VT_R8: m_vectorDoubles = arrValues; return true;
	}
	return false;
} // setDoubles 

/////////////////////////////////////////////////////////////////////////////
// set a double array into the given row and column 
// returns false if outside of the matrix dimensions
// and if the number of values is not the same as the
// number of elements in a column
bool CVariantVector::setDoubles
(	ULONG ulRow, ULONG ulColumn, vector<DOUBLE>& arrValues 
)
{
	const ULONG ulRows = getRows();
	if ( ulRow >= ulRows )
	{
		return false;
	}
	const ULONG ulColumns = getColumns();
	if ( ulColumn >= ulColumns )
	{
		return false;
	}
	const ULONG ulColumnElements = getColumnElements();
	const ULONG ulElements = (ULONG)arrValues.size();
	ULONG ulIndex = getIndex( ulRow, ulColumn );
	for ( auto& dValue : arrValues )
	{
		setDouble( ulIndex++, dValue );
	}
	return true;
} // setDoubles

/////////////////////////////////////////////////////////////////////////////
// get a double array from the given row and column 
// returns false if outside of the matrix dimensions
// all of the elements of the row are returned
bool CVariantVector::getDoubles
(	ULONG ulRow, ULONG ulColumn, vector<DOUBLE>& arrValues 
)
{
	arrValues.clear();
	const ULONG ulRows = getRows();
	if ( ulRow >= ulRows )
	{
		return false;
	}
	const ULONG ulColumns = getColumns();
	if ( ulColumn >= ulColumns )
	{
		return false;
	}
	const ULONG ulElements = getColumnElements();

	arrValues.resize( ulElements );
	ULONG ulIndex = getIndex( ulRow, ulColumn );
	for ( ULONG ulElement = 0; ulElement < ulElements; ulElement++ )
	{
		arrValues[ ulElement ] = getDouble( ulIndex++ );
	}
	return true;
} // getDoubles

/////////////////////////////////////////////////////////////////////////////
// set a variant vector into the given row and column 
// returns false if outside of the matrix dimensions,
// the number of values is not the same as the number of 
// elements in a column, and the types do not match
bool CVariantVector::setColumn
(	ULONG ulRow, 
	ULONG ulColumn, 
	CVariantVector& arrValues 
)
{
	const ULONG ulRows = getRows();
	if ( ulRow >= ulRows )
	{
		return false;
	}
	const ULONG ulColumns = getColumns();
	if ( ulColumn >= ulColumns )
	{
		return false;
	}
	const VARENUM eType = arrValues.getType();
	if ( eType != getType() )
	{
		return false;
	}
	const ULONG ulColumnElements = getColumnElements();
	const ULONG ulElements = arrValues.count();
	if ( ulColumnElements != ulElements )
	{
		return false;
	}
	const ULONG ulBytes = getColumnSizeInBytes();
	const ULONG ulIndex = getIndex( ulRow, ulColumn );

	void* pDest = getData( ulIndex );
	void* pSource = arrValues.getData();
	::CopyMemory( pDest, pSource, ulBytes );
	return true;
} // setColumn
	
/////////////////////////////////////////////////////////////////////////////
// getData(COleSafeArray&)
//
// Exports the internal buffer to a COM SafeArray of the active VARIANT type.
/////////////////////////////////////////////////////////////////////////////
void CVariantVector::getData( COleSafeArray& sa )
{
	const VARENUM vt = getType();
	const ULONG ulElements = count();
	if ( ulElements == 0 )
	{
		return;
	}
	void* pData = getData();
	sa.CreateOneDim( vt, ulElements, (const void*)pData );
} // getData

/////////////////////////////////////////////////////////////////////////////
// setData(VARIANT&)
//
// Imports a SafeArray into the internal buffer, converting type as needed.
/////////////////////////////////////////////////////////////////////////////
bool CVariantVector::setData( VARIANT& data )
{
	bool bOK = false;
	if ( !( data.vt & VT_ARRAY ) )
	{
		TRACE( _T( "CVariantVector::setData error: expected an array.\n" ) );
		return bOK;
	}
	const VARENUM nType = (VARENUM)( data.vt & VT_TYPEMASK );
	const ULONG ulValues =
		(ULONG)CHelper::GetOneDimensionalElementCount( data );

	char* pData = 0;
	::SafeArrayAccessData( data.parray, (void**)&pData );
	bOK = setData( nType, ulValues, pData );
	::SafeArrayUnaccessData( data.parray );
	bOK = ulValues == count();
	return bOK;
} // setData

/////////////////////////////////////////////////////////////////////////////
