/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "CHelper.h"
#include <vector>
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// this class is used to buffer data in a type safe representation
// that can contain simple vector, two dimensional array, and three 
// dimensional data as an array of arrays

// columns refer to the columns of a matrix and defaults to one
// column. elements refer to the number of values in a level for each column
// and also defaults to one.  the default constructor will therefore create
// a simple vector.

// element refers to a single value of an intrinsic type (long, float, etc.)

// all indices are zero based (i.e. row, column, element)

class CVariantVector
{
// protected data
protected:
	// empty value is used to initialize data vector during a resize
	double m_dEmptyValue;
	
	// data type of the curve
	//  VT_EMPTY		undefined
	//  VT_R4			4 byte real
	//  VT_R8			8 byte real
	//  VT_I1			signed char
	//  VT_UI1			unsigned char
	//  VT_I2			2 byte signed int
	//  VT_UI2			unsigned short
	//  VT_I4			4 byte signed int
	//  VT_UI4			unsigned long
	//  VT_I8			signed 64-bit int
	//  VT_UI8			unsigned 64-bit int
	VARENUM m_vtType;

	// by default the data is treated as a vector of elements
	// but by setting the following, access can be provided
	// to support a matrix of values
	ULONG m_ulColumns; // defaults to one

	// number of elements per column
	ULONG m_ulColumnElements; // defaults to one

	// units are stored here as a reference but
	// are not required internally
	CString m_csUnits;
	
	// unit class is stored here as a reference but
	// are not required internally
	CString m_csUnitClass;

	// multiplier value used for conversions
	DOUBLE m_dMultiplier;

	// additive value used for conversions
	DOUBLE m_dAdditive;
	
	// data dependent buffers
	vector<CHAR> m_vectorChars;
	vector<BYTE> m_vectorBytes;
	vector<SHORT> m_vectorShorts;
	vector<USHORT> m_vectorUShorts;
	vector<LONG> m_vectorLongs;
	vector<ULONG> m_vectorULongs;
	vector<LONGLONG> m_vectorLongLongs;
	vector<ULONGLONG> m_vectorULongLongs;
	vector<FLOAT> m_vectorFloats;
	vector<DOUBLE> m_vectorDoubles;

	// first live value is the first non-null value
	DOUBLE m_dFirstLive;

	// level of first live value 
	ULONG m_ulLevelFirstLive;

	// first live value is the first non-null value
	DOUBLE m_dLastLive;

	// level of last live value 
	ULONG m_ulLevelLastLive;

	// minimum value
	DOUBLE m_dMinimum;

	// level of minimum value 
	ULONG m_ulLevelMinimum;

	// Maximum value
	DOUBLE m_dMaximum;

	// level of maximum value 
	ULONG m_ulLevelMaximum;

	// Average value
	DOUBLE m_dAverage;

// public attributes
public:
	// number of elements of data
	ULONG count();
	// number of elements of data
	__declspec( property( get=count ))
		ULONG Elements;
	
	// units are stored here as a reference but
	// are not required internally
	inline CString getUnits()
	{ 
		return m_csUnits; 
	}
	// units are stored here as a reference but
	// are not required internally
	inline void setUnits( LPCTSTR value = _T( "" ))
	{ 
		m_csUnits = value; 
	}
	// units are stored here as a reference but
	// are not required internally
	__declspec( property( get=getUnits, put=setUnits ))
		CString Units;

	// unit class is stored here as a reference but
	// are not required internally
	inline CString getUnitClass()
	{ 
		return m_csUnitClass; 
	}
	// unit class is stored here as a reference but
	// are not required internally
	inline void setUnitClass( LPCTSTR value = _T( "" ))
	{ 
		m_csUnitClass = value; 
	}
	// unit class is stored here as a reference but
	// are not required internally
	__declspec( property( get=getUnitClass, put=setUnitClass ))
		CString UnitClass;

