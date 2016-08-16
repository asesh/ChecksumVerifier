// App.h: 
// Copyright (C) 2011 Asesh Shrestha. All rights reserved

#ifndef APP_H
#define APP_H

// CApp class
class CApp
{
public:
	CApp(); // Default constructor
	~CApp(); // Destructor

	void setInstance(HINSTANCE hInstance);

	HINSTANCE getInstance() const;

private:
	HINSTANCE m_hInstance;
};

extern CApp *g_pApp;

#endif // APP_H