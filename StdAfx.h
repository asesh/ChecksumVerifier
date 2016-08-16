//
//

#ifndef STDAFX_H
#define STDAFX_H

// Disable warning messages
#pragma warning(disable: 4996 995)

#pragma comment(lib, "imagehlp") // Image help library
#pragma comment(lib, "ole32") // COM
#pragma comment(lib, "comctl32") // Common control

#define _WIN32_DCOM
#define _WIN32_WINNT 0x600

#include <windows.h>

#include <wincrypt.h>

#include <commctrl.h>

#include <imagehlp.h>

#include <objbase.h>

#include <shlobj.h>

#include <strsafe.h>

#include <cmath>

#include <fstream>

#include <list> // list

#include <string>

#include "resource.h"

#include "Macros.h"

#include "MD5.h" // Implementation of MD5
#include "SHA.h" // Implementation of SHAs
#include "sha1.h"

#endif // STDAFX_H