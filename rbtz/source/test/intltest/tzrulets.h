/*
*******************************************************************************
* Copyright (C) 2007, International Business Machines Corporation and         *
* others. All Rights Reserved.                                                *
*******************************************************************************
*/

#ifndef _TIMEZONERULETEST_
#define _TIMEZONERULETEST_

#include "unicode/utypes.h"
#include "caltztst.h"

#if !UCONFIG_NO_FORMATTING

/**
 * Tests for TimeZoneRule, RuleBasedTimeZone and VTimeZone
 */
class TimeZoneRuleTest : public CalendarTimeZoneTest {
    // IntlTest override
    void runIndexedTest(int32_t index, UBool exec, const char*& name, char* par);
public:
    void TestSimpleRuleBasedTimeZone(void);
    void TestHistoricalRuleBasedTimeZone(void);
    void TestOlsonTransition(void);
    void TestRBTZTransition(void);
    void TestHasEquivalentTransitions(void);
    void TestVTimeZoneRoundTrip(void);
    void TestVTimeZoneRoundTripPartial(void);
    void TestVTimeZoneSimpleWrite(void);
    void TestVTimeZoneHeaderProps(void);
    void TestGetSimpleRules(void);

private:
    UDate getUTCMillis(int32_t year, int32_t month, int32_t dom,
        int32_t hour=0, int32_t min=0, int32_t sec=0, int32_t msec=0);
};

#endif /* #if !UCONFIG_NO_FORMATTING */
 
#endif // _TIMEZONERULETEST_
