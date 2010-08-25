/********************************************************************
 * COPYRIGHT:
 * Copyright (c) 2010, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
//
//   file:  alphaindex.h
//          Alphabetic Index Tests.
//

#ifndef ALPHAINDEXTST_H
#define ALPHAINDEXTST_H

#include "intltest.h"

class AlphabeticIndexTest: public IntlTest {
public:
    AlphabeticIndexTest();
    virtual ~AlphabeticIndexTest();

    virtual void runIndexedTest(int32_t index, UBool exec, const char* &name, char* par = NULL );

    virtual void APITest();
    virtual void ManyLocalesTest();
};

#endif
