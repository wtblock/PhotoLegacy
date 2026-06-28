/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <queue>

/***********************************************************************

At the app level do the following to implement ThreadHelp:

Add an OnIdle override to the app class declaration:
	virtual BOOL OnIdle(LONG lCount);

Add to the app class implementation:
	BOOL CPlotStudioApp::OnIdle( LONG lCount )
	{	BOOL bMore = CWinAppEx::OnIdle( lCount );
		if ( !bMore )
		{	( (CMainFrame*)m_pMainWnd )->TransferQuitMessages();
		}
		return bMore;
	}

Add as a contributor base class to the main window class
	class CMainFrame : public CMDIFrameWndEx, public ThreadHelp

Add to the main window class declaration:
	afx_msg LRESULT OnQueueQuitMessage(WPARAM wParam, LPARAM lParam);

Add to the main window class message map:
	ON_REGISTERED_MESSAGE(QUEUE_QUIT_MESSAGE, OnQueueQuitMessage)

Add to the main window class member functions:
	LRESULT CMainFrame::OnQueueQuitMessage(WPARAM wp, LPARAM lp)
	{	return ThreadHelp::OnQueueQuitMessage( wp, lp );
	}

For all other contributory DLLs or OCXs, only call the static functions
	ThreadHelp::UpdateUI();
	UINT ThreadHelp::WaitForCWinThread( CWinThread* pThread );

***********************************************************************/

class ThreadHelp
{
public:
	LRESULT OnQueueQuitMessage( WPARAM wParam, LPARAM lParam );

	void TransferQuitMessages();

	// Pump messages while diverting quit messages for app idle processing.
	// Quit messages are routed to a queue maintained by the app main window.
	// The app main window
	static void UpdateUI();

	// Wait for a worker thread from the main UI thread, while continuing
	// to pump messages.
	static UINT WaitForCWinThread( CWinThread* pThread );

	static UINT QUEUE_QUIT_MESSAGE;

private:
	std::queue<MSG*> quitMessageQueue;
};