	// empty value is used to initialize data vectors during a resize
	inline double getEmptyValue()
	{ 
		return m_dEmptyValue; 
	}
	// empty value is used to initialize data vectors during a resize
	inline void setEmptyValue( double value = -9999.0 )
	{ 
		m_dEmptyValue = value; 
	}
	// empty value is used to initialize data vectors during a resize
	__declspec( property( get=getEmptyValue, put=setEmptyValue ))
		double Null;
	
	// multiplier value used for conversions
	inline double getMultiplier()
	{ 
		return m_dMultiplier; 
	}
	// multiplier value used for conversions
	inline void setMultiplier( double value = -9999.0 )
	{ 
		m_dMultiplier = value; 
	}
	// multiplier value used for conversions
	__declspec( property( get=getMultiplier, put=setMultiplier ))
		double Multiplier;
	
	// additive value used for conversions
	inline double getAdditive()
	{ 
		return m_dAdditive; 
	}
	// additive value used for conversions
	inline void setAdditive( double value = -9999.0 )
	{ 
		m_dAdditive = value; 
	}
	// additive value used for conversions
	__declspec( property( get=getAdditive, put=setAdditive ))
		double Additive;
	
	// number of columns (defaults to one) of a matrix
	inline ULONG getColumns()
	{ 
		return m_ulColumns; 
	}
	// number of columns (defaults to one) of a matrix
	inline void setColumns( ULONG value = 1 )
	{ 
		m_ulColumns = value; 
	}
	// number of columns (defaults to one) of a matrix
	__declspec( property( get=getColumns, put=setColumns ))
		ULONG Columns;
	
	// number of elements per column
	inline ULONG getColumnElements()
	{ 
		return m_ulColumnElements; 
	}
	// number of elements per column
	inline void setColumnElements( ULONG value = 1 )
	{ 
		m_ulColumnElements = value; 
	}
	// number of elements per column
	__declspec( property( get=getColumnElements, put=setColumnElements ))
		ULONG ColumnElements;

	// number of elements in a row
	inline ULONG getRowElements()
	{
		const ULONG ulColumns = Columns;
		const ULONG ulElements = ColumnElements;
		const ULONG ulRowSize = ulColumns * ulElements;
		return ulRowSize;
	}
	// number of elements in a row
	__declspec( property( get=getRowElements ))
		ULONG RowElements;

	// get the number of rows (defaults to number of elements)
	inline ULONG getRows()
	{
		ULONG ulRows = 0;
		const ULONG ulCount = Elements;
		if ( ulCount == 0 )
		{
			return ulRows;
		}
		const ULONG ulRowSize = RowElements;
		ulRows = ulCount / ulRowSize;
		return ulRows;
	}
	// get the number of rows (defaults to number of elements)
	__declspec( property( get=getRows ))
		ULONG Rows;

	// calculate an index based on row, column, and element
	inline ULONG getIndex
	(	ULONG ulRow = 0, // equivalent to a level number
		ULONG ulColumn = 0, // matrix column
		ULONG ulElement = 0 
	)
	{
		const ULONG ulRowElements = RowElements;
		const ULONG ulColumnElements = ColumnElements;
		const ULONG ulIndex = 
			ulRowElements * ulRow + ulColumnElements * ulColumn + ulElement;
		return ulIndex;
	}
	
	// data type of the curve
	inline VARENUM getType()
	{ 
		return m_vtType; 
	}
	// data type of the curve
	inline void setType( VARENUM value = VT_EMPTY )
	{ 
		m_vtType = value; 
	}
	// data type of the curve
	__declspec( property( get=getType, put=setType ))
		VARENUM Type;

	// change the data type of an existing buffer by copying from one 
	// buffer to another as a byte array...it is up to the application
	// to insure this makes sense.  For example, if the vector is sized
	// for 10 doubles (80 bytes), data is copied into the vector as
	// bytes, and changeType is used to put the data back to doubles, then
	// the byte array should be 80 bytes long.
	bool changeType( VARENUM value );
	
