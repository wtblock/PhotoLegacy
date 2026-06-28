/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include "VariantVector.h"
#include "CHelper.h"
#include <gdiplus.h>

using namespace Gdiplus;

/////////////////////////////////////////////////////////////////////////////
// container holding the property
	class CImageProperties;

/////////////////////////////////////////////////////////////////////////////
class CImageProperty
{
// public data type
public:
	class CPropertyItem
	{
	public:
		PROPID id;		// ID of this property
		ULONG length;	// Length of the property value, in bytes
		WORD type;		// Type of the value, as one of TAG_TYPE_XXX
						// defined above
		VOID* value;	// property value

		CPropertyItem( ULONG ulID, WORD wType, ULONG ulLength )
		{
			id = ulID;
			type = wType;
			length = ulLength;
			value = (void*)malloc( ulLength );
		}
		~CPropertyItem()
		{
			free( value );
		}
		// Conversion operator to Gdiplus::PropertyItem* 
		operator Gdiplus::PropertyItem*() 
		{
			return reinterpret_cast<Gdiplus::PropertyItem*>(this); 
		}
	};

// protected data
protected:
	// data container
	CVariantVector m_PropertyValue;

	// the parent container of properties
	CImageProperties* m_pContainer;

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

	// the length of the property in bytes
	ULONG m_ulLength;

	// modification flag
	bool m_bModified;

// public properties
public:
	// pathname of the image that contains this metadata
	_variant_t GetValue();
	// pathname of the image that contains this metadata
	void SetValue( _variant_t value );
	// pathname of the image that contains this metadata
	__declspec( property( get = GetValue, put = SetValue ) )
		_variant_t Value;

	// the data in the property 
	inline CVariantVector* GetData()
	{
		return &m_PropertyValue;
	}
	// the data in the property 
	__declspec( property( get = GetData ))
		CVariantVector* Data;
	
	// the parent container of properties
	CImageProperties* GetContainer()
	{
		return m_pContainer;
	}
	// the parent container of properties
	void SetContainer( CImageProperties* value )
	{
		m_pContainer = value;
	}
	// the parent container of properties
	__declspec( property( get = GetContainer, put = SetContainer ))
		CImageProperties* Container;
	
	// void* pointer to the raw data
	inline void* GetDataItem()
	{
		return Data->getData();
	}
	// item pointer from original property
	__declspec( property( get = GetDataItem ))
		void* DataItem;
	
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
	__declspec( property( get = GetDataType, put = SetDataType ))
		VARENUM DataType;
	
	// image property type 
	inline WORD GetPropertyType()
	{
		return m_wPropertyType;
	}
	// image property type 
	void SetPropertyType( WORD value );
	// image property type 
	__declspec( property( get = GetPropertyType, put = SetPropertyType ) )
		WORD PropertyType;

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

	// image property label
	inline CString GetPropertyKey()
	{
		CString value;
		const CString csGroup = PropertyGroup;
		const CString csLabel = PropertyLabel;
		value.Format( L"%s|%s", csGroup, csLabel );
		return value;
	}
	// image property label
	inline void SetPropertyKey( CString value )
	{
		int nStart = 0;
		const CString csGroup = value.Tokenize( L"|", nStart );
		const CString csLabel = value.Tokenize( L"|", nStart );
		PropertyGroup = csGroup;
		PropertyLabel = csLabel;
	}
	// image property label
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

	// the length of the property in bytes
	inline ULONG GetLength()
	{
		return m_ulLength;
	}
	// the length of the property in bytes
	inline void SetLength( ULONG value )
	{
		m_ulLength = value;
	}
	// the length of the property in bytes
	__declspec( property( get = GetLength, put = SetLength ))
		ULONG Length;

	// return component configuration as a comma separated string
	CString GetComponentConfiguration();
	// return component configuration as a comma separated string
	__declspec( property( get = GetComponentConfiguration ))
		CString ComponentConfiguration;

	// return dropdown options
	vector<CString> GetDropdownChoices();
	// return dropdown options
	__declspec( property( get = GetDropdownChoices ))
		vector<CString> DropdownChoices;

	// return meaningful string representing the value
	CString GetUserInterfaceValue();
	// return meaningful string representing the value
	__declspec( property( get = GetUserInterfaceValue ))
		CString UserInterfaceValue;

	// return a comma separated value of the data
	CString GetCommaSeparatedValue();
	// return a comma separated value of the data
	__declspec( property( get = GetCommaSeparatedValue ))
		CString CommaSeparatedValue;

	// modification flag
	inline bool GetModified()
	{
		return m_bModified;
	}
	// modification flag
	void SetModified( bool value );
	// modification flag
	__declspec( property( get = GetModified, put = SetModified ) )
		bool Modified;

// public methods
public:
	/////////////////////////////////////////////////////////////////////////////
	void GetPropertyItem( unique_ptr<CPropertyItem>& pItem )
	{
		pItem = unique_ptr<CPropertyItem>
		(
			new CPropertyItem
			(
				PropertyID, PropertyType, Data->NumberOfBytes
			)
		);
		::CopyMemory( pItem->value, Data->getData(), pItem->length );
	}

	/////////////////////////////////////////////////////////////////////////////
	// given an input void pointer to some data and the length of the data 
	// in bytes, return a vector of 
	template <class T> bool CopyArray
	(
		void* pValue, // array of value of type T
		ULONG bytes, // the length of the array in bytes
		T tType // let the template know the type
	)
	{
		bool value = false;
		vector<T> data;
		value = CHelper::ArrayToVector( pValue, bytes, tType, data );
		if ( value )
		{
			VARENUM varType = DataType;
			VARIANT varData;
			value = Data->CopyVector( data, varType, &varData );
			if ( value )
			{
				Data->setData( varData );
				::VariantClear( &varData );
			}
		}

		return value;
	} // CopyArray

	/////////////////////////////////////////////////////////////////////////////
	// given an input void pointer to some data and the length of the data 
	// in bytes, return a vector of 
	bool CopyText
	(
		void* pValue, // array of value of type char
		ULONG bytes // the length of the array in bytes
	)
	{
		bool value = false;
		value = Data->setData( VT_I1, bytes, (char*)pValue );
		return value;
	} // CopyText

	/////////////////////////////////////////////////////////////////////////////
	bool UpdateProperty( Image* image );

	// generates a property item for this property
	void GeneratePropertyItem( unique_ptr<Gdiplus::PropertyItem>& pItem );

// protected overrides
protected:

// public overrides
public:

// public constructor/destructor
public:
	CImageProperty();
	~CImageProperty();

};

/////////////////////////////////////////////////////////////////////////////
