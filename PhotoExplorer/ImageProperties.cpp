/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ImageProperties.h"
#include "CHelper.h"
#include "MainFrm.h"
#include "PhotoExplorerDoc.h"
#include "ShellListView.h"
#include "PropertyGridMultilineText.h"
#include <memory>
#include <fstream>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
CImageProperties::CImageProperties()
{
	ModifiedValues = 0;

	// populate the type cross reference
	for 
	( 
		WORD wIndex = PropertyTagTypeChar; 
		wIndex <= PropertyTagTypeSRational;
		wIndex++
	)
	{
		shared_ptr<VARENUM> node;
		switch ( wIndex )
		{
			case PropertyTagTypeUndefined: // documentation say to be treated like CHAR
			case PropertyTagTypeChar: // not defined in GDI+, treated as an array of character
			{
				node = shared_ptr<VARENUM>( new VARENUM( VT_I1 ));
				break;
			}
			case PropertyTagTypeByte:
			{
				node = shared_ptr<VARENUM>( new VARENUM( VT_UI1 ));
				break;
			}
			case PropertyTagTypeASCII:
			{
				node = shared_ptr<VARENUM>( new VARENUM( VT_I1  ) );
				break;
			}
			case PropertyTagTypeShort:
			{
				node = shared_ptr<VARENUM>( new VARENUM( VT_UI2 ) );
				break;
			}
			case PropertyTagTypeRational: // array of two: numerator / denominator
			case PropertyTagTypeLong:
			{
				node = shared_ptr<VARENUM>( new VARENUM( VT_UI4 ) );
				break;
			}
			case PropertyTagTypeSRational: // array of two: numerator / denominator
			case PropertyTagTypeSLONG:
			{
				node = shared_ptr<VARENUM>( new VARENUM( VT_I4 ) );
				break;
			}
		}

		// build the cross reference
		m_TypeCollection.add( wIndex, node );
	}

	// cross reference of property tag IDs with descriptive text key
	// where the key is in the format of "group|label"
	map<PROPID, CString> mapPropertyKeys =
	{
		{ PropertyTagDocumentName /* 0x010D */, L"User|1. Filename" },
		{ PropertyTagImageTitle /* 0x0320 */, L"Image|Title" },
		{ PropertyTagDateTime /* 0x0132 */, L"Image|Date" },
		{ PropertyTagImageDescription /* 0x010E */, L"Label|1. Title" },
		{ PropertyTagExifUserComment /* 0x9286 */, L"Image|Comment" },
		{ PropertyTagArtist /* 0x013B */, L"User|2. Artist" },
		{ PropertyTagSoftwareUsed /* 0x0131 */, L"User|4. Software" },
		{ PropertyTagCopyright /* 0x8298 */, L"User|3. Copyright" },
		{ PropertyTagRating /*0x4746 */, L"X|Rating stars" },
		{ PropertyTagRatingPercent /* 0x4749 */, L"X|Rating percent" },

		{ PropertyTagGpsLatitudeRef /* 0x0001 */, L"GPS|Latitude reference" }, // N or S
		{ PropertyTagGpsLatitude /* 0x0002 */, L"GPS|Latitude" },
		{ PropertyTagGpsLongitudeRef /* 0x0003 */, L"GPS|Longitude reference" }, // E or W
		{ PropertyTagGpsLongitude /* 0x0004 */, L"GPS|Longitude" },
		{ PropertyTagGpsAltitudeRef /* 0x0005 */, L"GPS|Altitude reference" },
		{ PropertyTagGpsAltitude /* 0x0006 */, L"GPS|Altitude" },

		{ PropertyTagImageWidth /* 0x0100 */, L"Image|Width" },
		{ PropertyTagImageHeight /* 0x0101 */, L"Image|Height" },
		{ PropertyTagEquipMake /* 0x010F */, L"Equipment|Make" },
		{ PropertyTagEquipModel /* 0x0110 */, L"Equipment|Model" },
		{ PropertyTagOrientation /* 0x0112 */, L"Image|Orientation" },
		{ PropertyTagXResolution /* 0x011A */, L"Image|X resolution" },
		{ PropertyTagYResolution /* 0x011B */, L"Image|Y resolution" },
		{ PropertyTagResolutionUnit /* 0x0128 */, L"Image|Resolution unit" },
		{ PropertyTagJPEGInterFormat /* 0x0201 */, L"Image|JPEG inter format" },
		{ PropertyTagJPEGInterLength /* 0x0202 */, L"Image|JPEG inter length" },
		{ PropertyTagYCbCrPositioning /* 0x0213 */, L"Image|Y Cb Cr positioning" },
		{ PropertyTagLuminanceTable /* 0x5090 */, L"X|Luminance table" },
		{ PropertyTagChrominanceTable /* 0x5091 */, L"X|Chrominance table" },

		{ PropertyTagThumbnailData /* 0x501B */, L"X|Bits" }, // RAW thumbnail bits
		{ PropertyTagThumbnailImageWidth /* 0x5020 */, L"Thumbnail|Width" }, // Thumbnail width
		{ PropertyTagThumbnailImageHeight /* 0x5021 */, L"Thumbnail|Height" }, // Thumbnail height
		{ PropertyTagThumbnailCompression /* 0x5023 */, L"Thumbnail|Compression scheme" }, // Compression Scheme
		{ PropertyTagThumbnailResolutionX /* 0x502D */, L"Thumbnail|X resolution" },
		{ PropertyTagThumbnailResolutionY /* 0x502E */, L"Thumbnail|Y resolution" },
		{ PropertyTagThumbnailResolutionUnit /* 0x5030 */, L"Thumbnail|Resolution unit" },
		{ PropertyTagThumbnailInteropIndex /* 0x5041 */, L"Thumbnail|Interop index" },
		{ PropertyTagThumbnailInteropVersion /* 0x5042 */, L"Thumbnail|Interop version" },

		{ PropertyTagExifExposureTime /* 0x829A */, L"Camera|Exposure time" },
		{ PropertyTagExifFNumber /* 0x829D */, L"Camera|F number" },
		{ PropertyTagICCProfile /* 0x8773 */, L"X|ICC Profile" },
		{ PropertyTagExifExposureProg /* 0x8822 */, L"Camera|Exposure program" },
		{ PropertyTagExifISOSpeed /* 0x8827 */, L"Camera|ISO speed" },
		{ PropertyTagExifVer /* 0x9000 */, L"Camera|Version" },
		{ PropertyTagExifDTOrig /* 0x9003 */, L"Label|4. Date taken" },
		{ PropertyTagExifDTDigitized /* 0x9004 */, L"Camera|Digitize date" },
		{ PropertyTagOffsetTime /* 0x9010 */, L"Image|Offset time" }, // time zone offset
		{ PropertyTagOrgOffsetTime /* 0x9011 */, L"Image|Offset time original" }, // original time zone offset
		{ PropertyTagExifCompConfig /* 0x9101 */, L"Camera|Component's configuration" },
		{ PropertyTagExifCompBPP /* 0x9102 */, L"Camera|Component's BPP" },
		{ PropertyTagExifShutterSpeed /* 0x9201 */, L"Camera|Shutter speed" },
		{ PropertyTagExifAperture /* 0x9202 */, L"Camera|Aperture" },
		{ PropertyTagExifBrightness /* 0x9203 */, L"Camera|Brightness" },
		{ PropertyTagExifExposureBias /* 0x9204 */, L"Camera|Exposure bias" },
		{ PropertyTagExifMaxAperture /* 0x9205 */, L"Camera|Max aperture" },
		{ PropertyTagExifMeteringMode /* 0x9207 */, L"Camera|Metering mode" },
		{ PropertyTagExifLightSource /* 0x9208 */, L"Camera|Light source" },
		{ PropertyTagExifFlash /* 0x9209 */, L"Camera|Flash" },
		{ PropertyTagExifFocalLength /* 0x920A */, L"Camera|Focal length" },
		{ PropertyTagExifMakerNote /* 0x927C */, L"Camera|Maker note" },
		{ PropertyTagExifDTSubsec /* 0x9290 */, L"Camera|Date subseconds" },
		{ PropertyTagExifDTOrigSS /* 0x9291 */, L"Camera|Date original subseconds" },
		{ PropertyTagExifDTDigSS /* 0x9292 */, L"Camera|Date digitized subseconds" },
		{ PropertyTagXPTitle /* 0x9C9B */, L"XP|Title" },
		{ PropertyTagXPComment /* 0x9C9C */, L"Label|3. Comment" },
		{ PropertyTagXPKeywords /* 0x9C9E */, L"User|5. Keywords" },
		{ PropertyTagXPSubject /* 0x9C9F */, L"Label|2. Location" },
		{ PropertyTagExifFPXVer /* 0xA000 */, L"Camera|FPX version" },
		{ PropertyTagExifColorSpace /* 0xA001 */, L"Camera|Color space" },
		{ PropertyTagExifPixXDim /* 0xA002 */, L"Camera|Pix X dimension" },
		{ PropertyTagExifPixYDim /* 0xA003 */, L"Camera|Pix Y dimension" },
		{ PropertyTagExifFileSource /* 0xA300 */, L"Camera|Image source" },
		{ PropertyTagExifCustomRendered /* 0xA401 */, L"Camera|Custom rendered" },
		{ PropertyTagExifExposureMode /* 0xA402 */, L"Camera|Exposure mode" },
		{ PropertyTagExifWhiteBalance /* 0xA403 */, L"Camera|White balance" },
		{ PropertyTagExifDigitalZoomRatio /* 0xA404 */, L"Camera|Digital zoom ratio" },
		{ PropertyTagExifFocalLengthIn35mmFilm /* 0xA405 */, L"Camera|Focal length in 35mm film" },
		{ PropertyTagExifSceneCaptureType /* 0xA406 */, L"Camera|Scene capture type" },
		{ PropertyTagExifGainControl /* 0xA407 */, L"Camera|Gain control" },
		{ PropertyTagExifContrast /* 0xA408 */, L"Camera|Contrast" },
		{ PropertyTagExifSaturation /* 0xA409 */, L"Camera|Saturation" },
		{ PropertyTagExifSharpness /* 0xA40A */, L"Camera|Sharpness" },
		{ PropertyTagExifUniqueImageID /* 0xA420 */, L"Camera|Unique image ID" },
		{ PropertyTagPrintIMVersion /* 0xC4A5 */, L"Camera|Print IM version" },
	};

	map<CString, CString> mapDesc =
	{
		{ L"Camera|Aperture", L"Lens aperture. The unit is the APEX value." },
		{ L"Camera|Brightness", L"Brightness value. The unit is the APEX value. Ordinarily it is given in the range of -99.99 to 99.99." },
		{ L"Camera|Color space", L"Color space specifier. Normally sRGB (=1) is used to define the color space based on the PC monitor conditions and environment. If a color space other than sRGB is used, Uncalibrated (=0xFFFF) is set. " },
		{ L"Camera|Component's configuration", L"Information specific to compressed data. The channels of each component are arranged in order from the first component to the fourth. For uncompressed data, the data arrangement is given in the PropertyTagPhotometricInterp tag." },
		{ L"Camera|Component's BPP", L"Information specific to compressed data. The compression mode used for a compressed image is indicated in unit Bits Per Pixel(BPP)." },
		{ L"Camera|Contrast", L"" },
		{ L"Camera|Custom rendered", L"" },
		{ L"Camera|Date digitized subseconds", L"Null-terminated character string that specifies a fraction of a second for the digitized date." },
		{ L"Camera|Date original subseconds", L"Null-terminated character string that specifies a fraction of a second for the original date." },
		{ L"Camera|Date subseconds", L"Null-terminated character string that specifies a fraction of a second for the date taken." },
		{ L"Camera|Digital zoom ratio", L"" },
		{ L"Camera|Digitize date", L"Date and time when the image was stored as digital data. " },
		{ L"Camera|Exposure bias", L"Exposure bias. The unit is the APEX value. Ordinarily it is given in the range -99.99 to 99.99." },
		{ L"Camera|Exposure mode", L"" },
		{ L"Camera|Exposure program", L"Class of the program used by the camera to set exposure when the picture is taken." },
		{ L"Camera|Exposure time", L"Exposure time, measured in seconds." },
		{ L"Camera|F number", L"The f-number is also known as the focal ratio, f-ratio, or f-stop, and it is key in determining the depth of field, diffraction, and exposure of a photograph. " },
		{ L"Camera|FPX version", L"FlashPix format version supported by an FPXR file. If the FPXR function supports FlashPix format version 1.0, recording 0100 as a 4-byte ASCII string. " },
		{ L"Camera|Gain control", L"" },
		{ L"Camera|Light source", L"Type of light source." },
		{ L"Camera|Flash", L"Flash status. This tag is recorded when an image is taken using a strobe light (flash). Bit 0 indicates the flash firing status, and bits 1 and 2 indicate the flash return status." },
		{ L"Camera|Focal length", L"Actual focal length, in millimeters, of the lens. Conversion is not made to the focal length of a 35 millimeter film camera." },
		{ L"Camera|Focal length in 35mm film", L"" },
		{ L"Camera|Image source", L"The image source. If a Digital Still Camera (DSC) recorded the image, the value of this tag is 3." },
		{ L"Camera|ISO speed", L"ISO speed and ISO latitude of the camera or input device as specified in ISO 12232." },
		{ L"Camera|Maker note", L"Note tag. A tag used by manufacturers of EXIF writers to record information. The contents are up to the manufacturer." },
		{ L"Camera|Max aperture", L"Smallest F number of the lens. The unit is the APEX value. Ordinarily it is given in the range of 00.00 to 99.99, but it is not limited to this range." },
		{ L"Camera|Metering mode", L"Metering mode in photography refers to the way a camera determines exposure by measuring the brightness of a scene before capturing an image." },
		{ L"Camera|Original date", L"Date and time when the original image data was generated." },
		{ L"Camera|Pix X dimension", L"Information specific to compressed data. When a compressed file is recorded, the valid width of the meaningful image must be recorded in this tag, whether or not there is padding data or a restart marker. This tag should not exist in an uncompressed file." },
		{ L"Camera|Pix Y dimension", L"Information specific to compressed data. When a compressed file is recorded, the valid height of the meaningful image must be recorded in this tag whether or not there is padding data or a restart marker. This tag should not exist in an uncompressed file." },
		{ L"Camera|Print IM version", L"" },
		{ L"Camera|Saturation", L"" },
		{ L"Camera|Scene capture type", L"" },
		{ L"Camera|Sharpness", L"" },
		{ L"Camera|Shutter speed", L"Shutter speed. The unit is the Additive System of Photographic Exposure (APEX) value." },
		{ L"Camera|Unique image ID", L"" },
		{ L"Camera|White balance", L"" },
		{ L"Camera|Version", L"Version of the EXIF standard supported. Nonexistence of this field is taken to mean nonconformance to the standard. Conformance to the standard is indicated by recording 0210 as a 4-byte ASCII string. Because the type is PropertyTagTypeUndefined, there is no NULL terminator." },
		{ L"Equipment|Make", L"Null-terminated character string that specifies the manufacturer of the equipment used to record the image." },
		{ L"Equipment|Model", L"Null-terminated character string that specifies the model name or model number of the equipment used to record the image." },
		{ L"GPS|Altitude", L"Altitude, in meters, based on the reference altitude." },
		{ L"GPS|Altitude reference", L"Reference altitude, in meters." },
		{ L"GPS|Latitude", L"Latitude. Latitude is expressed as three rational values giving the degrees, minutes, and seconds respectively. When degrees, minutes, and seconds are expressed, the format is dd/1, mm/1, ss/1. When degrees and minutes are used and, for example, fractions of minutes are given up to two decimal places, the format is dd/1, mmmm/100, 0/1." },
		{ L"GPS|Latitude reference", L"Null-terminated character string that specifies whether the latitude is north or south. N specifies north latitude, and S specifies south latitude." },
		{ L"GPS|Longitude", L"Longitude. Longitude is expressed as three rational values giving the degrees, minutes, and seconds respectively. When degrees, minutes and seconds are expressed, the format is ddd/1, mm/1, ss/1. When degrees and minutes are used and, for example, fractions of minutes are given up to two decimal places, the format is ddd/1, mmmm/100, 0/1." },
		{ L"GPS|Longitude reference", L"Null-terminated character string that specifies whether the longitude is east or west longitude. E specifies east longitude, and W specifies west longitude." },
		{ L"ICC|Profile", L"An ICC profile is a set of data that characterizes a color input or output device, or a color space, according to standards promulgated by the International Color Consortium (ICC)." },
		{ L"Image|Comment", L"Comment tag. A tag used by EXIF users to write keywords or comments about the image besides those in description and without the character-code limitations of the description." },
		{ L"X|Chrominance table", L"Chrominance table. The luminance table and the chrominance table are used to control JPEG quality. A valid luminance or chrominance table has 64 entries of type PropertyTagTypeShort. If an image has either a luminance table or a chrominance table, then it must have both tables." },
		{ L"Image|Description", L"Null-terminated character string that specifies the context of the image." },
		{ L"Image|Height", L"Number of pixel rows." },
		{ L"Image|JPEG inter format", L"Offset to the start of a JPEG bitstream." },
		{ L"Image|JPEG inter length", L"Length, in bytes, of the JPEG bitstream." },
		{ L"X|Luminance table", L"Luminance table. The luminance table and the chrominance table are used to control JPEG quality. A valid luminance or chrominance table has 64 entries of type PropertyTagTypeShort. If an image has either a luminance table or a chrominance table, then it must have both tables." },
		{ L"Image|Offset time", L"" },
		{ L"Image|Offset time original", L"" },
		{ L"Image|Orientation", L"Image orientation viewed in terms of rows and columns." },
		{ L"Image|Resolution unit", L"Unit of measure for the horizontal resolution and the vertical resolution." },
		{ L"Image|Title", L"Null-terminated character string that specifies the title of the image." },
		{ L"Image|Width", L"Number of pixels per row." },
		{ L"Image|X resolution", L"Number of pixels per resolution unit in the image width (x) direction." },
		{ L"Image|Y Cb Cr positioning", L"Position of chrominance components in relation to the luminance component." },
		{ L"Image|Y resolution", L"Number of pixels per resolution unit in the image height (y) direction." },
		{ L"X|Bits", L"Raw thumbnail bits in JPEG or RGB format. Depends on PropertyTagThumbnailFormat." },
		{ L"Thumbnail|Compression scheme", L"Compression scheme used for thumbnail image data." },
		{ L"Thumbnail|Height", L"Number of pixel rows in the thumbnail image." },
		{ L"Thumbnail|Interop index", L"" },
		{ L"Thumbnail|Interop version", L"" },
		{ L"Thumbnail|Width", L"Number of pixels per row in the thumbnail image." },
		{ L"Thumbnail|X resolution", L"Thumbnail resolution in the width direction. " },
		{ L"Thumbnail|Y resolution", L"Thumbnail resolution in the height direction." },
		{ L"Thumbnail|Resolution unit", L"Unit of measure for the horizontal resolution and the vertical resolution of the thumbnail image." },
		{ L"Label|1. Title", L"Title associated with the image." },
		{ L"Label|2. Location", L"The subject of the image." },
		{ L"Label|3. Comment", L"Information about the context." },
		{ L"User|5. Keywords", L"Keywords separated by semicolons." },
		{ L"User|1. Filename", L"Null-terminated character string that specifies the name of the document from which the image was scanned." },
		{ L"User|2. Artist", L"Null-terminated character string that specifies the name of the person who created the image." },
		{ L"User|3. Copyright", L"Null-terminated character string that contains copyright information." },
		{ L"Label|4. Date taken", L"Date and time the image was created." },
		{ L"X|Rating stars", L"Zero to five stars." },
		{ L"X|Rating percent", L"The five star rating converted to percent." },
		{ L"User|4. Software", L"Null-terminated character string that specifies the name and version of the software or firmware of the device used to generate the image." },
	};

	map<CString, WORD> mapTypes =
	{
		{ L"Camera|Aperture", PropertyTagTypeRational },
		{ L"Camera|Brightness", PropertyTagTypeRational },
		{ L"Camera|Color space", PropertyTagTypeShort },
		{ L"Camera|Component's configuration", PropertyTagTypeByte },
		{ L"Camera|Component's BPP", PropertyTagTypeRational },
		{ L"Camera|Contrast", PropertyTagTypeShort },
		{ L"Camera|Custom rendered", PropertyTagTypeShort },
		{ L"Camera|Date digitized subseconds", PropertyTagTypeASCII },
		{ L"Camera|Date original subseconds", PropertyTagTypeASCII },
		{ L"Camera|Date subseconds", PropertyTagTypeASCII },
		{ L"Camera|Digital zoom ratio", PropertyTagTypeRational },
		{ L"Camera|Digitize date", PropertyTagTypeASCII },
		{ L"Camera|Exposure bias", PropertyTagTypeSRational },
		{ L"Camera|Exposure mode", PropertyTagTypeShort },
		{ L"Camera|Exposure program", PropertyTagTypeShort },
		{ L"Camera|Exposure time", PropertyTagTypeRational },
		{ L"Camera|F number", PropertyTagTypeRational },
		{ L"Camera|FPX version", PropertyTagTypeUndefined },
		{ L"Camera|Gain control", PropertyTagTypeShort },
		{ L"Camera|Light source", PropertyTagTypeShort },
		{ L"Camera|Flash", PropertyTagTypeShort },
		{ L"Camera|Focal length", PropertyTagTypeRational },
		{ L"Camera|Focal length in 35mm film", PropertyTagTypeShort },
		{ L"Camera|Image source", PropertyTagTypeByte },
		{ L"Camera|ISO speed", PropertyTagTypeShort },
		{ L"Camera|Maker note", PropertyTagTypeASCII },
		{ L"Camera|Max aperture", PropertyTagTypeRational },
		{ L"Camera|Metering mode", PropertyTagTypeShort },
		{ L"Camera|Original date", PropertyTagTypeASCII },
		{ L"Camera|Pix X dimension", PropertyTagTypeLong },
		{ L"Camera|Pix Y dimension", PropertyTagTypeLong },
		{ L"Camera|Print IM version", PropertyTagTypeUndefined },
		{ L"Camera|Saturation", PropertyTagTypeShort },
		{ L"Camera|Scene capture type", PropertyTagTypeShort },
		{ L"Camera|Sharpness", PropertyTagTypeShort },
		{ L"Camera|Shutter speed", PropertyTagTypeSRational },
		{ L"Camera|Unique image ID", PropertyTagTypeASCII },
		{ L"Camera|White balance", PropertyTagTypeShort },
		{ L"Camera|Version", PropertyTagTypeUndefined },
		{ L"Equipment|Make", PropertyTagTypeASCII },
		{ L"Equipment|Model", PropertyTagTypeASCII },
		{ L"GPS|Altitude", PropertyTagTypeRational },
		{ L"GPS|Altitude reference", PropertyTagTypeShort },
		{ L"GPS|Latitude", PropertyTagTypeRational },
		{ L"GPS|Latitude reference", PropertyTagTypeASCII },
		{ L"GPS|Longitude", PropertyTagTypeRational },
		{ L"GPS|Longitude reference", PropertyTagTypeASCII },
		{ L"ICC|Profile", PropertyTagTypeByte },
		{ L"X|Chrominance table", PropertyTagTypeShort },
		{ L"Image|Comment", PropertyTagTypeASCII },
		{ L"Image|Description", PropertyTagTypeASCII },
		{ L"Image|Height", PropertyTagTypeLong },
		{ L"Image|JPEG inter format", PropertyTagTypeLong },
		{ L"Image|JPEG inter length", PropertyTagTypeLong },
		{ L"X|Luminance table", PropertyTagTypeShort },
		{ L"Image|Offset time", PropertyTagTypeASCII },
		{ L"Image|Offset time original", PropertyTagTypeASCII },
		{ L"Image|Orientation", PropertyTagTypeShort },
		{ L"Image|Resolution unit", PropertyTagTypeShort },
		{ L"Image|Title", PropertyTagTypeASCII },
		{ L"Image|Width", PropertyTagTypeLong },
		{ L"Image|X resolution", PropertyTagTypeRational },
		{ L"Image|Y Cb Cr positioning", PropertyTagTypeShort },
		{ L"Image|Y resolution", PropertyTagTypeRational },
		{ L"X|Bits", PropertyTagTypeByte },
		{ L"Thumbnail|Compression scheme", PropertyTagTypeShort },
		{ L"Thumbnail|Height", PropertyTagTypeShort },
		{ L"Thumbnail|Interop index", PropertyTagTypeASCII },
		{ L"Thumbnail|Interop version", PropertyTagTypeUndefined },
		{ L"Thumbnail|Width", PropertyTagTypeShort },
		{ L"Thumbnail|X resolution", PropertyTagTypeRational },
		{ L"Thumbnail|Y resolution", PropertyTagTypeRational },
		{ L"Thumbnail|Resolution unit", PropertyTagTypeShort },
		{ L"Label|1. Title", PropertyTagTypeASCII },

		// these three are actually bytes, but defined as shorts
		// to indicate they are stored a UNICODE
		{ L"Label|2. Location", PropertyTagTypeShort },
		{ L"Label|3. Comment", PropertyTagTypeShort },
		{ L"User|5. Keywords", PropertyTagTypeShort },

		{ L"User|1. Filename", PropertyTagTypeASCII },
		{ L"User|2. Artist", PropertyTagTypeASCII },
		{ L"User|3. Copyright", PropertyTagTypeASCII },
		{ L"Label|4. Date taken", PropertyTagTypeASCII },
		{ L"X|Rating stars", PropertyTagTypeShort },
		{ L"X|Rating percent", PropertyTagTypeShort },
		{ L"User|4. Software", PropertyTagTypeASCII },
	};

	// cross reference keys and IDs
	shared_ptr<CString> pNode;
	shared_ptr<ULONG> pID;

	// map keys to property tags
	shared_ptr<CPropertyTag> pTags;

	for ( auto& node : mapPropertyKeys )
	{
		// create an index of the property ids to their keys 
		// "group|label" 
		CString csKey = node.second;
		ULONG ulID = node.first;
		pNode = shared_ptr<CString>( new CString( csKey ));
		m_IdCollection.add( ulID, pNode );

		// create a collection of property tags indexed by 
		// their keys that provide groups, labels,
		// descriptions, etc.
		pTags = shared_ptr<CPropertyTag>( new CPropertyTag );
		m_PropertyTags.add( csKey, pTags );
		pTags->PropertyKey = csKey;
		pTags->PropertyID = ulID;

		{
			map<CString, CString>::iterator posEnd = mapDesc.end();
			map<CString, CString>::iterator pos = mapDesc.find( csKey );
			if ( pos != posEnd )
			{
				pTags->PropertyDescription = pos->second;

			} else
			{
				CString csError;
				csError.Format( L"Description key %s was not found.", (LPCTSTR)csKey );
			}
		}
		{
			map<CString, WORD>::iterator posEnd = mapTypes.end();
			map<CString, WORD>::iterator pos = mapTypes.find( csKey );
			if ( pos != posEnd )
			{
				WORD wType = pos->second;
				pTags->PropertyType = wType;
				VARENUM varType = *m_TypeCollection.find( wType );
				pTags->DataType = varType;

			} else
			{
				CString csError;
				csError.Format( L"Property type %s was not found.", (LPCTSTR)csKey );
			}
		}
		// process short enumerations
		switch ( ulID )
		{
			case PropertyTagThumbnailResolutionUnit:
			case PropertyTagResolutionUnit:
			{
				pTags->AddEnumerations( 2, L"inches" );
				pTags->AddEnumerations( 3, L"centimeters" );
				break;
			}
			case PropertyTagExifExposureMode:
			{
				pTags->AddEnumerations( 0, L"Auto exposure" );
				pTags->AddEnumerations( 1, L"Manual exposure" );
				pTags->AddEnumerations( 2, L"Auto bracket" );
				break;
			}
			case PropertyTagExifMeteringMode:
			{
				pTags->AddEnumerations( 0, L"Unknown" );
				pTags->AddEnumerations( 1, L"Average" );
				pTags->AddEnumerations( 2, L"Center - weighted average" );
				pTags->AddEnumerations( 3, L"Spot" );
				pTags->AddEnumerations( 4, L"Multi - spot" );
				pTags->AddEnumerations( 5, L"Multi - segment" );
				pTags->AddEnumerations( 6, L"Partial" );
				break;
			}
			case PropertyTagExifWhiteBalance:
			{
				pTags->AddEnumerations( 0, L"Auto white balance" );
				pTags->AddEnumerations( 1, L"Manual white balance" );
				break;
			}
			case PropertyTagExifSceneCaptureType:
			{
				pTags->AddEnumerations( 0, L"Standard" );
				pTags->AddEnumerations( 1, L"Landscape" );
				pTags->AddEnumerations( 2, L"Portrait" );
				pTags->AddEnumerations( 3, L"Night scene" );
				break;
			}
			case PropertyTagExifExposureProg:
			{
				pTags->AddEnumerations( 0, L"Not specified" );
				pTags->AddEnumerations( 1, L"Manual" );
				pTags->AddEnumerations( 2, L"Program AE( Automatic Exposure )" );
				pTags->AddEnumerations( 3, L"Aperture Priority AE" );
				pTags->AddEnumerations( 4, L"Shutter Priority AE" );
				pTags->AddEnumerations( 5, L"Creative Program( Camera sets both shutter speed and aperture )" );
				pTags->AddEnumerations( 6, L"Action Program( Optimized for fast - moving subjects )" );
				pTags->AddEnumerations( 7, L"Portrait Mode( Optimized for portraits )" );
				pTags->AddEnumerations( 8, L"Landscape Mode( Optimized for landscapes )" );
				pTags->AddEnumerations( 9, L"Bulb1" );
				break;
			}
			case PropertyTagExifFlash:
			{
				pTags->AddEnumerations( 0x0000, L"Flash did not fire" );
				pTags->AddEnumerations( 0x0001, L"Flash fired" );
				pTags->AddEnumerations( 0x0005, L"Strobe return light not detected" );
				pTags->AddEnumerations( 0x0007, L"Strobe return light detected" );
				pTags->AddEnumerations( 0x0009, L"Flash fired, compulsory flash mode" );
				pTags->AddEnumerations( 0x000D, L"Flash fired, compulsory flash mode, return light not detected" );
				pTags->AddEnumerations( 0x000F, L"Flash fired, compulsory flash mode, return light detected" );
				pTags->AddEnumerations( 0x0010, L"Flash did not fire, compulsory flash mode" );
				pTags->AddEnumerations( 0x0018, L"Flash did not fire, auto mode" );
				pTags->AddEnumerations( 0x0019, L"Flash fired, auto mode" );
				pTags->AddEnumerations( 0x001D, L"Flash fired, auto mode, return light not detected" );
				pTags->AddEnumerations( 0x001F, L"Flash fired, auto mode, return light detected" );
				pTags->AddEnumerations( 0x0020, L"No flash function" );
				pTags->AddEnumerations( 0x0041, L"Flash fired, red - eye reduction mode" );
				pTags->AddEnumerations( 0x0045, L"Flash fired, red - eye reduction mode, return light not detected" );
				pTags->AddEnumerations( 0x0047, L"Flash fired, red - eye reduction mode, return light detected" );
				pTags->AddEnumerations( 0x0049, L"Flash fired, compulsory flash mode, red - eye reduction mode" );
				pTags->AddEnumerations( 0x004D, L"Flash fired, compulsory flash mode, red - eye reduction mode, return light not detected" );
				pTags->AddEnumerations( 0x004F, L"Flash fired, compulsory flash mode, red - eye reduction mode, return light detected" );
				pTags->AddEnumerations( 0x0059, L"Flash fired, auto mode, red - eye reduction mode" );
				pTags->AddEnumerations( 0x005D, L"Flash fired, auto mode, return light not detected, red - eye reduction mode" );
				pTags->AddEnumerations( 0x005F, L"Flash fired, auto mode, return light detected, red - eye reduction mode" );
				break;
			}
			case PropertyTagExifLightSource:
			{
				pTags->AddEnumerations( 0, L"unknown" );
				pTags->AddEnumerations( 1, L"Daylight" );
				pTags->AddEnumerations( 2, L"Flourescent" );
				pTags->AddEnumerations( 3, L"Tungsten" );
				pTags->AddEnumerations( 17, L"Standard Light A" );
				pTags->AddEnumerations( 18, L"Standard Light B" );
				pTags->AddEnumerations( 19, L"Standard Light C" );
				pTags->AddEnumerations( 20, L"D55" );
				pTags->AddEnumerations( 21, L"D65" );
				pTags->AddEnumerations( 22, L"D75" );
				pTags->AddEnumerations( 255, L"other" );
				break;
			}
			case PropertyTagGpsAltitudeRef:
			{
				pTags->AddEnumerations( 0, L"Above Sea Level" );
				pTags->AddEnumerations( 1, L"Below Sea Level" );
				break;
			}
			case PropertyTagYCbCrPositioning:
			{
				pTags->AddEnumerations( 1, L"centered" );
				pTags->AddEnumerations( 2, L"co-sited" );
				break;
			}
			case PropertyTagThumbnailCompression:
			{
				pTags->AddEnumerations( 1, L"Uncompressed" );
				pTags->AddEnumerations( 6, L"JPEG compression( commonly used for thumbnails )" );
				break;
			}
			case PropertyTagExifColorSpace:
			{
				pTags->AddEnumerations( 1, L"sRGB" );
				break;
			}
			case PropertyTagOrientation:
			{
				pTags->AddEnumerations( 1, L"Horizontal( normal )" );
				pTags->AddEnumerations( 2, L"Mirror horizontal" );
				pTags->AddEnumerations( 3, L"Rotate 180 degrees" );
				pTags->AddEnumerations( 4, L"Mirror vertical" );
				pTags->AddEnumerations( 5, L"Mirror horizontal and rotate 270 CW" );
				pTags->AddEnumerations( 6, L"Rotate 90 CW" );
				pTags->AddEnumerations( 7, L"Mirror horizontal and rotate 90 CW" );
				pTags->AddEnumerations( 8, L"Rotate 270 CW" );
				break;
			}
			case PropertyTagExifFileSource:
			{
				pTags->AddEnumerations( 3, L"Digital Still Camera" );
				break;
			}
			case PropertyTagExifSharpness:
			case PropertyTagExifSaturation:
			case PropertyTagExifContrast:
			case PropertyTagExifCustomRendered:
			{
				pTags->AddEnumerations( 0, L"Normal" );
				break;
			}
			case PropertyTagExifGainControl:
			{
				pTags->AddEnumerations( 0, L"None" );
				break;
			}
		}
	}

	// populate the date and time IDs
	WORD wIndex = 0;
	m_DateIDs.add
	(
		L"Year", shared_ptr<WORD>( new WORD( wIndex++ ) )
	);
	m_DateIDs.add
	(
		L"Month", shared_ptr<WORD>( new WORD( wIndex++ ) )
	);
	m_DateIDs.add
	(
		L"Day", shared_ptr<WORD>( new WORD( wIndex++ ) )
	);
	m_DateIDs.add
	(
		L"Hour", shared_ptr<WORD>( new WORD( wIndex++ ) )
	);
	m_DateIDs.add
	(
		L"Minute", shared_ptr<WORD>( new WORD( wIndex++ ) )
	);
	m_DateIDs.add
	(
		L"Second", shared_ptr<WORD>( new WORD( wIndex++ ) )
	);

} // CImageProperties

