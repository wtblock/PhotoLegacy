/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ImageProperty.h"
#include "ImageProperties.h"
#include "ExifRotation.h"
#include "GlobalPosition.h"
#include "MainFrm.h"
#include "PhotoExplorerDoc.h"
#include "ImageView.h"
#include "PropertyTag.h"

/////////////////////////////////////////////////////////////////////////////
CImageProperty::CImageProperty()
{
	DataType = VT_EMPTY;
	PropertyType = 0;
	PropertyID = 0;
	Length = 0;
	Container = nullptr;
	Modified = false;
}

/////////////////////////////////////////////////////////////////////////////
CImageProperty::~CImageProperty()
{
}

/////////////////////////////////////////////////////////////////////////////
// modification flag
void CImageProperty::SetModified( bool value )
{
	if ( value != m_bModified )
	{
		CImageProperties* pHost = Container;
		WORD wModified = pHost->ModifiedValues;
		if ( value == true )
		{
			wModified++;

		} else
		{
			wModified--;
		}
		pHost->ModifiedValues = wModified;
		m_bModified = value;
	}
} // SetModified

/////////////////////////////////////////////////////////////////////////////
_variant_t CImageProperty::GetValue()
{
	CVariantVector* pData = Data;
	_variant_t value = pData->getData();

	return value;
} // GetValue

/////////////////////////////////////////////////////////////////////////////
void CImageProperty::SetValue( _variant_t value )
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
	CImageView* pView = (CImageView*)pFrame->ImageView;

	CVariantVector* pData = Data;
	CImageProperties* pHost = Container;
	const CString csKey = PropertyKey;
	const CString csGroup = PropertyGroup;
	const CString csLabel = PropertyLabel;

	shared_ptr<CPropertyTag> pTag = pHost->PropertyTag[ csKey ];
	const ULONG ulID = pTag->PropertyID;

	// remember the modification state in case something goes wrong
	bool bModified = Modified;

	// assume this will work and mark the property as modified
	Modified = true;

	CStringA csValue( value );
	vector<CHAR> arrValues;
	char* pBuffer = csValue.GetBuffer();
	const ULONG ulLen = csValue.GetLength() + 1;

	if ( csGroup == L"User" || csGroup == L"Label")
	{
		switch ( ulID )
		{
			case PropertyTagXPSubject:
			case PropertyTagXPComment:
			case PropertyTagXPKeywords: // unicode string
			{
				CString csInput( value );
				TCHAR* pValue = reinterpret_cast<TCHAR*>( csInput.GetBuffer() );
				ULONG ulChars = csInput.GetLength() * sizeof( TCHAR ) + sizeof( TCHAR );
				char* pChars = reinterpret_cast<char*>( pValue );
				Modified = pData->setData( VT_I1, ulChars, pChars );
				break;
			}
			default:
			{
				Modified = pData->setData( VT_I1, ulLen, pBuffer );
			}
		}

	} else
	{
		switch ( ulID )
		{
			case PropertyTagOrientation:
			{
				const USHORT usValue = pTag->Enum[ (CString)value ];
				pData->setValue( 0, _variant_t( usValue ) );
				pDoc->Rotator->ExifOrientation = usValue;
				pView->Invalidate();
				break;
			}
			case PropertyTagExifDTOrig:
			case PropertyTagExifDTDigitized:
			{
				Modified = pData->setData( VT_I1, ulLen, pBuffer );
				break;
			}
			default:
			{
				// return the modified flag to the entry value if we get here
				Modified = bModified;
			}
		}
	}

} // SetValue

/////////////////////////////////////////////////////////////////////////////
void CImageProperty::SetPropertyType( WORD value )
{
	m_wPropertyType = value;
	if ( Container != nullptr )
	{
		const bool bFound = Container->Types->Exists[ value ];
		if ( bFound )
		{
			DataType = *Container->Types->find( value );
		}
	}
} // SetPropertyType

