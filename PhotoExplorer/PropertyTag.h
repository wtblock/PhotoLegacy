/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
class CPropertyTag
{
// public types
public:
	typedef enum 
	{
		teSimple = 0,
		teDate,
		teEnum,
		teGPS,
		teRotation
	} TAG_ENTRY_TYPE;

// protected data
protected:
	// data type 
	VARENUM m_varDataType;

	// image property type 
	WORD m_wPropertyType;

	// image property ID
	ULONG m_ulPropertyID;

	// image property group
	CString m_csPropertyGroup;

	// image property label
	CString m_csPropertyLabel;

	// image property description
	CString m_csPropertyDescription;

	// read only property
	bool m_bReadonly;

	// specialize data entry type
	TAG_ENTRY_TYPE m_eEntryType;

	// enumeration collection with value keys
	CKeyedCollection<USHORT, CString> m_EnumValues;

	// enumeration collection with text keys
	CKeyedCollection<CString, USHORT> m_EnumText;


// public properties
public:
	// image property ID
	inline ULONG GetPropertyID()
	{
		return m_ulPropertyID;
	}
	// image property ID
	inline void SetPropertyID( ULONG value )
	{
		m_ulPropertyID = value;
	}
	// image property ID
	__declspec( property( get = GetPropertyID, put = SetPropertyID ))
		ULONG PropertyID;

	// data type 
	inline VARENUM GetDataType()
	{
		return m_varDataType;
	}
	// data type 
	inline void SetDataType( VARENUM value )
	{
		m_varDataType = value;
	}
	// data type 
	__declspec( property( get = GetDataType, put = SetDataType ) )
		VARENUM DataType;

	// image property type 
	inline WORD GetPropertyType()
	{
		return m_wPropertyType;
	}
	// image property type 
	inline void SetPropertyType( WORD value )
	{
		m_wPropertyType = value;
	}
	// image property type 
	__declspec( property( get = GetPropertyType, put = SetPropertyType ) )
		WORD PropertyType;

	// image property group
	inline CString GetPropertyGroup()
	{
		return m_csPropertyGroup;
	}
	// image property group
	inline void SetPropertyGroup( CString value )
	{
		m_csPropertyGroup = value;
	}
	// image property group
	__declspec( property( get = GetPropertyGroup, put = SetPropertyGroup ) )
		CString PropertyGroup;

	// image property label
	inline CString GetPropertyLabel()
	{
		return m_csPropertyLabel;
	}
	// image property label
	inline void SetPropertyLabel( CString value )
	{
		m_csPropertyLabel = value;
	}
	// image property label
	__declspec( property( get = GetPropertyLabel, put = SetPropertyLabel ) )
		CString PropertyLabel;

	// image property key
	inline CString GetPropertyKey()
	{
		CString value;
		value.Format( L"%s|%s", PropertyGroup, PropertyLabel );
		return value;
	}
	// image property key
	inline void SetPropertyKey( CString value )
	{
		int nStart = 0;
		PropertyGroup = value.Tokenize( L"|", nStart );
		PropertyLabel = value.Tokenize( L"|", nStart );
	}
	// image property key
	__declspec( property( get = GetPropertyKey, put = SetPropertyKey ) )
		CString PropertyKey;

	// image property description
	inline CString GetPropertyDescription()
	{
		return m_csPropertyDescription;
	}
	// image property description
	inline void SetPropertyDescription( CString value )
	{
		m_csPropertyDescription = value;
	}
	// image property description
	__declspec( property( get = GetPropertyDescription, put = SetPropertyDescription ) )
		CString PropertyDescription;

	// read only property
	inline bool GetReadonly()
	{
		return m_bReadonly;
	}
	// read only property
	inline void SetReadonly( bool value )
	{
		m_bReadonly = value;
	}
	// read only property
	__declspec( property( get = GetReadonly, put = SetReadonly ) )
		bool Readonly;

	// specialize data entry type
	inline TAG_ENTRY_TYPE GetEntryType()
	{
		return m_eEntryType;
	}
	// specialize data entry type
	inline void SetEntryType( TAG_ENTRY_TYPE value )
	{
		m_eEntryType = value;
	}
	// specialize data entry type
	__declspec( property( get = GetEntryType, put = SetEntryType ) )
		TAG_ENTRY_TYPE EntryType;

	// enumeration collection with value keys
	inline CKeyedCollection<USHORT, CString>* GetEnumValues()
	{
		return &m_EnumValues;
	}
	// enumeration collection with value keys
	__declspec( property( get = GetEnumValues ) )
		CKeyedCollection<USHORT, CString>* EnumValues;

	// enumeration collection with text keys
	inline CKeyedCollection<CString, USHORT>* GetEnumText()
	{
		return &m_EnumText;
	}
	// enumeration collection with text keys
	__declspec( property( get = GetEnumText ) )
		CKeyedCollection<CString, USHORT>* EnumText;

	// enumeration collection with text keys
	inline USHORT GetEnum( CString text )
	{
		USHORT value = 0;
		if ( EnumText->Exists[ text ] )
		{
			value = *EnumText->find( text );
		}
		return value;
	}
	// enumeration collection with text keys
	__declspec( property( get = GetEnum ) )
		USHORT Enum[];

// protected methods
protected:

// public methods
public:
	// add enumeration values
	void AddEnumerations( USHORT usValue, CString csValue )
	{
		m_EnumValues.add( usValue, shared_ptr<CString>( new CString( csValue )));
		m_EnumText.add( csValue, shared_ptr<USHORT>( new USHORT( usValue )));
		EntryType = teEnum;
	}

	// get a vector of enumeration text in value order
	vector<CString> GetEnumerationText()
	{
		vector<CString> value;
		for ( auto& node : m_EnumValues.Items )
		{
			value.push_back( *node.second );
		}
		return value;
	}

// protected overrides
protected:

// public overrides
public:

// public constructor/destructor
public:
	CPropertyTag();
	~CPropertyTag()
	{
	}
};