/////////////////////////////////////////////////////////////////////////////
CImageProperties::~CImageProperties()
{

} // ~CImageProperties

/////////////////////////////////////////////////////////////////////////////
BYTE CImageProperties::GetPropertyTypeSize( WORD wType )
{
	BYTE value = 1;
	switch ( wType )
	{
		case PropertyTagTypeUndefined:
		case PropertyTagTypeASCII:
		case PropertyTagTypeChar:
		{
			value = sizeof( char ); break;
		}
		case PropertyTagTypeByte:
		{
			value = sizeof( byte ); break;
		}
		case PropertyTagTypeShort:
		{
			value = sizeof( USHORT ); break;
		}
		case PropertyTagTypeRational:
		case PropertyTagTypeLong:
		{
			value = sizeof( ULONG ); break;
		}
		case PropertyTagTypeSRational:
		case PropertyTagTypeSLONG:
		{
			value = sizeof( LONG ); break;
		}
	}
	return value;
} // GetPropertyTypeSize

/////////////////////////////////////////////////////////////////////////////
CString CImageProperties::GetPropertyTypeName( WORD wType )
{
	CString value( L"Error" );
	switch ( wType )
	{
		case PropertyTagTypeUndefined: 
		{
			value = L"UNDEFINED"; break;
		}
		case PropertyTagTypeChar: 
		{
			value = L"CHAR"; break;
		}
		case PropertyTagTypeByte:
		{
			value = L"BYTE"; break;
		}
		case PropertyTagTypeASCII:
		{
			value = L"ASCII"; break;
		}
		case PropertyTagTypeShort:
		{
			value = L"USHORT"; break;
		}
		case PropertyTagTypeRational: 
		{
			value = L"RATIONAL"; break;
		}
		case PropertyTagTypeLong:
		{
			value = L"ULONG"; break;
		}
		case PropertyTagTypeSRational: 
		{
			value = L"SRATIONAL"; break;
		}
		case PropertyTagTypeSLONG:
		{
			value = L"LONG"; break;
		}
	}
	return value;
} // GetPropertyTypeName

