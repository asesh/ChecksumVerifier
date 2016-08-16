// ChecksumVerifier.cpp: Implementation of CChecksumVerifier class for verifying checksums
// Copyright (C) 2008-2011 Asesh Shrestha. All rights reserved

#include "StdAfx.h"
#include "ChecksumVerifierFrame.h"
#include "ChecksumVerifier.h"

//---------------------------------------------------------------------------
// Name: CChecksumVerifier
// Desc: Default constructor
//---------------------------------------------------------------------------
CChecksumVerifier::CChecksumVerifier()
{
	m_pSHA1 = NULL;
	m_pSHA256 = NULL;
	//m_pSHA512 = NULL;
	m_pMD5 = NULL;

	m_hReadSourceFile = m_hWriteDestFile = NULL; // File handles

	m_hChecksumThread = NULL;

	m_hReadEvent = NULL; // Event

	m_bCancelOperation = false; // Set cancel operation to false to signal the the processing of checksum not be cancelled
	m_bOperationComplete = false; // Initially set that the operation is not complete

	::InitializeCriticalSection(&m_oChecksumCriticalSection); // Initialize the critical section

	m_nProgress = 0;
}

//---------------------------------------------------------------------------
// Name: ~CChecksumVerifier
// Desc: Destructor
//---------------------------------------------------------------------------
CChecksumVerifier::~CChecksumVerifier()
{
	::DeleteCriticalSection(&m_oChecksumCriticalSection); // Delete the ciritical section
}

//---------------------------------------------------------------------------
// Name: getProgressStats
// Desc: Get the progress statistics
//---------------------------------------------------------------------------
int CChecksumVerifier::getProgressStats() const
{
	return m_nProgress;
}

