/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "KeyedCollection.h"
#include "ImageProperty.h"
#include "PropertyTag.h"
#include "MainFrm.h"
#include "PropertiesWnd.h"
#include "Date.h"
#include <vector>
#include <map>

using namespace std;

// adding this type to handle a couple of cases where the type was undefined
// Image property types and I could see the output was an array of characters
#define PropertyTagTypeChar 0

// similarly, I encountered some property tags that are not defined in GDI+
// and I determined what the data was to create these ID tags
// Image property ID tags
#define PropertyTagRating					0x4746
#define PropertyTagRatingPercent			0x4749
#define PropertyTagThumbnailInteropIndex	0x5041
#define PropertyTagThumbnailInteropVersion	0x5042
#define PropertyTagOffsetTime				0x9010
#define PropertyTagOrgOffsetTime			0x9011
#define PropertyTagExifMakerNote			0x927C

#define PropertyTagXPTitle					0x9C9B
#define PropertyTagXPComment				0x9C9C
#define PropertyTagXPKeywords				0x9C9E
#define PropertyTagXPSubject				0x9C9F

#define PropertyTagPrintIMVersion			0xC4A5

class CImageProperties
{
	////////////////////////////////////////////////////////////////////////////
	// this class creates a fast look up of the mime type and class ID as 
	// defined by GDI+ for common file extensions
	class CExtension
	{
		// protected definitions
	protected:
		typedef struct tagExtensionLookup
		{
			CString m_csFileExtension;
			CString m_csMimeType;

		} EXTENSION_LOOKUP;

		typedef struct tagClassLookup
		{
			CString m_csMimeType;
			CLSID m_ClassID;

		} CLASS_LOOKUP;

		// protected data
	protected:
		// current file extension
		CString m_csFileExtension;

		// current mime type
		CString m_csMimeType;

		// current class ID
		CLSID m_ClassID;

		// cross reference of file extensions to mime types
		CKeyedCollection<CString, CString> m_mapExtensions;

		// cross reference of mime types to class IDs
		CKeyedCollection<CString, CLSID> m_mapMimeTypes;

		// public properties
	public:
		// current file extension
		inline CString GetFileExtension()
		{
			return m_csFileExtension;
		}
		// current file extension
		void SetFileExtension( CString value );
		// current file extension
		__declspec( property( get = GetFileExtension, put = SetFileExtension ) )
			CString FileExtension;

		// image extension associated with the current file extension
		inline CString GetMimeType()
		{
			return m_csMimeType;
		}
		// image extension associated with the current file extension
		inline void SetMimeType( CString value )
		{
			m_csMimeType = value;
		}
		// get image extension associated with the current file extension
		__declspec( property( get = GetMimeType, put = SetMimeType ) )
			CString MimeType;

		// class ID associated with the current file extension
		inline CLSID GetClassID()
		{
			return m_ClassID;
		}
		// class ID associated with the current file extension
		inline void SetClassID( CLSID value )
		{
			m_ClassID = value;
		}
		// class ID associated with the current file extension
		__declspec( property( get = GetClassID, put = SetClassID ) )
			CLSID ClassID;

		// public methods
	public:

		// protected methods
	protected:

		// public virtual methods
	public:

		// protected virtual methods
	protected:

		// public construction
	public:
		CExtension()
		{
			// extension conversion table
			static EXTENSION_LOOKUP ExtensionLookup[] =
			{
				{ _T( ".bmp" ), _T( "image/bmp" ) },
				{ _T( ".dib" ), _T( "image/bmp" ) },
				{ _T( ".rle" ), _T( "image/bmp" ) },
				{ _T( ".gif" ), _T( "image/gif" ) },
				{ _T( ".jpeg" ), _T( "image/jpeg" ) },
				{ _T( ".jpg" ), _T( "image/jpeg" ) },
				{ _T( ".jpe" ), _T( "image/jpeg" ) },
				{ _T( ".jfif" ), _T( "image/jpeg" ) },
				{ _T( ".png" ), _T( "image/png" ) },
				{ _T( ".tiff" ), _T( "image/tiff" ) },
				{ _T( ".tif" ), _T( "image/tiff" ) }
			};

			// build a cross reference of file extensions to 
			// mime types
			const int nPairs = _countof( ExtensionLookup );
			for ( int nPair = 0; nPair < nPairs; nPair++ )
			{
				const CString csKey =
					ExtensionLookup[ nPair ].m_csFileExtension;

				shared_ptr<CString> pValue = shared_ptr<CString>
					(
						new CString( ExtensionLookup[ nPair ].m_csMimeType )
						);

				// add the pair to the collection
				m_mapExtensions.add( csKey, pValue );
			}
		}
	};

