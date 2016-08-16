// SHA.cpp: Implementation of SHA1 hash algorithm
// Copyright (C) 2011 Asesh Shrestha. All rights reserved

#include "StdAfx.h"
#include "SHA.h"

//---------------------------------------------------------------------------
// Name: CSHA1
// Desc: Default constructor
//---------------------------------------------------------------------------
CSHA1::CSHA1()
{
	m_dwMessageDigest[0] = 0x67452301;
	m_dwMessageDigest[1] = 0xEFCDAB89;
	m_dwMessageDigest[2] = 0x98BADCFE;
	m_dwMessageDigest[3] = 0x10324A76;
	m_dwMessageDigest[4] = 0xC3D2E1F0;
}

//---------------------------------------------------------------------------
// Name: ~CSHA1
// Desc: 
//---------------------------------------------------------------------------
CSHA1::~CSHA1()
{
}

//---------------------------------------------------------------------------
// Name: compute
// Desc: Compute the SHA1 hash
//---------------------------------------------------------------------------
bool CSHA1::compute(std::string oDataString)
{
	WCHAR szFinalHash[65]; // Will hold the formatted hash string
	char szMessageArray[65]; // Will hold the data read from the file

	UINT uiCount;

	DWORD dwTemp;

	DWORD dwA, dwB, dwC, dwD, dwE, dwF;

	dwA = m_dwMessageDigest[0];
	dwB = m_dwMessageDigest[1];
	dwC = m_dwMessageDigest[2];
	dwD = m_dwMessageDigest[3];
	dwE = m_dwMessageDigest[4];

	// Check to see if the number of bytes is greater than 64 bytes
	if(oDataString.length() > 64) // The length of the string is greater than 64 bytes
		return false; // Since the string length is greater than 64 bytes return

	// Copy the input data to a buffer
	::strncpy(szMessageArray, oDataString.data(), oDataString.length());
	if(::strlen(szMessageArray) <= 0)
		return false;

	szMessageArray[oDataString.length()] &= 0xFF & 0x80; // Append 1 bit to the message

	// Check if the length of the data is less than 80 bytes
	if(oDataString.length() < 80) // The length of the data is less than 80 bytes
	{
		// Now since the length of the data is less than 80 bytes we pad the data to make it 
		for(uiCount = static_cast<UINT>(oDataString.length()); uiCount < 64; uiCount++)
		{
			szMessageArray[uiCount + 1] &= 0xFF & 0x0; // Pad 0s to the remaining bits
		}
	}
	else // The length of the data is greater than 80 bytes
	{
		return false;
	}

	// Copy the 16 DWORDs from the input to Ws
	::memcpy(m_dwBlockOfData, szMessageArray, sizeof(szMessageArray));

	// Fill the other 64 DWORDs
	for(uiCount = 16; uiCount <= 79; uiCount++)
	{
		m_dwBlockOfData[uiCount] = LEFT_CIRCULAR_ROTATION(m_dwBlockOfData[uiCount - 3] ^ m_dwBlockOfData[uiCount - 8] ^ m_dwBlockOfData[uiCount - 14] ^ m_dwBlockOfData[uiCount - 16], 1); // Left shift by 1 bit
	}

	// Now copy the first 16 DWORD in the 80 DWORD array
	for(uiCount = 0; uiCount < 80; uiCount++)
	{
		// 0 =< uiCount <= 19
		if(uiCount >= 0 && uiCount <= 19)
		{
			dwF = (dwB & dwC) | (~dwB & dwD);
			m_dwConstant = 0x5A827999;
		}

		// 20 =< uiCount <= 39
		if(uiCount >= 20 && uiCount <= 39)
		{
			dwF = (dwB ^ dwC ^ dwD);
			m_dwConstant = 0x6ED9EBA1;
		}

		// 40 =< uiCount <= 59
		if(uiCount >= 40 && uiCount <= 59)
		{
			dwF = (dwB & dwC) | (dwB & dwD) | (dwC & dwD);
			m_dwConstant = 0x8F1BBCDC;
		}

		// 60 =< uiCount <= 79
		if(uiCount >= 60 && uiCount <= 79)
		{
			dwF = dwB ^ dwC ^ dwD;
			m_dwConstant = 0xCA62C1D6;
		}

		dwTemp = LEFT_CIRCULAR_ROTATION(dwA, 5) + dwF + dwE + m_dwBlockOfData[uiCount] + m_dwConstant;

		dwE = dwD;
		dwD = dwC;
		dwC = LEFT_CIRCULAR_ROTATION(dwB, 30);
		dwB = dwA;
		dwA = dwTemp;
	}

	m_dwMessageDigest[0] += dwA;
	m_dwMessageDigest[1] += dwB;
	m_dwMessageDigest[2] += dwC;
	m_dwMessageDigest[3] += dwD;
	m_dwMessageDigest[4] += dwE;

	// Format the accumulated message digest in hex to a string
	::swprintf(szFinalHash, L"%x%x%x%x%x", m_dwMessageDigest[0], m_dwMessageDigest[1], m_dwMessageDigest[2], m_dwMessageDigest[3], m_dwMessageDigest[4]);
	m_oHashString = szFinalHash;

	return true;
}

//---------------------------------------------------------------------------
// Name: getHashString
// Desc: Get the checksum hash string
//---------------------------------------------------------------------------
std::wstring CSHA1::getHashString() const
{
	return m_oHashString;
}

//---------------------------------------------------------------------------
// Name: 
// Desc: 
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Name: CSHA256
// Desc: Default constructor
//---------------------------------------------------------------------------
CSHA256::CSHA256()
{
	m_dwMessageDigest[0] = 0x6a09e667;
	m_dwMessageDigest[1] = 0xbb67ae85;
	m_dwMessageDigest[2] = 0x3c6ef372;
	m_dwMessageDigest[3] = 0xa54ff53a;
	m_dwMessageDigest[4] = 0x510e527f;
	m_dwMessageDigest[5] = 0x9b05688c;
	m_dwMessageDigest[6] = 0x1f83d9ab;
	m_dwMessageDigest[7] = 0x5be0cd19;
}

//---------------------------------------------------------------------------
// Name: ~CSHA256
// Desc: 
//---------------------------------------------------------------------------
CSHA256::~CSHA256()
{
}

//---------------------------------------------------------------------------
// Name: compute
// Desc: Compute the SHA256 hash
//---------------------------------------------------------------------------
bool CSHA256::compute(std::string oDataString)
{
	////////////////////////////////////////////////////////////////
	// SHA-256
	////////////////////////////////////////////////////////////////
	// Round constants
	static DWORD dwSHA256RoundConstants[] =
	{
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	return true;
}

//---------------------------------------------------------------------------
// Name: getHashString
// Desc: Get the hash string
//---------------------------------------------------------------------------
std::wstring CSHA256::getHashString() const
{
	return m_oHashString;
}