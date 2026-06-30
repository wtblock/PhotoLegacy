/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include <vector>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CPropertyTag
//
// Metadata definition class for Photo Explorer. Each instance of CPropertyTag
// describes a single EXIF/GDI+ metadata field, including its ID, type,
// group, label, description, read-only status, and any specialized entry
// behavior (date, enum, GPS, rotation). It also maintains enumeration tables
// for properties that require dropdown lists.
//
// Purpose:
//   • Define the structure and behavior of a metadata property.
//   • Provide group/label/description information for the property grid.
//   • Specify the EXIF/GDI+ property ID and type (WORD, VARENUM).
//   • Indicate whether the property is editable or read-only.
//   • Support specialized entry types (date, enum, GPS, rotation).
//   • Maintain enumeration mappings for dropdown-based properties.
//
// Why this class exists:
//   Raw EXIF/GDI+ metadata is identified only by numeric IDs and type codes.
//   To present metadata meaningfully in the UI, Photo Explorer needs a rich
//   definition layer that describes each property in human terms:
//     – Group (“Camera”, “Image”, “GPS”, “Thumbnail”)
//     – Label (“Exposure Time”, “ISO Speed”, “Date Taken”)
//     – Description (tooltip text)
//     – Data type (VARENUM)
//     – Property type (EXIF type code)
//     – Enumeration values (e.g., flash modes, metering modes)
//   CPropertyTag provides this definition layer.
//
// Responsibilities:
//   • Store metadata definition fields:
//       – PropertyID (EXIF/GDI+ ID)
//       – DataType (VARENUM)
//       – PropertyType (EXIF type code)
//       – PropertyGroup (category name)
//       – PropertyLabel (display name)
//       – PropertyDescription (tooltip)
//       – Readonly flag
//       – EntryType (simple, date, enum, GPS, rotation)
//   • Provide a combined key (“group|label”) for dictionary lookup.
//   • Maintain enumeration tables:
//       – EnumValues: USHORT → text
//       – EnumText: text → USHORT
//   • Provide helper methods for enumeration lookup and ordering.
//   • Support property grid integration through descriptive metadata.
//
// Interaction with other components:
//   • CImageProperties — uses CPropertyTag to build property grid entries,
//     determine editing behavior, and map EXIF IDs to UI keys.
//   • CPropertyGridCtrl — uses enumeration lists and entry types to render
//     dropdowns and specialized editors.
//   • CPropertiesWnd — displays properties using group/label/description
//     information from CPropertyTag.
//   • EXIF/GDI+ metadata system — provides raw property IDs and types.
//
// Key Features:
//   • Full metadata definition for each EXIF/GDI+ property.
//   • Enumeration support for dropdown-based properties.
//   • Specialized entry types for date, GPS, and rotation fields.
//   • Read-only flag for properties that cannot be edited.
//   • Combined “group|label” key for dictionary lookup.
//   • Bidirectional enumeration mapping (value→text and text→value).
//
// Internal Structure:
//   • m_varDataType — VARENUM type for property grid editing.
//   • m_wPropertyType — EXIF type code (BYTE, ASCII, SHORT, LONG, RATIONAL).
//   • m_ulPropertyID — numeric EXIF/GDI+ property ID.
//   • m_csPropertyGroup — category name.
//   • m_csPropertyLabel — display label.
//   • m_csPropertyDescription — tooltip text.
//   • m_bReadonly — indicates whether the property is editable.
//   • m_eEntryType — specialized entry behavior.
//   • m_EnumValues — USHORT→CString enumeration table.
//   • m_EnumText — CString→USHORT reverse lookup table.
//
// This class defines the metadata vocabulary of Photo Explorer, enabling
// structured, descriptive, and user-friendly presentation of EXIF/GDI+ data
// throughout the application.
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

