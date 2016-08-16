// MainFrame.cpp: Main frame implementation
// Copyright (C) 2008-2011 Asesh Shrestha. All rights reserved

#include "StdAfx.h"
#include "MainFrame.h"
#include "App.h"
#include "ChecksumVerifier.h"
#include "ChecksumVerifierFrame.h"

//---------------------------------------------------------------------------
// Name: CMainFrame
// Desc: Default constructor
//---------------------------------------------------------------------------
CMainFrame::CMainFrame()
{
	m_hDlg = NULL;
	m_hSourcePathEditWnd = NULL;
	m_hChecksumTypeComboWnd = NULL;
}

//---------------------------------------------------------------------------
// Name: ~CMainFrame
// Desc: Destructor
//---------------------------------------------------------------------------
CMainFrame::~CMainFrame()
{
}

//---------------------------------------------------------------------------
// Name: create
// Desc: Create the main window
//---------------------------------------------------------------------------
BOOL CMainFrame::create()
{
	// Create a modeless dialog box
	m_hDlg = ::CreateDialog(g_pApp->getInstance(), MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, dlgProc);
	if(m_hDlg == NULL)
		return FALSE;

	// Extend the glass frame into the client area
	//extendFrameIntoClientArea();

	// Set icon
	//::SendMessage(m_hDlg, WM_SETICON, static_cast<WPARAM>(ICON_SMALL), reinterpret_cast<LPARAM>(::LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_SHARED)));

	// Load application icon
	HICON hIcon = reinterpret_cast<HICON>(::LoadImage(::GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAIN_WINDOW_ICON), IMAGE_ICON, 48, 48, LR_SHARED));
	if(hIcon)
		::SendMessage(m_hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: getWindow
// Desc: Get the window handle
//---------------------------------------------------------------------------
HWND CMainFrame::getWindowHandle() const
{
	return m_hDlg;
}

//---------------------------------------------------------------------------
// Name: show
// Desc: Show the main window
//---------------------------------------------------------------------------
BOOL CMainFrame::show() const
{
	return ::ShowWindow(m_hDlg, SW_SHOW);
}

//---------------------------------------------------------------------------
// Name: onInit
// Desc: Initialize the dialog box
//---------------------------------------------------------------------------
void CMainFrame::onInit(HWND hDlg, WPARAM wpParam, LPARAM lpParam)
{
	RECT oClientRect;

	WCHAR *pszAlgorithms[] = {L"MD5 (128-bit)", L"SHA-1 (160-bit)", L"SHA-256 (256-bit)", L"SHA-512 (512-bit)"};

	// Retrieve the handles of all the child controls
	m_hSourcePathEditWnd = ::GetDlgItem(hDlg, IDC_SOURCE_PATH_EDIT); // Source file edit control
	m_hSaveHashPathEditWnd = ::GetDlgItem(hDlg, IDC_SAVE_HASH_PATH_EDIT); // Save hash path edit control
	m_hChecksumTypeComboWnd = ::GetDlgItem(hDlg, IDC_CHECKSUM_TYPE_COMBO); // Combo box holding checksum types

	// Add all the hash elements to the combo box
	for(DWORD dwCount = 0; dwCount < 4; dwCount++)
		::SendMessage(m_hChecksumTypeComboWnd, CB_INSERTSTRING, dwCount, reinterpret_cast<LPARAM>(pszAlgorithms[dwCount]));

	::SendMessage(m_hChecksumTypeComboWnd, CB_SETCURSEL, 1, 0); // Set the current selection in the checksum type combobox to SHA1 (160-bit)
	//::SendMessage(m_hFileChecksumRadionWnd, BM_SETCHECK, BST_CHECKED, 0); // 

#ifndef _DEBUG // Release versions

	::EnableWindow(m_hChecksumTypeComboWnd, FALSE); // Disable the combo box

#endif // _DEBUG

	//::GetClientRect(hDlg, &oClientRect);
	//::SetWindowPos(hDlg, NULL, 0, 0, oClientRect.right, oClientRect.bottom - 55, SWP_NOMOVE | SWP_NOZORDER);

	// Set banner cue text
	Edit_SetCueBannerText(m_hSourcePathEditWnd, L"Enter the name of the file or folder");
	Edit_SetCueBannerText(m_hSaveHashPathEditWnd, L"Enter the file name to save the results to");
}

//---------------------------------------------------------------------------
// Name: onCommand
// Desc: Handle WM_COMMAND message
//---------------------------------------------------------------------------
void CMainFrame::onCommand(WPARAM wpParam, LPARAM lpParam)
{
	switch(LOWORD(wpParam)) // Control identifier
	{
	case IDOK:
	case IDCANCEL:
		::SendMessage(m_hDlg, WM_CLOSE, 0, 0);
		break;

	case IDB_VERIFY_CHECKSUM_BUTTON: // 'Verify Checksum' button has been clicked
		{
			// Get the input file or directory from the edit control
			::SendMessage(m_hSourcePathEditWnd, WM_GETTEXT, ARRAY_SIZE(g_pChecksumData->m_szSourceFilePath, WCHAR), reinterpret_cast<LPARAM>(g_pChecksumData->m_szSourceFilePath));
			if(g_pChecksumData->m_szSourceFilePath[0] == '\0') // Check for NULL character
			{
				// Nothing has been entered in the edit control
				::SetFocus(m_hSourcePathEditWnd); // Set the focus on the edit control
				break;
			}

			// Format the necessary data and information the user has provided before processing the checksum
			g_pChecksumData->m_oChecksumType = static_cast<EChecksumType>(::SendMessage(m_hChecksumTypeComboWnd, CB_GETCURSEL, 0, 0)); // Get the selection from the combo box

			// Determine if the user has chosen to save the results to a file
			if(::IsDlgButtonChecked(m_hDlg, IDC_SAVE_RESULT_CHECK) == BST_CHECKED) // The user has chosen to save the result
			{
				g_pChecksumData->m_bSave = true;
				::GetWindowText(m_hSaveHashPathEditWnd, g_pChecksumData->m_szSaveResultPath, ARRAY_SIZE(g_pChecksumData->m_szSaveResultPath, WCHAR));
			}
			else // The user has not chosen to save the result
				g_pChecksumData->m_bSave = false;

			// Display the progress dialog box
			::DialogBox(g_pApp->getInstance(), MAKEINTRESOURCE(IDD_PROGRESS_DIALOG), m_hDlg, checksumVerifierDlgProc);
		}
		break;

	case IDC_BROWSE_FILES_DIR_SPLIT_BUTTON:
		openOpenFileDialog();
		break;

	case IDM_BROWSE_FILE_MENU:
		openOpenFileDialog();
		break;

	case IDM_BROWSE_FOLDER_MENU:
		openBrowseForDirDialog();
		break;

	case IDC_BROWSE_SAVE_RESULT_BUTTON: // Browse for save result file button
		openSaveFileDialog();
		break;

	case IDB_ABOUT_BUTTON: // About button
		::DialogBox(g_pApp->getInstance(), MAKEINTRESOURCE(IDD_ABOUT_DIALOG),	m_hDlg, aboutDlgProc);
		break;

	case IDC_CHECKSUM_TYPE_COMBO:
		switch(HIWORD(wpParam))
		{
		case CBN_DROPDOWN:
			//::SendMessage(::GetDlgItem(m_hDlg, LOWORD(wpParam)), CB_SHOWDROPDOWN, TRUE, 0);
			break;
		}
		break;
	}

	switch(HIWORD(wpParam)) // Control-defined notification codes
	{
	case BN_CLICKED: // The button has been clicked
		switch(LOWORD(wpParam)) // Identifier of the controls
		{
		//case IDC_FILE_CHECKSUM_RADIO: // Compute file checksum radio button
		//	::SendMessage(m_hFileChecksumRadionWnd, BM_SETCHECK, BST_CHECKED, 0);
		//	::EnableWindow(m_hSourcePathEditWnd, TRUE);
		//	::EnableWindow(m_hFileBrowseButtonWnd, TRUE);
		//	::EnableWindow(m_hSaveFileEditWnd, FALSE);
		//	//::EnableWindow(::GetDlgItem(m_hDlg, IDC_DEST_DIR_PATH_EDIT), FALSE);
		//	::EnableWindow(m_hFileBrowseButtonWnd, FALSE);
		//	::EnableWindow(m_hDirBrowseButtonWnd, FALSE);
		//	break;

		//case IDC_COMPUTE_FILE_HASH_RADIO: // Compute file hash radio button
		//	::EnableWindow(m_hSourcePathEditWnd, TRUE);
		//	::SendMessage(m_hHashEditWnd, EM_SETREADONLY, static_cast<WPARAM>(FALSE), 0);
		//	break;

		//case IDC_DIR_CHECKSUM_RADIO: // Compute directory checksum radio button
		//	::SendMessage(m_hDirChecksumRadioWnd, BM_SETCHECK, BST_CHECKED, 0);
		//	::EnableWindow(m_hSourcePathEditWnd, FALSE);
		//	::EnableWindow(m_hFileBrowseButtonWnd, FALSE);
		//	::EnableWindow(m_hSaveFileEditWnd, TRUE);
		//	//::EnableWindow(::GetDlgItem(m_hDlg, IDC_DEST_DIR_PATH_EDIT), TRUE);
		//	//::EnableWindow(::GetDlgItem(m_hDlg, IDB_BROWSE_FOR_SAVE_FILE_BUTTON), TRUE);
		//	//::EnableWindow(::GetDlgItem(m_hDlg, IDB_BROWSE_FOR_DIR_BUTTON), TRUE);
		//	break;

		case IDC_SAVE_RESULT_CHECK: // Save result checkbox toggled
			if(::IsDlgButtonChecked(m_hDlg, IDC_SAVE_RESULT_CHECK) == BST_CHECKED) // The save result checkbox is checked
			{
				::SendMessage(m_hSaveHashPathEditWnd, EM_SETREADONLY, FALSE, 0);
				::EnableWindow(::GetDlgItem(m_hDlg, IDC_BROWSE_SAVE_RESULT_BUTTON), TRUE);
			}
			else // The save result checkbox is unchecked
			{
				::SendMessage(m_hSaveHashPathEditWnd, EM_SETREADONLY, TRUE, 0);
				::EnableWindow(::GetDlgItem(m_hDlg, IDC_BROWSE_SAVE_RESULT_BUTTON), FALSE);
			}
			break;
		}
		break;

	case CBN_SELCHANGE: // Selection in the combo box has changed
		switch(LOWORD(wpParam)) // Control identifier
		{
		case IDC_CHECKSUM_TYPE_COMBO:
			// Get and set the current selection
			::SendMessage(m_hChecksumTypeComboWnd, CB_SETCURSEL, ::SendMessage(m_hChecksumTypeComboWnd, CB_GETCURSEL, 0, 0), 0);
			break;

		default:
			::DefWindowProc(m_hChecksumTypeComboWnd, CBN_SELCHANGE, wpParam, lpParam);
			break;
		}
		break;

	case CBN_SELENDCANCEL:
		switch(LOWORD(wpParam)) // Control identifier
		{
		case IDC_CHECKSUM_TYPE_COMBO:
			break;
		}
		break;

	case CBN_SELENDOK:
		switch(LOWORD(wpParam))
		{
		case IDC_CHECKSUM_TYPE_COMBO:
			// Get and set the current selection
			::SendMessage(m_hChecksumTypeComboWnd, CB_SETCURSEL, ::SendMessage(m_hChecksumTypeComboWnd, CB_GETCURSEL, 0, 0), 0);
			break;
		}
		break;
	}
}

//---------------------------------------------------------------------------
// Name: onNotify
// Desc: Process WM_NOTIFY message
//---------------------------------------------------------------------------
void CMainFrame::onNotify(WPARAM wpParam, LPARAM lpParam)
{
	switch((reinterpret_cast<NMHDR *>(lpParam))->code) // Notification code
	{
	case BCN_DROPDOWN: // The user has selected the split button part of 'Browse' button
		NMBCDROPDOWN *pDropDown = reinterpret_cast<NMBCDROPDOWN *>(lpParam);

		POINT oScreenPoint;

		oScreenPoint.x = pDropDown->rcButton.left; // X coordinate of the menu
		oScreenPoint.y = pDropDown->rcButton.bottom; // Y coordinate of the menu
		::ClientToScreen(pDropDown->hdr.hwndFrom, &oScreenPoint); // Convert the client coordinates to screen coordinates since TrackPopupMenu uses screen coordinates

		// Create a popup menu
		HMENU hMenu = ::CreatePopupMenu();
		::AppendMenu(hMenu, MF_BYPOSITION, IDM_BROWSE_FILE_MENU, L"F&ile"); // Append "File" to the menu
		::AppendMenu(hMenu, MF_BYPOSITION, IDM_BROWSE_FOLDER_MENU, L"F&older"); // Append "Folder" to the menu
		::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, oScreenPoint.x, oScreenPoint.y, 0, m_hDlg, NULL); // Display the popup menu
		::DestroyMenu(hMenu); // Destroy the menu
		break;
	}
}