	// protected data
protected:
	// pathname of the image that contains this metadata
	CString m_csPathname;

	// the number of modified values
	WORD m_wModifiedValues;

	// this class creates a fast look up of the mime type and class ID as 
	// defined by GDI+ for common file extensions
	CExtension m_Extension;

	// this class records the date and time information in each image file 
	// referenced
	CDate m_Date;

	// collection of property tag types and their coresponding VARENUM 
	CKeyedCollection<WORD, VARENUM> m_TypeCollection;

	// collection of image PROPID tags and their corresponding keys
	CKeyedCollection<ULONG, CString> m_IdCollection;

	// collection of property tag definitions that defines constant 
	// information about specific properties like there group, label,
	// description, enumeration for dropdowns, etc.
	CKeyedCollection<CString, CPropertyTag> m_PropertyTags;

	// a collection of image properties indexed by property label
	CKeyedCollection<CString, CImageProperty> m_Properties;

	// date and time identifiers: 
	//		("Year", "Month", "Day", "Hour", "Minute", and "Second"
	// are associated with the indices 0 through 5
	// the date and time properties are in the following format:
	//		"year, month, day, hour, minute, second"
	// and the index identifies which position the ID is in the string
	CKeyedCollection<CString, WORD> m_DateIDs;

// public properties
public:
	// date and time identifiers: 
	//		("Year", "Month", "Day", "Hour", "Minute", and "Second"
	// are associated with the indices 0 through 5
	// the date and time properties are in the following format:
	//		"year, month, day, hour, minute, second"
	// and the index identifies which position the ID is in the string
	inline WORD GetDateTimeID( CString csLabel )
	{
		WORD value = 9999;
		if ( m_DateIDs.Exists[ csLabel ])
		{
			value = *m_DateIDs.find( csLabel );
		}
		return value;
	}
	// date and time identifiers: 
	//		("Year", "Month", "Day", "Hour", "Minute", and "Second"
	// are associated with the indices 0 through 5
	// the date and time properties are in the following format:
	//		"year, month, day, hour, minute, second"
	// and the index identifies which position the ID is in the string
	__declspec( property( get = GetDateTimeID ) )
		WORD DateTimeID[];

	// pathname of the image that contains this metadata
	inline CString GetPathname()
	{
		return m_csPathname;
	}
	// pathname of the image that contains this metadata
	inline void SetPathname( CString value )
	{
		m_csPathname = value;
	}
	// pathname of the image that contains this metadata
	__declspec( property( get = GetPathname, put = SetPathname ) )
		CString Pathname;

	// this class creates a fast look up of the mime type and class ID as 
	// defined by GDI+ for common file extensions
	inline CExtension* GetExtension()
	{
		return &m_Extension;
	}
	// this class creates a fast look up of the mime type and class ID as 
	// defined by GDI+ for common file extensions
	__declspec( property( get = GetExtension ) )
		CExtension* Extension;

	// this class records the date and time information in each image file 
	// referenced
	inline CDate* GetDate()
	{
		return &m_Date;
	}
	// this class records the date and time information in each image file 
	// referenced
	__declspec( property( get = GetDate ) )
		CDate* Date;

	// the number of modified values
	inline WORD GetModifiedValues()
	{
		return m_wModifiedValues;
	}
	// the number of modified values
	inline void SetModifiedValues( WORD value )
	{
		m_wModifiedValues = value;
	}
	// the number of modified values
	__declspec( property( get = GetModifiedValues, put = SetModifiedValues ) )
		WORD ModifiedValues;

