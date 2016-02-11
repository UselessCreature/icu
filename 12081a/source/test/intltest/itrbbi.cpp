/*
**********************************************************************
* Copyright (C) 1998-2012, International Business Machines Corporation 
* and others.  All Rights Reserved.
**********************************************************************
*/
/***********************************************************************
*   Date        Name        Description
*   12/14/99    Madhu        Creation.
***********************************************************************/
/**
 * IntlTestRBBI is the medium level test class for RuleBasedBreakIterator
 */

#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION

#include "intltest.h"
#include "itrbbi.h"
#include "rbbiapts.h"
#include "rbbitst.h"


void IntlTestRBBI::runIndexedTest( int32_t index, UBool exec, const char* &name, char* par )
{
    if (exec) {
        logln("TestSuite RuleBasedBreakIterator: ");
    }
    TESTCASE_AUTO_BEGIN;
    TESTCASE_AUTO_CLASS(RBBIAPITest);
    TESTCASE_AUTO_CLASS(RBBITest);
    TESTCASE_AUTO_CLASS(RBBIMonkeyTest);
    TESTCASE_AUTO_END;
}

#endif /* #if !UCONFIG_NO_BREAK_ITERATION */
