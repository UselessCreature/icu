/*
*******************************************************************************
*
*   Copyright (C) 2002-2007, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*   file name:  uset.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2002mar07
*   created by: Markus W. Scherer
*
*   C version of UnicodeSet.
*/


/**
 * \file
 * \brief C API: Unicode Set
 *
 * <p>This is a C wrapper around the C++ UnicodeSet class.</p>
 */

#ifndef __USET_H__
#define __USET_H__

#include "unicode/utypes.h"
#include "unicode/uchar.h"

#ifndef UCNV_H
struct USet;
/**
 * A UnicodeSet.  Use the uset_* API to manipulate.  Create with
 * uset_open*, and destroy with uset_close.
 * @stable ICU 2.4
 */
typedef struct USet USet;
#endif

/**
 * Bitmask values to be passed to uset_openPatternOptions() or
 * uset_applyPattern() taking an option parameter.
 * @stable ICU 2.4
 */
enum {
    /**
     * Ignore white space within patterns unless quoted or escaped.
     * @stable ICU 2.4
     */
    USET_IGNORE_SPACE = 1,  

    /**
     * Enable case insensitive matching.  E.g., "[ab]" with this flag
     * will match 'a', 'A', 'b', and 'B'.  "[^ab]" with this flag will
     * match all except 'a', 'A', 'b', and 'B'. This performs a full
     * closure over case mappings, e.g. U+017F for s.
     *
     * The resulting set is a superset of the input for the code points but
     * not for the strings.
     * It performs a case mapping closure of the code points and adds
     * full case folding strings for the code points, and reduces strings of
     * the original set to their full case folding equivalents.
     *
     * This is designed for case-insensitive matches, for example
     * in regular expressions. The full code point case closure allows checking of
     * an input character directly against the closure set.
     * Strings are matched by comparing the case-folded form from the closure
     * set with an incremental case folding of the string in question.
     *
     * The closure set will also contain single code points if the original
     * set contained case-equivalent strings (like U+00DF for "ss" or "Ss" etc.).
     * This is not necessary (that is, redundant) for the above matching method
     * but results in the same closure sets regardless of whether the original
     * set contained the code point or a string.
     *
     * @stable ICU 2.4
     */
    USET_CASE_INSENSITIVE = 2,  

    /**
     * Enable case insensitive matching.  E.g., "[ab]" with this flag
     * will match 'a', 'A', 'b', and 'B'.  "[^ab]" with this flag will
     * match all except 'a', 'A', 'b', and 'B'. This adds the lower-,
     * title-, and uppercase mappings as well as the case folding
     * of each existing element in the set.
     * @stable ICU 3.2
     */
    USET_ADD_CASE_MAPPINGS = 4,
    
    /**
     * Enough for any single-code point set
     * @internal
     */
    USET_SERIALIZED_STATIC_ARRAY_CAPACITY=8
};

#ifndef U_HIDE_DRAFT_API

/**
 * The two values for whether span() and similar functions continue while
 * the current character is contained vs. not contained in the set.
 * More readable than using TRUE and FALSE, but the numeric values are the same
 * as TRUE and FALSE which are returned from contains().
 * @draft ICU 3.8
 */
enum USetSpanCondition {
    /**
     * Continue a span() while the current character is not contained in the set
     * (while contains(current)==FALSE).
     * When span() returns, the substring between where it started and the position
     * it returned consists only of characters that are not in the set.
     * @draft ICU 3.8
     */
    USET_SPAN_WHILE_NOT_CONTAINED = 0,
    /**
     * Continue a span() while the current character is contained in the set
     * (while contains(current)==TRUE).
     * When span() returns, the substring between where it started and the position
     * it returned consists only of characters that are in the set.
     * @draft ICU 3.8
     */
    USET_SPAN_WHILE_CONTAINED = 1
};
typedef enum USetSpanCondition USetSpanCondition;

#endif /* U_HIDE_DRAFT_API */

/**
 * A serialized form of a Unicode set.  Limited manipulations are
 * possible directly on a serialized set.  See below.
 * @stable ICU 2.4
 */
