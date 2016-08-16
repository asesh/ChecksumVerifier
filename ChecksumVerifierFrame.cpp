// ChecksumVerifierFrame.cpp: 
// Copyright (C) 2010-2011 Asesh Shrestha. All rights reserved

#include "StdAfx.h"
#include "MainFrame.h"
#include "ChecksumVerifier.h"
#include "ChecksumVerifierFrame.h"

//---------------------------------------------------------------------------
// Name: CChecksumVerifierFrame
// Desc: Default constructor
//---------------------------------------------------------------------------
CChecksumVerifierFrame::CChecksumVerifierFrame()
{
	m_hDlg = NULL;
	m_hCurrDirEditWnd = m_hCurrFileEditWnd = m_hStatusEditWnd = NULL;
	m_hSingleProgressbarWnd = m_hFullProgressbarWnd = NULL;
	m_hThreadPriorityComboWnd = NULL;

	m_hChecksumThread = NULL; // Checksum thread handle
}

//---------------------------------------------------------------------------
// Name: CChecksumVerifierFrame
// Desc: Destructor
//---------------------------------------------------------------------------
CChecksumVerifierFrame::~CChecksumVerifierFrame()
{

}

//---------------------------------------------------------------------------
// Name: onInitDialog
// Desc: Handle WM_INITDIALOG message
//---------------------------------------------------------------------------
 void CChecksumVerifierFrame::onInitDialog(HWND hDlg, WPARAM wpParam, LPARAM lpParam)
 {
	m_hDlg = hDlg; // Assign the main window handle

	// Thread priorities
	 WCHAR *pszThreadPriorities[] = 
	 {
		 L"Highest", L"Normal", L"Idle", L"Lowest", 
	 };

	// Retrieve the handles to the child controls
	m_hPauseButtonWnd = ::GetDlgItem(hDlg, IDB_PAUSE_BUTTON); // Pause button
	m_hCancelButtonWnd = ::GetDlgItem(hDlg, IDB_CANCEL_BUTTON); // Cancel button
	m_hCurrDirEditWnd = ::GetDlgItem(m_hDlg, IDC_SOURCE_DIRECTORY_EDIT); // Current directory edit control
	m_hCurrFileEditWnd = ::GetDlgItem(m_hDlg, IDC_FILE_NAME_PROGRESS_EDIT); // Current file edit control
	m_hHashEditWnd = ::GetDlgItem(m_hDlg, IDC_CHECKSUM_HASH_EDIT); // Hash edit control
	m_hStatusEditWnd = ::GetDlgItem(m_hDlg, IDC_STATUS_EDIT); // Status edit controls
	m_hSingleProgressbarWnd = ::GetDlgItem(m_hDlg, IDC_SINGLE_PROGRESSBAR); // Progressbar that shows a single file processing progress
	m_hFullProgressbarWnd = ::GetDlgItem(m_hDlg, IDC_FULL_PROGRESSBAR); // Progressbar that shows the overall file processing progress
	m_hThreadPriorityComboWnd = ::GetDlgItem(m_hDlg, IDC_THREAD_PRIORITY_COMBO); // Thread priority combo box

	 // Set the thread priorities text in the combo box
	 for(DWORD dwCount = 0; dwCount < ARRAY_SIZE(pszThreadPriorities, WCHAR *); dwCount++)
		 ::SendMessage(m_hThreadPriorityComboWnd, CB_INSERTSTRING, dwCount, reinterpret_cast<LPARAM>(pszThreadPriorities[dwCount])); // Insert the specified string

	 // Set the thread priority in the combo box to 'Normal'
	 ::SendMessage(m_hThreadPriorityComboWnd, CB_SETCURSEL, 1, 0);

	 ::SendMessage(m_hStatusEditWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(L"Initializing...Please wait\r\n"));
	 //::SendMessage(m_hStatusEditWnd, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(L"\r\nOperating completed successfully"));

	 // Modify the progress bars' style so that marquee animation is displayed before the operation really beings
	 ::SetWindowLongPtr(m_hSingleProgressbarWnd, GWL_STYLE, WS_CHILD | PBS_MARQUEE | WS_VISIBLE);
	 ::SetWindowPos(m_hSingleProgressbarWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
	 ::SetWindowLongPtr(m_hFullProgressbarWnd, GWL_STYLE, WS_CHILD | PBS_MARQUEE | WS_VISIBLE);
	 ::SetWindowPos(m_hFullProgressbarWnd, 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);

	 // Enable the marquee
	 ::SendMessage(m_hSingleProgressbarWnd, PBM_SETMARQUEE, TRUE, 50);
	 ::SendMessage(m_hFullProgressbarWnd, PBM_SETMARQUEE, TRUE, 50);

	 // Create a thread which will verify the checksum of the specified file or files in a directory
	 m_hChecksumThread = ::CreateThread(NULL, 0, verifyChecksumThread, NULL, CREATE_SUSPENDED, &dwChecksumThreadID); // Initially set it to supended state

	 //::SetTimer(hDlg, IDI_TIMER, 10, NULL); // Set a timer that will expire every 10 millliseconds
 }

//---------------------------------------------------------------------------
// Name: onCommand(WPARAM wpParam, LPARAM lpParam)
// Desc: Handle WM_COMMAND message
//---------------------------------------------------------------------------
void CChecksumVerifierFrame::onCommand(WPARAM wpParam, LPARAM lpParam)
{
	// Control identifier
	switch(LOWORD(wpParam))
	{
	//case IDOK:
	case IDCANCEL: // Close button has been pressed
		::SendMessage(m_hDlg, WM_CLOSE, 0, 0); // Close the window
		break;

	case IDB_CANCEL_BUTTON: // Cancel button has been pressed
		g_pChecksumVerifier->cancel();
		break;

	case IDB_PAUSE_BUTTON: // Pause button has been pressed
		static bool bPaused = false; // Initially the thread is not in a paused state
		if(bPaused) // The thread is paused
		{
			::ResumeThread(m_hChecksumThread); // Resume the thread
			bPaused = false; // Signal that the thread has been resumed
			::SetWindowText(m_hPauseButtonWnd, L"Pause");
		}
		else // The thread is not paused
		{
			// Since the thread is not paused, suspend the thread
			::SuspendThread(m_hChecksumThread); // Suspend the thread
			bPaused = true; // Signal that the thread is paused
			::SetWindowText(m_hPauseButtonWnd, L"Resume");
		}
		break;
	}

	// Control defined notification code
	switch(HIWORD(wpParam))
	{
	case CBN_SELCHANGE: // The selection in the combox box has changed
		g_pChecksumVerifier->setThreadPriority(static_cast<int>(::SendMessage(m_hThreadPriorityComboWnd, CB_GETCURSEL, 0, 0))); // Get the selection of the thread priority from the combox box
		break;
	}
}

//---------------------------------------------------------------------------
// Name: onActivate
// Desc: Handle WM_ACTIVATE message
//---------------------------------------------------------------------------
void CChecksumVerifierFrame::onActivate(WPARAM wpParam, LPARAM lpParam)
{
	switch(wpParam)
	{
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		break;

	case WA_INACTIVE:
		break;
	}
}

//---------------------------------------------------------------------------
// Name: onShowWindow
// Desc: Handle WM_SHOWWINDOW message
//---------------------------------------------------------------------------
void CChecksumVerifierFrame::onShowWindow(WPARAM wpParam, LPARAM lpParam)
{
	if(wpParam == TRUE) // The window is being shown
	{
		::ResumeThread(m_hChecksumThread);
	}
}

//---------------------------------------------------------------------------
// Name: onTimer
// Desc: Handle WM_TIMER message
//---------------------------------------------------------------------------
void CChecksumVerifierFrame::onTimer(WPARAM wpParam, LPARAM lpParam)
{
	int nProgress;

	switch(wpParam) // Timer ID
	{
	case IDI_TIMER:
		nProgress = g_pChecksumVerifier->getProgressStats();
		::SendMessage(m_hSingleProgressbarWnd, PBM_SETPOS, static_cast<WPARAM>(nProgress), 0);

		// Check to see if the thread is alive or not
		if(!g_pChecksumVerifier->isAlive()) // The thread is not alive
			::KillTimer(m_hDlg, IDI_TIMER); // Since the thread has terminated kill the timer
		//::SendMessage(m_hSingleProgressbarWnd, PBM_STEPIT, 0, 0);
		break;
	}
}

//---------------------------------------------------------------------------
// Name: onClose
// Desc: Handle WM_CLOSE message
//---------------------------------------------------------------------------
void CChecksumVerifierFrame::onClose()
{
	int nChoice;

	// Check if the checksum verification thread is alive
	if(g_pChecksumVerifier->isAlive()) // The checksum thread is alive
	{
		// Display a message box and ask the user if he wants to cancel the checksum verification operation
		nChoice = ::MessageBox(m_hDlg, L"Do you want to cancel the verification process which is in progress?", L"Confirmation", MB_YESNOCANCEL);
		if(nChoice != IDYES) // Yes not chosen
			return;

		// Disable the close button from the system menu
		::EnableMenuItem(::GetSystemMenu(m_hDlg, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED);

		::SetWindowText(m_hStatusEditWnd, L"The operation is being cancelled, please wait...\n\r");

		// The user has chosen 'Yes' to cancel the operation
		g_pChecksumVerifier->cancel();
	}

	//::KillTimer(m_hDlg, IDI_TIMER); // Kill the timer
	::WaitForSingleObject(m_hChecksumThread, INFINITE); // Wait for the thread to terminate
	::CloseHandle(m_hChecksumThread); // Close the handle of checksum verification thread

	::EndDialog(m_hDlg, 0); // End the dialog box
}

//---------------------------------------------------------------------------
// Name: enableChildControls
// Desc: Enable/disable child window controls
//---------------------------------------------------------------------------
BOOL CChecksumVerifierFrame::enableChildControls(BOOL bEnable)
{
	if(bEnable) // Enable all the child controls
	{
		::EnableWindow(m_hThreadPriorityComboWnd, TRUE);
		::EnableWindow(m_hPauseButtonWnd, TRUE);
		::EnableWindow(m_hCancelButtonWnd, TRUE);
	}
	else // Disable all the child controls
	{
		::EnableWindow(m_hThreadPriorityComboWnd, FALSE);
		::EnableWindow(m_hPauseButtonWnd, FALSE);
		::EnableWindow(m_hCancelButtonWnd, FALSE);
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: getWindowHandle
// Desc: Get the handle of the window
//---------------------------------------------------------------------------
HWND CChecksumVerifierFrame::getWindowHandle() const
{
	return m_hDlg;
}

//---------------------------------------------------------------------------
// Name: setSingleProgressbarState
// Desc: Change the progressbars style of a single progressbar
//---------------------------------------------------------------------------
BOOL CChecksumVerifierFrame::setSingleProgressbarStyle(EProgressbarStyle oProgressbarStyle)
{
	switch(oProgressbarStyle)
	{
	case Progressbar_Style_Smooth: // Smooth style
		::SetWindowLongPtr(m_hSingleProgressbarWnd, GWL_STYLE, WS_CHILD | WS_VISIBLE | PBS_SMOOTH);
		::SetWindowPos(m_hSingleProgressbarWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
		break;

	case Progressbar_Style_Marquee: // Marquee style
		::SetWindowLongPtr(m_hSingleProgressbarWnd, GWL_STYLE, WS_CHILD | WS_VISIBLE | PBS_MARQUEE);
		::SetWindowPos(m_hSingleProgressbarWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
		break;	
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: setFullProgressbarStyle
// Desc: Change the progressbars style of the full progressbar
//---------------------------------------------------------------------------
BOOL CChecksumVerifierFrame::setFullProgressbarStyle(EProgressbarStyle oProgressbarStyle)
{
	switch(oProgressbarStyle)
	{
	case Progressbar_Style_Smooth: // Smooth style
		::SetWindowLongPtr(m_hFullProgressbarWnd, GWL_STYLE, WS_CHILD | WS_VISIBLE | PBS_SMOOTH);
		::SetWindowPos(m_hFullProgressbarWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
		break;

	case Progressbar_Style_Marquee: // Marquee style
		::SetWindowLongPtr(m_hFullProgressbarWnd, GWL_STYLE, WS_CHILD | WS_VISIBLE | PBS_MARQUEE);
		::SetWindowPos(m_hFullProgressbarWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
		break;	
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: setSingleProgressbarState
// Desc: Change the progressbars style of the single progressbar
//---------------------------------------------------------------------------
BOOL CChecksumVerifierFrame::setSingleProgressbarState(EProgressbarState oProgressbarState)
{
	switch(oProgressbarState)
	{
	case Progressbar_State_Normal: // Normal state
		::SendMessage(m_hSingleProgressbarWnd, PBM_SETSTATE, PBST_NORMAL, 0);
		break;

	case Progressbar_State_Error: // Error state
		::SendMessage(m_hSingleProgressbarWnd, PBM_SETSTATE, PBST_ERROR, 0);
		break;

	case Progressbar_State_Paused: // Paused state
		::SendMessage(m_hSingleProgressbarWnd, PBM_SETSTATE, PBST_PAUSED, 0);
		break;
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: setFullProgressbarState
// Desc: Change the progressbars style of the full progressbar
//---------------------------------------------------------------------------
BOOL CChecksumVerifierFrame::setFullProgressbarState(EProgressbarState oProgressbarState)
{
	switch(oProgressbarState)
	{
	case Progressbar_State_Normal: // Normal state
		::SendMessage(m_hFullProgressbarWnd, PBM_SETSTATE, PBST_NORMAL, 0);
		break;

	case Progressbar_State_Error: // Error state
		::SendMessage(m_hFullProgressbarWnd, PBM_SETSTATE, PBST_ERROR, 0);
		break;	

	case Progressbar_State_Paused: // Paused state
		::SendMessage(m_hFullProgressbarWnd, PBM_SETSTATE, PBST_PAUSED, 0);
		break;
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: setStatusMessage
// Desc: Post status message in the edit control
//---------------------------------------------------------------------------
BOOL CChecksumVerifierFrame::setStatusMessage(std::wstring oStatusMessageString)
{
	return ::SetWindowText(m_hStatusEditWnd, oStatusMessageString.data());
}

//---------------------------------------------------------------------------
// Name: setDirectoryPathString
// Desc: Post status message in the edit control
//---------------------------------------------------------------------------
BOOL CChecksumVerifierFrame::setDirectoryPathString(std::wstring oDirectoryPathString)
{
	return ::SetWindowText(m_hCurrDirEditWnd, oDirectoryPathString.data());
}

//---------------------------------------------------------------------------
// Name: setFilePathString
// Desc: Post status message in the edit control
//---------------------------------------------------------------------------
BOOL CChecksumVerifierFrame::setFilePathString(std::wstring oFilePathString)
{
	return ::SetWindowText(m_hCurrFileEditWnd, oFilePathString.data());
}

//---------------------------------------------------------------------------
// Name: setChecksumHashString
// Desc: Post status message in the edit control
//---------------------------------------------------------------------------
BOOL CChecksumVerifierFrame::setChecksumHashString(std::wstring oChecksumHashString)
{
	return ::SetWindowText(m_hHashEditWnd, oChecksumHashString.data());
}

//---------------------------------------------------------------------------
// Name: setSingleProgressbarPos
// Desc: Set the position of the single progressbar
//---------------------------------------------------------------------------
LRESULT CChecksumVerifierFrame::setSingleProgressbarPos(UINT uiPos)
{
	return ::SendMessage(m_hSingleProgressbarWnd, PBM_SETPOS, static_cast<WPARAM>(uiPos), 0);
}

//---------------------------------------------------------------------------
// Name: setFullProgressbarPos
// Desc: Set the position of the full progressbar
//---------------------------------------------------------------------------
LRESULT CChecksumVerifierFrame::setFullProgressbarPos(UINT uiPos)
{
	return ::SendMessage(m_hFullProgressbarWnd, PBM_DELTAPOS, static_cast<WPARAM>(uiPos), 0);
}

//---------------------------------------------------------------------------
// Name: checksumVerifierDlgProc
// Desc: Window procedure for checksum verifier dialog procedure
//---------------------------------------------------------------------------
INT_PTR CALLBACK checksumVerifierDlgProc(HWND hDlg, UINT uiMsg, WPARAM wpParam, LPARAM lpParam)
{
	switch(uiMsg)
	{
	case WM_INITDIALOG:
		g_pChecksumVerifierFrame->onInitDialog(hDlg, wpParam, lpParam);
		break;

	case WM_COMMAND:
		g_pChecksumVerifierFrame->onCommand(wpParam, lpParam);
		break;

	case WM_ACTIVATE:
		g_pChecksumVerifierFrame->onActivate(wpParam, lpParam);
		break;

	case WM_SHOWWINDOW:
		g_pChecksumVerifierFrame->onShowWindow(wpParam, lpParam);
		break;

	case WM_TIMER:
		g_pChecksumVerifierFrame->onTimer(wpParam, lpParam);
		break;

	case WM_CLOSE:
		g_pChecksumVerifierFrame->onClose();
		break;
	}

	return 0;
}

CChecksumVerifierFrame g_oChecksumVerifierFrame;
CChecksumVerifierFrame *g_pChecksumVerifierFrame = &g_oChecksumVerifierFrame;