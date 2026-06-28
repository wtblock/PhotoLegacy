/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "pch.h"
#include "ExifRotation.h"
#include "CHelper.h"
#include <gdiplus.h>

/////////////////////////////////////////////////////////////////////////////
CString CExifRotation::GetDescription()
{
	CString value;
	switch ( m_eRotation )
	{
		case eRotateNormal: value = L"Horizontal( normal )"; break; 
		case eRotateMirrorX: value = L"Mirror horizontal"; break;
		case eRotateRotate180: value = L"Rotate 180 degrees"; break;
		case eRotateMirrorY: value = L"Mirror vertical"; break;
		case eRotateMirrorXRotate270: value = L"Mirror horizontal and rotate 270 CW"; break;
		case eRotateRotate90: value = L"Rotate 90 CW"; break;
		case eRotateMirrorXRotate90: value = L"Mirror horizontal and rotate 90 CW"; break;
		case eRotateRotate270: value = L"Rotate 270 CW"; break;
	}

	Description = value;

	return value;
} // GetDescription

/////////////////////////////////////////////////////////////////////////////
// rotate the given image based on the orientation
void CExifRotation::ApplyOrientation( Gdiplus::Bitmap* image )
{
	const ExifRotations orientation = Rotation;
	switch ( orientation )
	{
		case eRotateNormal:
			image->RotateFlip( Gdiplus::RotateNoneFlipNone );
			break;
		case eRotateRotate180:
			image->RotateFlip( Gdiplus::Rotate180FlipNone );
			break;
		case eRotateRotate90:
			image->RotateFlip( Gdiplus::Rotate90FlipNone );
			break;
		case eRotateRotate270:
			image->RotateFlip( Gdiplus::Rotate270FlipNone );
			break;
		default:
			image->RotateFlip( Gdiplus::RotateNoneFlipNone );
			break;
	}
} // ApplyOrientation

/////////////////////////////////////////////////////////////////////////////
// rotate the given image based on the orientation
void CExifRotation::ApplyOrientation( Gdiplus::Image* image )
{
	const ExifRotations orientation = Rotation;
	switch ( orientation )
	{
		case eRotateNormal:
			image->RotateFlip( Gdiplus::RotateNoneFlipNone );
			break;
		case eRotateRotate180:
			image->RotateFlip( Gdiplus::Rotate180FlipNone );
			break;
		case eRotateRotate90:
			image->RotateFlip( Gdiplus::Rotate90FlipNone );
			break;
		case eRotateRotate270:
			image->RotateFlip( Gdiplus::Rotate270FlipNone );
			break;
		default:
			image->RotateFlip( Gdiplus::RotateNoneFlipNone );
			break;
	}
} // ApplyOrientation

/////////////////////////////////////////////////////////////////////////////
