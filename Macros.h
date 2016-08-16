// Macros.h: 
// Copyright (C) 2010-2011 Asesh Shrestha. All rights reserved

#ifndef MACROS_H
#define MACROS_H

#define ARRAY_SIZE(X, Y) (sizeof(X)/sizeof(Y))

#define DELETE_FREE(X) { if(X) { delete X; X = NULL; }}
#define DELETE_ARRAY_FREE(X) { if(X) { delete [] X; X = NULL; }}


#endif // MACROS_H