typedef struct USerializedSet {
    /**
     * The serialized Unicode Set.
     * @stable ICU 2.4
     */
    const uint16_t *array;
    /**
     * The length of the array that contains BMP characters.
     * @stable ICU 2.4
     */
    int32_t bmpLength;
    /**
     * The total length of the array.
     * @stable ICU 2.4
     */
    int32_t length;
    /**
     * A small buffer for the array to reduce memory allocations.
     * @stable ICU 2.4
     */
    uint16_t staticArray[USET_SERIALIZED_STATIC_ARRAY_CAPACITY];
} USerializedSet;

/*********************************************************************
 * USet API
 *********************************************************************/

/**
 * Creates a USet object that contains the range of characters
 * start..end, inclusive.
 * @param start first character of the range, inclusive
 * @param end last character of the range, inclusive
 * @return a newly created USet.  The caller must call uset_close() on
 * it when done.
 * @stable ICU 2.4
 */
U_STABLE USet* U_EXPORT2
uset_open(UChar32 start, UChar32 end);

/**
 * Creates a set from the given pattern.  See the UnicodeSet class
 * description for the syntax of the pattern language.
 * @param pattern a string specifying what characters are in the set
 * @param patternLength the length of the pattern, or -1 if null
 * terminated
 * @param ec the error code
 * @stable ICU 2.4
 */
U_STABLE USet* U_EXPORT2
uset_openPattern(const UChar* pattern, int32_t patternLength,
                 UErrorCode* ec);

/**
 * Creates a set from the given pattern.  See the UnicodeSet class
 * description for the syntax of the pattern language.
 * @param pattern a string specifying what characters are in the set
 * @param patternLength the length of the pattern, or -1 if null
 * terminated
 * @param options bitmask for options to apply to the pattern.
 * Valid options are USET_IGNORE_SPACE and USET_CASE_INSENSITIVE.
 * @param ec the error code
 * @stable ICU 2.4
 */
U_STABLE USet* U_EXPORT2
uset_openPatternOptions(const UChar* pattern, int32_t patternLength,
                 uint32_t options,
                 UErrorCode* ec);

/**
 * Disposes of the storage used by a USet object.  This function should
 * be called exactly once for objects returned by uset_open().
 * @param set the object to dispose of
 * @stable ICU 2.4
 */
U_STABLE void U_EXPORT2
uset_close(USet* set);

#ifndef U_HIDE_DRAFT_API

/**
 * Returns a copy of this object.
 * If this set is frozen, then the clone will be frozen as well.
 * Use uset_cloneAsThawed() for a mutable clone of a frozen set.
 * @param set the original set
 * @return the newly allocated copy of the set
 * @see uset_cloneAsThawed
 * @draft ICU 3.8
 */
U_DRAFT USet * U_EXPORT2
uset_clone(const USet *set);

//----------------------------------------------------------------
// Freezable API
//----------------------------------------------------------------

/**
 * Determines whether the set has been frozen (made immutable) or not.
 * See the ICU4J Freezable interface for details.
 * @param set the set
 * @return TRUE/FALSE for whether the set has been frozen
 * @see uset_freeze
 * @see uset_cloneAsThawed
 * @draft ICU 3.8
 */
U_DRAFT UBool U_EXPORT2
uset_isFrozen(const USet *set);

/**
 * Freeze the set (make it immutable).
 * Once frozen, it cannot be unfrozen and is therefore thread-safe
 * until it is deleted.
 * See the ICU4J Freezable interface for details.
 * Freezing the set may also make some operations faster, for example
 * uset_contains() and uset_span().
 * A frozen set will not be modified. (It remains frozen.)
 * @param set the set
 * @return the same set, now frozen
 * @see uset_isFrozen
 * @see uset_cloneAsThawed
 * @draft ICU 3.8
 */
U_DRAFT void U_EXPORT2
uset_freeze(USet *set);

/**
 * Clone the set and make the clone mutable.
 * See the ICU4J Freezable interface for details.
 * @param set the set
 * @return the mutable clone
 * @see uset_freeze
 * @see uset_isFrozen
 * @see uset_clone
 * @draft ICU 3.8
 */
U_DRAFT USet * U_EXPORT2
uset_cloneAsThawed(const USet *set);

#endif /* U_HIDE_DRAFT_API */