/////////////////////////////////////////////////////////////////////////////
CString CImageProperties::GetDumpPropertyItem( Gdiplus::PropertyItem* pItem )
{
	// PROPID	id;		// ID of this property
	// ULONG	length;	// Length of the property value, in bytes
	// WORD		type;	// Type of the value, as one of TAG_TYPE_XXX
	// VOID*	value;	// property value
	CString value;
	ULONG ulID = pItem->id;
	WORD wType = pItem->type;
	BYTE bSize = PropertyTypeSize[ wType ];
	ULONG ulLength = pItem->length;
	ULONG ulValues = ulLength / bSize;
	CString csType = PropertyTypeName[ wType ];
	value.Format
	( 
		L"ID: 0x%04X, Type: %10s, Bytes: % 6d, Values: % 6d, ",
		ulID, (LPCTSTR)csType, ulLength, ulValues
	);

	CString csValues;
	switch ( wType )
	{
		case PropertyTagTypeUndefined:
		case PropertyTagTypeChar:
		{	
			char Type = ' ';
			csValues = ValuesCSV( pItem->value, Type, ulLength );
			break;
		}
		case PropertyTagTypeByte:
		{	
			BYTE Type = 0;
			csValues = ValuesCSV( pItem->value, Type, ulLength );
			break;
		}
		case PropertyTagTypeASCII:
		{	
			csValues =
				CString( CA2CT( reinterpret_cast<char*>( pItem->value ) ) );
			//csValues = ValuesCSV( pItem->value, Type, ulLength );
			break;
		}
		case PropertyTagTypeSRational: 
		case PropertyTagTypeSLONG:
		{
			LONG Type = 0;
			csValues = ValuesCSV( pItem->value, Type, ulLength );
			break;
		}
		default :
		{
			ULONG Type = 0;
			csValues = ValuesCSV( pItem->value, Type, ulLength );
		}
	}

	value += csValues;

	return value;
} // GetDumpPropertyItem