/////////////////////////////////////////////////////////////////////////////
// return component configuration as a comma separated string
// Information specific to compressed data.The channels of each 
// component are arranged in order from the first component to 
// the fourth.For uncompressed data, the data arrangement is 
// given in the PropertyTagPhotometricInterp tag.
// However, because PropertyTagPhotometricInterp can only 
// express the order of Y, Cb, and Cr, this tag is provided for 
// cases when compressed data uses components other than 
// Y, Cb, and Cr and to support other sequences.
CString CImageProperty::GetComponentConfiguration()
{
	CString value;
	void* pData = DataItem;
	WORD wType = PropertyType;
	ULONG ulItem = PropertyID;
	if ( ulItem != PropertyTagExifCompConfig )
	{
		return value;
	}

	const ULONG ulLength = Length;
	ULONG ulValues = ulLength / sizeof( BYTE );
	BYTE* pValue = reinterpret_cast<BYTE*>( pData );
	for ( ULONG ulValue = 0; ulValue < ulValues; ulValue++ )
	{
		BYTE bData = pValue[ ulValue ];
		CString csData;
		switch ( bData )
		{
			case 0: csData = L"-"; break; // undefined
			case 1: csData = L"Y"; break;
			case 2: csData = L"Cb"; break;
			case 3: csData = L"Cr"; break;
			case 4: csData = L"R"; break;
			case 5: csData = L"G"; break;
			case 6: csData = L"B"; break;
		}
		value += csData;
		value += L", ";
	}
	value.TrimRight( L", " );
	return value;
} // GetComponentConfiguration

/////////////////////////////////////////////////////////////////////////////
vector<CString> CImageProperty::GetDropdownChoices()
{
	vector<CString> value;

	WORD wType = PropertyType;
	const ULONG ulItem = PropertyID;
	CImageProperties* pParent = Container;
	void* pData = DataItem;

	const CString csKey = pParent->IdKey[ ulItem ];
	CKeyedCollection<CString, CPropertyTag>* pTags = pParent->PropertyTags;
	shared_ptr<CPropertyTag> pTag = pTags->find( csKey );

	if ( pTag->EntryType == CPropertyTag::teEnum )
	{
		value = pTag->GetEnumerationText();
	}

	return value;
} // GetDropdownChoices

