/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// Here's a quick rundown of EXIF orientation codes:
//
//	1 : Horizontal( normal )
//	2 : Mirror horizontal
//	3 : Rotate 180 degrees
//	4 : Mirror vertical
//	5 : Mirror horizontal and rotate 270 CW
//	6 : Rotate 90 CW
//	7 : Mirror horizontal and rotate 90 CW
//	8 : Rotate 270 CW
//
class CExifRotation
{
public:
	enum ExifRotations
	{
		eRotateNormal = 1,
		eRotateMirrorX = 2,
		eRotateRotate180 = 3,
		eRotateMirrorY = 4,
		eRotateMirrorXRotate270 = 5,
		eRotateRotate90 = 6,
		eRotateMirrorXRotate90 = 7,
		eRotateRotate270 = 8
	};

// protected data
protected:
	ExifRotations m_eRotation;
	CString m_csDescription;

// public properties
public:
	// rotation state
	inline ExifRotations GetRotation()
	{
		return m_eRotation;
	}
	// rotation state
	inline void SetRotation( ExifRotations value )
	{
		m_eRotation = value;
	}
	// rotation state
	__declspec( property( get = GetRotation, put = SetRotation ))
		ExifRotations Rotation;

	// Exif Orientation Code is stored as an unsigned short
	inline USHORT GetExifOrientation()
	{
		const USHORT value( m_eRotation );
		return value;
	}
	// Exif Orientation Code is stored as an unsigned short
	inline void SetExifOrientation( USHORT value )
	{
		m_eRotation = ExifRotations( value );
	}
	// Exif Orientation Code is stored as an unsigned short
	__declspec( property( get = GetExifOrientation, put = SetExifOrientation ))
		USHORT ExifOrientation;

	// rotation description
	CString GetDescription();
	// rotation description
	inline void SetDescription( CString value )
	{
		m_csDescription = value;
	}
	// rotation description
	__declspec( property( get = GetDescription, put = SetDescription ))
		CString Description;

// public methods
public:
	// rotate the given image based on the orientation
	void ApplyOrientation( Gdiplus::Image* image );

	// rotate the given image based on the orientation
	void ApplyOrientation( Gdiplus::Bitmap* image );

// protected overrides
protected:

// public overrides
public:

// public constructor/destructor
public:
	CExifRotation()
	{
		m_eRotation = eRotateNormal;
	}
	~CExifRotation()
	{
	}
};