/**
 * Causes the USet object to represent the range <code>start - end</code>.
 * If <code>start > end</code> then this USet is set to an empty range.
 * A frozen set will not be modified.
 * @param set the object to set to the given range
 * @param start first character in the set, inclusive
 * @param end last character in the set, inclusive
 * @stable ICU 3.2
 */
U_STABLE void U_EXPORT2
uset_set(USet* set,
         UChar32 start, UChar32 end);

/**
 * Modifies the set to represent the set specified by the given
 * pattern. See the UnicodeSet class description for the syntax of 
 * the pattern language. See also the User Guide chapter about UnicodeSet.
 * <em>Empties the set passed before applying the pattern.</em>
 * A frozen set will not be modified.
 * @param set               The set to which the pattern is to be applied. 
 * @param pattern           A pointer to UChar string specifying what characters are in the set.
 *                          The character at pattern[0] must be a '['.
 * @param patternLength     The length of the UChar string. -1 if NUL terminated.
 * @param options           A bitmask for options to apply to the pattern.
 *                          Valid options are USET_IGNORE_SPACE and USET_CASE_INSENSITIVE.
 * @param status            Returns an error if the pattern cannot be parsed.
 * @return                  Upon successful parse, the value is either
 *                          the index of the character after the closing ']' 
 *                          of the parsed pattern.
 *                          If the status code indicates failure, then the return value 
 *                          is the index of the error in the source.
 *                                  
 * @stable ICU 2.8
 */
U_STABLE int32_t U_EXPORT2 
uset_applyPattern(USet *set,
                  const UChar *pattern, int32_t patternLength,
                  uint32_t options,
                  UErrorCode *status);

/**
 * Modifies the set to contain those code points which have the given value
 * for the given binary or enumerated property, as returned by
 * u_getIntPropertyValue.  Prior contents of this set are lost.
 * A frozen set will not be modified.
 *
 * @param set the object to contain the code points defined by the property
 *
 * @param prop a property in the range UCHAR_BIN_START..UCHAR_BIN_LIMIT-1
 * or UCHAR_INT_START..UCHAR_INT_LIMIT-1
 * or UCHAR_MASK_START..UCHAR_MASK_LIMIT-1.
 *
 * @param value a value in the range u_getIntPropertyMinValue(prop)..
 * u_getIntPropertyMaxValue(prop), with one exception.  If prop is
 * UCHAR_GENERAL_CATEGORY_MASK, then value should not be a UCharCategory, but
 * rather a mask value produced by U_GET_GC_MASK().  This allows grouped
 * categories such as [:L:] to be represented.
 *
 * @param ec error code input/output parameter
 *
 * @stable ICU 3.2
 */
U_STABLE void U_EXPORT2
uset_applyIntPropertyValue(USet* set,
                           UProperty prop, int32_t value, UErrorCode* ec);

/**
 * Modifies the set to contain those code points which have the
 * given value for the given property.  Prior contents of this
 * set are lost.
 * A frozen set will not be modified.
 *
 * @param set the object to contain the code points defined by the given
 * property and value alias
 *
 * @param prop a string specifying a property alias, either short or long.
 * The name is matched loosely.  See PropertyAliases.txt for names and a
 * description of loose matching.  If the value string is empty, then this
 * string is interpreted as either a General_Category value alias, a Script
 * value alias, a binary property alias, or a special ID.  Special IDs are
 * matched loosely and correspond to the following sets:
 *
 * "ANY" = [\\u0000-\\U0010FFFF],
 * "ASCII" = [\\u0000-\\u007F],
 * "Assigned" = [:^Cn:].
 *
 * @param propLength the length of the prop, or -1 if NULL
 *
 * @param value a string specifying a value alias, either short or long.
 * The name is matched loosely.  See PropertyValueAliases.txt for names
 * and a description of loose matching.  In addition to aliases listed,
 * numeric values and canonical combining classes may be expressed
 * numerically, e.g., ("nv", "0.5") or ("ccc", "220").  The value string
 * may also be empty.
 *
 * @param valueLength the length of the value, or -1 if NULL
 *
 * @param ec error code input/output parameter
 *
 * @stable ICU 3.2
 */
U_STABLE void U_EXPORT2
uset_applyPropertyAlias(USet* set,
                        const UChar *prop, int32_t propLength,
                        const UChar *value, int32_t valueLength,
                        UErrorCode* ec);