	// get a value at a given index
	COleVariant getValue( ULONG ulIndex );
	// get a value at a row, column, and element
	inline COleVariant getValue
	( 
		ULONG ulRow, ULONG ulColumn, ULONG ulElement = 0 
	)
	{
		const ULONG ulIndex = getIndex( ulRow, ulColumn, ulElement );
		return getValue( ulIndex );
	}
	// set a value at a given index
	void setValue( ULONG ulIndex, COleVariant var );
	// set a value at a given index
	inline void setValue
	( 
		COleVariant var, ULONG ulRow, ULONG ulColumn = 0, ULONG ulElement = 0 
	)
	{	const ULONG ulIndex = getIndex( ulRow, ulColumn, ulElement );
		setValue( ulIndex, var );
	}
	
	// get a double value at a given index and return null value if unsuccessful
	DOUBLE getDouble( ULONG ulIndex );
	// get a double value at a given row, column, and element.
	// return null value if unsuccessful
	inline DOUBLE getDouble( ULONG ulRow, ULONG ulColumn, ULONG ulElement = 0 )
	{	const ULONG ulIndex = getIndex( ulRow, ulColumn, ulElement );
		return getDouble( ulIndex );
	}
	// set a double value at a given index and return false if unsuccessful
	bool setDouble( ULONG ulIndex, DOUBLE dValue );
	// set a double value at the given row, column, and element
	// and return false if unsuccessful
	inline bool setDouble
	(	DOUBLE dValue, ULONG ulRow, ULONG ulColumn = 0, ULONG ulElement = 0 
	)
	{	const ULONG ulIndex = getIndex( ulRow, ulColumn, ulElement );
		return setDouble( ulIndex, dValue );
	}
	// append a double value and return the count
	ULONG appendDouble( DOUBLE dValue );

	// set a double array into the vector and return false if unsuccessful
	bool setDoubles( vector<DOUBLE>& arrValues );
	// set a double array into the given row and column 
	// returns false if outside of the matrix dimensions
	// and if the number of values is not the same as the
	// number of elements in a column
	bool setDoubles( ULONG ulRow, ULONG ulColumn, vector<DOUBLE>& arrValues );
	// get a double array from the given row and column 
	// returns false if outside of the matrix dimensions
	// all of the elements of the row are returned
	bool getDoubles( ULONG ulRow, ULONG ulColumn, vector<DOUBLE>& arrValues );
	// set a variant vector into the given row and column 
	// returns false if outside of the matrix dimensions,
	// the number of values is not the same as the number of 
	// elements in a column, and the types do not match
	bool setColumn( ULONG ulRow, ULONG ulColumn, CVariantVector& arrValues );
	
	// data element size in bytes (0 for unsupported type)
	inline short getElementSizeInBytes()
	{
		return getElementSizeInBytes( getType() );
	}
	// data element size in bytes (0 for unsupported type)
	__declspec( property( get = getElementSizeInBytes ) )
		short ElementSizeInBytes;

	// get the column size in bytes
	inline ULONG getColumnSizeInBytes()
	{
		const ULONG ulElementSize = getElementSizeInBytes();
		const ULONG ulElements = getColumnElements();
		const ULONG ulBytes = ulElementSize * ulElements;
		return ulBytes;
	}
	// get the column size in bytes
	__declspec( property( get = getColumnSizeInBytes ) )
		ULONG ColumnSizeInBytes;

	// get the row size in bytes
	inline ULONG getRowSizeInBytes()
	{
		const ULONG ulElementSize = getElementSizeInBytes();
		const ULONG ulElements = getRowElements();
		const ULONG ulBytes = ulElementSize * ulElements;
		return ulBytes;
	}
	// get the row size in bytes
	__declspec( property( get = getRowSizeInBytes ) )
		ULONG RowSizeInBytes;

