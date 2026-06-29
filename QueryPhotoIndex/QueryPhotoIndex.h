/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "resource.h"
#include "PhotoIndexBuilder.h"

// we need to link to the GDI+ library
#pragma comment(lib, "gdiplus.lib")

	/////////////////////////////////////////////////////////////////////////////
	// QueryPhotoIndex
	//
	// A lightweight console application that loads the binary photo index
	// (image table + inverted index) and executes text queries against it.
	//
	// Architecture:
	//   • All indexing logic (tokenization, inverted index construction,
	//     metadata extraction, XPComment parsing, path normalization) is
	//     performed by the Common library.
	//   • QueryPhotoIndex simply loads the prebuilt index, evaluates queries,
	//     groups results by album/folder, and prints them to the console.
	//
	// Responsibilities:
	//   • Initialize GDI+
	//   • Load the binary index via CPhotoIndexBuilder::LoadBinaryIndex()
	//   • Parse command-line query text
	//   • Tokenize and normalize query terms
	//   • Perform AND/OR evaluation using the inverted index
	//   • Collect matching image paths into m_queryResults
	//   • Optionally group results by folder (album)
	//   • Print results to console
	//
	// This file intentionally contains no indexing logic. All search,
	// tokenization, and metadata handling is delegated to the Common library.
	/////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////
	// m_gdiplusToken
	//
	// Global GDI+ startup token used by GdiplusStartup() and GdiplusShutdown().
	// Required because QueryPhotoIndex may load images to display metadata
	// (dimensions, DPI, XPComment) or to group results by album.
	/////////////////////////////////////////////////////////////////////////////
	ULONG_PTR m_gdiplusToken;

	/////////////////////////////////////////////////////////////////////////////
	// MAP_IMAGES
	//
	// Maps image filename → GDI+ Image object.
	// Used when QueryPhotoIndex needs to load thumbnails or inspect metadata.
	//
	// MAP_ALBUM
	//
	// Maps album/folder name → MAP_IMAGES.
	// Allows grouping query results by folder.
	//
	// MAP_INDEX
	//
	// Legacy mapping of image filename → comment string.
	// No longer used now that the Common library provides LoadedIndex.
	/////////////////////////////////////////////////////////////////////////////
	typedef CKeyedCollection<CString, Image> MAP_IMAGES;
	typedef CKeyedCollection<CString, MAP_IMAGES> MAP_ALBUM;
	typedef CKeyedCollection<CString, CString> MAP_INDEX;

	/////////////////////////////////////////////////////////////////////////////
	// m_loadedIndex
	//
	// The fully loaded search index produced by BuildPhotoIndex:
	//
	//   • imageTable     → vector of ImageRecord entries
	//   • invertedIndex  → map<token, vector<imageIDs>>
	//
	// Loaded via CPhotoIndexBuilder::LoadBinaryIndex().
	//
	// This structure contains everything needed to evaluate queries without
	// touching the file system or re-reading image metadata.
	/////////////////////////////////////////////////////////////////////////////
	CPhotoIndexBuilder::LoadedIndex m_loadedIndex;

	/////////////////////////////////////////////////////////////////////////////
	// m_queryResults
	//
	// Vector of absolute or relative image paths that matched the user’s query.
	// Populated after evaluating tokens against the inverted index.
	//
	// Order is typically sorted by folder, then filename.
	/////////////////////////////////////////////////////////////////////////////
	std::vector<CString> m_queryResults;

	/////////////////////////////////////////////////////////////////////////////
	// m_mapAlbums
	//
	// Maps folder/album name → collection of images in that folder.
	// Used to group query results by album for cleaner console output.
	//
	// Example:
	//   "Family" → { IMG_001.jpg, IMG_002.jpg, ... }
	//   "Vacations" → { IMG_100.jpg, IMG_101.jpg, ... }
	/////////////////////////////////////////////////////////////////////////////
	MAP_ALBUM m_mapAlbums;

	/////////////////////////////////////////////////////////////////////////////
	// m_keyFolders
	//
	// Sorted list of folder names used to control output order when printing
	// grouped results.
	//
	// Example:
	//   ["Family", "Vacations", "Work", "Misc"]
	/////////////////////////////////////////////////////////////////////////////
	CKeyedCollection<CString, int> m_keyFolders;

	/////////////////////////////////////////////////////////////////////////////
	// m_csQuery
	//
	// Raw query string entered by the user on the command line.
	//
	// Examples:
	//   "mary beth"
	//   "liam | katherine"
	//   "birthday 2020"
	//
	// Tokenization and normalization are performed by CHelper::Tokenize()
	// and CHelper::NormalizeToken().
	/////////////////////////////////////////////////////////////////////////////
	CString m_csQuery;

	/////////////////////////////////////////////////////////////////////////////
	// m_csWorkingFolder
	//
	// Base folder used to resolve relative paths in the loaded index.
	// Typically the same folder passed to BuildPhotoIndex.
	//
	// Example:
	//   "C:\Users\wtblo\LocalDocs\Photographs"
	/////////////////////////////////////////////////////////////////////////////
	CString m_csWorkingFolder;

	/////////////////////////////////////////////////////////////////////////////
	// m_nImages
	//
	// Total number of images in the loaded index.
	// Useful for reporting statistics or validating query results.
	/////////////////////////////////////////////////////////////////////////////
	UINT m_nImages;

	/////////////////////////////////////////////////////////////////////////////

