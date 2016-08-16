// SHA.h: 
// Copyright (C) 2011 Asesh Shrestha. All rights reserved

#ifndef SHA_H
#define SHA_H

#define LEFT_CIRCULAR_ROTATION(X, Y) ((X >> Y) | X << (32 - Y))

// CSHA1 class
class CSHA1
{
public: // Public methods
	CSHA1(); // Default constructor
	~CSHA1(); // Destructor

	bool compute(std::string oDataString); // Compute the SHA1 hash
	std::wstring getHashString() const; // Get the checksum hash string

private: // Private attributes
	DWORD m_dwMessageDigest[5], m_dwBlockOfData[80], m_dwConstant;

	std::wstring m_oHashString; // Will hold the checksum hash string
};

// CSHA256 class
class CSHA256
{
public:
	CSHA256(); // Default constructor
	~CSHA256(); // Destructor

	bool compute(std::string oDataString); // Compute the SHA-256 hash
	std::wstring getHashString() const; // Get the checksum hash string

private:
	DWORD m_dwMessageDigest[8], m_dwBlockOfData[80], m_dwConstant;

	std::wstring m_oHashString; // Will hold the checksum hash string
};

#endif // SHA_H