// MainFrame.h: 
// Copyright (C) 2008-2011 Asesh Shrestha. All rights reserved

#ifndef MAINFRAME_H
#define MAINFRAME_H

// Function prototypes
INT_PTR CALLBACK dlgProc(HWND hDlg, UINT uiMsg, WPARAM wpParam, LPARAM lpParam); // Main dialog box procedure
INT_PTR CALLBACK aboutDlgProc(HWND hDlg, UINT uiMsg, WPARAM wpParam, LPARAM lpParam); // About dialog box procedure
INT_PTR CALLBACK progressDlgProc(HWND hDlg, UINT uiMsg, WPARAM wpParam, LPARAM lpParam); // Checksum verification progress dialog box procedure

// CMainFrame class
class CMainFrame
{
public:
	CMainFrame(); // Default constructor
	~CMainFrame(); // Destructor

	BOOL create(); // Create the main window

	HWND getWindowHandle() const; // Get the window handle

	BOOL show() const; // Show the main window
	void runMainMessageLoop();

	void onInit(HWND hDlg, WPARAM wpParam, LPARAM lpParam); // Handle WM_INITDIALOG message
	void onCommand(WPARAM wpParam, LPARAM lpParam); // Handle WM_COMMAND message
	void onKeyDown(WPARAM wpParam, LPARAM lpParam); // Handle WM_KEYDOWN message
	void onNotify(WPARAM wpParam, LPARAM lpParam); // Handle WM_NOTIFY message
	void onShowWindow(WPARAM wpParam, LPARAM lpParam); // Handle WM_SHOWWINDOW message
	void onClose(); // Handle WM_CLOSE message
	void onDestroy(); // Handle WM_DESTROY message

	bool openBrowseForDirDialog(); // Display browser for folder dialog
	BOOL openOpenFileDialog(); // Display open file dialog
	BOOL openSaveFileDialog(); // Display save file dialog

private:
	// Window handles
	//
	HWND m_hDlg; // Main window handle
	HWND m_hSourcePathEditWnd, m_hSaveHashPathEditWnd; // Edit controls
	HWND m_hChecksumTypeComboWnd; // Combo boxes
	//
	// Window handles

	WCHAR m_szFolderPath[MAX_PATH * 2], m_szFileName[MAX_PATH * 2];

	OPENFILENAME m_oOpenFileName;

	MSG m_oMSG; // Holds message data
};

extern CMainFrame *g_pMainFrame;

#endif // MAINFRAME_H