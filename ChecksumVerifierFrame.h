// ChecksumVerifierFrame.h: 
// Copyright (C) 2010-2011 Asesh Shrestha. All rights reserved

#ifndef CHECKSUMVERIFIERFRAME_H
#define CHECKSUMVERIFIERFRAME_H

#define IDI_TIMER 1000

// Forward declarations
enum EProgressbarState;
enum EProgressbarStyle;

// Function prototypes
INT_PTR CALLBACK checksumVerifierDlgProc(HWND hDlg, UINT uiMsg, WPARAM wpParam, LPARAM lpParam);

// CChecksumVerifierFrame class
class CChecksumVerifierFrame
{ 
public: // Public methods
	CChecksumVerifierFrame(); // Default constructor
	~CChecksumVerifierFrame(); // Destructor

	void onInitDialog(HWND hDlg, WPARAM wpParam, LPARAM lpParam); // Handle WM_INITDIALOG message
	void onCommand(WPARAM wpParam, LPARAM lpParam); // Handle WM_COMMAND message
	void onActivate(WPARAM wpParam, LPARAM lpParam); // Handle WM_ACTIVATE message
	void onShowWindow(WPARAM wpParam, LPARAM lpParam); // Handle WM_SHOWWINDOW message
	void onTimer(WPARAM wpParam, LPARAM lpParam); // Handle WM_TIMER message
	void onClose(); // Handle WM_CLOSE message

	BOOL enableChildControls(BOOL bEnable); // Enable/disable child controls

	HWND getWindowHandle() const; // Retrieve the window handle
	//HWND getSingleProgressbarHandle() const; // Retrieve the handle to probressbar which will show the progress of a single file
	//HWND getFullProgressbarHandle() const; // Retrieve the handle to the progressbar which will show the overall progress
	//HWND getStatusEditControlHandle() const; // Retrieve the handle to the status edit control

	BOOL setSingleProgressbarStyle(EProgressbarStyle oProgressbarStyle); // Set the style of the single progressbar
	BOOL setFullProgressbarStyle(EProgressbarStyle oProgressbarStyle); // Set the style of the full progressbar
	BOOL setSingleProgressbarState(EProgressbarState oProgressbarState); // Set the single progressbar state
	BOOL setFullProgressbarState(EProgressbarState oProgressbarState); // Set the full progressbar state
	BOOL setStatusMessage(std::wstring oStatusMessageString); // Post status message in the edit control
	BOOL setDirectoryPathString(std::wstring oDirectoryPathString); // Post the directory path of the directory being processed
	BOOL setFilePathString(std::wstring oFilePathString); // Post the file path of the file being processed
	BOOL setChecksumHashString(std::wstring oChecksumHashString); // Post the hash of the specified file
	LRESULT setSingleProgressbarPos(UINT uiValue); // 
	LRESULT setFullProgressbarPos(UINT uiValue); // 

private:
	HWND m_hDlg;
	HWND m_hPauseButtonWnd, m_hCancelButtonWnd;
	HWND m_hCurrDirEditWnd, m_hCurrFileEditWnd, m_hHashEditWnd, m_hStatusEditWnd;
	HWND m_hSingleProgressbarWnd, m_hFullProgressbarWnd;
	HWND m_hThreadPriorityComboWnd;

	HANDLE m_hChecksumThread; // Checksum thread handle

	DWORD dwChecksumThreadID; // Checksum thread ID

	bool m_bThreadStarted;
};

extern CChecksumVerifierFrame *g_pChecksumVerifierFrame;

// EProgressbarState enumeration
enum EProgressbarState
{
	Progressbar_State_Normal, // Normal state
	Progressbar_State_Error, // Error state
	Progressbar_State_Paused, // Paused state
};

// EProgressbarStyle enumeration
enum EProgressbarStyle
{
	Progressbar_Style_Smooth, // Smooth style
	Progressbar_Style_Marquee, // Marquee style
};

#endif // CHECKSUMVERIFIERFRAME_H