/////////////////////////////////////////////////////////////////////////////
// apply album labels
void CImageProperties::ApplyAlbumLabels()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
	
	// if the shift key is down, then overwrite existing values and not
	// just undefined values
	const bool bShift = CHelper::ShiftKeyDown();

	const bool bTitle = pDoc->AlbumTitle;
	const bool bLocation = pDoc->AlbumLocation;
	const bool bComment = pDoc->AlbumComment;
	const bool bDate = pDoc->AlbumDate;

	const bool bArtist = pDoc->AlbumArtist;
	const bool bCopyright = pDoc->AlbumCopyright;
	const bool bSoftware = pDoc->AlbumSoftware;
	const bool bKeywords = pDoc->AlbumKeywords;

	CString csTitle = pDoc->Title;
	CString csLocation = pDoc->Location;
	CString csComment = pDoc->Comment;
	CString csDate = pDoc->Date;

	CString csArtist = pDoc->Artist;
	CString csCopyright = pDoc->Copyright;
	CString csSoftware = pDoc->Software;
	CString csKeywords = pDoc->Keywords;

	CString csTitleKey = L"Label|1. Title";
	CString csLocationKey = L"Label|2. Location";
	CString csCommentKey = L"Label|3. Comment";
	CString csDateKey = L"Label|4. Date taken";

	CString csArtistKey = L"User|2. Artist";
	CString csCopyrightKey = L"User|3. Copyright";
	CString csSoftwareKey = L"User|4. Software";
	CString csKeywordsKey = L"User|5. Keywords";

	if ( bTitle && ( bShift || !csTitle.IsEmpty() ) )
	{
		shared_ptr<CImageProperty> pProp = ImageProperty[ csTitleKey ];
		if ( pProp == nullptr )
		{
			CreateStringProperty( csTitleKey, csTitle );
			pProp = ImageProperty[ csTitleKey ];
			pProp->Modified = true;

		} else
		{
			ULONG ulTitleElements = pProp->Data->Elements;

			// empty values contain a single null character
			if ( bShift || ulTitleElements < 2 )
			{
				VARENUM varType = pProp->DataType;
				CStringA csValue( csTitle );
				void* pBuffer = csValue.GetBuffer();
				ULONG ulLen = csValue.GetLength() + 1;
				pProp->Data->setData( varType, ulLen, (char*)pBuffer );
				csValue.ReleaseBuffer();
				pProp->Modified = true;
			}
		}
	}

	if ( bDate && ( bShift || !csDate.IsEmpty() ) )
	{
		shared_ptr<CImageProperty> pProp = ImageProperty[ csDateKey ];
		if ( pProp == nullptr )
		{
			CreateStringProperty( csDateKey, csDate );
			pProp = ImageProperty[ csDateKey ];
			pProp->Modified = true;

		} else
		{
			ULONG ulDateElements = pProp->Data->Elements;

			// empty values contain a single null character
			if ( bShift || ulDateElements < 2 )
			{
				VARENUM varType = pProp->DataType;
				CStringA csValue( csDate );
				void* pBuffer = csValue.GetBuffer();
				ULONG ulLen = csValue.GetLength() + 1;
				pProp->Data->setData( varType, ulLen, (char*)pBuffer );
				csValue.ReleaseBuffer();
				pProp->Modified = true;
			}
		}
	}

	if ( bComment && ( bShift || !csComment.IsEmpty() ) )
	{
		shared_ptr<CImageProperty> pProp = ImageProperty[ csCommentKey ];
		if ( pProp == nullptr )
		{
			CreateStringProperty( csCommentKey, csComment );
			pProp = ImageProperty[ csCommentKey ];
			pProp->Modified = true;

		} else
		{
			ULONG ulCommentElements = pProp->Data->Elements;

			// empty values contain a single null character
			if ( bShift || ulCommentElements < 2 )
			{
				VARENUM varType = pProp->DataType;
				void* pBuffer = csComment.GetBuffer();
				ULONG ulLen = csComment.GetLength() + 1;
				pProp->Data->setData( varType, ulLen, (char*)pBuffer );
				csComment.ReleaseBuffer();
				pProp->Modified = true;
			}
		}
	}

	if ( bLocation && ( bShift || !csLocation.IsEmpty() ) )
	{
		shared_ptr<CImageProperty> pProp = ImageProperty[ csLocationKey ];
		if ( pProp == nullptr )
		{
			CreateStringProperty( csLocationKey, csLocation );
			pProp = ImageProperty[ csLocationKey ];
			pProp->Modified = true;

		} else
		{
			ULONG ulLocationElements = pProp->Data->Elements;

			// empty values contain a single null character
			if ( bShift || ulLocationElements < 2 )
			{
				VARENUM varType = pProp->DataType;
				void* pBuffer = csLocation.GetBuffer();
				ULONG ulLen = csLocation.GetLength() + 1;
				pProp->Data->setData( varType, ulLen, (char*)pBuffer );
				csLocation.ReleaseBuffer();
				pProp->Modified = true;
			}
		}
	}

	if ( bArtist && ( bShift || !csArtist.IsEmpty() ) )
	{
		shared_ptr<CImageProperty> pProp = ImageProperty[ csArtistKey ];
		if ( pProp == nullptr )
		{
			CreateStringProperty( csArtistKey, csArtist );
			pProp = ImageProperty[ csArtistKey ];
			pProp->Modified = true;

		} else
		{
			ULONG ulArtistElements = pProp->Data->Elements;

			// empty values contain a single null character
			if ( bShift || ulArtistElements < 2 )
			{
				VARENUM varType = pProp->DataType;
				CStringA csValue( csArtist );
				void* pBuffer = csValue.GetBuffer();
				ULONG ulLen = csValue.GetLength() + 1;
				pProp->Data->setData( varType, ulLen, (char*)pBuffer );
				csValue.ReleaseBuffer();
				pProp->Modified = true;
			}
		}
	}

	if ( bCopyright && ( bShift || !csCopyright.IsEmpty() ) )
	{
		shared_ptr<CImageProperty> pProp = ImageProperty[ csCopyrightKey ];
		if ( pProp == nullptr )
		{
			CreateStringProperty( csCopyrightKey, csCopyright );
			pProp = ImageProperty[ csCopyrightKey ];
			pProp->Modified = true;

		} else
		{
			ULONG ulCopyrightElements = pProp->Data->Elements;

			// empty values contain a single null character
			if ( bShift || ulCopyrightElements < 2 )
			{
				VARENUM varType = pProp->DataType;
				CStringA csValue( csCopyright );
				void* pBuffer = csValue.GetBuffer();
				ULONG ulLen = csValue.GetLength() + 1;
				pProp->Data->setData( varType, ulLen, (char*)pBuffer );
				csValue.ReleaseBuffer();
				pProp->Modified = true;
			}
		}
	}

	if ( bSoftware && ( bShift || !csSoftware.IsEmpty() ) )
	{
		shared_ptr<CImageProperty> pProp = ImageProperty[ csSoftwareKey ];
		if ( pProp == nullptr )
		{
			CreateStringProperty( csSoftwareKey, csSoftware );
			pProp = ImageProperty[ csSoftwareKey ];
			pProp->Modified = true;

		} else
		{
			ULONG ulSoftwareElements = pProp->Data->Elements;

			// empty values contain a single null character
			if ( bShift || ulSoftwareElements < 2 )
			{
				VARENUM varType = pProp->DataType;
				CStringA csValue( csSoftware );
				void* pBuffer = csValue.GetBuffer();
				ULONG ulLen = csValue.GetLength() + 1;
				pProp->Data->setData( varType, ulLen, (char*)pBuffer );
				csValue.ReleaseBuffer();
				pProp->Modified = true;
			}
		}
	}

	if ( bKeywords && ( bShift || !csKeywords.IsEmpty() ) )
	{
		shared_ptr<CImageProperty> pProp = ImageProperty[ csKeywordsKey ];
		if ( pProp == nullptr )
		{
			CreateStringProperty( csKeywordsKey, csKeywords );
			pProp = ImageProperty[ csKeywordsKey ];
			pProp->Modified = true;

		} else
		{
			ULONG ulKeywordsElements = pProp->Data->Elements;

			// empty values contain a single null character
			if ( bShift || ulKeywordsElements < 2 )
			{
				VARENUM varType = pProp->DataType;
				CStringA csValue( csKeywords );
				void* pBuffer = csValue.GetBuffer();
				ULONG ulLen = csValue.GetLength() + 1;
				pProp->Data->setData( varType, ulLen, (char*)pBuffer );
				csValue.ReleaseBuffer();
				pProp->Modified = true;
			}
		}
	}

} // ApplyAlbumLabels