//---------------------------------------------------------------------------
// Name: cancel
// Desc: Cancel the operation
//---------------------------------------------------------------------------
BOOL CChecksumVerifier::cancel()
{
	m_bCancelOperation = true;

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: isAlive
// Desc: Returns a boolean value indicating if the thread is alive or not
//---------------------------------------------------------------------------
BOOL CChecksumVerifier::isAlive() const
{
	return !(m_bOperationComplete == TRUE);
}

//---------------------------------------------------------------------------
// Name: setThreadPriority
// Desc: Set the thread priority
//---------------------------------------------------------------------------
BOOL CChecksumVerifier::setThreadPriority(int nThreadPriority)
{
	switch(nThreadPriority)
	{
	case 0: // Highest
		return ::SetThreadPriority(m_hChecksumThread, THREAD_PRIORITY_HIGHEST);
		break;

	case 1: // Normal
		return ::SetThreadPriority(m_hChecksumThread, THREAD_PRIORITY_NORMAL);
		break;

	case 2: // Idle
		return ::SetThreadPriority(m_hChecksumThread, THREAD_PRIORITY_IDLE);
		break;

	case 3: // Lowest
		return ::SetThreadPriority(m_hChecksumThread, THREAD_PRIORITY_LOWEST);
		break;
	}

	return FALSE;
}

//---------------------------------------------------------------------------
// Name: verifyChecksum
// Desc: Verifies checksum
//---------------------------------------------------------------------------
BOOL CChecksumVerifier::verifyChecksum()
{
	m_bCancelOperation = m_bOperationComplete = false; // Initially set that the cancel operation to false and operation is incomplete

	// Keep trying to acquire the ownership of the critical section object
	while(!::TryEnterCriticalSection(&m_oChecksumCriticalSection))
	{
		::Sleep(10); // Sleep before trying again

		// Check if the user has cancelled the operation
		if(m_bCancelOperation)
			return FALSE;
	}

	// Allocate memory for hashing algorithms based on the algorithm the user has chosen to verify
	switch(g_pChecksumData->m_oChecksumType)
	{
	case Checksum_Type_MD5: // The user has chosen MD5
		m_pMD5 = new CMD5();
		m_oChecksumDataString = L"MD5: ";
		break;

	case Checksum_Type_SHA1: // The user has chosen SHA1
		m_pSHA1 = new SHA1();
		m_oChecksumDataString = L"SHA1: ";
		break;

	case Checksum_Type_SHA256: // The user has chosen SHA256
		m_pSHA256 = new CSHA256();
		m_oChecksumDataString = L"SHA-256: ";
		break;

	case Checksum_Type_SHA512: // The user has chosen SHA512
		m_oChecksumDataString = L"SHA-512: ";
		break;
	}

	// Get the attributes of the chosen file
	DWORD dwFileAttribute = ::GetFileAttributes(g_pChecksumData->m_szSourceFilePath);
	if(dwFileAttribute == FILE_ATTRIBUTE_DIRECTORY) // The file is a directory
		verifyDirectoryChecksum(g_pChecksumData->m_szSourceFilePath); // Verify checksum of files in the specified directory
	else // It's a file and not a directory
		verifyFileChecksum(g_pChecksumData->m_szSourceFilePath); // Verify file checksum

	// Deallocate the memory
	DELETE_FREE(m_pMD5);
	DELETE_FREE(m_pSHA1);
	DELETE_FREE(m_pSHA256);

	// Now that everything has finished, disable the child controls
	g_pChecksumVerifierFrame->enableChildControls(FALSE);

	// Leave the ownership of the critical section object
	::LeaveCriticalSection(&m_oChecksumCriticalSection);

	m_bOperationComplete = true; // Now that everything is done, set the operation complete flag

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: verifyFileChecksum
// Desc: Verify the file checksum
//---------------------------------------------------------------------------
BOOL CChecksumVerifier::verifyFileChecksum(std::wstring oFilePathString)
{
	DWORD dwAllocatedBufferSize; // Will hold the size of allocated buffer
	DWORD dwSingleFilePercentageProgress;

	DWORD dwNumOfBytesProcessed, dwNumOfBytesLeftToProcess;

	char *pszReadData = NULL; // Will hold the bytes read from the opened file
	//char szDataBlock[64]; // Will hold 64 bytes of block of data from the input

	std::wstring oFileNameString, oDirPathString; // Will hold the broken filename and directory path
	std::wstring::const_reverse_iterator iterString; // Iterator for traversing through the filepath

	WCHAR szStatusMessage[MAX_PATH * 4];

	std::wstring oMemoryAllocationFailureString = L"Error allocating memory for the specified operation. Enough memory might not be available on your system\t";

	OVERLAPPED oReadOverlapped = {0}; // For overlapped operation

	LARGE_INTEGER oFileSize = {0}, oFileSizeRead = {0}, oTotalFileSizeRead = {0}; // Will hold the size of the opened file

	// Now open the file for reading asynchronously
	m_hReadSourceFile = ::CreateFile(oFilePathString.data(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if(m_hReadSourceFile == INVALID_HANDLE_VALUE) // Error opening the specified file
	{
		::StringCbPrintf(szStatusMessage, ARRAY_SIZE(szStatusMessage, WCHAR), L"Error opening \"%s\"\n", oFilePathString.data());
		g_pChecksumVerifierFrame->setStatusMessage(szStatusMessage); // Display the error message
		g_pChecksumVerifierFrame->setSingleProgressbarState(Progressbar_State_Error); // Set error state on single progressbar
		g_pChecksumVerifierFrame->setFullProgressbarState(Progressbar_State_Error); // Set error state on full progressbar

		return FALSE; // Return error
	}

	// Separate the filename and the directory path and display it in their corresponding edit control
	//
	iterString = oFilePathString.crbegin();
	while(*iterString != L'\\')
	{
		oFileNameString += *iterString;

		iterString++;
	}

	while(iterString != oFilePathString.crend())
	{
		oDirPathString += *iterString;

		iterString++;
	}

	oFilePathString.clear();

	iterString = oFileNameString.crbegin();
	while(iterString != oFileNameString.crend())
	{
		oFilePathString += *iterString;

		iterString++;
	}
	oFileNameString = oFilePathString; // Filename path string

	oFilePathString.clear();

	iterString = oDirPathString.crbegin();
	while(iterString != oDirPathString.crend())
	{
		oFilePathString += *iterString;

		iterString++;
	}
	oDirPathString = oFilePathString; // Directory path string
	//
	// Separate the filename and the directory path and display it in their corresponding edit control

	oFilePathString += oFileNameString; // Format the full path string

	// Set the texts in the edit controls
	g_pChecksumVerifierFrame->setDirectoryPathString(oDirPathString); // Set the directory the file resides in in the edit control
	g_pChecksumVerifierFrame->setFilePathString(oFileNameString); // Set the file path of the file that is going to be processed in the dialog box
	g_pChecksumVerifierFrame->setChecksumHashString(L"Pending"); // Set the hash to pending for now because it's going to be processed

	// Format the filename data and display it's processing status message
	::StringCbPrintf(szStatusMessage, ARRAY_SIZE(szStatusMessage, WCHAR), L"Processing \"%s\"...", oFilePathString.data());
	g_pChecksumVerifierFrame->setStatusMessage(szStatusMessage);

	// Retrieve the size of the file
	::GetFileSizeEx(m_hReadSourceFile, &oFileSize);

	// Reject the operation if the file size is less than 100 bytes
	if(oFileSize.QuadPart < 100) // The file size is less than 100 bytes
	{
		// Set the progressbars state to error
		g_pChecksumVerifierFrame->setSingleProgressbarState(Progressbar_State_Error);
		g_pChecksumVerifierFrame->setFullProgressbarState(Progressbar_State_Error);

		g_pChecksumVerifierFrame->setStatusMessage(L"Error: The size of the specified file must be greater than 100 bytes");

		::CloseHandle(m_hReadSourceFile); // Close the file

		return FALSE; // Return error
	}

	// Reject the file if it's size is greater than 4 GB
	if(oFileSize.QuadPart > 4294967296) // The size of the file is greater than 4 GB
	{
		// Set the progressbars state to error
		g_pChecksumVerifierFrame->setSingleProgressbarState(Progressbar_State_Error);
		g_pChecksumVerifierFrame->setFullProgressbarState(Progressbar_State_Error);

		g_pChecksumVerifierFrame->setStatusMessage(L"Error: The size of the specified file is greater than 4 GB");

		::CloseHandle(m_hReadSourceFile); // Close the file

		return FALSE; // Return error
	}

	m_oTotalFilesSize.QuadPart += oFileSize.QuadPart; // Accumulate the size of the current file

	// Now since we will read the file in 100 MB per-chunk; if the size of the file is greater than 100 MB then break it into 100 MB per-chunk
	if(oFileSize.QuadPart >= 104857600) // The size of the file is greater than or equal to 100 MB
	{
		dwAllocatedBufferSize = 104857600; // 100 MB of buffer size

		// Since we are going to allocate 100 MB of memory, enough memory might not be available on the machine so to prevent memory allocation
		// failure, wrap it under try-catch clause
		try
		{
			pszReadData = new char[dwAllocatedBufferSize]; // Allocate memory of 100 MB
		}
		catch(std::bad_alloc &) // Memory allocation failed
		{
			g_pChecksumVerifierFrame->setSingleProgressbarState(Progressbar_State_Error); // Error has occured so reflect it in the progressbar too

			::CloseHandle(m_hReadSourceFile); // Close the file handle before returning

			g_pChecksumVerifierFrame->setStatusMessage(L"Error: Memory allocation failure");

			return FALSE; // Since enough memory couldn't be allocated, exit the thread
		}
	}
	else // The size of the file is less than 100 MB
	{
		// Now since the size of the specified file is less than 100 MB, allocate memory based on the size of the file
		dwAllocatedBufferSize = oFileSize.LowPart;
		pszReadData = new char[oFileSize.LowPart]; // Allocate memory based on the size of the file
	}

	dwSingleFilePercentageProgress = oFileSize.LowPart / 100; // Divide the file size into 100 equal parts for displaying the progress in the progressbar

	// Change the style of the progressbars to smooth
	g_pChecksumVerifierFrame->setSingleProgressbarStyle(Progressbar_Style_Smooth);
	g_pChecksumVerifierFrame->setFullProgressbarStyle(Progressbar_Style_Smooth);

	// Read the contents of the file
	while(::ReadFile(m_hReadSourceFile, pszReadData, dwAllocatedBufferSize, NULL, &oReadOverlapped) == FALSE && ::GetLastError() == ERROR_IO_PENDING)
	{
		// Check if the user has chosen to cancel the operation
		if(m_bCancelOperation) // The user has chosen to cancel the operation
			break; // So get out of the loop

		// Retrieve the result of an overlapped operation
		if(::GetOverlappedResult(m_hReadSourceFile, &oReadOverlapped, &oFileSizeRead.LowPart, FALSE)) // == FALSE && ::GetLastError() == ERROR_IO_INCOMPLETE) // It's not EOF
		{
			oReadOverlapped.Offset += oFileSizeRead.LowPart; // Increase an offset by the total number of bytes read
			oTotalFileSizeRead.QuadPart += oFileSizeRead.QuadPart; // Increase the total number of bytes read

			dwNumOfBytesLeftToProcess = oFileSizeRead.LowPart; // Number of bytes left to process

			//::StringCbCopyNA(szDataBlock, sizeof(szDataBlock), pszReadData, 64); // Now copy the 64 bytes from the buffer

			// Checksum type
			switch(g_pChecksumData->m_oChecksumType)
			{
			case Checksum_Type_MD5: // MD5
				break;

			case Checksum_Type_SHA1: // SHA1
				m_pSHA1->Input(pszReadData, oFileSizeRead.LowPart);
				break;

			case Checksum_Type_SHA256: // SHA-256
				break;

			case Checksum_Type_SHA512: // SHA-512
				break;
			}

			// Increment the ticks in the progressbar to show the progress
			g_pChecksumVerifierFrame->setSingleProgressbarPos(oTotalFileSizeRead.LowPart / dwSingleFilePercentageProgress);
		}
		else // GetOverlappedResult returned FALSE
		{
			// Check if EOF has been reached
			if(::GetLastError() == ERROR_HANDLE_EOF) // EOF reached
				break; // Since EOF has been reached break the loop
		}
	}

	// Free the memory that was allocated for the buffer
	delete [] pszReadData;

	::CloseHandle(m_hReadSourceFile); // Close the file we are reading from

	// Now check if the total number of bytes read is equal to the total size of the file then we will assume there's no error
	// otherwise we will assume an error has occurred
	if(oFileSize.QuadPart != oTotalFileSizeRead.QuadPart) // The file sizes are not equal
	{
		g_pChecksumVerifierFrame->setSingleProgressbarState(Progressbar_State_Error); // Set the error state of the single progressbar to error since an error has occurred

		// Now since there was an error processing the specified file, post an error message
		if(m_bCancelOperation)
			::StringCbCat(szStatusMessage, ARRAY_SIZE(szStatusMessage, WCHAR), L" Cancelled\n\r");
		else
			::StringCbPrintf(szStatusMessage, ARRAY_SIZE(szStatusMessage, WCHAR), L"Error processing \"%s\"\n\r", oFilePathString.data());
		g_pChecksumVerifierFrame->setStatusMessage(szStatusMessage);
	}
	else // The specified file was successfully processed
	{
		::StringCbCat(szStatusMessage, ARRAY_SIZE(szStatusMessage, WCHAR), L"Done\n\r");
		g_pChecksumVerifierFrame->setStatusMessage(szStatusMessage); // Display the Done message in the status text box

		g_pChecksumVerifierFrame->setChecksumHashString(m_pSHA1->getHashString()); // Set the computed hash in the text box

		// Check if the user has specified to save the hash data
		if(g_pChecksumData->m_bSave) // The user has chosen to save the hash data
		{
			if(g_pChecksumData->m_szSaveResultPath[0] == L'\0') // The file name to save the hash is empty
				oFileNameString += L"_Hash.txt";
			else // The file name to save the hash is not empty
				oFileNameString = g_pChecksumData->m_szSaveResultPath; // Assign the file name to save the hash to

			m_oChecksumDataString += m_pSHA1->getHashString(); // Append the hash of the file

			// Create a file in which we will write the checksum data into
			m_hWriteDestFile = ::CreateFile(oFileNameString.data(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if(m_hWriteDestFile != INVALID_HANDLE_VALUE) // The creation of the file succeeded
			{
				DWORD dwNumOfBytesWritten;
				::WriteFile(m_hWriteDestFile, m_oChecksumDataString.data(), m_oChecksumDataString.length() * 2, &dwNumOfBytesWritten, NULL);
				::CloseHandle(m_hWriteDestFile);
			}
			else // The creation of the file didn't succeed
			{
				g_pChecksumVerifierFrame->setStatusMessage(L"The hash data couldn't be saved in an external file\n\r");
			}
		}
		// The user has specified not to save the checksum data
	}

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: verifyDirectoryChecksum
// Desc: Verify the directory checksum
//---------------------------------------------------------------------------
BOOL CChecksumVerifier::verifyDirectoryChecksum(std::wstring oDirectoryPathString)
{
	WIN32_FIND_DATA oWinFindData = {0};

	// Set the current directory to the specified path
	if(!::SetCurrentDirectory(oDirectoryPathString.data()))
		return FALSE;

	g_pChecksumVerifierFrame->setStatusMessage(L"Initializing, please wait...\n\r");

	// Initialize the search for files in the directories
	HANDLE hFindFile = ::FindFirstFile(L"*.*", &oWinFindData);
	if(hFindFile == INVALID_HANDLE_VALUE) // Error occurred
	{
		g_pChecksumVerifierFrame->setStatusMessage(L"Error, searching for files in the specified directory\n\r");
		g_pChecksumVerifierFrame->setSingleProgressbarState(Progressbar_State_Error);
		g_pChecksumVerifierFrame->setFullProgressbarState(Progressbar_State_Error);
		return FALSE;
	}

	// Set the full progressbar style to marquee
	g_pChecksumVerifierFrame->setFullProgressbarStyle(Progressbar_Style_Marquee);

	// Loop to scan the specified folder
	do
	{
		// Check if the file found is a directory or not
		if((oWinFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) // The file found is a directory
			continue; // Since it's a directory, skip it and search for another file

		m_pSHA1->Reset();

		//verifyFileChecksum(oWinFindData.cFileName);

		///m_oTotalFilesSize.QuadPart += oWinFindData.nFileSizeLow + oWinFindData.nFileSizeHigh;

	} while(::FindNextFile(hFindFile, &oWinFindData) && ::GetLastError() != ERROR_FILE_NOT_FOUND);

	// Now that we have finished searching close the file search handle
	::FindClose(hFindFile);

	g_pChecksumVerifierFrame->setSingleProgressbarStyle(Progressbar_Style_Smooth);
	g_pChecksumVerifierFrame->setFullProgressbarStyle(Progressbar_Style_Smooth);

	g_pChecksumVerifierFrame->setSingleProgressbarPos(100);
	g_pChecksumVerifierFrame->setFullProgressbarPos(100);

	return TRUE;
}

//---------------------------------------------------------------------------
// Name: 
// Desc: 
//---------------------------------------------------------------------------

CChecksumVerifier g_oChecksumVerifier;
CChecksumVerifier *g_pChecksumVerifier = &g_oChecksumVerifier;

SChecksumData g_oChecksumData;
SChecksumData *g_pChecksumData = &g_oChecksumData;

//---------------------------------------------------------------------------
// Name: verifyChecksumThread
// Type: Thread
// Desc: Thread which verifies checksum
//---------------------------------------------------------------------------
DWORD WINAPI verifyChecksumThread(LPVOID lpParam)
{
	return (g_pChecksumVerifier->verifyChecksum() == TRUE ? 0 : 1); // Verify the checksum

	//return 0;
}