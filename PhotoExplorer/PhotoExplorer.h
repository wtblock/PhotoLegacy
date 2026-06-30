/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

using namespace Gdiplus;

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

/////////////////////////////////////////////////////////////////////////////
// Photo Explorer — Architectural Overview
//
// Photo Explorer is a full-featured MFC document/view application designed
// to browse, inspect, and modify metadata for large collections of family
// photographs. The application was originally generated using the MFC
// Application Wizard with nearly all advanced UI features enabled, and has
// since evolved into a highly customized, multi-pane, multi-document
// metadata exploration tool.
//
// Core Design:
//   • Multi-document MDI application (CMDIFrameWndEx)
//   • Document/View architecture (CPhotoExplorerDoc / CPhotoExplorerView)
//   • XML-based album files (*.album) representing collections of images
//   • Rich, Visual Studio–style docking interface
//   • Thumbnail list + full-image preview split view
//   • Multiple auxiliary panes for navigation, metadata, and diagnostics
//
// Major UI Components:
//   1. Folder Explorer Pane
//      - Displays the hierarchical album folder structure.
//      - Allows navigation by year, event, or custom grouping.
//      - Selecting a folder loads its images into the active document.
//
//   2. Calendar Pane (Wizard-generated; currently unused)
//      - Originally intended for date-based navigation.
//      - Remains available for future enhancements.
//
//   3. Output Window (three tabs)
//      - Progress: shows background operations such as thumbnail generation.
//      - Warnings: metadata inconsistencies, missing fields, etc.
//      - Errors: file access issues, invalid metadata, or parsing failures.
//
//   4. Properties Pane
//      - Displays and edits metadata for the selected image.
//      - Uses a custom property grid (CPropertyGridCtrl).
//      - Supports multi-line editing via CPropertyGridMultilineText.
//      - Synchronizes changes with the active document.
//
//   5. Split View (CPhotoExplorerView)
//      - Left: Thumbnail list box showing all images in the selected folder.
//      - Right: Full-size image preview with metadata overlay.
//      - Selecting a thumbnail updates the preview and the Properties pane.
//
// Document Architecture:
//   • Each open album is represented by a CPhotoExplorerDoc instance.
//   • Album files are XML-based and contain:
//       – Folder path
//       – Image list
//       – Metadata fields (Title, Location, Content, Date Taken, etc.)
//   • Documents are independent; multiple albums can be open simultaneously.
//   • MDI tabs allow quick switching between albums.
//
// View Architecture:
//   • The view is split into two functional regions:
//       – Thumbnail region (owner-drawn list box)
//       – Image preview region (GDI+ rendering)
//   • The view communicates with:
//       – Properties pane (metadata updates)
//       – Output window (diagnostics)
//       – Document (image list, metadata, file paths)
//
// Wizard-Generated Infrastructure:
//   • Docking manager and auto-hide panes
//   • Menu bar, toolbars, status bar
//   • MDI tabbed groups
//   • Customizable toolbars
//   • Visual styles (Office 2003, VS2005, Office 2007, Windows 7)
//   • Window manager dialog
//   • User toolbar support
//
// Application-Specific Enhancements:
//   • Custom metadata property grid with multi-line editing
//   • Thumbnail generation and caching
//   • Full-image rendering with EXIF metadata extraction
//   • XML album loading/saving
//   • Folder-based navigation with date sorting
//   • Output window integration for progress and diagnostics
//   • Rich metadata editing pipeline (UI → PropertyGrid → Document → View)
//
// Workflow Summary:
//   1. User selects a folder in the Folder Explorer.
//   2. Document loads all images and metadata from disk.
//   3. Thumbnail list is populated.
//   4. User selects a thumbnail.
//   5. Full-size preview is rendered.
//   6. Properties pane displays editable metadata.
//   7. User edits metadata; changes propagate to the document.
//   8. Document marks itself modified and updates the view.
//   9. User saves the album as an XML *.album file.
//
// Photo Explorer provides a powerful, intuitive interface for navigating,
// inspecting, and editing large collections of family photographs, combining
// the flexibility of MFC’s docking architecture with custom metadata and
// rendering logic tailored specifically for photographic archives.
/////////////////////////////////////////////////////////////////////////////
class CPhotoExplorerApp : public CWinAppEx
{

// public data
public:
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

// protected methods
protected:
	// Gdiplus initialization
	GdiplusStartupInput m_gdiplusStartupInput;
	ULONG_PTR m_gdiplusToken;

// public methods
public:
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

// protected overrides
protected:

// public overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle( LONG lCount );
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

// public constructor/destructor
public:
	CPhotoExplorerApp() noexcept;

};

/////////////////////////////////////////////////////////////////////////////
extern CPhotoExplorerApp theApp;

/////////////////////////////////////////////////////////////////////////////