///////////////////////////////////////////////////////////////////////////
// save property changes to the selected image and its thumbnail
void CImageProperties::SaveProperties()
{
	const WORD wChanges = ModifiedValues;
	if ( wChanges > 0 )
	{
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
		CShellListView* pListView = (CShellListView*)pFrame->ListView;
		
		CString csPath = pDoc->ImagePath;
		if ( !::PathFileExists( csPath ))
		{
			ModifiedValues = 0;
			return;
		}

		CString csExt = CHelper::GetExtension( csPath );
		Extension->FileExtension = csExt;

		CString csError;
		unique_ptr<Bitmap> pBitmap = 
			CHelper::LoadBitmapFromFile( csPath, csError );
		if ( pBitmap == nullptr )
		{
			pListView->ErrorsText = csError;
			ModifiedValues = 0;
			return;
		}

		shared_ptr<Bitmap> pThumbnail = pDoc->Thumbnail[ csPath ];

		// loop through the properties looking for modifications
		for ( auto& node : m_Properties.Items )
		{
			shared_ptr<CImageProperty> pProp = node.second;
			if ( pProp->Modified )
			{
				pProp->Modified = false;
				unique_ptr<CImageProperty::CPropertyItem> pItem;
				pProp->GetPropertyItem( pItem );
				ULONG ulID = pItem->id;
				void* pBuffer = nullptr;
				ULONG ulLength = 0;
				switch ( ulID )
				{
					case PropertyTagXPSubject:
					case PropertyTagXPComment:
					case PropertyTagXPKeywords:
					{
						pItem->type = PropertyTagTypeByte;
						CString csValue( (LPCTSTR)pItem->value );
						csValue = pDoc->ResolveShortcuts( csValue );
						ulLength =
							(ULONG)csValue.GetLength() * sizeof( WCHAR ) + 
							sizeof( WCHAR );
						pBuffer = (void*)csValue.GetBuffer();
						// create a GDI+ property item
						unique_ptr<CImageProperty::CPropertyItem>pI1 =
							unique_ptr<CImageProperty::CPropertyItem>
							(
								new CImageProperty::CPropertyItem
								( 
									ulID, pItem->type, ulLength 
								)
							);

						// copy the string into the property item value
						::CopyMemory( pI1->value, pBuffer, ulLength );
						// cast it to a pointer that will make the compiler 
						// happy
						Gdiplus::PropertyItem* pI2 =
							reinterpret_cast<Gdiplus::PropertyItem*>
							( 
								pI1.get() 
							);

						// finally our goal was to write the album value to 
						// the image
						pBitmap->SetPropertyItem( pI2 );
						csValue.ReleaseBuffer();

						// if the thumbnail is defined, it has all of the
						// properties of the selected image
						if ( pThumbnail != nullptr )
						{
							// pass in the new item
							pThumbnail->SetPropertyItem( pI2 );
						}
						break;
					}
					case PropertyTagImageDescription:
					case PropertyTagArtist:
					case PropertyTagSoftwareUsed:
					case PropertyTagExifDTOrig:
					case PropertyTagCopyright:
					{
						CStringA csValueA( (char*)pItem->value );
						CString csValue = 
							pDoc->ResolveShortcuts( CString( csValueA ));
						csValueA = CStringA( csValue );
						ulLength = (ULONG)csValueA.GetLength() + 1;
						pBuffer = (void*)csValueA.GetBuffer();
						// create a GDI+ property item
						unique_ptr<CImageProperty::CPropertyItem>pI1 =
							unique_ptr<CImageProperty::CPropertyItem>
							(
								new CImageProperty::CPropertyItem
								( 
									ulID, pItem->type, ulLength 
								)
							);

						// copy the string into the property item value
						::CopyMemory( pI1->value, pBuffer, ulLength );
						// cast it to a pointer that will make the compiler 
						// happy
						Gdiplus::PropertyItem* pI2 =
							reinterpret_cast<Gdiplus::PropertyItem*>
							( 
								pI1.get() 
							);

						// finally our goal was to write the album value to 
						// the image
						pBitmap->SetPropertyItem( pI2 );
						csValueA.ReleaseBuffer();

						// if the thumbnail is defined, it has all of the
						// properties of the selected image
						if ( pThumbnail != nullptr )
						{
							// pass in the new item
							pThumbnail->SetPropertyItem( pI2 );
						}
						break;
					}
					default:
					{
						// pass in the new item
						pBitmap->SetPropertyItem
						(
							(Gdiplus::PropertyItem*)pItem.get()
						);

						// if the thumbnail is defined, it has all of the
						// properties of the selected image
						if ( pThumbnail != nullptr )
						{
							// pass in the new item
							pThumbnail->SetPropertyItem
							(
								(Gdiplus::PropertyItem*)pItem.get()
							);
						}
					}
				}
			}
		}

		// save the bitmap using the class ID associated with
		// the file's extension
		CLSID classID = Extension->ClassID;
		Gdiplus::Status result = pBitmap->Save( csPath, &classID );
		if ( result != Gdiplus::Ok )
		{
			CString csError = CHelper::GdiPlusStatusToText( result );
			CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
			CShellListView* pListView = (CShellListView*)pFrame->ListView;
			pListView->ErrorsText = csError + L": saving file:";
			pListView->ErrorsText = csPath;
		}

		ModifiedValues = 0;
	}
} // SaveProperties

