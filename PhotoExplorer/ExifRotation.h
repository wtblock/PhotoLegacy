/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CExifRotation
//
// Utility class that interprets and applies EXIF orientation codes to
// GDI+ Image and Bitmap objects. Many cameras and phones store rotation
// information in the EXIF metadata instead of physically rotating the
// pixel data. This class reads that orientation code and applies the
// correct transformation so images display upright in Photo Explorer.
//
// Purpose:
//   • Decode EXIF orientation values (1–8) into meaningful rotation and
//     mirroring operations.
//   • Provide a simple interface for applying the correct transformation
//     to GDI+ Image or Bitmap objects.
//   • Ensure that images appear with the correct orientation regardless
//     of how they were captured or stored.
//
// Why this class exists:
//   Modern devices often store images “sideways” and rely on EXIF metadata
//   to indicate how the viewer should rotate them. Without honoring these
//   orientation codes, images would appear rotated 90°, 180°, or mirrored.
//   CExifRotation centralizes this logic so Photo Explorer always displays
//   images correctly.
//
// Responsibilities:
//   • Store the EXIF orientation code (Rotation / ExifOrientation).
//   • Provide a human-readable description (Description property).
//   • Apply the correct rotation/mirroring to:
//       – Gdiplus::Image
//       – Gdiplus::Bitmap
//   • Map EXIF codes to GDI+ RotateFlipType operations.
//
// Supported EXIF Orientation Codes:
//   1 : Normal (no rotation)
//   2 : Mirror horizontal
//   3 : Rotate 180 degrees
//   4 : Mirror vertical
//   5 : Mirror horizontal + rotate 270° CW
//   6 : Rotate 90° CW
//   7 : Mirror horizontal + rotate 90° CW
//   8 : Rotate 270° CW
//
// Interaction with other components:
//   • Used by image-loading routines in Photo Explorer to correct
//     orientation immediately after loading a JPEG.
//   • Ensures thumbnails and full-size previews match the intended
//     orientation stored in EXIF metadata.
//   • Helps maintain consistency between Photo Explorer, Photo Printer,
//     and external viewers.
//
// This class provides a clean, reliable way to interpret EXIF orientation
// metadata and ensures that all images display correctly throughout the
// application.
/////////////////////////////////////////////////////////////////////////////
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