	// total number of bytes in the array
	inline ULONG getNumberOfBytes()
	{
		const ULONG ulElementSize = getElementSizeInBytes();
		const ULONG ulElements = Elements;
		const ULONG ulBytes = ulElementSize * ulElements;
		return ulBytes;
	}
	// total number of bytes in the array
	__declspec( property( get=getNumberOfBytes ))
		ULONG NumberOfBytes;
	
	// first live value is the first non-null value
	inline double getFirstLive()
	{
		return m_dFirstLive;
	}
	// first live value is the first non-null value
	inline void setFirstLive( double value = -9999.0 )
	{
		m_dFirstLive = value;
	}
	// first live value is the first non-null value
	__declspec( property( get = getFirstLive, put = setFirstLive ) )
		double FirstLive;

	// level of first live value 
	inline ULONG getLevelFirstLive()
	{
		return m_ulLevelFirstLive;
	}
	// level of first live value 
	inline void setLevelFirstLive( ULONG value = -9999.0 )
	{
		m_ulLevelFirstLive = value;
	}
	// level of first live value 
	__declspec( property( get = getLevelFirstLive, put = setLevelFirstLive ) )
		ULONG LevelFirstLive;

	// last live value is the last non-null value
	inline double getLastLive()
	{
		return m_dLastLive;
	}
	// last live value is the last non-null value
	inline void setLastLive( double value = -9999.0 )
	{
		m_dLastLive = value;
	}
	// last live value is the last non-null value
	__declspec( property( get = getLastLive, put = setLastLive ) )
		double LastLive;

	// level of last live value 
	inline ULONG getLevelLastLive()
	{
		return m_ulLevelLastLive;
	}
	// level of last live value 
	inline void setLevelLastLive( ULONG value = -9999.0 )
	{
		m_ulLevelLastLive = value;
	}
	// level of last live value 
	__declspec( property( get = getLevelLastLive, put = setLevelLastLive ) )
		ULONG LevelLastLive;

	// minimum value
	inline double getMinimum()
	{
		return m_dMinimum;
	}
	// minimum value
	inline void setMinimum( double value = -9999.0 )
	{
		m_dMinimum = value;
	}
	// minimum value
	__declspec( property( get = getMinimum, put = setMinimum ) )
		double Minimum;

	// level of minimum value 
	inline ULONG getLevelMinimum()
	{
		return m_ulLevelMinimum;
	}
	// level of minimum value 
	inline void setLevelMinimum( ULONG value = -9999.0 )
	{
		m_ulLevelMinimum = value;
	}
	// level of minimum value 
	__declspec( property( get = getLevelMinimum, put = setLevelMinimum ) )
		ULONG LevelMinimum;

	// Maximum value
	inline double getMaximum()
	{
		return m_dMaximum;
	}
	// Maximum value
	inline void setMaximum( double value = -9999.0 )
	{
		m_dMaximum = value;
	}
	// Maximum value
	__declspec( property( get = getMaximum, put = setMaximum ) )
		double Maximum;

	// level of maximum value 
	inline ULONG getLevelMaximum()
	{
		return m_ulLevelMaximum;
	}
	// level of maximum value 
	inline void setLevelMaximum( ULONG value = -9999.0 )
	{
		m_ulLevelMaximum = value;
	}
	// level of maximum value 
	__declspec( property( get = getLevelMaximum, put = setLevelMaximum ) )
		ULONG LevelMaximum;

	// Average value
	inline double getAverage()
	{
		return m_dAverage;
	}
	// Average value
	inline void setAverage( double value = -9999.0 )
	{
		m_dAverage = value;
	}
	// Average value
	__declspec( property( get = getAverage, put = setAverage ) )
		double Average;

// public methods
public:
	// create a variant vector
	void Create
	(	double dNull, // the empty value
		VARENUM eType, // variant type
		ULONG ulColumns = 1, // matrix columns
		ULONG ulColumnElements = 1, // elements per column
		ULONG ulRows = 0, // number of rows (if set will fill with nulls)
		LPCTSTR pcszUnits = _T( "" ) // units
	);