/////////////////////////////////////////////////////////////////////////////
// add a property to our collection given the property structure
// stored in the image
bool CImageProperties::AddProperty( Gdiplus::PropertyItem* pItem )
{
	bool value = false;
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;

	WORD wType = pItem->type;
	const ULONG ulItem = (ULONG)pItem->id;
	const ULONG ulLength = pItem->length;
	ULONG ulValues = 0;
	switch ( ulItem )
	{
		case PropertyTagExifMakerNote:
		case PropertyTagPrintIMVersion:
		{
			return value;
		}
	}

	const bool bId = m_IdCollection.Exists[ ulItem ];
	if ( !bId )
	{
		CString csError;
		csError.Format( L"Property ID 0x%04X was not found.", ulItem );
		pDoc->WarningsText = csError;
		CString csData = DumpPropertyItem[ pItem ];
		pDoc->WarningsText = csData;
		return value;
	} 

	const bool bType = m_TypeCollection.Exists[ wType ];
	if ( !bType )
	{
		CString csError;
		csError.Format( L"Property type % d was not found.", wType );
		pDoc->WarningsText = csError;
		return value;
	} 

	CString csKey = *m_IdCollection.find( ulItem );
	int nStart = 0;
	const bool bProperty = Properties->Exists[ csKey ];
	if ( bProperty )
	{
		CString csError;
		csError.Format( L"Property key %s is duplicated.", (LPCTSTR)csKey );
		pDoc->WarningsText = csError;
		return value;
	}

	shared_ptr<CPropertyTag> propTag = m_PropertyTags.find( csKey );

	// override the type if necessary
	if ( propTag->PropertyType != wType )
	{
		wType = propTag->PropertyType;
		pItem->type = wType;
	}

	VARENUM varType = propTag->DataType;
	if ( varType == VT_UNKNOWN )
	{
		CString csError;
		csError.Format( L"Property key %s is unknown type.", (LPCTSTR)csKey );
		pDoc->WarningsText = csError;
		return value;
	}

	CString csDesc = propTag->PropertyDescription;
	CString csGroup = propTag->PropertyGroup;
	CString csLabel = propTag->PropertyLabel;

	shared_ptr<CImageProperty> pProperty = shared_ptr<CImageProperty>
	(
		new CImageProperty
	);

	pProperty->DataType = varType;
	pProperty->PropertyType = wType;
	pProperty->PropertyID = ulItem;
	pProperty->Length = ulLength;
	pProperty->PropertyLabel = csLabel;
	pProperty->PropertyGroup = csGroup;
	pProperty->PropertyDescription = csDesc;
	pProperty->Container = this;

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
			value = pProperty->CopyText( pItem->value, ulLength );
			break;
		}
		case PropertyTagTypeASCII:
		{
			CStringW strW( (wchar_t*)pItem->value );
			ULONG ulW = strW.GetLength() + 1;
			CStringA strA( (char*)pItem->value );
			ULONG ulA = strA.GetLength() + 1;
			if ( ulA == ulLength )
			{
				value = pProperty->CopyText( pItem->value, ulLength );

			} else
			{
				LPSTR pBuffer = strA.GetBufferSetLength( ulW );
				CHelper::CStringToASCII( strW, pBuffer, ulW );
				value = pProperty->CopyText( pBuffer, ulLength );
				strA.ReleaseBuffer();
			}
			break;
		}
		case PropertyTagTypeByte:
		{
			BYTE tType = 0;
			value = pProperty->CopyArray( pItem->value, ulLength, tType );
			break;
		}
		case PropertyTagTypeShort:
		{
			USHORT tType = 0;
			value = pProperty->CopyArray( pItem->value, ulLength, tType );
			break;
		}
		case PropertyTagTypeRational:
		case PropertyTagTypeLong:
		{
			ULONG tType = 0;
			value = pProperty->CopyArray( pItem->value, ulLength, tType );
			break;
		}
		case PropertyTagTypeSRational:
		case PropertyTagTypeSLONG:
		{
			LONG tType = 0;
			value = pProperty->CopyArray( pItem->value, ulLength, tType );
			break;
		}
	}

	Properties->add( csKey, pProperty );

	return value;
} // AddProperty