/**
 * Return true if the given position, in the given pattern, appears
 * to be the start of a UnicodeSet pattern.
 *
 * @param pattern a string specifying the pattern
 * @param patternLength the length of the pattern, or -1 if NULL
 * @param pos the given position
 * @stable ICU 3.2
 */
U_STABLE UBool U_EXPORT2
uset_resemblesPattern(const UChar *pattern, int32_t patternLength,
                      int32_t pos);

/**
 * Returns a string representation of this set.  If the result of
 * calling this function is passed to a uset_openPattern(), it
 * will produce another set that is equal to this one.
 * @param set the set
 * @param result the string to receive the rules, may be NULL
 * @param resultCapacity the capacity of result, may be 0 if result is NULL
 * @param escapeUnprintable if TRUE then convert unprintable
 * character to their hex escape representations, \\uxxxx or
 * \\Uxxxxxxxx.  Unprintable characters are those other than
 * U+000A, U+0020..U+007E.
 * @param ec error code.
 * @return length of string, possibly larger than resultCapacity
 * @stable ICU 2.4
 */
U_STABLE int32_t U_EXPORT2
uset_toPattern(const USet* set,
               UChar* result, int32_t resultCapacity,
               UBool escapeUnprintable,
               UErrorCode* ec);

/**
 * Adds the given character to the given USet.  After this call,
 * uset_contains(set, c) will return TRUE.
 * A frozen set will not be modified.
 * @param set the object to which to add the character
 * @param c the character to add
 * @stable ICU 2.4
 */
U_STABLE void U_EXPORT2
uset_add(USet* set, UChar32 c);

/**
 * Adds all of the elements in the specified set to this set if
 * they're not already present.  This operation effectively
 * modifies this set so that its value is the <i>union</i> of the two
 * sets.  The behavior of this operation is unspecified if the specified
 * collection is modified while the operation is in progress.
 * A frozen set will not be modified.
 *
 * @param set the object to which to add the set
 * @param additionalSet the source set whose elements are to be added to this set.
 * @stable ICU 2.6
 */
U_STABLE void U_EXPORT2
uset_addAll(USet* set, const USet *additionalSet);

/**
 * Adds the given range of characters to the given USet.  After this call,
 * uset_contains(set, start, end) will return TRUE.
 * A frozen set will not be modified.
 * @param set the object to which to add the character
 * @param start the first character of the range to add, inclusive
 * @param end the last character of the range to add, inclusive
 * @stable ICU 2.2
 */
U_STABLE void U_EXPORT2
uset_addRange(USet* set, UChar32 start, UChar32 end);

/**
 * Adds the given string to the given USet.  After this call,
 * uset_containsString(set, str, strLen) will return TRUE.
 * A frozen set will not be modified.
 * @param set the object to which to add the character
 * @param str the string to add
 * @param strLen the length of the string or -1 if null terminated.
 * @stable ICU 2.4
 */
U_STABLE void U_EXPORT2
uset_addString(USet* set, const UChar* str, int32_t strLen);

/**
 * Adds each of the characters in this string to the set. Thus "ch" => {"c", "h"}
 * If this set already any particular character, it has no effect on that character.
 * A frozen set will not be modified.
 * @param set the object to which to add the character
 * @param str the source string
 * @param strLen the length of the string or -1 if null terminated.
 * @draft ICU 3.4
 */
U_DRAFT void U_EXPORT2
uset_addAllCodePoints(USet* set, const UChar *str, int32_t strLen);

/**
 * Removes the given character from the given USet.  After this call,
 * uset_contains(set, c) will return FALSE.
 * A frozen set will not be modified.
 * @param set the object from which to remove the character
 * @param c the character to remove
 * @stable ICU 2.4
 */
U_STABLE void U_EXPORT2
uset_remove(USet* set, UChar32 c);

/**
 * Removes the given range of characters from the given USet.  After this call,
 * uset_contains(set, start, end) will return FALSE.
 * A frozen set will not be modified.
 * @param set the object to which to add the character
 * @param start the first character of the range to remove, inclusive
 * @param end the last character of the range to remove, inclusive
 * @stable ICU 2.2
 */
