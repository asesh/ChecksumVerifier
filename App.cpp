// App.cpp: 
// Copyright (C) 2011 Asesh Shrestha. All rights reserved

#include "StdAfx.h"
#include "MainFrame.h"
#include "App.h"

// WinMain
int WINAPI wWinMain(HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPWSTR lpCmdLine,
					int nCmdShow)
{
	MSG oMsg;

#ifdef _DEBUG // Debug version

	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);

#endif // _DEBUG

	// Initialize the COM library
	if(FAILED(::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
		return 1;

	// Initialize common controls
	INITCOMMONCONTROLSEX oInitCommonControls = {0};
	oInitCommonControls.dwSize = sizeof(INITCOMMONCONTROLSEX);
	oInitCommonControls.dwICC = ICC_WIN95_CLASSES;
	::InitCommonControlsEx(&oInitCommonControls);

	// Retrieve the operating system version
	OSVERSIONINFO oOSVersionInfo = {0};
	oOSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&oOSVersionInfo);

	// Retrieve the system information
	SYSTEM_INFO oSystemInfo = {0};
	::GetSystemInfo(&oSystemInfo);

	g_pApp->setInstance(hInstance); // Assign the app instance handle

	// Create the main window
	if(!g_pMainFrame->create())
		return 1;

	// Show the window
	g_pMainFrame->show();

	// Main message loop
	while(::GetMessage(&oMsg, NULL, 0, 0))
		::IsDialogMessage(g_pMainFrame->getWindowHandle(), &oMsg);

	// Uninitialize the COM library
	::CoUninitialize();

	return static_cast<int>(oMsg.wParam);
}

//---------------------------------------------------------------------------
// Name: CApp
// Desc: Default constructor
//---------------------------------------------------------------------------
CApp::CApp()
{
	m_hInstance = NULL;
}

//---------------------------------------------------------------------------
// Name: ~CApp
// Desc: Destructor
//---------------------------------------------------------------------------
CApp::~CApp()
{
}

//---------------------------------------------------------------------------
// Name: setInstance
// Desc: Set the instance handle of the application
//---------------------------------------------------------------------------
void CApp::setInstance(HINSTANCE hInstance)
{
	m_hInstance = hInstance;
}

//---------------------------------------------------------------------------
// Name: getInstance
// Desc: Get the handle to the application instance
//---------------------------------------------------------------------------
HINSTANCE CApp::getInstance() const
{
	return m_hInstance;
}

CApp g_oApp;
CApp *g_pApp = &g_oApp;