	// pointer to the properties pane
	inline CPropertiesWnd* GetPropertiesPane()
	{
		CPropertiesWnd* value = nullptr;
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		value = pFrame->PropertiesPane;
		return value;
	}
	// pointer to the properties pane
	__declspec( property( get = GetPropertiesPane ) )
		CPropertiesWnd* PropertiesPane;

	// property key associated with a property ID
	// where the key is in the form of "group|label"
	inline CString GetIdKey( ULONG id )
	{
		CString value( L"Unknown" );
		const bool bKey = IDs->Exists[ id ];
		if ( bKey )
		{
			value = *IDs->find( id );
		}
		return value;
	}
	// property key associated with a property ID
	// where the key is in the form of "group|label"
	__declspec( property( get = GetIdKey ))
		CString IdKey[];

	// collection of property tag types and their coresponding VARENUM 
	inline CKeyedCollection<WORD, VARENUM>* GetTypes()
	{
		return &m_TypeCollection;
	}
	// collection of property tag types and their coresponding VARENUM 
	__declspec( property( get = GetTypes ) )
		CKeyedCollection<WORD, VARENUM>* Types;

	// collection of image PROPID tags and their corresponding keys
	inline CKeyedCollection<ULONG, CString>* GetIDs()
	{
		return &m_IdCollection;
	}
	// collection of image PROPID tags and their corresponding keys
	__declspec( property( get = GetIDs ) )
		CKeyedCollection<ULONG, CString>* IDs;

	// collection of property tag definitions that defines constant 
	// information about specific properties like their group, label,
	// description, enumeration for dropdowns, etc.
	inline CKeyedCollection<CString, CPropertyTag>* GetPropertyTags()
	{
		return &m_PropertyTags;
	}
	// collection of property tag definitions that defines constant 
	// information about specific properties like their group, label,
	// description, enumeration for dropdowns, etc.
	__declspec( property( get = GetPropertyTags ) )
		CKeyedCollection<CString, CPropertyTag>* PropertyTags;

	// property tag definitions that defines constant 
	// information about specific properties like their group,
	// label, description, enumeration for dropdowns, etc.
	shared_ptr<CPropertyTag> GetPropertyTag( CString csKey )
	{
		shared_ptr<CPropertyTag> value;

		if ( m_PropertyTags.Exists[ csKey ] )
		{
			value = m_PropertyTags.find( csKey );
		}

		return value;
	}
	// property tag definitions that defines constant 
	// information about specific properties like their group,
	// label, description, enumeration for dropdowns, etc.
	__declspec( property( get = GetPropertyTag ) )
		shared_ptr<CPropertyTag> PropertyTag[];

	// a collection of image properties indexed by property label
	inline CKeyedCollection<CString, CImageProperty>* GetProperties()
	{
		return &m_Properties;
	}
	// a collection of image properties indexed by property label
	__declspec( property( get = GetProperties ) )
		CKeyedCollection<CString, CImageProperty>* Properties;

	// access a property by key
	shared_ptr<CImageProperty> GetImageProperty( CString csKey )
	{
		shared_ptr<CImageProperty> value;

		if ( Properties->Exists[ csKey ] )
		{
			value = Properties->find( csKey );
		}

		return value;
	}
	// access a property by key
	__declspec( property( get = GetImageProperty ) )
		shared_ptr<CImageProperty> ImageProperty[];

	// variant type associated with this property
	inline VARENUM GetVariantType( WORD type )
	{
		VARENUM value = VT_ERROR;
		const bool bLabel = Types->Exists[ type ];
		if ( bLabel )
		{
			value = *Types->find( type );
		}
		return value;
	}
	// variant type associated with this property
	__declspec( property( get = GetVariantType ) )
		VARENUM VariantType[];