//---------------------------------------------------------------------------
// Name: onKeyDown
// Desc: Process WM_KEYDOWN message
//---------------------------------------------------------------------------
void CMainFrame::onKeyDown(WPARAM wpParam, LPARAM lpParam)
{
	switch(LOWORD(wpParam))
	{
	case VK_ESCAPE: // Escape button has been pressed
		::SendMessage(m_hDlg, WM_CLOSE, 0, 0); // Send a message to close the window
		break;
	}
}

//---------------------------------------------------------------------------
// Name: onShowWindow
// Desc: Process WM_SHOWWINDOW message
//---------------------------------------------------------------------------
void CMainFrame::onShowWindow(WPARAM wpParam, LPARAM lpParam)
{
}

//---------------------------------------------------------------------------
// Name: onClose
// Desc: Process WM_CLOSE message
//---------------------------------------------------------------------------
void CMainFrame::onClose()
{
	::DestroyWindow(m_hDlg);
}

//---------------------------------------------------------------------------
// Name: onDestroy
// Desc: Process WM_DESTROY message
//---------------------------------------------------------------------------
void CMainFrame::onDestroy()
{
	::PostQuitMessage(0);
}

//---------------------------------------------------------------------------
// Name: openBrowseForDirDlg
// Desc: Open the browser for directory dialog box
//---------------------------------------------------------------------------
bool CMainFrame::openBrowseForDirDialog()
{
	LPITEMIDLIST pItemIDList;

	BROWSEINFO oBrowseInfo = {0};
	oBrowseInfo.hwndOwner = m_hDlg;
	oBrowseInfo.pszDisplayName = m_szFolderPath;
	oBrowseInfo.lpszTitle = L"Choose a folder";
	//oBrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
	//oBrowseInfo.pidlRoot = &oItemIDList;
	pItemIDList = ::SHBrowseForFolder(&oBrowseInfo);
	if(pItemIDList != NULL)
	{
		::SHGetPathFromIDList(pItemIDList, m_szFolderPath); // Convert the item ID list to path
		::SetDlgItemText(m_hDlg, IDC_SOURCE_PATH_EDIT, m_szFolderPath); // Display the selected path in the edit control

		// Free the ID list returned by the SHBrowseForFolder
		::CoTaskMemFree(reinterpret_cast<void *>(pItemIDList));
	}

	return true;
}