	// assignment operator
	CVariantVector& operator=( CVariantVector& src );

	// convert values
	bool Convert( DOUBLE dMult, DOUBLE dAdd );

	// find the minimum and maximum values
	bool FindMinMax( DOUBLE& dMinimum, DOUBLE& dMaximum );	

	// calculate statistics--rather than making a pass through the data
	// separately for each statistic, calculate them all in one pass
	bool CalculateStatistics();

	// copy data into safe array
	void getData( COleSafeArray& sa );

	// assigns the data	from a safe array
	bool setData( VARIANT& data );
	
	// clear the data
	void clear();
	
	// set data from a char* pointer
	bool setData( VARENUM nType, ULONG ulValues, char* pData );
	
	// get a void pointer to the data and optionally to 
	// a specific index of the data
	void* getData( ULONG ulIndex = 0 );

	// get a void pointer to the data at a given row, column, and element
	// where the data is potentially formatted as a matrix which can 
	// be an array of arrays. for a simple two dimensional array, the
	// column will be zero and the element is the column.
	void* getData
	(	ULONG ulRow, // row of the table
		ULONG ulColumn, // matrix column
		ULONG ulElement = 0 // element of the column
	);
	
	// resize the data to the given number of elements. if the size
	// is extended, the new levels are filled with null values.
	// if the size is reduced, data will be deleted at the end.
	// if bProtectContents is true, the existing data will remain
	// unchanged except when the size is reduced.
	// if bProtectContents is false, the entire content will be
	// filled with null values.
	void resize
	(	// new size of the of the vector in values
		ULONG values, 
		// protect existing content when true or null the values when false
		bool bProtectContents = false 
	);
	
	// data element size in bytes (0 for unsupported type)
	static short getElementSizeInBytes( VARENUM nType );

	// copy a variant array to a vector
	template <class T>
	static inline bool CopyVariant
	(	VARIANT& Source, // source safe array
		VARENUM eVar, // type of variant expected
		vector<T>& Dest // vector is returned
	)
	{
		Dest.clear();
		bool bOK = Source.vt == ( VT_ARRAY | eVar );
		if ( !bOK )
		{
			return false;
		}

		// number of source values
		const long lValues =
			CHelper::GetOneDimensionalElementCount( Source );
		if ( lValues == 0 )
		{
			return false;
		}

		COleSafeArray sa( Source );
		T* pData = 0;
		sa.AccessData( (void**)&pData );
		for ( long lValue = 0; lValue < lValues; lValue++ )
		{
			const T value( pData[ lValue ] );
			Dest.push_back( value );
		}
		sa.UnaccessData();
		return true;
	}

	// copy vector into a safe array
	template <class T>
	static bool CopyVector
	(	vector<T>& Source, // source vector
		VARENUM eVar, // destination type corresponding to source type
		VARIANT* Dest // returned safe array
	)
	{
		::VariantInit( Dest );
		const long lValues = (long)Source.size();
		if ( lValues == 0 )
		{
			return false;
		}

		// create a 1 dimensional safe array of given type and initialize
		COleSafeArray sa;
		T* pData = &Source[ 0 ];
		sa.CreateOneDim( eVar, lValues, pData );

		// copy out the result
		::VariantCopy( Dest, &sa );
		return true;
	}

// construction / destruction
public:
	CVariantVector(void);
	CVariantVector
	(	double dNull, // the empty value
		VARENUM eType, // variant type
		ULONG ulColumns = 1, // matrix columns
		ULONG ulColumnElements = 1, // elements per column
		ULONG ulRows = 0, // number of rows (if set will fill with nulls)
		LPCTSTR pcszUnits = _T( "" ) // units
	);
	virtual ~CVariantVector(void);
};