/////////////////////////////////////////////////////////////////////////////
CString CImageProperty::GetUserInterfaceValue()
{
	WORD wType = PropertyType;
	const ULONG ulItem = PropertyID;
	CImageProperties* pParent = Container;
	void* pData = DataItem;

	const CString csKey = pParent->IdKey[ ulItem ];
	const ULONG ulLength = Length;
	CString csValue;
	ULONG ulValues = 0;
	CString csAltitudeRef( L"Above Sea Level" );

	// Property Tags contain information that is constant to properties
	CKeyedCollection<CString, CPropertyTag>* pTags = pParent->PropertyTags;
	shared_ptr<CPropertyTag> pTag = pTags->find( csKey );
	CPropertyTag::TAG_ENTRY_TYPE eEntry = pTag->EntryType;

	// special handling for some undefined data types
	if ( wType == PropertyTagTypeUndefined )
	{
		if
		(
			ulItem == PropertyTagExifFPXVer ||
			ulItem == PropertyTagExifVer
		)
		{
			wType = PropertyTagTypeChar;
		}
	}

	switch ( wType )
	{
		case PropertyTagTypeUndefined:
		case PropertyTagTypeChar:
		{
			CHAR tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			break;
		}
		case PropertyTagTypeByte:
		{
			BYTE tType = 0;
			if ( ulItem == PropertyTagExifCompConfig )
			{
				csValue = ComponentConfiguration;

			} else
			{
				CString csDescription( L"unknown" );
				CString csTemp = CHelper::ArrayToCSV( pData, ulLength, tType );
				BYTE* pValue = reinterpret_cast<BYTE*>( pData );
				BYTE bData = pValue[ 0 ];
				if ( eEntry == CPropertyTag::teEnum )
				{
					CKeyedCollection<USHORT, CString>* pEnums = pTag->EnumValues;
					if ( pEnums->Exists[ bData ] )
					{
						csDescription = *pEnums->find( bData );
						csValue.Format( L"%s [%s]", (LPCTSTR)csDescription, (LPCTSTR)csTemp );
					}
				} else
				{
					csValue = csTemp;
				}
			}
			break;
		}
		case PropertyTagTypeASCII:
		{
			csValue = CString( CA2CT( reinterpret_cast<char*>( pData ) ) );
			break;
		}
		case PropertyTagTypeShort:
		{
			USHORT tType = 0;
			CString csDescription( L"unknown" );
			CString csTemp = CHelper::ArrayToCSV( pData, ulLength, tType );
			USHORT* pValue = reinterpret_cast<USHORT*>( pData );
			if ( pValue == nullptr )
			{
				break;
			}
			USHORT usData = pValue[ 0 ];

			// process short enumerations
			switch ( ulItem )
			{
				case PropertyTagXPSubject:
				case PropertyTagXPComment:
				case PropertyTagXPKeywords: // unicode string
				{
					void* pBuffer = csValue.GetBufferSetLength( ulLength );
					::CopyMemory( pBuffer, pData, ulLength );
					csValue.ReleaseBuffer();
					break;
				}
				case PropertyTagOrientation:
				{
					CExifRotation rotator;
					rotator.Rotation = (CExifRotation::ExifRotations)usData;
					csDescription = rotator.Description;
					csValue.Format( L"%s [%s]", (LPCTSTR)csDescription, (LPCTSTR)csTemp );
					break;
				}
				default:
				{
					if ( eEntry == CPropertyTag::teEnum )
					{
						CKeyedCollection<USHORT, CString>* pEnums = pTag->EnumValues;
						if ( pEnums->Exists[ usData ] )
						{
							csDescription = *pEnums->find( usData );
							csValue.Format( L"%s [%s]", (LPCTSTR)csDescription, (LPCTSTR)csTemp );
						}
					} else
					{
						csValue = csTemp;
					}
				}
			}
			break;
		}
		case PropertyTagTypeLong:
		{
			ULONG tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			break;
		}
		case PropertyTagTypeRational:
		{
			ULONG tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			vector<ULONG> arrRaw;

			// process rational interpretations
			switch ( ulItem )
			{
				case PropertyTagGpsLatitude:
				case PropertyTagGpsLongitude:
				{
					const bool bOK =
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						CGlobalPosition GPS;
						GPS.RawData = arrRaw;
						csValue = GPS.Output;
					}
					break;
				}
				case PropertyTagGpsAltitude:
				case PropertyTagExifFocalLength:
				case PropertyTagExifCompBPP:
				case PropertyTagExifAperture:
				case PropertyTagExifMaxAperture:
				case PropertyTagExifDigitalZoomRatio:
				case PropertyTagThumbnailResolutionX:
				case PropertyTagThumbnailResolutionY:
				case PropertyTagExifFNumber:
				case PropertyTagXResolution:
				case PropertyTagYResolution:
				case PropertyTagExifBrightness:
				{
					const bool bOK =
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						const size_t nSize = arrRaw.size();
						if ( nSize == 2 && arrRaw[ 1 ] != 0 )
						{
							const float fNumerator = float( arrRaw[ 0 ] );
							const float fDenominator = float( arrRaw[ 1 ] );
							const float fValue = fNumerator / fDenominator;
							if ( ulItem == PropertyTagGpsAltitude )
							{
								csValue.Format( L"%g m", fValue );

							} else
							{
								csValue.Format( L"%g", fValue );
							}
						}
					}
					break;
				}
				case PropertyTagExifExposureTime:
				{
					const bool bOK =
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						const size_t nSize = arrRaw.size();
						if ( nSize == 2 )
						{
							csValue.Format( L"%d/%d", arrRaw[ 0 ], arrRaw[ 1 ] );
						}
					}
					break;
				}
			}
			break;
		}
		case PropertyTagTypeSLONG:
		{
			LONG tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			break;
		}
		case PropertyTagTypeSRational:
		{
			LONG tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			vector<LONG> arrRaw;

			// process rational interpretations
			switch ( ulItem )
			{
				case PropertyTagExifShutterSpeed:
				{
					const bool bOK =
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						const size_t nSize = arrRaw.size();
						if ( nSize == 2 )
						{
							csValue.Format( L"%d/%d", arrRaw[ 0 ], arrRaw[ 1 ] );
						}
					}
					break;
				}
				case PropertyTagExifBrightness:
				case PropertyTagExifExposureBias:
				{
					const bool bOK =
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						const size_t nSize = arrRaw.size();
						if ( nSize == 2 && arrRaw[ 1 ] != 0 )
						{
							const float fNumerator = float( arrRaw[ 0 ] );
							const float fDenominator = float( arrRaw[ 1 ] );
							const float fValue = fNumerator / fDenominator;
							csValue.Format( L"%g", fValue );
						}
					}
					break;
				}
			}
			break;
		}
	}
	
	return csValue;
} // GetUserInterfaceValue

