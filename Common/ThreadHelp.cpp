/////////////////////////////////////////////////////////////////////////////
// Copyright © by W. T. Block, all rights reserved
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "pch.h"
#include "ThreadHelp.h"

/////////////////////////////////////////////////////////////////////////////
UINT ThreadHelp::QUEUE_QUIT_MESSAGE = 
	::RegisterWindowMessage(_T( "QUEUE_QUIT_MESSAGE" ));

/////////////////////////////////////////////////////////////////////////////
LRESULT ThreadHelp::OnQueueQuitMessage(WPARAM wp, LPARAM lp)
{
	quitMessageQueue.push( (MSG*)lp );

	return 0;
} // OnQueueQuitMessage

/////////////////////////////////////////////////////////////////////////////
void ThreadHelp::TransferQuitMessages()
{	while ( !quitMessageQueue.empty() )
	{	MSG* msg = quitMessageQueue.front();
		quitMessageQueue.pop();
		PostMessage( msg->hwnd, msg->message, msg->wParam, msg->lParam );
		LocalFree( msg );
	}
} // TransferQuitMessages

/////////////////////////////////////////////////////////////////////////////
void ThreadHelp::UpdateUI()
{	MSG msg;
	while ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{	if 
		(	msg.message == WM_CLOSE || 
			msg.message == WM_QUIT || 
				msg.message == WM_SYSCOMMAND && 
				( msg.wParam & 0xfff0 ) == SC_CLOSE 
		)
		{	MSG* pMsgToQueue = (MSG*)LocalAlloc( LMEM_FIXED, sizeof( MSG ));
			PeekMessage( pMsgToQueue, NULL, 0, 0, PM_REMOVE );
			CWnd* pWnd = AfxGetMainWnd();
			if ( pWnd && ::IsWindow( pWnd->m_hWnd ))
			{	pWnd->PostMessage
				(	QUEUE_QUIT_MESSAGE, 0, (LPARAM)pMsgToQueue 
				);
			}
		}
		else
		{	AfxPumpMessage();
		}
	}
} // UpdateUI

/////////////////////////////////////////////////////////////////////////////
UINT ThreadHelp::WaitForCWinThread( CWinThread* pThread )
{	HANDLE hThread = *pThread;
	for ( ; ; )
	{	DWORD dwWaitResult = 
			MsgWaitForMultipleObjects
			(	1, &hThread, FALSE, INFINITE, QS_ALLINPUT 
			);
		if ( dwWaitResult == WAIT_OBJECT_0 )
		{	DWORD retval;
			GetExitCodeThread( hThread, &retval );
			delete pThread;
			return retval;
		}
		else
		{	UpdateUI();
		}
	}
} // WaitForCWinThread

/////////////////////////////////////////////////////////////////////////////
