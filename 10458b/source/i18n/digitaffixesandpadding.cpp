/*
 * Copyright (C) 2015, International Business Machines
 * Corporation and others.  All Rights Reserved.
 *
 * file name: digitformatter.cpp
 */

#include "digitaffixesandpadding.h"

#include "digitlst.h"
#include "digitaffix.h"
#include "valueformatter.h"
#include "uassert.h"
#include "charstr.h"

U_NAMESPACE_BEGIN

UBool
DigitAffixesAndPadding::needsPluralRules() const {
    return (
            fPositivePrefix.hasMultipleVariants() ||
            fPositiveSuffix.hasMultipleVariants() ||
            fNegativePrefix.hasMultipleVariants() ||
            fNegativeSuffix.hasMultipleVariants());
}

UnicodeString &
DigitAffixesAndPadding::formatInt32(
        int32_t value,
        const ValueFormatter &formatter,
        FieldPositionHandler &handler,
        const PluralRules *optPluralRules,
        UnicodeString &appendTo,
        UErrorCode &status) const {
    if (U_FAILURE(status)) {
        return appendTo;
    }
    if (optPluralRules != NULL || fWidth > 0 || !formatter.isFastFormattable(value)) {
        DigitList digitList;
        digitList.set(value);
        return format(
                digitList,
                formatter,
                handler,
                optPluralRules,
                appendTo,
                status);
    }
    UBool bPositive = value >= 0;
    const DigitAffix *prefix = bPositive ? &fPositivePrefix.getOtherVariant() : &fNegativePrefix.getOtherVariant();
    const DigitAffix *suffix = bPositive ? &fPositiveSuffix.getOtherVariant() : &fNegativeSuffix.getOtherVariant();
    if (value < 0) {
        value = -value;
    }
    prefix->format(handler, appendTo);
    formatter.formatInt32(value, handler, appendTo);
    return suffix->format(handler, appendTo);
}

UnicodeString &
DigitAffixesAndPadding::format(
        DigitList &value,
        const ValueFormatter &formatter,
        FieldPositionHandler &handler,
        const PluralRules *optPluralRules,
        UnicodeString &appendTo,
        UErrorCode &status) const {
    formatter.round(value, status);
    if (U_FAILURE(status)) {
        return appendTo;
    }
    UBool bPositive = value.isPositive();
    const PluralAffix *pluralPrefix = bPositive ? &fPositivePrefix : &fNegativePrefix;
    const PluralAffix *pluralSuffix = bPositive ? &fPositiveSuffix : &fNegativeSuffix;
    
    const DigitAffix *prefix;
    const DigitAffix *suffix;
    if (optPluralRules == NULL) {
        prefix = &pluralPrefix->getOtherVariant();
        suffix = &pluralSuffix->getOtherVariant();
    } else {
        UnicodeString count(formatter.select(*optPluralRules, value));
        prefix = &pluralPrefix->getByVariant(count);
        suffix = &pluralSuffix->getByVariant(count);
    }
    value.setPositive(TRUE);
    if (fWidth <= 0) {
        prefix->format(handler, appendTo);
        formatter.format(value, handler, appendTo);
        return suffix->format(handler, appendTo);
    }
    int32_t codePointCount = prefix->countChar32() + formatter.countChar32(value) + suffix->countChar32();
    int32_t paddingCount = fWidth - codePointCount;
    switch (fPadPosition) {
    case kPadBeforePrefix:
        appendPadding(paddingCount, appendTo);
        prefix->format(handler, appendTo);
        formatter.format(value, handler, appendTo);
        return suffix->format(handler, appendTo);
    case kPadAfterPrefix:
        prefix->format(handler, appendTo);
        appendPadding(paddingCount, appendTo);
        formatter.format(value, handler, appendTo);
        return suffix->format(handler, appendTo);
    case kPadBeforeSuffix:
        prefix->format(handler, appendTo);
        formatter.format(value, handler, appendTo);
        appendPadding(paddingCount, appendTo);
        return suffix->format(handler, appendTo);
    case kPadAfterSuffix:
        prefix->format(handler, appendTo);
        formatter.format(value, handler, appendTo);
        suffix->format(handler, appendTo);
        return appendPadding(paddingCount, appendTo);
    default:
        U_ASSERT(FALSE);
        return appendTo;
    }
}

UnicodeString &
DigitAffixesAndPadding::appendPadding(int32_t paddingCount, UnicodeString &appendTo) const {
    for (int32_t i = 0; i < paddingCount; ++i) {
        appendTo.append(fPadChar);
    }
    return appendTo;
}


U_NAMESPACE_END