U_STABLE void U_EXPORT2
uset_removeRange(USet* set, UChar32 start, UChar32 end);

/**
 * Removes the given string to the given USet.  After this call,
 * uset_containsString(set, str, strLen) will return FALSE.
 * A frozen set will not be modified.
 * @param set the object to which to add the character
 * @param str the string to remove
 * @param strLen the length of the string or -1 if null terminated.
 * @stable ICU 2.4
 */
U_STABLE void U_EXPORT2
uset_removeString(USet* set, const UChar* str, int32_t strLen);

/**
 * Removes from this set all of its elements that are contained in the
 * specified set.  This operation effectively modifies this
 * set so that its value is the <i>asymmetric set difference</i> of
 * the two sets.
 * A frozen set will not be modified.
 * @param set the object from which the elements are to be removed
 * @param removeSet the object that defines which elements will be
 * removed from this set
 * @stable ICU 3.2
 */
U_STABLE void U_EXPORT2
uset_removeAll(USet* set, const USet* removeSet);

/**
 * Retain only the elements in this set that are contained in the
 * specified range.  If <code>start > end</code> then an empty range is
 * retained, leaving the set empty.  This is equivalent to
 * a boolean logic AND, or a set INTERSECTION.
 * A frozen set will not be modified.
 *
 * @param set the object for which to retain only the specified range
 * @param start first character, inclusive, of range to be retained
 * to this set.
 * @param end last character, inclusive, of range to be retained
 * to this set.
 * @stable ICU 3.2
 */
U_STABLE void U_EXPORT2
uset_retain(USet* set, UChar32 start, UChar32 end);

/**
 * Retains only the elements in this set that are contained in the
 * specified set.  In other words, removes from this set all of
 * its elements that are not contained in the specified set.  This
 * operation effectively modifies this set so that its value is
 * the <i>intersection</i> of the two sets.
 * A frozen set will not be modified.
 *
 * @param set the object on which to perform the retain
 * @param retain set that defines which elements this set will retain
 * @stable ICU 3.2
 */
U_STABLE void U_EXPORT2
uset_retainAll(USet* set, const USet* retain);

/**
 * Reallocate this objects internal structures to take up the least
 * possible space, without changing this object's value.
 * A frozen set will not be modified.
 *
 * @param set the object on which to perfrom the compact
 * @stable ICU 3.2
 */
U_STABLE void U_EXPORT2
uset_compact(USet* set);

/**
 * Inverts this set.  This operation modifies this set so that
 * its value is its complement.  This operation does not affect
 * the multicharacter strings, if any.
 * A frozen set will not be modified.
 * @param set the set
 * @stable ICU 2.4
 */
U_STABLE void U_EXPORT2
uset_complement(USet* set);

/**
 * Complements in this set all elements contained in the specified
 * set.  Any character in the other set will be removed if it is
 * in this set, or will be added if it is not in this set.
 * A frozen set will not be modified.
 *
 * @param set the set with which to complement
 * @param complement set that defines which elements will be xor'ed
 * from this set.
 * @stable ICU 3.2
 */
U_STABLE void U_EXPORT2
uset_complementAll(USet* set, const USet* complement);

/**
 * Removes all of the elements from this set.  This set will be
 * empty after this call returns.
 * A frozen set will not be modified.
 * @param set the set
 * @stable ICU 2.4
 */
U_STABLE void U_EXPORT2
uset_clear(USet* set);

/**
 * Returns TRUE if the given USet contains no characters and no
 * strings.
 * @param set the set
 * @return true if set is empty
 * @stable ICU 2.4
 */
U_STABLE UBool U_EXPORT2
uset_isEmpty(const USet* set);

/**
 * Returns TRUE if the given USet contains the given character.
 * This function works faster with a frozen set.
 * @param set the set
 * @param c The codepoint to check for within the set
 * @return true if set contains c
 * @stable ICU 2.4
 */
U_STABLE UBool U_EXPORT2
uset_contains(const USet* set, UChar32 c);

/**
 * Returns TRUE if the given USet contains all characters c
 * where start <= c && c <= end.
 * @param set the set
 * @param start the first character of the range to test, inclusive
 * @param end the last character of the range to test, inclusive
 * @return TRUE if set contains the range
 * @stable ICU 2.2
 */
