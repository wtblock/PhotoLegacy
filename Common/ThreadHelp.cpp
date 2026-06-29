/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "pch.h"
#include "ThreadHelp.h"

/////////////////////////////////////////////////////////////////////////////
// Register the custom quit-message forwarding message
/////////////////////////////////////////////////////////////////////////////
UINT ThreadHelp::QUEUE_QUIT_MESSAGE =
::RegisterWindowMessage(_T("QUEUE_QUIT_MESSAGE"));

/////////////////////////////////////////////////////////////////////////////
// OnQueueQuitMessage
//
// Receives a deferred quit/close message (MSG*) and pushes it into the
// internal queue for later replay during OnIdle().
/////////////////////////////////////////////////////////////////////////////
LRESULT ThreadHelp::OnQueueQuitMessage(WPARAM wp, LPARAM lp)
{
	quitMessageQueue.push((MSG*)lp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// TransferQuitMessages
//
// Called from the application's OnIdle() handler.
//
// Replays all queued WM_CLOSE/WM_QUIT messages back to their original
// windows using PostMessage(). Frees the MSG structures after posting.
//
// This ensures that quit messages are only processed when the app is idle,
// not while worker threads or long operations are running.
/////////////////////////////////////////////////////////////////////////////
void ThreadHelp::TransferQuitMessages()
{
	while (!quitMessageQueue.empty())
	{
		MSG* msg = quitMessageQueue.front();
		quitMessageQueue.pop();

		PostMessage(msg->hwnd, msg->message, msg->wParam, msg->lParam);
		LocalFree(msg);
	}
}

/////////////////////////////////////////////////////////////////////////////
// UpdateUI
//
// Pumps UI messages while intercepting WM_CLOSE/WM_QUIT/SC_CLOSE.
//
// If a quit/close message is detected:
//   • Remove it from the queue
//   • Forward it to the main window via QUEUE_QUIT_MESSAGE
//   • Store the MSG* for deferred processing
//
// Otherwise:
//   • Call AfxPumpMessage() to process normal UI messages
//
// This keeps the UI responsive and prevents premature shutdown during
// long-running operations or thread waits.
/////////////////////////////////////////////////////////////////////////////
void ThreadHelp::UpdateUI()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (msg.message == WM_CLOSE ||
			msg.message == WM_QUIT ||
			(msg.message == WM_SYSCOMMAND &&
				(msg.wParam & 0xfff0) == SC_CLOSE))
		{
			MSG* pMsgToQueue = (MSG*)LocalAlloc(LMEM_FIXED, sizeof(MSG));
			PeekMessage(pMsgToQueue, NULL, 0, 0, PM_REMOVE);

			CWnd* pWnd = AfxGetMainWnd();
			if (pWnd && ::IsWindow(pWnd->m_hWnd))
			{
				pWnd->PostMessage(
					QUEUE_QUIT_MESSAGE,
					0,
					(LPARAM)pMsgToQueue);
			}
		}
		else
		{
			AfxPumpMessage();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// WaitForCWinThread
//
// Waits for a worker CWinThread to finish while continuing to pump UI
// messages via UpdateUI().
//
// Uses MsgWaitForMultipleObjects to wait on both:
//   • The worker thread handle
//   • The UI message queue
//
// When the thread exits:
//   • Retrieves its exit code
//   • Deletes the CWinThread object
//   • Returns the exit code
//
// This prevents UI freeze and ensures quit messages are handled safely.
/////////////////////////////////////////////////////////////////////////////
UINT ThreadHelp::WaitForCWinThread(CWinThread* pThread)
{
	HANDLE hThread = *pThread;

	for (;;)
	{
		DWORD dwWaitResult =
			MsgWaitForMultipleObjects(
				1, &hThread, FALSE, INFINITE, QS_ALLINPUT);

		if (dwWaitResult == WAIT_OBJECT_0)
		{
			DWORD retval;
			GetExitCodeThread(hThread, &retval);
			delete pThread;
			return retval;
		}
		else
		{
			UpdateUI();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