/////////////////////////////////////////////////////////////////////////////
bool CImageProperties::CreateShortProperty( CString& csKey, USHORT& usValue )
{
	bool value = m_PropertyTags.Exists[ csKey ];
	if ( !value )
	{
		return value;
	}

	if ( false == Properties->Exists[ csKey ] )
	{
		shared_ptr<CPropertyTag> propTag = m_PropertyTags.find( csKey );
		shared_ptr<CImageProperty> pProperty = shared_ptr<CImageProperty>
		(
			new CImageProperty
		);

		CString csGroup = propTag->PropertyGroup;
		CString csLabel = propTag->PropertyLabel;
		CString csDesc = propTag->PropertyDescription;;
		ULONG ulID = propTag->PropertyID;
		WORD wType = propTag->PropertyType;
		VARENUM varType = propTag->DataType;

		pProperty->DataType = varType;
		pProperty->PropertyType = wType;
		pProperty->PropertyID = ulID;
		pProperty->Length = sizeof( USHORT );
		pProperty->PropertyGroup = csGroup;
		pProperty->PropertyLabel = csLabel;
		pProperty->PropertyDescription = csDesc;
		pProperty->Container = this;

		pProperty->Data->setData( varType, 2, (char*)&usValue );

		value = Properties->add( csKey, pProperty );
	}

	return value;
} // CreateShortProperty

/////////////////////////////////////////////////////////////////////////////
// create a new string property
bool CImageProperties::CreateStringProperty
(
	CString& csKey, CString& csValue
)
{
	bool value = m_PropertyTags.Exists[ csKey ];
	if ( !value )
	{
		return value;
	}

	if ( false == Properties->Exists[ csKey ] )
	{
		shared_ptr<CPropertyTag> propTag = m_PropertyTags.find( csKey );
		shared_ptr<CImageProperty> pProperty = shared_ptr<CImageProperty>
		(
			new CImageProperty
		);

		CString csGroup = propTag->PropertyGroup;
		CString csLabel = propTag->PropertyLabel;
		CString csDesc = propTag->PropertyDescription;;
		ULONG ulID = propTag->PropertyID;
		WORD wType = propTag->PropertyType;
		VARENUM varType = propTag->DataType;
		//VARENUM varType = VT_I1;

		// Convert the CString to a CStringA (ASCII)
		CStringA asciiString( csValue );

		void* pChars = nullptr;
		ULONG ulLen = 0;
		ULONG ulChars = csValue.GetLength() + 1;

		if ( varType == VT_I1 )
		{
			ulLen = ulChars + 1;
			pChars = asciiString.GetBuffer();

		} else // unicode
		{
			ulLen = ulChars * sizeof( WCHAR );
			pChars = csValue.GetBuffer();
		}

		pProperty->DataType = varType;
		pProperty->PropertyType = wType;
		pProperty->PropertyID = ulID;
		pProperty->Length = ulLen;
		pProperty->PropertyGroup = csGroup;
		pProperty->PropertyLabel = csLabel;
		pProperty->PropertyDescription = csDesc;
		pProperty->Container = this;

		value = pProperty->Data->setData( varType, ulChars, (char*)pChars );

		if ( true == value )
		{
			value = Properties->add( csKey, pProperty );
		}

		if ( varType == VT_I1 )
		{
			asciiString.ReleaseBuffer();

		} else // unicode
		{
			csValue.ReleaseBuffer();
		}
	}

	return value;
} // CreateStringProperty

/////////////////////////////////////////////////////////////////////////////
// change existing string property
bool CImageProperties::ChangeStringProperty
(
	CString& csKey, CString& csValue
)
{
	bool value = m_PropertyTags.Exists[ csKey ];
	if ( !value )
	{
		return value;
	}

	if ( true == Properties->Exists[ csKey ] )
	{
		shared_ptr<CPropertyTag> propTag = m_PropertyTags.find( csKey );
		shared_ptr<CImageProperty> pProperty = Properties->find( csKey );

		CString csGroup = propTag->PropertyGroup;
		CString csLabel = propTag->PropertyLabel;
		CString csDesc = propTag->PropertyDescription;;
		ULONG ulID = propTag->PropertyID;
		WORD wType = propTag->PropertyType;
		VARENUM varType = propTag->DataType;

		// Convert the CString to a CStringA (ASCII)
		CStringA asciiString( csValue );
		const ULONG ulLen = asciiString.GetLength() + 1;

		pProperty->DataType = varType;
		pProperty->PropertyType = wType;
		pProperty->PropertyID = ulID;
		pProperty->Length = ulLen;
		pProperty->PropertyGroup = csGroup;
		pProperty->PropertyLabel = csLabel;
		pProperty->PropertyDescription = csDesc;
		pProperty->Container = this;
			
		LPSTR pChars = asciiString.GetBuffer();
		value = pProperty->CopyText( pChars, ulLen );

		if ( true == value )
		{
			value = Properties->add( csKey, pProperty );
		}
		asciiString.ReleaseBuffer();
	}

	return value;
} // ChangeStringProperty

/////////////////////////////////////////////////////////////////////////////
void CImageProperties::AddPropertyPanelDate
(
	CMFCPropertyGridProperty* pGroup,
	CString csLabel, CString csValue, CString csDescription
)
{
	CMFCPropertyGridProperty* pProp = nullptr;

	vector<int> v;
	int nStart = 0;
	CString csToken = csValue.Tokenize( L": ", nStart );
	while ( !csToken.IsEmpty() )
	{
		int nItem = (int)_tstol( csToken );
		v.push_back( nItem );
		csToken = csValue.Tokenize( L": ", nStart );
	}
	COleDateTime oDT( COleDateTime::GetCurrentTime() );

	pProp = new CMFCPropertyGridProperty( csLabel, 0, TRUE );
	pProp->SetDescription( csDescription );

	CMFCPropertyGridProperty* pSub = new CMFCPropertyGridProperty
	(
		_T( "Year" ), (_variant_t)v[ 0 ], csDescription
	);
	pSub->EnableSpinControl( TRUE, 1800, oDT.GetYear() );
	pProp->AddSubItem( pSub );

	pSub = new CMFCPropertyGridProperty
	(
		_T( "Month" ), (_variant_t)v[ 1 ], csDescription
	);
	pSub->EnableSpinControl( TRUE, 1, 12 );
	pProp->AddSubItem( pSub );

	pSub = new CMFCPropertyGridProperty
	(
		_T( "Day" ), (_variant_t)v[ 2 ], csDescription
	);
	pSub->EnableSpinControl( TRUE, 1, 31 );
	pProp->AddSubItem( pSub );

	pSub = new CMFCPropertyGridProperty
	(
		_T( "Hour" ), (_variant_t)v[ 3 ], csDescription
	);
	pSub->EnableSpinControl( TRUE, 0, 23 );
	pProp->AddSubItem( pSub );

	pSub = new CMFCPropertyGridProperty
	(
		_T( "Minute" ), (_variant_t)v[ 4 ], csDescription
	);
	pSub->EnableSpinControl( TRUE, 0, 59 );
	pProp->AddSubItem( pSub );

	pSub = new CMFCPropertyGridProperty
	(
		_T( "Second" ), (_variant_t)v[ 5 ], csDescription
	);
	pSub->EnableSpinControl( TRUE, 0, 59 );
	pProp->AddSubItem( pSub );

	pGroup->AddSubItem( pProp );
} // AddPropertyPanelDate

/////////////////////////////////////////////////////////////////////////////
// add property panel multiline
void CImageProperties::AddPropertyPanelMultiline
(
	CMFCPropertyGridProperty* pGroup,
	CString csLabel, CString csValue, CString csDescription
)
{
	CPropertyGridMultilineText* pProp =
		new CPropertyGridMultilineText
		(
			csLabel, csValue, csDescription
		);
	pProp->SetDescription( csDescription );

	pGroup->AddSubItem( pProp );

} // AddPropertyPanelMultiline