U_STABLE UBool U_EXPORT2
uset_containsRange(const USet* set, UChar32 start, UChar32 end);

/**
 * Returns TRUE if the given USet contains the given string.
 * @param set the set
 * @param str the string
 * @param strLen the length of the string or -1 if null terminated.
 * @return true if set contains str
 * @stable ICU 2.4
 */
U_STABLE UBool U_EXPORT2
uset_containsString(const USet* set, const UChar* str, int32_t strLen);

/**
 * Returns the index of the given character within this set, where
 * the set is ordered by ascending code point.  If the character
 * is not in this set, return -1.  The inverse of this method is
 * <code>charAt()</code>.
 * @param set the set
 * @param c the character to obtain the index for
 * @return an index from 0..size()-1, or -1
 * @stable ICU 3.2
 */
U_STABLE int32_t U_EXPORT2
uset_indexOf(const USet* set, UChar32 c);

/**
 * Returns the character at the given index within this set, where
 * the set is ordered by ascending code point.  If the index is
 * out of range, return (UChar32)-1.  The inverse of this method is
 * <code>indexOf()</code>.
 * @param set the set
 * @param index an index from 0..size()-1 to obtain the char for
 * @return the character at the given index, or (UChar32)-1.
 * @stable ICU 3.2
 */
U_STABLE UChar32 U_EXPORT2
uset_charAt(const USet* set, int32_t index);

/**
 * Returns the number of characters and strings contained in the given
 * USet.
 * @param set the set
 * @return a non-negative integer counting the characters and strings
 * contained in set
 * @stable ICU 2.4
 */
U_STABLE int32_t U_EXPORT2
uset_size(const USet* set);

/**
 * Returns the number of items in this set.  An item is either a range
 * of characters or a single multicharacter string.
 * @param set the set
 * @return a non-negative integer counting the character ranges
 * and/or strings contained in set
 * @stable ICU 2.4
 */
U_STABLE int32_t U_EXPORT2
uset_getItemCount(const USet* set);

/**
 * Returns an item of this set.  An item is either a range of
 * characters or a single multicharacter string.
 * @param set the set
 * @param itemIndex a non-negative integer in the range 0..
 * uset_getItemCount(set)-1
 * @param start pointer to variable to receive first character
 * in range, inclusive
 * @param end pointer to variable to receive last character in range,
 * inclusive
 * @param str buffer to receive the string, may be NULL
 * @param strCapacity capacity of str, or 0 if str is NULL
 * @param ec error code
 * @return the length of the string (>= 2), or 0 if the item is a
 * range, in which case it is the range *start..*end, or -1 if
 * itemIndex is out of range
 * @stable ICU 2.4
 */
U_STABLE int32_t U_EXPORT2
uset_getItem(const USet* set, int32_t itemIndex,
             UChar32* start, UChar32* end,
             UChar* str, int32_t strCapacity,
             UErrorCode* ec);

/**
 * Returns true if set1 contains all the characters and strings
 * of set2. It answers the question, 'Is set1 a superset of set2?'
 * @param set1 set to be checked for containment
 * @param set2 set to be checked for containment
 * @return true if the test condition is met
 * @stable ICU 3.2
 */
U_STABLE UBool U_EXPORT2
uset_containsAll(const USet* set1, const USet* set2);

/**
 * Returns true if this set contains all the characters
 * of the given string. This is does not check containment of grapheme
 * clusters, like uset_containsString.
 * @param set set of characters to be checked for containment
 * @param str string containing codepoints to be checked for containment
 * @param strLen the length of the string or -1 if null terminated.
 * @return true if the test condition is met
 * @draft ICU 3.4
 */
U_DRAFT UBool U_EXPORT2
uset_containsAllCodePoints(const USet* set, const UChar *str, int32_t strLen);

/**
 * Returns true if set1 contains none of the characters and strings
 * of set2. It answers the question, 'Is set1 a disjoint set of set2?'
 * @param set1 set to be checked for containment
 * @param set2 set to be checked for containment
 * @return true if the test condition is met
 * @stable ICU 3.2
 */
U_STABLE UBool U_EXPORT2
uset_containsNone(const USet* set1, const USet* set2);