//---------------------------------------------------------------------------
// Name: openOpenFileDialog
// Desc: Display open file dialog box
//---------------------------------------------------------------------------
BOOL CMainFrame::openOpenFileDialog()
{
	::memset(m_szFileName, 0, sizeof(m_szFileName));
	::memset(&m_oOpenFileName, 0, sizeof(OPENFILENAME));
	m_oOpenFileName.lStructSize = sizeof(OPENFILENAME);
	m_oOpenFileName.hwndOwner = m_hDlg;
	m_oOpenFileName.lpstrFilter = L" All Files \0*.*\0\0";
	m_oOpenFileName.lpstrFile = m_szFileName;
	m_oOpenFileName.nMaxFile = ARRAY_SIZE(m_szFileName, WCHAR);
	m_oOpenFileName.lpstrTitle = L"Open a specific file for computing checksum";
	m_oOpenFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if(!::GetOpenFileName(&m_oOpenFileName))
		return FALSE;

	::SetDlgItemText(m_hDlg, IDC_SOURCE_PATH_EDIT, m_szFileName);

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: openSaveFileDialog
// Desc: Display save file dialog box
//---------------------------------------------------------------------------
BOOL CMainFrame::openSaveFileDialog()
{
	::memset(m_szFileName, 0, sizeof(m_szFileName));
	::memset(&m_oOpenFileName, 0, sizeof(OPENFILENAME));
	m_oOpenFileName.lStructSize = sizeof(OPENFILENAME);
	m_oOpenFileName.hwndOwner = m_hDlg;
	m_oOpenFileName.lpstrFilter = L" Text File (.txt) \0*.txt \0 All Files (*.*) \0 *.*\0\0";
	m_oOpenFileName.lpstrFile = m_szFileName;
	m_oOpenFileName.nMaxFile = ARRAY_SIZE(m_szFileName, WCHAR);
	m_oOpenFileName.lpstrTitle = L"Save the results in a text file";
	m_oOpenFileName.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
	if(!::GetSaveFileName(&m_oOpenFileName))
		return FALSE;

	::SetWindowText(m_hSaveHashPathEditWnd, m_szFileName);

	//::SendMessage(m_hSaveHashPathEditWnd, EM_SCROLLCARET, 0, 0);

	//::SetFocus(m_hSaveHashPathEditWnd);

	return TRUE;
}

CMainFrame g_oMainFrame;
CMainFrame *g_pMainFrame = &g_oMainFrame;

//---------------------------------------------------------------------------
// Name: dlgProc
// Desc: Main dialog box procedure
//---------------------------------------------------------------------------
INT_PTR CALLBACK dlgProc(HWND hDlg, UINT uiMsg, WPARAM wpParam, LPARAM lpParam)
{
	// Parse messages
	switch(uiMsg)
	{
	case WM_INITDIALOG:
		g_pMainFrame->onInit(hDlg, wpParam, lpParam);
		break;

	case WM_COMMAND:
		g_pMainFrame->onCommand(wpParam, lpParam);
		break;

	case WM_NOTIFY:
		g_pMainFrame->onNotify(wpParam, lpParam);
		break;

	case WM_KEYDOWN:
		g_pMainFrame->onKeyDown(wpParam, lpParam);
		break;

	case WM_SHOWWINDOW:
		g_pMainFrame->onShowWindow(wpParam, lpParam);
		break;

	case WM_CLOSE:
		g_pMainFrame->onClose();
		break;

	case WM_DESTROY:
		g_pMainFrame->onDestroy();
		break;
	}

	return 0;
}

//---------------------------------------------------------------------------
// Name: aboutDlgProc
// Desc: About dialog box procedure
//---------------------------------------------------------------------------
INT_PTR CALLBACK aboutDlgProc(HWND hDlg, UINT uiMsg, WPARAM wpParam, LPARAM lpParam)
{
	HDC hDc;

	switch(uiMsg)
	{
	case WM_PAINT:
		hDc = ::GetDC(hDlg);
		::DrawIconEx(hDc, 20, 26, ::LoadIcon(::GetModuleHandle(NULL), reinterpret_cast<LPCWSTR>(IDI_MAIN_WINDOW_ICON)), 32, 32, 0, NULL, DI_NORMAL);
		::ReleaseDC(hDlg, hDc);
		break;

	case WM_COMMAND:
		switch(LOWORD(wpParam))
		{
		case IDOK:
		case IDCANCEL:
			::EndDialog(hDlg, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch(reinterpret_cast<NMHDR *>(lpParam)->code)
		{
		case NM_CLICK:
			switch(reinterpret_cast<NMHDR *>(lpParam)->idFrom)
			{
			case IDC_SITE_SYSLINK:
				::ShellExecute(NULL, L"open", L"http://aseshshrestha.design.officelive.com/", NULL, NULL, SW_NORMAL);
				break;
			}
			break;
		}
		break;
	}

	return 0;
}
