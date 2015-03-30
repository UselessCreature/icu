/*
*******************************************************************************
* Copyright (C) 2015, International Business Machines
* Corporation and others.  All Rights Reserved.
*******************************************************************************
* affixpatternparser.h
*
* created on: 2015jan06
* created by: Travis Keep
*/

#ifndef __AFFIX_PATTERN_PARSER_H__
#define __AFFIX_PATTERN_PARSER_H__

#include "unicode/utypes.h"
#include "unicode/uobject.h"
#include "unicode/unistr.h"
#include "pluralaffix.h"

U_NAMESPACE_BEGIN

class PluralRules;
class FixedPrecision;
class DecimalFormatSymbols;

/**
 * A representation of the various forms of a particular currency according
 * to some locale and usage context.
 * 
 * Includes the symbol, ISO code form, and long form(s) of the currency name
 * for each plural variation.
 */
class U_I18N_API CurrencyAffixInfo : public UMemory {
public:
    /**
     * Symbol is \u00a4; ISO form is \u00a4\u00a4;
     *  long form is \u00a4\u00a4\u00a4.
     */
    CurrencyAffixInfo();

    /**
     * The symbol form of the currency.
     */
    UnicodeString fSymbol;

    /**
     * The ISO form of the currency, usually three letter abbreviation.
     */
    UnicodeString fISO;

    /**
     * The long forms of the currency keyed by plural variation.
     */
    PluralAffix fLong;

    UBool
    equals(const CurrencyAffixInfo &other) const {
        return (fSymbol == other.fSymbol)
                && (fISO == other.fISO)
                && (fLong.equals(other.fLong));
    }

    /**
     * Intializes this instance.
     *
     * @param locale the locale for the currency forms.
     * @param rules The plural rules for the locale. 
     * @param currency the null terminated, 3 character ISO code of the
     * currency. If NULL, resets this instance as if it were just created.
     * In this case, the first 2 parameters may be NULL as well.
     * @param status any error returned here.
     */
    void set(
            const char *locale, const PluralRules *rules,
            const UChar *currency, UErrorCode &status);

    /**
     * Returns true if this instance is the default. That is has no real
     * currency. For instance never initialized with set()
     * or reset with set(NULL, NULL, NULL, status).
     */
    UBool isDefault() const;

    /**
     * Adjusts the precision used for a particular currency.
     * @param currency the null terminated, 3 character ISO code of the
     * currency.
     * @param usage the usage of the currency
     * @param precision min/max fraction digits and rounding increment
     *  adjusted.
     * @params status any error reported here.
     */
    static void adjustPrecision(
            const UChar *currency, const UCurrencyUsage usage,
            FixedPrecision &precision, UErrorCode &status);
};

class AffixPatternIterator;

/**
 * A locale agnostic representation of an affix pattern.
 */
class U_I18N_API AffixPattern : public UMemory {
public:

    /**
     * The token types that can appear in an affix pattern.
     */
    enum ETokenType {
        kLiteral,
        kPercent,
        kPerMill,
        kCurrency,
        kNegative
    };

    /**
     * An empty affix pattern.
     */
    AffixPattern()
            : tokens(), literals(), hasCurrencyToken(FALSE),
              hasPercentToken(FALSE), hasPermillToken(FALSE),  char32Count(0) {
    }

    /**
     * Adds a string literal to this affix pattern.
     */
    void addLiteral(const UChar *, int32_t start, int32_t len);

    /**
     * Adds a token to this affix pattern. t must not be kLiteral as
     * the addLiteral() method adds literals. 
     * @param t the token type to add
     */
    void add(ETokenType t);

    /**
     * Adds a currency token with specific count to this affix pattern.
     * @param count the token count. Used to distinguish between
     *  one, two, or three currency symbols. Note that adding a currency
     *  token with count=2 (Use ISO code) is different than adding two
     *  currency tokens each with count=1 (two currency symbols).
     */
    void addCurrency(uint8_t count);

    /**
     * Makes this instance be an empty affix pattern.
     */
    void remove();

    /**
     * Provides an iterator over the tokens in this instance.
     * @param result this is initialized to point just before the
     *   first token of this instance. Caller must call nextToken()
     *   on the iterator once it is set up to have it actually point
     *   to the first token. This first call to nextToken() will return
     *   FALSE if the AffixPattern being iterated over is empty.
     * @return result
     */
    AffixPatternIterator &iterator(AffixPatternIterator &result) const;

    /**
     * Returns TRUE if this instance has currency tokens in it.
     */
    UBool usesCurrency() const {
        return hasCurrencyToken;
    }

    UBool usesPercent() const {
        return hasPercentToken;
    }

    UBool usesPermill() const {
        return hasPermillToken;
    }

    /**
     * Returns the number of code points a string of this instance
     * would have if none of the special tokens were escaped.
     * Used to compute the padding size.
     */
    int32_t countChar32() const {
        return char32Count;
    }


