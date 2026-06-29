/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <queue>

/////////////////////////////////////////////////////////////////////////////
// ThreadHelp
//
// Utility class that provides safe UI message pumping during long-running
// operations and controlled handling of quit/close messages.
//
// Purpose:
//   • Keep the UI responsive while worker threads run
//   • Prevent WM_QUIT / WM_CLOSE from terminating the app prematurely
//   • Queue quit messages and replay them only when the app is idle
//   • Provide a safe MsgWaitForMultipleObjects loop for CWinThread waits
//
// Usage:
//   1. The main frame window derives from ThreadHelp
//   2. The app overrides OnIdle() and calls TransferQuitMessages()
//   3. ThreadHelp::UpdateUI() is called from worker loops to pump messages
//   4. ThreadHelp::WaitForCWinThread() waits for worker threads safely
//
// This class is used by PlotStudio and PhotoPrinter to ensure that
// progress dialogs, long-running operations, and background threads
// do not freeze the UI or cause premature shutdown.
/////////////////////////////////////////////////////////////////////////////
class ThreadHelp
{
public:
	///////////////////////////////////////////////////////////////////////////
	// OnQueueQuitMessage
	//
	// Handler for QUEUE_QUIT_MESSAGE. Receives a pointer to a MSG structure
	// containing a WM_CLOSE/WM_QUIT message that was intercepted by UpdateUI().
	//
	// The message is pushed into quitMessageQueue for later replay.
	///////////////////////////////////////////////////////////////////////////
	LRESULT OnQueueQuitMessage(WPARAM wParam, LPARAM lParam);

	///////////////////////////////////////////////////////////////////////////
	// TransferQuitMessages
	//
	// Called from the app's OnIdle() handler. Replays all queued quit/close
	// messages back to their original windows using PostMessage().
	//
	// This ensures that WM_QUIT/WM_CLOSE messages are only processed when
	// the application is idle and not in the middle of a worker-thread wait.
	///////////////////////////////////////////////////////////////////////////
	void TransferQuitMessages();

	///////////////////////////////////////////////////////////////////////////
	// UpdateUI
	//
	// Pumps UI messages while diverting WM_CLOSE/WM_QUIT messages into the
	// application's quit queue. This keeps the UI responsive during long
	// operations and prevents premature shutdown.
	//
	// Behavior:
	//   • If the next message is WM_CLOSE/WM_QUIT/SC_CLOSE:
	//         - Remove it from the queue
	//         - Forward it to the main window via QUEUE_QUIT_MESSAGE
	//   • Otherwise:
	//         - Call AfxPumpMessage() to process normal UI messages
	///////////////////////////////////////////////////////////////////////////
	static void UpdateUI();

	///////////////////////////////////////////////////////////////////////////
	// WaitForCWinThread
	//
	// Waits for a worker CWinThread to finish while continuing to pump UI
	// messages. Prevents UI freeze and ensures quit messages are handled
	// safely via UpdateUI().
	//
	// Returns the thread's exit code.
	///////////////////////////////////////////////////////////////////////////
	static UINT WaitForCWinThread(CWinThread* pThread);

	///////////////////////////////////////////////////////////////////////////
	// QUEUE_QUIT_MESSAGE
	//
	// Registered Windows message used to forward intercepted quit/close
	// messages to the main window for deferred processing.
	///////////////////////////////////////////////////////////////////////////
	static UINT QUEUE_QUIT_MESSAGE;

private:
	///////////////////////////////////////////////////////////////////////////
	// quitMessageQueue
	//
	// Queue of MSG* structures containing WM_CLOSE/WM_QUIT messages that
	// were intercepted by UpdateUI() and deferred until OnIdle().
	///////////////////////////////////////////////////////////////////////////
	std::queue<MSG*> quitMessageQueue;
};
