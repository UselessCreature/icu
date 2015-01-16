/*
 * Copyright (C) 2015, International Business Machines
 * Corporation and others.  All Rights Reserved.
 *
 * file name: sciformatter.cpp
 */

#include "unicode/utypes.h"

#include "sciformatter.h"
#include "digitformatter.h"
#include "digitgrouping.h"
#include "unicode/dcfmtsym.h"
#include "unicode/unum.h"
#include "fphdlimp.h"

U_NAMESPACE_BEGIN

SciFormatter::SciFormatter() : fExponent("E") {
}

SciFormatter::SciFormatter(const DecimalFormatSymbols &symbols) {
    setDecimalFormatSymbols(symbols);
}

void
SciFormatter::setDecimalFormatSymbols(
        const DecimalFormatSymbols &symbols) {
fExponent = symbols.getConstSymbol(DecimalFormatSymbols::kExponentialSymbol);
}

UnicodeString &
SciFormatter::format(
        const DigitList &positiveMantissa,
        int32_t exponent,
        const DigitFormatter &formatter,
        const DigitInterval &mantissaInterval,
        UBool alwaysShowDecimal,
        int32_t minExponentDigits,
        UBool alwaysShowExponentSign,
        FieldPositionHandler &handler,
        UnicodeString &appendTo) const {
    DigitGrouping grouping;
    formatter.format(
            positiveMantissa,
            grouping,
            mantissaInterval,
            alwaysShowDecimal,
            handler,
            appendTo);
    int32_t expBegin = appendTo.length();
    appendTo.append(fExponent);
    handler.addAttribute(
            UNUM_EXPONENT_SYMBOL_FIELD, expBegin, appendTo.length());
    return formatter.formatInt32(
            exponent,
            minExponentDigits,
            alwaysShowExponentSign,
            UNUM_EXPONENT_SIGN_FIELD,
            UNUM_EXPONENT_FIELD,
            handler,
            appendTo);
}

int32_t
SciFormatter::countChar32(
        int32_t exponent,
        const DigitFormatter &formatter,
        const DigitInterval &mantissaInterval,
        UBool alwaysShowDecimal,
        int32_t minExponentDigits,
        UBool alwaysShowExponentSign) const {
    DigitGrouping grouping;
    int32_t count = formatter.countChar32(
            grouping, mantissaInterval, alwaysShowDecimal);
    count += fExponent.countChar32();
    count += formatter.countChar32ForInt(
            exponent, minExponentDigits, alwaysShowExponentSign);
    return count;
}


U_NAMESPACE_END