/**
 * Returns true if set1 contains some of the characters and strings
 * of set2. It answers the question, 'Does set1 and set2 have an intersection?'
 * @param set1 set to be checked for containment
 * @param set2 set to be checked for containment
 * @return true if the test condition is met
 * @stable ICU 3.2
 */
U_STABLE UBool U_EXPORT2
uset_containsSome(const USet* set1, const USet* set2);

#ifndef U_HIDE_DRAFT_API

/*
 * Returns the length of the initial substring of the input string which
 * consists only of characters that are contained in this set (USET_SPAN_WHILE_CONTAINED),
 * or only of characters that are not contained in this set (USET_SPAN_WHILE_NOT_CONTAINED).
 * Similar to the strspn() C library function.
 * Unpaired surrogates are treated according to contains() of their surrogate code points.
 * This function works faster with a frozen set and with a non-negative string length argument.
 * @param set the set
 * @param s start of the string
 * @param length of the string; can be -1 for NUL-terminated
 * @spanCondition specifies the containment condition for characters in the initial substring
 * @return the length of the initial substring according to the spanCondition
 * @draft ICU 3.8
 */
U_DRAFT int32_t U_EXPORT2
uset_span(const USet *set, const UChar *s, int32_t length, USetSpanCondition spanCondition);

/*
 * Returns the start of the trailing substring of the input string which
 * consists only of characters that are contained in this set (USET_SPAN_WHILE_CONTAINED),
 * or only of characters that are not contained in this set (USET_SPAN_WHILE_NOT_CONTAINED).
 * Unpaired surrogates are treated according to contains() of their surrogate code points.
 * This function works faster with a frozen set and with a non-negative string length argument.
 * @param set the set
 * @param s start of the string
 * @param length of the string; can be -1 for NUL-terminated
 * @spanCondition specifies the containment condition for characters in the trailing substring
 * @return the start of the trailing substring according to the spanCondition
 * @draft ICU 3.8
 */
U_DRAFT int32_t U_EXPORT2
uset_spanBack(const USet *set, const UChar *s, int32_t length, USetSpanCondition spanCondition);

/*
 * Returns the length of the initial substring of the input string which
 * consists only of characters that are contained in this set (USET_SPAN_WHILE_CONTAINED),
 * or only of characters that are not contained in this set (USET_SPAN_WHILE_NOT_CONTAINED).
 * Similar to the strspn() C library function.
 * Malformed byte sequences are treated according to contains(0xfffd).
 * This function works faster with a frozen set and with a non-negative string length argument.
 * @param set the set
 * @param s start of the string (UTF-8)
 * @param length of the string; can be -1 for NUL-terminated
 * @spanCondition specifies the containment condition for characters in the initial substring
 * @return the length of the initial substring according to the spanCondition
 * @draft ICU 3.8
 */
U_DRAFT int32_t U_EXPORT2
uset_spanUTF8(const USet *set, const char *s, int32_t length, USetSpanCondition spanCondition);

/*
 * Returns the start of the trailing substring of the input string which
 * consists only of characters that are contained in this set (USET_SPAN_WHILE_CONTAINED),
 * or only of characters that are not contained in this set (USET_SPAN_WHILE_NOT_CONTAINED).
 * Malformed byte sequences are treated according to contains(0xfffd).
 * This function works faster with a frozen set and with a non-negative string length argument.
 * @param set the set
 * @param s start of the string (UTF-8)
 * @param length of the string; can be -1 for NUL-terminated
 * @spanCondition specifies the containment condition for characters in the trailing substring
 * @return the start of the trailing substring according to the spanCondition
 * @draft ICU 3.8
 */
U_DRAFT int32_t U_EXPORT2
uset_spanBackUTF8(const USet *set, const char *s, int32_t length, USetSpanCondition spanCondition);

#endif /* U_HIDE_DRAFT_API */

/**
 * Returns true if set1 contains all of the characters and strings
 * of set2, and vis versa. It answers the question, 'Is set1 equal to set2?'
 * @param set1 set to be checked for containment
 * @param set2 set to be checked for containment
 * @return true if the test condition is met
 * @stable ICU 3.2
 */
U_STABLE UBool U_EXPORT2
uset_equals(const USet* set1, const USet* set2);

/*********************************************************************
 * Serialized set API
 *********************************************************************/