    /**
     * Parses an affix pattern string appending it to an AffixPattern.
     * Parses affix pattern strings produced from using
     * DecimalFormatPatternParser to parse a format pattern. Affix patterns
     * include the positive prefix and suffix and the negative prefix
     * and suffix. This method expects affix patterns strings to be in the
     * same format that DecimalFormatPatternParser produces. Namely special
     * characters in the affix that correspond to a field type must be
     * prefixed with an apostrophe ('). These special character sequences
     * inluce minus (-), percent (%), permile (U+2030),
     * short currency (U+00a4), medium currency (u+00a4 * 2),
     * long currency (u+a4 * 3), and apostrophe (')
     * (apostrophe does not correspond to a field type but has to be escaped
     * because it itself is the escape character).
     * Since the expansion of these special character
     * sequences is locale dependent, these sequences are not expanded in
     * an AffixPattern instance.
     * If these special characters are not prefixed with an apostrophe in
     * the affix pattern string, then they are treated verbatim just as
     * any other character. If an apostrophe prefixes a non special
     * character in the affix pattern, the apostrophe is simply ignored.
     *
     * @param affixStr the string from DecimalFormatPatternParser
     * @param appendTo parsed result appended here.
     * @param status any error parsing returned here.
     */
    static AffixPattern &parseAffixString(
            const UnicodeString &affixStr,
            AffixPattern &appendTo,
            UErrorCode &status);

    UBool equals(const AffixPattern &other) const {
        return (tokens == other.tokens)
                && (literals == other.literals)
                && (hasCurrencyToken == other.hasCurrencyToken)
                && (hasPercentToken == other.hasPercentToken)
                && (hasPermillToken == other.hasPermillToken)
                && (char32Count == other.char32Count);
    }

private:
    UnicodeString tokens;
    UnicodeString literals;
    UBool hasCurrencyToken;
    UBool hasPercentToken;
    UBool hasPermillToken;
    int32_t char32Count;
    void add(ETokenType t, uint8_t count);

};

/**
 * An iterator over the tokens in an AffixPattern instance.
 */
class U_I18N_API AffixPatternIterator : public UMemory {
public:

    /**
     * Using an iterator without first calling iterator on an AffixPattern
     * instance to initialize the iterator results in
     * undefined behavior.
     */
    AffixPatternIterator() : nextLiteralIndex(0), nextTokenIndex(0), tokens(NULL), literals(NULL) { }
    /**
     * Advances this iterator to the next token. Returns FALSE when there
     * are no more tokens. Calling the other methods after nextToken()
     * returns FALSE results in undefined behavior.
     */ 
    UBool nextToken();

    /**
     * Returns the type of token.
     */
    AffixPattern::ETokenType getTokenType() const;

    /**
     * For literal tokens, returns the literal string. Calling this for
     * other token types results in undefined behavior.
     * @param result replaced with a read-only alias to the literal string.
     * @return result
     */
    UnicodeString &getLiteral(UnicodeString &result) const;

    /**
     * Returns the token length. Usually 1, but for currency tokens may
     * be 2 for ISO code and 3 for long form.
     */
    int32_t getTokenLength() const;
private:
    int32_t nextLiteralIndex;
    int32_t nextTokenIndex;
    const UnicodeString *tokens;
    const UnicodeString *literals;
    friend class AffixPattern;
    AffixPatternIterator(const AffixPatternIterator &);
    AffixPatternIterator &operator=(const AffixPatternIterator &);
};

/**
 * A locale aware class that converts locale independent AffixPattern
 * instances into locale dependent PluralAffix instances.
 */
class U_I18N_API AffixPatternParser : public UMemory {
public:
AffixPatternParser();
AffixPatternParser(const DecimalFormatSymbols &symbols);
void setDecimalFormatSymbols(const DecimalFormatSymbols &symbols);

/**
 * Contains the currency forms. Only needs to be initialized if the affix
 * patterns being parsed contain the currency symbol (U+00a4).
 */
CurrencyAffixInfo fCurrencyAffixInfo;

/**
 * Parses affixPattern appending the result to appendTo.
 * @param affixPattern The affix pattern.
 * @param appendTo The result of parsing affixPattern is appended here.
 * @param status any error returned here.
 * @return 2 if there is a percent symbol in affixPattern; 3 if there
 *   is a permill symbol; 0 otherwise.
 */
int32_t parse(
        const AffixPattern &affixPattern,
        PluralAffix &appendTo,
        UErrorCode &status) const;

UBool equals(const AffixPatternParser &other) const {
    return (fPercent == other.fPercent)
            && (fPermill == other.fPermill)
            && (fNegative == other.fNegative);
}

private:
UnicodeString fPercent;
UnicodeString fPermill;
UnicodeString fNegative;
};


U_NAMESPACE_END

#endif  // __AFFIX_PATTERN_PARSER_H__