/////////////////////////////////////////////////////////////////////////////
void CImageProperties::UpdatePropertyPane()
{
	CPropertiesWnd* pPanel = PropertiesPane;
	CMFCPropertyGridCtrl* pList = pPanel->PropList;

	// remove the existing contents
	pList->RemoveAll();
	pList->EnableHeaderCtrl( FALSE );
	pList->EnableDescriptionArea();
	pList->SetVSDotNetLook();
	pList->MarkModifiedProperties();

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	CPhotoExplorerDoc* pDoc = (CPhotoExplorerDoc*)pFrame->Document;
	if ( pDoc == nullptr )
	{
		return;
	}

	// the pathname we are browsing
	CString csPathname = pDoc->Folder;
	csPathname.TrimRight( L"\\" );
	const CString csFolder = CHelper::GetDataName( csPathname );

	struct ALBUM_DATA
	{
		CString csLabel;
		CString csValue;
		CString csDesc;
	};

	CString csTitle = pDoc->Title;
	CString csLocation = pDoc->Location;
	CString csComment = pDoc->Comment;
	CString csDate = pDoc->Date;
	CString csArtist = pDoc->Artist;
	CString csCopyright = pDoc->Copyright;
	CString csSoftware = pDoc->Software;
	CString csKeywords = pDoc->Keywords;

	vector<ALBUM_DATA> arrAlbum
	{
		{ L"Title", csTitle, L"The title of the album." },
		{ L"Location", csLocation, L"The location associated with the album." },
		{ L"Comment", csComment, L"A comment describing the album." },
		{ L"Date", csDate, L"The date associated with the album." },
		{ L"Artist", csArtist, L"The artist that created the album." },
		{ L"Copyright", csCopyright, L"The copyright owner of this work." },
		{ L"Software", csSoftware, L"The software used to create this work." },
		{ L"Keywords", csKeywords, L"Keywords separated by semicolons." },
	};

	COleDateTime oDT( COleDateTime::GetCurrentTime());
	CString csNow;
	csNow.Format
	( 
		L"%d:%d:%d %d:%d:%d", oDT.GetYear(), oDT.GetMonth(), oDT.GetDay(),
		oDT.GetHour(), oDT.GetMinute(), oDT.GetSecond()
	);

	if ( csDate.IsEmpty())
	{
		csDate = csNow;
	}

	vector<CString> arrGroups =
	{
		L"Album", L"Label", L"User", L"Image", L"Equipment", L"GPS", L"Camera", L"Thumbnail"
	};

	// create groups in the property panel 
	// in the order supplied by the array of groups
	for ( auto& csGroup : arrGroups )
	{
		const int nLen = csGroup.GetLength();

		CMFCPropertyGridProperty* pGroup = new CMFCPropertyGridProperty( csGroup );
		if ( csGroup == L"Album")
		{
			for ( auto& node : arrAlbum )
			{
				if ( node.csLabel == L"Comment" )
				{
					AddPropertyPanelMultiline( pGroup, node.csLabel, (_variant_t)csComment, node.csDesc );

				} else
				{
					CMFCPropertyGridProperty* pProp = nullptr;

					pProp = new CMFCPropertyGridProperty
					(
						node.csLabel, (_variant_t)node.csValue, node.csDesc
					);

					pGroup->AddSubItem( pProp );
				}
			}

			pList->AddProperty( pGroup );
			continue;
		}

		// loop through the properties
		for ( auto& node : m_Properties.Items )
		{
			shared_ptr<CImageProperty> pIP = node.second;
			const CString csPropertyGroup = pIP->PropertyGroup;
			if ( csGroup != csPropertyGroup )
			{
				continue;
			}

			CString csValue = pIP->UserInterfaceValue;
			const CString csDescription = pIP->PropertyDescription;
			const CString csLabel = pIP->PropertyLabel;

			const bool bMultiline = csLabel == L"3. Comment";

			//const bool bDate =
			//	csLabel == L"Digitize date" ||
			//	csLabel == L"Original date" ||
			//	csLabel == L"4. Date taken";

			if ( bMultiline )
			{
				AddPropertyPanelMultiline( pGroup, csLabel, csValue, csDescription );

			//} else if ( bDate && !csValue.IsEmpty() )
			//{
			//	AddPropertyPanelDate( pGroup, csLabel, csValue, csDescription );

			} else
			{
				CMFCPropertyGridProperty* pProp = nullptr;

				pProp = new CMFCPropertyGridProperty
				(
					csLabel, (_variant_t)csValue, csDescription
				);
				vector<CString> arrOptions = pIP->DropdownChoices;
				for ( auto& csOption : arrOptions )
				{
					pProp->AddOption( csOption );
				}

				pGroup->AddSubItem( pProp );
			}
		}

		pList->AddProperty( pGroup );
	}

	pPanel->AdjustLayout();
	pPanel->Invalidate();

} // UpdatePropertyPane

/////////////////////////////////////////////////////////////////////////////
// given an image pointer and an ASCII property ID, return the property value
CString CImageProperties::GetStringProperty( Gdiplus::Image* pImage, PROPID id )
{
	CString value;

	// get the size of the date property
	const UINT uiSize = pImage->GetPropertyItemSize( id );

	// if the property exists, it will have a non-zero size 
	if ( uiSize > 0 )
	{
		// using a smart pointer which will release itself
		// when it goes out of context
		unique_ptr<Gdiplus::PropertyItem> pItem = 
			unique_ptr<Gdiplus::PropertyItem>
			(
				(Gdiplus::PropertyItem*)malloc( uiSize )
			);

		// Get the property item.
		pImage->GetPropertyItem( id, uiSize, pItem.get() );

		// the property should be ASCII
		if ( pItem->type == PropertyTagTypeASCII )
		{
			value = (LPCSTR)pItem->value;
		}
	}

	return value;
} // GetStringProperty

/////////////////////////////////////////////////////////////////////////////
// get the current date taken, if any, from the given filename
CString CImageProperties::GetCurrentDateTaken( LPCTSTR lpszPathName )
{
	//USES_CONVERSION;

	CString value;

	// smart pointer to the image representing this file
	unique_ptr<Gdiplus::Image> pImage = unique_ptr<Gdiplus::Image>
	(
		//Gdiplus::Image::FromFile( T2CW( lpszPathName ) )
		Gdiplus::Image::FromFile( lpszPathName ) 
	);

	// test the date properties stored in the given image
	CString csOriginal =
		GetStringProperty( pImage.get(), PropertyTagExifDTOrig );
	CString csDigitized =
		GetStringProperty( pImage.get(), PropertyTagExifDTDigitized );

	// officially the original property is the date taken in this
	// format: "YYYY:MM:DD HH:MM:SS"
	m_Date.DateTaken = csOriginal;
	if ( m_Date.Okay )
	{
		value = csOriginal;

	} else // alternately use the date digitized
	{
		m_Date.DateTaken = csDigitized;
		if ( m_Date.Okay )
		{
			value = csDigitized;
		}
	}

	return value;
} // GetCurrentDateTaken

/////////////////////////////////////////////////////////////////////////////
// set the current file extension which will automatically lookup the
// related mime type and class ID and set their respective properties
void CImageProperties::CExtension::SetFileExtension( CString value )
{
	m_csFileExtension = value.MakeLower();

	if ( m_mapExtensions.Exists[ value ] )
	{
		MimeType = *m_mapExtensions.find( value );

		// populate the mime type map the first time it is referenced
		if ( m_mapMimeTypes.Count == 0 )
		{
			UINT num = 0;
			UINT size = 0;

			// gets the number of available image encoders and 
			// the total size of the array
			Gdiplus::GetImageEncodersSize( &num, &size );
			if ( size == 0 )
			{
				return;
			}

			Gdiplus::ImageCodecInfo* pImageCodecInfo =
				(Gdiplus::ImageCodecInfo*)malloc( size );
			if ( pImageCodecInfo == nullptr )
			{
				return;
			}

			// Returns an array of ImageCodecInfo objects that contain 
			// information about the image encoders built into GDI+.
			Gdiplus::GetImageEncoders( num, size, pImageCodecInfo );

			// populate the map of mime types the first time it is 
			// needed
			for ( UINT nIndex = 0; nIndex < num; ++nIndex )
			{
				CString csKey;
				//csKey = CW2A( pImageCodecInfo[ nIndex ].MimeType );
				csKey = pImageCodecInfo[ nIndex ].MimeType;
				CLSID classID = pImageCodecInfo[ nIndex ].Clsid;
				m_mapMimeTypes.add( csKey, shared_ptr<CLSID>( new CLSID( classID ) ) );
			}

			// clean up
			free( pImageCodecInfo );
		}

		ClassID = *m_mapMimeTypes.find( MimeType );

	} else
	{
		MimeType = _T( "" );
	}
} // CImageProperties::CExtension::SetFileExtension

/////////////////////////////////////////////////////////////////////////////
// Save the data inside pImage to the given filename
bool CImageProperties::Save( LPCTSTR lpszPathName, Gdiplus::Image* pImage )
{
	USES_CONVERSION;

	// save and overwrite the selected image file with current page
	int iValue =
		EncoderValue::EncoderValueVersionGif89 |
		EncoderValue::EncoderValueCompressionLZW |
		EncoderValue::EncoderValueFlush;

	Gdiplus::EncoderParameters param;
	param.Count = 1;
	param.Parameter[ 0 ].Guid = Gdiplus::EncoderSaveFlag;
	param.Parameter[ 0 ].Value = &iValue;
	param.Parameter[ 0 ].Type = EncoderParameterValueTypeLong;
	param.Parameter[ 0 ].NumberOfValues = 1;

	// filename plus extension
	const CString csData = CHelper::GetDataName( lpszPathName );

	// use the extension member class to get the class ID of the file
	CLSID clsid = m_Extension.ClassID;

	//Status status = pImage->Save( T2CW( csPath ), &clsid, &param );
	Status status = pImage->Save( lpszPathName, &clsid, &param );

	// return true if the save worked
	return status == Ok;
} // Save

/////////////////////////////////////////////////////////////////////////////