/**
 * Serializes this set into an array of 16-bit integers.  Serialization
 * (currently) only records the characters in the set; multicharacter
 * strings are ignored.
 *
 * The array
 * has following format (each line is one 16-bit integer):
 *
 *  length     = (n+2*m) | (m!=0?0x8000:0)
 *  bmpLength  = n; present if m!=0
 *  bmp[0]
 *  bmp[1]
 *  ...
 *  bmp[n-1]
 *  supp-high[0]
 *  supp-low[0]
 *  supp-high[1]
 *  supp-low[1]
 *  ...
 *  supp-high[m-1]
 *  supp-low[m-1]
 *
 * The array starts with a header.  After the header are n bmp
 * code points, then m supplementary code points.  Either n or m
 * or both may be zero.  n+2*m is always <= 0x7FFF.
 *
 * If there are no supplementary characters (if m==0) then the
 * header is one 16-bit integer, 'length', with value n.
 *
 * If there are supplementary characters (if m!=0) then the header
 * is two 16-bit integers.  The first, 'length', has value
 * (n+2*m)|0x8000.  The second, 'bmpLength', has value n.
 *
 * After the header the code points are stored in ascending order.
 * Supplementary code points are stored as most significant 16
 * bits followed by least significant 16 bits.
 *
 * @param set the set
 * @param dest pointer to buffer of destCapacity 16-bit integers.
 * May be NULL only if destCapacity is zero.
 * @param destCapacity size of dest, or zero.  Must not be negative.
 * @param pErrorCode pointer to the error code.  Will be set to
 * U_INDEX_OUTOFBOUNDS_ERROR if n+2*m > 0x7FFF.  Will be set to
 * U_BUFFER_OVERFLOW_ERROR if n+2*m+(m!=0?2:1) > destCapacity.
 * @return the total length of the serialized format, including
 * the header, that is, n+2*m+(m!=0?2:1), or 0 on error other
 * than U_BUFFER_OVERFLOW_ERROR.
 * @stable ICU 2.4
 */
U_STABLE int32_t U_EXPORT2
uset_serialize(const USet* set, uint16_t* dest, int32_t destCapacity, UErrorCode* pErrorCode);

/**
 * Given a serialized array, fill in the given serialized set object.
 * @param fillSet pointer to result
 * @param src pointer to start of array
 * @param srcLength length of array
 * @return true if the given array is valid, otherwise false
 * @stable ICU 2.4
 */
U_STABLE UBool U_EXPORT2
uset_getSerializedSet(USerializedSet* fillSet, const uint16_t* src, int32_t srcLength);

/**
 * Set the USerializedSet to contain the given character (and nothing
 * else).
 * @param fillSet pointer to result
 * @param c The codepoint to set
 * @stable ICU 2.4
 */
U_STABLE void U_EXPORT2
uset_setSerializedToOne(USerializedSet* fillSet, UChar32 c);

/**
 * Returns TRUE if the given USerializedSet contains the given
 * character.
 * @param set the serialized set
 * @param c The codepoint to check for within the set
 * @return true if set contains c
 * @stable ICU 2.4
 */
U_STABLE UBool U_EXPORT2
uset_serializedContains(const USerializedSet* set, UChar32 c);

/**
 * Returns the number of disjoint ranges of characters contained in
 * the given serialized set.  Ignores any strings contained in the
 * set.
 * @param set the serialized set
 * @return a non-negative integer counting the character ranges
 * contained in set
 * @stable ICU 2.4
 */
U_STABLE int32_t U_EXPORT2
uset_getSerializedRangeCount(const USerializedSet* set);

/**
 * Returns a range of characters contained in the given serialized
 * set.
 * @param set the serialized set
 * @param rangeIndex a non-negative integer in the range 0..
 * uset_getSerializedRangeCount(set)-1
 * @param pStart pointer to variable to receive first character
 * in range, inclusive
 * @param pEnd pointer to variable to receive last character in range,
 * inclusive
 * @return true if rangeIndex is valid, otherwise false
 * @stable ICU 2.4
 */
U_STABLE UBool U_EXPORT2
uset_getSerializedRange(const USerializedSet* set, int32_t rangeIndex,
                        UChar32* pStart, UChar32* pEnd);

#endif