/////////////////////////////////////////////////////////////////////////////
CString CImageProperty::GetCommaSeparatedValue()
{
	CString value;
	WORD wType = PropertyType;
	const ULONG ulItem = PropertyID;
	CImageProperties* pParent = Container;
	void* pData = DataItem;
	if ( pData == nullptr )
	{
		return value;
	}

	const CString csKey = pParent->IdKey[ ulItem ];
	const ULONG ulLength = Length;
	CString csValue;
	CString csType;
	ULONG ulValues = 0;
	CString csAltitudeRef( L"Above Sea Level" );

	// Property Tags contain information that is constant to properties
	CKeyedCollection<CString, CPropertyTag>* pTags = pParent->PropertyTags;
	shared_ptr<CPropertyTag> pTag = pTags->find( csKey );
	CPropertyTag::TAG_ENTRY_TYPE eEntry = pTag->EntryType;

	// special handling for some undefined data types
	if ( wType == PropertyTagTypeUndefined )
	{
		if
		(
			ulItem == PropertyTagExifFPXVer ||
			ulItem == PropertyTagExifVer
		)
		{
			wType = PropertyTagTypeChar;
		}
	}

	switch ( wType )
	{
		case PropertyTagTypeUndefined:
		case PropertyTagTypeChar:
		{
			csType = _T( "CHAR" );
			CHAR tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			break;
		}
		case PropertyTagTypeByte:
		{
			csType = _T( "BYTE" );
			BYTE tType = 0;
			if ( ulItem == PropertyTagExifCompConfig )
			{
				csValue = ComponentConfiguration;

			} else
			{
				CString csDescription( L"unknown" );
				CString csTemp = CHelper::ArrayToCSV( pData, ulLength, tType );
				BYTE* pValue = reinterpret_cast<BYTE*>( pData );
				BYTE bData = pValue[ 0 ];
				if ( eEntry == CPropertyTag::teEnum )
				{
					CKeyedCollection<USHORT, CString>* pEnums = pTag->EnumValues;
					if ( pEnums->Exists[ bData ] )
					{
						csDescription = *pEnums->find( bData );
						csValue.Format( L"%s [%s]", (LPCTSTR)csDescription, (LPCTSTR)csTemp );
					}
				} else
				{
					csValue = csTemp;
				}
			}
			break;
		}
		case PropertyTagTypeASCII:
		{
			csType = _T( "ASCII" );
			csValue =
				CString( CA2CT( reinterpret_cast<char*>( pData ) ) );
			break;
		}
		case PropertyTagTypeShort:
		{
			csType = _T( "USHORT" );
			USHORT tType = 0;
			CString csDescription( L"unknown" );
			CString csTemp = CHelper::ArrayToCSV( pData, ulLength, tType );
			USHORT* pValue = reinterpret_cast<USHORT*>( pData );
			USHORT usData = pValue[ 0 ];

			// process short enumerations
			switch ( ulItem )
			{
				case PropertyTagXPComment:
				case PropertyTagXPKeywords:
				case PropertyTagXPSubject:
				{
					csValue = (LPCTSTR)pData;
					break;
				}
				case PropertyTagOrientation:
				{
					CExifRotation rotator;
					rotator.Rotation = (CExifRotation::ExifRotations)usData;
					csDescription = rotator.Description;
					csValue.Format( L"%s [%s]", (LPCTSTR)csDescription, (LPCTSTR)csTemp );
					break;
				}
				default:
				{
					if ( eEntry == CPropertyTag::teEnum )
					{
						CKeyedCollection<USHORT, CString>* pEnums = pTag->EnumValues;
						if ( pEnums->Exists[ usData ] )
						{
							csDescription = *pEnums->find( usData );
							csValue.Format( L"%s [%s]", (LPCTSTR)csDescription, (LPCTSTR)csTemp );
						}
					} else
					{
						csValue = csTemp;
					}
				}
			}
			break;
		}
		case PropertyTagTypeLong:
		{
			csType = _T( "ULONG" );
			ULONG tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			break;
		}
		case PropertyTagTypeRational:
		{
			csType = _T( "Rational" );
			ULONG tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			vector<ULONG> arrRaw;

			// process rational interpretations
			switch ( ulItem )
			{
				case PropertyTagGpsLatitude:
				case PropertyTagGpsLongitude:
				{
					const bool bOK = 
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						CGlobalPosition GPS;
						GPS.RawData = arrRaw;
						csValue = GPS.Output;
					}
					break;
				}
				case PropertyTagGpsAltitude:
				case PropertyTagExifFocalLength:
				case PropertyTagExifCompBPP:
				case PropertyTagExifAperture:
				case PropertyTagExifMaxAperture:
				case PropertyTagExifDigitalZoomRatio:
				case PropertyTagThumbnailResolutionX:
				case PropertyTagThumbnailResolutionY:
				case PropertyTagExifFNumber :
				case PropertyTagXResolution :
				case PropertyTagYResolution :
				{
					const bool bOK =
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						const size_t nSize = arrRaw.size();
						if ( nSize == 2 && arrRaw[ 1 ] != 0 )
						{
							const float fNumerator = float( arrRaw[ 0 ] );
							const float fDenominator = float( arrRaw[ 1 ] );
							const float fValue = fNumerator / fDenominator;
							if ( ulItem == PropertyTagGpsAltitude )
							{
								csValue.Format( L"%g m", fValue );

							} else
							{
								csValue.Format( L"%g", fValue );
							}
						}
					}
					break;
				}
				case PropertyTagExifExposureTime :
				{
					const bool bOK =
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						const size_t nSize = arrRaw.size();
						if ( nSize == 2 )
						{
							csValue.Format( L"%d/%d", arrRaw[ 0 ], arrRaw[ 1 ] );
						}
					}
					break;
				}
			}
			break;
		}
		case PropertyTagTypeSLONG:
		{
			csType = _T( "LONG" );
			LONG tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			break;
		}
		case PropertyTagTypeSRational:
		{
			csType = _T( "SRational" );
			LONG tType = 0;
			csValue = CHelper::ArrayToCSV( pData, ulLength, tType );
			vector<LONG> arrRaw;

			// process rational interpretations
			switch ( ulItem )
			{
				case PropertyTagExifShutterSpeed:
				{
					const bool bOK =
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						const size_t nSize = arrRaw.size();
						if ( nSize == 2 )
						{
							csValue.Format( L"%d/%d", arrRaw[ 0 ], arrRaw[ 1 ] );
						}
					}
					break;
				}
				case PropertyTagExifBrightness:
				case PropertyTagExifExposureBias:
				{
					const bool bOK =
						CHelper::ArrayToVector( pData, ulLength, tType, arrRaw );
					if ( bOK )
					{
						const size_t nSize = arrRaw.size();
						if ( nSize == 2 && arrRaw[ 1 ] != 0 )
						{
							const float fNumerator = float( arrRaw[ 0 ] );
							const float fDenominator = float( arrRaw[ 1 ] );
							const float fValue = fNumerator / fDenominator;
							csValue.Format( L"%g", fValue );
						}
					}
					break;
				}
			}
			break;
		}
	}

	// the kind of information being returned
	CString csID;
	csID.Format( _T( "0x%04X" ), ulItem );

	value.Format
	(
		_T( "LABEL: %40s, TYPE: %10s, LENGTH: % 7d, ID: %s, VALUE: %s" ),
		csKey, csType, ulLength, csID, csValue
	);	
	
	return value;
} // GetCommaSeparatedValue

/////////////////////////////////////////////////////////////////////////////
bool CImageProperty::UpdateProperty( Image* image )
{
	bool value = true;

	// Allocate memory for the property
	const ULONG ulBytes = Data->NumberOfBytes;
	Gdiplus::PropertyItem* propItem =
		(Gdiplus::PropertyItem*)malloc( sizeof( Gdiplus::PropertyItem ) + ulBytes );
	propItem->id = PropertyID;
	propItem->length = ulBytes;
	propItem->type = PropertyType; 
	propItem->value = 
		(VOID*)( (BYTE*)propItem + sizeof( Gdiplus::PropertyItem ) );

	// Copy the value to the property
	memcpy( propItem->value, Data->getData(), ulBytes );

	try
	{
		// Set the property in the image
		image->SetPropertyItem( propItem );
	}
	catch ( exception& e )
	{
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
		pDoc->ErrorsText = CString( e.what());
		value = false;
	}

	// Clean up
	free( propItem );

	return value;
} // UpdateProperty

/////////////////////////////////////////////////////////////////////////////
