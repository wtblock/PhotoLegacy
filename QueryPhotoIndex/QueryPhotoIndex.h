/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "resource.h"
#include "CHelper.h"
#include "PhotoIndexBuilder.h"

// we need to link to the GDI+ library
#pragma comment(lib, "gdiplus.lib")

/////////////////////////////////////////////////////////////////////////////
// used for gdiplus library
ULONG_PTR m_gdiplusToken;

	typedef CKeyedCollection<CString, Image> MAP_IMAGES;
	typedef CKeyedCollection<CString, MAP_IMAGES> MAP_ALBUM;
	typedef CKeyedCollection<CString, CString> MAP_INDEX;

	// New: loaded search index (image table + inverted index)
	CPhotoIndexBuilder::LoadedIndex m_loadedIndex;

	//// collection of image names and their corresponding comments
	//MAP_INDEX m_mapIndex;
	std::vector<CString> m_queryResults;

	// collection of albums (folders) where each album is a collection
	// of bitmaps
	MAP_ALBUM m_mapAlbums;

	// sorted folders 
	CKeyedCollection< CString, int > m_keyFolders;

	// query
	CString m_csQuery;

	// working folder
	CString m_csWorkingFolder;

	// number of images in the document
	UINT m_nImages;

