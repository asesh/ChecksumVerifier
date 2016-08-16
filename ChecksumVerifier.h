// ChecksumVerifier.h: Declaration of CChecksumVerifier class
// Copyright (C) 2008-2011 Asesh Shrestha. All rights reserved

#ifndef CHECKSUMVERIFIER_H
#define CHECKSUMVERIFIER_H

// QWORD
typedef __int64 QWORD;

// Prototypes
DWORD WINAPI verifyChecksumThread(LPVOID lpParam); // Thread for verifying checksum

// Forward declarations
enum EChecksumType;
struct SChecksumData;
struct SChecksumHash;

// CChecksumVerifier class
class CChecksumVerifier
{
public: // Public methods
	CChecksumVerifier(); // Default constructor
	~CChecksumVerifier(); // Destructor

	int getProgressStats() const; // Get the progress statistics

	BOOL pause(); // Pause the operation
	BOOL cancel(); // Cancel the operation

	BOOL isAlive() const; // Check if the operation is alive

	BOOL setThreadPriority(int nThreadPriority); // Set the thread priority

	BOOL verifyChecksum(); // Verify checksum
	BOOL verifyFileChecksum(std::wstring oFilePathString); // Verify file checksum
	BOOL verifyDirectoryChecksum(std::wstring oDirectoryPathString); // Verify directory checksum

private: // Private attributes

	SHA1 *m_pSHA1;
	CSHA256 *m_pSHA256;
	//CSHA512 *m_pSHA512;
	CMD5 *m_pMD5;

	HANDLE m_hReadSourceFile, m_hWriteDestFile; // File handles

	HANDLE m_hChecksumThread; // Thread handle

	HANDLE m_hReadEvent; // Event handle

	CRITICAL_SECTION m_oChecksumCriticalSection; // Checksum critical section

	DWORD m_dwChecksumThreadID; // Thread ID

	OVERLAPPED m_oOverLapped; // Asynchronous input/output

	WCHAR m_szFilePath[MAX_PATH * 2];

	std::wstring m_oChecksumHashString, m_oChecksumDataString;

	bool m_bCancelOperation, m_bPauseOperation, m_bOperationComplete;

	LARGE_INTEGER m_oTotalFilesSize;

	DWORD dwFullFilePercentageProgress;

	DWORD m_dwThreadPriority; // Will hold the thread priority
	
	int m_nProgress; // Will hold the progress of the checksum
};

extern CChecksumVerifier *g_pChecksumVerifier;

// EChecksumType enumeration
enum EChecksumType
{
	Checksum_Type_MD5, // MD5 checksum
	Checksum_Type_SHA1, // SHA-1 or SHA-160-bit checksum
	Checksum_Type_SHA256, // SHA-2 or SHA-256-bit checksum
	Checksum_Type_SHA512, // SHA-512 checksum
};

// SChecksumData structure
struct SChecksumData
{
	EChecksumType m_oChecksumType; // Checksum type

	WCHAR m_szSourceFilePath[MAX_PATH * 2]; // Directory or file path
	WCHAR m_szSaveResultPath[MAX_PATH * 2]; // Save result file path

	bool m_bSave;
};
extern SChecksumData *g_pChecksumData;

#endif // CHECKSUMVERIFIER_H