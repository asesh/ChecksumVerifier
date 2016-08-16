// MD5.h: 
// Copyright (C) 2011 Asesh Shrestha. All rights reserved

#ifndef MD5_H
#define MD5_H

// CMD5 class
class CMD5
{
public:
	CMD5(); // Default constructor
	~CMD5(); // Destructor

	bool compute(BYTE *pbArray); // Compute the MD5 hash

private:
};

#endif // MD5_H