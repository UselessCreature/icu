/*
**********************************************************************
*   Copyright (C) 2004-2004, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*   file name:  iotest.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2004apr06
*   created by: George Rhoten
*/

#ifndef IOTEST_H
#define IOTEST_H 1

#include "unicode/utypes.h"
#include "unicode/ctest.h"

U_CFUNC void
addStringTest(TestNode** root);

U_CFUNC void
addFileTest(TestNode** root);

U_CDECL_BEGIN
extern const UChar NEW_LINE[];
U_CDECL_END

#define STANDARD_TEST_NUM_RANGE 1000


#endif