	// size of the property type
	BYTE GetPropertyTypeSize( WORD wType );
	// size of the property type
	__declspec( property( get = GetPropertyTypeSize ) )
		BYTE PropertyTypeSize[];

	// ASCII name for property type
	CString GetPropertyTypeName( WORD wType );
	// ASCII name for property type
	__declspec( property( get = GetPropertyTypeName ) )
		CString PropertyTypeName[];

	// dump property item
	CString GetDumpPropertyItem( Gdiplus::PropertyItem* pItem );
	// dump property item
	__declspec( property( get = GetDumpPropertyItem ) )
		CString DumpPropertyItem[];

// protected methods
protected:
	// add property panel date
	void AddPropertyPanelDate
	(
		CMFCPropertyGridProperty* pGroup,
		CString csLabel, CString csValue, CString csDescription
	);
	// add property panel multiline
	void AddPropertyPanelMultiline
	(
		CMFCPropertyGridProperty* pGroup,
		CString csLabel, CString csValue, CString csDescription
	);
// public methods
public:
	// return a comma separated value string of the given values
	template <class T> 
	CString ValuesCSV( void* pValue, T Type, ULONG bytes )
	{
		CString value;
		CString csTemp;
		T* pValues = reinterpret_cast<T*>( pValue );
		ULONG ulValues = bytes / sizeof( T );
		if ( ulValues > 50 )
		{
			ulValues = 50;
		}
		bool bDone = false;
		bool bChar = is_same<T, char>::value;
		for ( ULONG ulValue = 0; ulValue < ulValues; ulValue++ )
		{
			if ( bChar )
			{
				char val = char( pValues[ ulValue ] );
				value += CString( val );

			} else
			{
				T val = pValues[ ulValue ];
				csTemp.Format( L"%d, ", val );
				value += csTemp;
			}
		}

		value.TrimRight( L" ," );

		return value;
	}

	// clear the propeties collection
	void ClearProperties()
	{
		m_Properties.clear();
	}

	// a replacement method to: 
	// unique_ptr<Bitmap> pBitmap(Bitmap::FromFile((LPCTSTR)csPath));
	// which leaves the file open even after the bitmap pointer has been freed.
	unique_ptr<Bitmap> LoadBitmapFromFile( const CString& csPath );

	// apply album labels
	void ApplyAlbumLabels();

	// save property changes to the selected image and its thumbnail
	void SaveProperties();

	// add a property
	bool AddProperty( Gdiplus::PropertyItem* pItem );

	// create a new short property
	bool CreateShortProperty( CString& csTitle, USHORT& usValue );

	// create a new string property
	bool CreateStringProperty( CString& csTitle, CString& csValue );

	// change existing string property
	bool ChangeStringProperty( CString& csTitle, CString& csValue );

	// update the property panel with current selected properties
	void UpdatePropertyPane();

	/////////////////////////////////////////////////////////////////////////////
	// get the current date taken, if any, from the given filename
	CString GetCurrentDateTaken( LPCTSTR lpszPathName );

	/////////////////////////////////////////////////////////////////////////////
	// given an image pointer and an ASCII property ID, return the property value
	CString GetStringProperty( Gdiplus::Image* pImage, PROPID id );

	// Save the data inside pImage to the given filename but relocated to the 
	// sub-folder "Corrected"
	bool Save( LPCTSTR lpszPathName, Gdiplus::Image* pImage );

	/////////////////////////////////////////////////////////////////////////////
	// test to see if a file is already open
	bool IsFileOpen( const CString& filename )
	{
		CFile file;
		try
		{
			// Attempt to open the file in read mode with exclusive access
			if ( file.Open( filename, CFile::modeRead | CFile::shareExclusive ) )
			{
				file.Close(); // Close the file if opened successfully
				return false; // File is not open by another process
			}
		}
		catch ( CFileException* e )
		{
			e->Delete(); // Handle the exception and return true (file is open)
			return true;
		}
		return true; // If open fails without throwing, consider it open
	}

// protected overrides
protected:

// public overrides
public:

// public constructor/destructor
public:
	CImageProperties();
	~CImageProperties();
};

