/*
**********************************************************************
*   Copyright (C) 2016 and later: Unicode, Inc. and others.
*   License & terms of use: http://www.unicode.org/copyright.html
**********************************************************************
*   Date        Name        Description
*   05/24/01    aliu        Creation.
**********************************************************************
*/

#include "unicode/utypes.h"

#if !UCONFIG_NO_TRANSLITERATION

#include "unicode/ustring.h"
#include "unicode/uchar.h"
#include "toupptrn.h"
#include "ustr_imp.h"
#include "cpputils.h"

U_NAMESPACE_BEGIN

UOBJECT_DEFINE_RTTI_IMPLEMENTATION(UppercaseTransliterator)

/**
 * Constructs a transliterator.
 */
UppercaseTransliterator::UppercaseTransliterator() :
    CaseMapTransliterator(UNICODE_STRING("Any-Upper", 9), ucase_toFullUpper)
{
}

/**
 * Destructor.
 */
UppercaseTransliterator::~UppercaseTransliterator() {
}

/**
 * Copy constructor.
 */
UppercaseTransliterator::UppercaseTransliterator(const UppercaseTransliterator& o) :
    CaseMapTransliterator(o)
{
}

/**
 * Assignment operator.
 */
/*UppercaseTransliterator& UppercaseTransliterator::operator=(
                             const UppercaseTransliterator& o) {
    CaseMapTransliterator::operator=(o);
    return *this;
}*/

/**
 * Transliterator API.
 */
Transliterator* UppercaseTransliterator::clone(void) const {
    return new UppercaseTransliterator(*this);
}

U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_TRANSLITERATION */
