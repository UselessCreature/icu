/********************************************************************
 * COPYRIGHT: 
 * Copyright (c) 1999-2003, International Business Machines Corporation and
 * others. All Rights Reserved.
 ********************************************************************/
/************************************************************************
*   Date        Name        Description
*   12/14/99    Madhu        Creation.
*   01/12/2000  Madhu        updated for changed API
************************************************************************/

#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION

#include "unicode/uchar.h"
#include "intltest.h"
#include "unicode/rbbi.h"
#include "unicode/schriter.h"
#include "rbbiapts.h"
#include "rbbidata.h"
#include "cstring.h"

/**
 * API Test the RuleBasedBreakIterator class
 */



void RBBIAPITest::TestCloneEquals()
{

    UErrorCode status=U_ZERO_ERROR;
    RuleBasedBreakIterator* bi1     = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createCharacterInstance(Locale::getDefault(), status);
    RuleBasedBreakIterator* biequal = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createCharacterInstance(Locale::getDefault(), status);
    RuleBasedBreakIterator* bi3     = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createCharacterInstance(Locale::getDefault(), status);
    RuleBasedBreakIterator* bi2     = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createWordInstance(Locale::getDefault(), status);
    if(U_FAILURE(status)){
        errln((UnicodeString)"FAIL : in construction");
        return;
    }


    UnicodeString testString="Testing word break iterators's clone() and equals()";
    bi1->setText(testString);
    bi2->setText(testString);
    biequal->setText(testString);

    bi3->setText("hello");

    logln((UnicodeString)"Testing equals()");

    logln((UnicodeString)"Testing == and !=");
    UBool b = (*bi1 != *biequal);
    b |= *bi1 == *bi2;
    b |= *bi1 == *bi3;
    if (b) {
        errln((UnicodeString)"ERROR:1 RBBI's == and != operator failed.");
    }

    if(*bi2 == *biequal || *bi2 == *bi1  || *biequal == *bi3)
        errln((UnicodeString)"ERROR:2 RBBI's == and != operator  failed.");


    // Quick test of RulesBasedBreakIterator assignment - 
    // Check that
    //    two different iterators are !=
    //    they are == after assignment
    //    source and dest iterator produce the same next() after assignment.
    //    deleting one doesn't disable the other.
    logln("Testing assignment");
    RuleBasedBreakIterator *bix = (RuleBasedBreakIterator *)BreakIterator::createLineInstance(Locale::getEnglish(), status);
    if(U_FAILURE(status)){
        errln((UnicodeString)"FAIL : in construction");
        return;
    }

    RuleBasedBreakIterator biDefault, biDefault2;
    if(U_FAILURE(status)){
        errln((UnicodeString)"FAIL : in construction of default iterator");
        return;
    }
    if (biDefault == *bix) {
        errln((UnicodeString)"ERROR: iterators should not compare ==");
        return;
    }
    if (biDefault != biDefault2) {
        errln((UnicodeString)"ERROR: iterators should compare ==");
        return;
    }


    UnicodeString   HelloString("Hello Kitty");
    bix->setText(HelloString);
    if (*bix == *bi2) {
        errln(UnicodeString("ERROR: strings should not be equal before assignment."));
    }
    *bix = *bi2;
    if (*bix != *bi2) {
        errln(UnicodeString("ERROR: strings should be equal before assignment."));
    }

    int bixnext = bix->next();
    int bi2next = bi2->next();
    if (! (bixnext == bi2next && bixnext == 7)) {
        errln(UnicodeString("ERROR: iterators behaved differently after assignment."));
    }
    delete bix;
    if (bi2->next() != 8) {
        errln(UnicodeString("ERROR: iterator.next() failed after deleting copy."));
    }



    logln((UnicodeString)"Testing clone()");
    RuleBasedBreakIterator* bi1clone=(RuleBasedBreakIterator*)bi1->clone();
    RuleBasedBreakIterator* bi2clone=(RuleBasedBreakIterator*)bi2->clone();

    if(*bi1clone != *bi1 || *bi1clone  != *biequal  ||  
      *bi1clone == *bi3 || *bi1clone == *bi2)
        errln((UnicodeString)"ERROR:1 RBBI's clone() method failed");

    if(*bi2clone == *bi1 || *bi2clone == *biequal ||  
       *bi2clone == *bi3 || *bi2clone != *bi2)
        errln((UnicodeString)"ERROR:2 RBBI's clone() method failed");

    if(bi1->getText() != bi1clone->getText()   ||
       bi2clone->getText() != bi2->getText()   || 
       *bi2clone == *bi1clone )
        errln((UnicodeString)"ERROR: RBBI's clone() method failed");

    delete bi1clone;
    delete bi2clone;
    delete bi1;
    delete bi3;
    delete bi2;
    delete biequal;
}

void RBBIAPITest::TestBoilerPlate()
{
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator* a = BreakIterator::createLineInstance(Locale("hi"), status);
    BreakIterator* b = BreakIterator::createLineInstance(Locale("hi_IN"),status);
    if(*a!=*b){
        errln("Failed: boilerplate method operator!= does not return correct results");
    }
    BreakIterator* c = BreakIterator::createLineInstance(Locale("th"),status);
    if(*c==*a){
        errln("Failed: boilerplate method opertator== does not return correct results");
    }
    delete a;
    delete b;
    delete c;
}

void RBBIAPITest::TestgetRules()
{
    UErrorCode status=U_ZERO_ERROR;

    RuleBasedBreakIterator* bi1=(RuleBasedBreakIterator*)RuleBasedBreakIterator::createCharacterInstance(Locale::getDefault(), status);
    RuleBasedBreakIterator* bi2=(RuleBasedBreakIterator*)RuleBasedBreakIterator::createWordInstance(Locale::getDefault(), status);
    if(U_FAILURE(status)){
        errln((UnicodeString)"FAIL: in construction");
        delete bi1;
        delete bi2;
        return;
    }



    logln((UnicodeString)"Testing toString()");

    bi1->setText((UnicodeString)"Hello there");

    RuleBasedBreakIterator* bi3 =(RuleBasedBreakIterator*)bi1->clone();

    UnicodeString temp=bi1->getRules();
    UnicodeString temp2=bi2->getRules();
    UnicodeString temp3=bi3->getRules();
    if( temp2.compare(temp3) ==0 || temp.compare(temp2) == 0 || temp.compare(temp3) != 0)
        errln((UnicodeString)"ERROR: error in getRules() method");

    delete bi1;
    delete bi2;
    delete bi3;
}
void RBBIAPITest::TestHashCode()
{
    UErrorCode status=U_ZERO_ERROR;
    RuleBasedBreakIterator* bi1     = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createCharacterInstance(Locale::getDefault(), status);
    RuleBasedBreakIterator* bi3     = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createCharacterInstance(Locale::getDefault(), status);
    RuleBasedBreakIterator* bi2     = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createWordInstance(Locale::getDefault(), status);
    if(U_FAILURE(status)){
        errln((UnicodeString)"FAIL : in construction");
        delete bi1;
        delete bi2;
        delete bi3;
        return;
    }


    logln((UnicodeString)"Testing hashCode()");

    bi1->setText((UnicodeString)"Hash code");
    bi2->setText((UnicodeString)"Hash code");
    bi3->setText((UnicodeString)"Hash code");

    RuleBasedBreakIterator* bi1clone= (RuleBasedBreakIterator*)bi1->clone();
    RuleBasedBreakIterator* bi2clone= (RuleBasedBreakIterator*)bi2->clone();

    if(bi1->hashCode() != bi1clone->hashCode() ||  bi1->hashCode() != bi3->hashCode() ||
        bi1clone->hashCode() != bi3->hashCode() || bi2->hashCode() != bi2clone->hashCode())
        errln((UnicodeString)"ERROR: identical objects have different hashcodes");

    if(bi1->hashCode() == bi2->hashCode() ||  bi2->hashCode() == bi3->hashCode() ||
        bi1clone->hashCode() == bi2clone->hashCode() || bi1clone->hashCode() == bi2->hashCode())
        errln((UnicodeString)"ERROR: different objects have same hashcodes");

    delete bi1clone;
    delete bi2clone; 
    delete bi1;
    delete bi2;
    delete bi3;

}
void RBBIAPITest::TestGetSetAdoptText()
{
    logln((UnicodeString)"Testing getText setText ");
    UErrorCode status=U_ZERO_ERROR;
    UnicodeString str1="first string.";
    UnicodeString str2="Second string.";
    RuleBasedBreakIterator* charIter1 = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createCharacterInstance(Locale::getDefault(), status);
    RuleBasedBreakIterator* wordIter1 = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createWordInstance(Locale::getDefault(), status);
    if(U_FAILURE(status)){
        errln((UnicodeString)"FAIL : in construction");
            return;
    }


    CharacterIterator* text1= new StringCharacterIterator(str1);
    CharacterIterator* text1Clone = text1->clone();
    CharacterIterator* text2= new StringCharacterIterator(str2);
    CharacterIterator* text3= new StringCharacterIterator(str2, 3, 10, 3); //  "ond str"
    
    wordIter1->setText(str1);
    if(wordIter1->getText() != *text1)
       errln((UnicodeString)"ERROR:1 error in setText or getText ");
    if(wordIter1->current() != 0)
        errln((UnicodeString)"ERROR:1 setText did not set the iteration position to the beginning of the text, it is" + wordIter1->current() + (UnicodeString)"\n");

    wordIter1->next(2);

    wordIter1->setText(str2);
    if(wordIter1->current() != 0)
        errln((UnicodeString)"ERROR:2 setText did not reset the iteration position to the beginning of the text, it is" + wordIter1->current() + (UnicodeString)"\n");


    charIter1->adoptText(text1Clone);
    if( wordIter1->getText() == charIter1->getText() || 
        wordIter1->getText() != *text2 ||  charIter1->getText() != *text1 )
        errln((UnicodeString)"ERROR:2 error is getText or setText()");

    RuleBasedBreakIterator* rb=(RuleBasedBreakIterator*)wordIter1->clone();
    rb->adoptText(text1);
    if(rb->getText() != *text1)
        errln((UnicodeString)"ERROR:1 error in adoptText ");
    rb->adoptText(text2);
    if(rb->getText() != *text2)
        errln((UnicodeString)"ERROR:2 error in adoptText ");

    // Adopt where iterator range is less than the entire orignal source string.
    rb->adoptText(text3);
    if(rb->preceding(2) != 3) {
        errln((UnicodeString)"ERROR:3 error in adoptText ");
    }
    if(rb->following(11) != BreakIterator::DONE) {
        errln((UnicodeString)"ERROR:4 error in adoptText ");
    }

    delete wordIter1;
    delete charIter1;
    delete rb;

 } 

  
void RBBIAPITest::TestIteration()
{
    // This test just verifies that the API is present.
    // Testing for correct operation of the break rules happens elsewhere.

    UErrorCode status=U_ZERO_ERROR;
    RuleBasedBreakIterator* bi  = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createCharacterInstance(Locale::getDefault(), status);
    if (U_FAILURE(status) || bi == NULL)  {
        errln("Failure creating character break iterator.  Status = %s", u_errorName(status));
    }
    delete bi;

    status=U_ZERO_ERROR;
    bi  = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createWordInstance(Locale::getDefault(), status);
    if (U_FAILURE(status) || bi == NULL)  {
        errln("Failure creating Word break iterator.  Status = %s", u_errorName(status));
    }
    delete bi;

    status=U_ZERO_ERROR;
    bi  = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createLineInstance(Locale::getDefault(), status);
    if (U_FAILURE(status) || bi == NULL)  {
        errln("Failure creating Line break iterator.  Status = %s", u_errorName(status));
    }
    delete bi;

    status=U_ZERO_ERROR;
    bi  = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createSentenceInstance(Locale::getDefault(), status);
    if (U_FAILURE(status) || bi == NULL)  {
        errln("Failure creating Sentence break iterator.  Status = %s", u_errorName(status));
    }
    delete bi;

    status=U_ZERO_ERROR;
    bi  = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createTitleInstance(Locale::getDefault(), status);
    if (U_FAILURE(status) || bi == NULL)  {
        errln("Failure creating Title break iterator.  Status = %s", u_errorName(status));
    }
    delete bi;

    status=U_ZERO_ERROR;
    bi  = (RuleBasedBreakIterator*)RuleBasedBreakIterator::createCharacterInstance(Locale::getDefault(), status);
    if (U_FAILURE(status) || bi == NULL)  {
        errln("Failure creating character break iterator.  Status = %s", u_errorName(status));
        return;   // Skip the rest of these tests.
    }


    UnicodeString testString="0123456789";
    bi->setText(testString);

    int32_t i;
    i = bi->first();
    if (i != 0) {
        errln("Incorrect value from bi->first().  Expected 0, got %d.", i);
    }

    i = bi->last();
    if (i != 10) {
        errln("Incorrect value from bi->last().  Expected 10, got %d", i);
    }

    //
    // Previous
    //
    bi->last();
    i = bi->previous();
    if (i != 9) {
        errln("Incorrect value from bi->last() at line %d.  Expected 9, got %d", __LINE__, i);
    }


    bi->first();
    i = bi->previous();
    if (i != BreakIterator::DONE) {
        errln("Incorrect value from bi->previous() at line %d.  Expected DONE, got %d", __LINE__, i);
    }

    //
    // next()
    //
    bi->first();
    i = bi->next();
    if (i != 1) {
        errln("Incorrect value from bi->next() at line %d.  Expected 1, got %d", __LINE__, i);
    }

    bi->last();
    i = bi->next();
    if (i != BreakIterator::DONE) {
        errln("Incorrect value from bi->next() at line %d.  Expected DONE, got %d", __LINE__, i);
    }


    //
    //  current()
    //
    bi->first();
    i = bi->current();
    if (i != 0) {
        errln("Incorrect value from bi->previous() at line %d.  Expected 0, got %d", __LINE__, i);
    }

    bi->next();
    i = bi->current();
    if (i != 1) {
        errln("Incorrect value from bi->previous() at line %d.  Expected 1, got %d", __LINE__, i);
    }

    bi->last();
    bi->next();
    i = bi->current();
    if (i != 10) {
        errln("Incorrect value from bi->previous() at line %d.  Expected 10, got %d", __LINE__, i);
    }

    bi->first();
    bi->previous();
    i = bi->current();
    if (i != 0) {
        errln("Incorrect value from bi->previous() at line %d.  Expected 0, got %d", __LINE__, i);
    }


    //
    // Following()
    //
    i = bi->following(4);
    if (i != 5) {
        errln("Incorrect value from bi->following() at line %d.  Expected 5, got %d", __LINE__, i);
    }

    i = bi->following(9);
    if (i != 10) {
        errln("Incorrect value from bi->following() at line %d.  Expected 10, got %d", __LINE__, i);
    }

    i = bi->following(10);
    if (i != BreakIterator::DONE) {
        errln("Incorrect value from bi->following() at line %d.  Expected DONE, got %d", __LINE__, i);
    }


    //
    // Preceding
    //
    i = bi->preceding(4);
    if (i != 3) {
        errln("Incorrect value from bi->preceding() at line %d.  Expected 3, got %d", __LINE__, i);
    }

    i = bi->preceding(10);
    if (i != 9) {
        errln("Incorrect value from bi->preceding() at line %d.  Expected 9, got %d", __LINE__, i);
    }

    i = bi->preceding(1);
    if (i != 0) {
        errln("Incorrect value from bi->preceding() at line %d.  Expected 0, got %d", __LINE__, i);
    }

    i = bi->preceding(0);
    if (i != BreakIterator::DONE) {
        errln("Incorrect value from bi->preceding() at line %d.  Expected DONE, got %d", __LINE__, i);
    }


    //
    // isBoundary()
    //
    bi->first();
    if (bi->isBoundary(3) != TRUE) {
        errln("Incorrect value from bi->isBoudary() at line %d.  Expected TRUE, got FALSE", __LINE__, i);
    }
    i = bi->current();
    if (i != 3) {
        errln("Incorrect value from bi->current() at line %d.  Expected 3, got %d", __LINE__, i);
    }


    if (bi->isBoundary(11) != FALSE) {
        errln("Incorrect value from bi->isBoudary() at line %d.  Expected FALSE, got TRUE", __LINE__, i);
    }
    i = bi->current();
    if (i != 10) {
        errln("Incorrect value from bi->current() at line %d.  Expected 10, got %d", __LINE__, i);
    }

    //
    // next(n)
    //
    bi->first();
    i = bi->next(4);
    if (i != 4) {
        errln("Incorrect value from bi->next() at line %d.  Expected 4, got %d", __LINE__, i);
    }

    i = bi->next(6);
    if (i != 10) {
        errln("Incorrect value from bi->next() at line %d.  Expected 10, got %d", __LINE__, i);
    }

    bi->first();
    i = bi->next(11);
    if (i != BreakIterator::DONE) {
        errln("Incorrect value from bi->next() at line %d.  Expected BreakIterator::DONE, got %d", __LINE__, i);
    }

    delete bi;

}






void RBBIAPITest::TestBuilder() {
     UnicodeString rulesString1 = "$Letters = [:L:];\n"
                                  "$Numbers = [:N:];\n"
                                  "$Letters+;\n"
                                  "$Numbers+;\n"
                                  "[^$Letters $Numbers];\n"
                                  "!.*;\n";
     UnicodeString testString1  = "abc123..abc";
                                // 01234567890
     int32_t bounds1[] = {0, 3, 6, 7, 8, 11};
     UErrorCode status=U_ZERO_ERROR;
     UParseError    parseError;
     
     RuleBasedBreakIterator *bi = new RuleBasedBreakIterator(rulesString1, parseError, status);
     if(U_FAILURE(status)) {
         errln("FAIL : in construction");
     } else {
         bi->setText(testString1);
         doBoundaryTest(*bi, testString1, bounds1);
     }
     delete bi;
}


//
//  TestQuoteGrouping
//       Single quotes within rules imply a grouping, so that a modifier
//       following the quoted text (* or +) applies to all of the quoted chars.
//
void RBBIAPITest::TestQuoteGrouping() {
     UnicodeString rulesString1 = "#Here comes the rule...\n"
                                  "'$@!'*;\n"   //  (\$\@\!)*
                                  ".;\n";

     UnicodeString testString1  = "$@!$@!X$@!!X";
                                // 0123456789012
     int32_t bounds1[] = {0, 6, 7, 10, 11, 12};
     UErrorCode status=U_ZERO_ERROR;
     UParseError    parseError;
     
     RuleBasedBreakIterator *bi = new RuleBasedBreakIterator(rulesString1, parseError, status);
     if(U_FAILURE(status)) {
         errln("FAIL : in construction");
     } else {
         bi->setText(testString1);
         doBoundaryTest(*bi, testString1, bounds1);
     }
     delete bi;
}

//
//  TestWordStatus
//      Test word break rule status constants.
//
void RBBIAPITest::TestWordStatus() {

     
     UnicodeString testString1 =   //                  Ideographic    Katakana       Hiragana
             CharsToUnicodeString("plain word 123.45 \\u9160\\u9161 \\u30a1\\u30a2 \\u3041\\u3094");
                                // 012345678901234567  8      9    0  1      2    3  4      5    6
     int32_t bounds1[] =     {     0,   5,6, 10,11, 17,18,  19,   20,21,         23,24,    25,  26};
     int32_t tag_lo[]  = {UBRK_WORD_NONE,     UBRK_WORD_LETTER, UBRK_WORD_NONE,    UBRK_WORD_LETTER,
                          UBRK_WORD_NONE,     UBRK_WORD_NUMBER, UBRK_WORD_NONE,
                          UBRK_WORD_IDEO,     UBRK_WORD_IDEO,   UBRK_WORD_NONE,
                          UBRK_WORD_KANA,     UBRK_WORD_NONE,   UBRK_WORD_KANA,    UBRK_WORD_KANA};

     int32_t tag_hi[]  = {UBRK_WORD_NONE_LIMIT, UBRK_WORD_LETTER_LIMIT, UBRK_WORD_NONE_LIMIT, UBRK_WORD_LETTER_LIMIT,
                          UBRK_WORD_NONE_LIMIT, UBRK_WORD_NUMBER_LIMIT, UBRK_WORD_NONE_LIMIT,
                          UBRK_WORD_IDEO_LIMIT, UBRK_WORD_IDEO_LIMIT,   UBRK_WORD_NONE_LIMIT,
                          UBRK_WORD_KANA_LIMIT, UBRK_WORD_NONE_LIMIT,   UBRK_WORD_KANA_LIMIT, UBRK_WORD_KANA_LIMIT};

     UErrorCode status=U_ZERO_ERROR;
     
     RuleBasedBreakIterator *bi = (RuleBasedBreakIterator *)BreakIterator::createWordInstance(Locale::getDefault(), status);
     if(U_FAILURE(status)) {
         errln("FAIL : in construction");
     } else {
         bi->setText(testString1);
         // First test that the breaks are in the right spots.
         doBoundaryTest(*bi, testString1, bounds1);

         // Then go back and check tag values
         int32_t i = 0;
         int32_t pos, tag;
         for (pos = bi->first(); pos != BreakIterator::DONE; pos = bi->next(), i++) {
             if (pos != bounds1[i]) {
                 errln("FAIL: unexpected word break at postion %d", pos);
                 break;
             }
             tag = bi->getRuleStatus();
             if (tag < tag_lo[i] || tag >= tag_hi[i]) {
                 errln("FAIL: incorrect tag value %d at position %d", tag, pos);
                 break;
             }
         }
     }
     delete bi;
}


//
//   Bug 2190 Regression test.   Builder crash on rule consisting of only a
//                               $variable reference
void RBBIAPITest::TestBug2190() {
     UnicodeString rulesString1 = "$aaa = abcd;\n"
                                  "$bbb = $aaa;\n"
                                  "$bbb;\n";
     UnicodeString testString1  = "abcdabcd";
                                // 01234567890
     int32_t bounds1[] = {0, 4, 8};
     UErrorCode status=U_ZERO_ERROR;
     UParseError    parseError;
     
     RuleBasedBreakIterator *bi = new RuleBasedBreakIterator(rulesString1, parseError, status);
     if(U_FAILURE(status)) {
         errln("FAIL : in construction");
     } else {
         bi->setText(testString1);
         doBoundaryTest(*bi, testString1, bounds1);
     }
     delete bi;
}


void RBBIAPITest::TestRegistration() {
  UErrorCode status = U_ZERO_ERROR;
  BreakIterator* thai_word = BreakIterator::createWordInstance("th_TH", status);

  // ok to not delete these if we exit because of error?
  BreakIterator* thai_char = BreakIterator::createCharacterInstance("th_TH", status);
  BreakIterator* root_word = BreakIterator::createWordInstance("", status);
  BreakIterator* root_char = BreakIterator::createCharacterInstance("", status);

  URegistryKey key = BreakIterator::registerInstance(thai_word, "xx", UBRK_WORD, status);
  {
    if (*thai_word == *root_word) {
      errln("thai not different from root");
    }
  }

  {
    BreakIterator* result = BreakIterator::createWordInstance("xx_XX", status);
    UBool fail = *result != *thai_word;
    delete result;
    if (fail) {
      errln("bad result for xx_XX/word");
    }
  }
  
  {
    BreakIterator* result = BreakIterator::createCharacterInstance("th_TH", status);
    UBool fail = *result != *thai_char;
    delete result;
    if (fail) {
      errln("bad result for th_TH/char");
    }
  }
  
  {
    BreakIterator* result = BreakIterator::createCharacterInstance("xx_XX", status);
    UBool fail = *result != *root_char;
    delete result;
    if (fail) {
      errln("bad result for xx_XX/char");
    }
  }
    
  {
    StringEnumeration* avail = BreakIterator::getAvailableLocales();
    UBool found = FALSE;
	const UnicodeString* p;
    while (p = avail->snext(status)) {
      if (p->compare("xx") == 0) {
	found = TRUE;
	break;
      }
    }
    delete avail;
    if (!found) {
      errln("did not find test locale");
    }
  }

  {
    UBool unreg = BreakIterator::unregister(key, status);
    if (!unreg) {
      errln("unable to unregister");
    }
  }

  {
    BreakIterator* result = BreakIterator::createWordInstance("xx", status);
    BreakIterator* root = BreakIterator::createWordInstance("", status);
    UBool fail = *root != *result;
    delete root;
    delete result;
    if (fail) {
      errln("did not get root break");
    }
  }

  {
    StringEnumeration* avail = BreakIterator::getAvailableLocales();
    UBool found = FALSE;
    const UnicodeString* p;
    while (p = avail->snext(status)) {
      if (p->compare("xx") == 0) {
	found = TRUE;
	break;
      }
    }
    delete avail;
    if (found) {
      errln("found test locale");
    }
  }

  // that_word was adopted by factory
  delete thai_char;
  delete root_word;
  delete root_char;
}

void RBBIAPITest::RoundtripRule(const char *dataFile) {
    UErrorCode status = U_ZERO_ERROR;
    UParseError parseError;
	parseError.line = 0;
	parseError.offset = 0;
    UDataMemory *data = udata_open(NULL, "brk", dataFile, &status);
    uint32_t length;
    const UChar *builtSource;
    const uint8_t *rbbiRules;
    const uint8_t *builtRules;

    if (U_FAILURE(status)) {
        errln("Can't open \"%s\"", dataFile);
        return;
    }

    builtRules = (const uint8_t *)udata_getMemory(data);
    builtSource = (const UChar *)(builtRules + ((RBBIDataHeader*)builtRules)->fRuleSource);
    RuleBasedBreakIterator *brkItr = new RuleBasedBreakIterator(builtSource, parseError, status);
    if (U_FAILURE(status)) {
        errln("createRuleBasedBreakIterator: ICU Error \"%s\"  at line %d, column %d\n",
                u_errorName(status), parseError.line, parseError.offset);
        return;
    };
    rbbiRules = brkItr->getBinaryRules(length);
    logln("Comparing \"%s\" len=%d", dataFile, length);
    if (memcmp(builtRules, rbbiRules, (int32_t)length) != 0) {
        errln("Built rules and rebuilt rules are different %s", dataFile);
        return;
    }
    delete brkItr;
    udata_close(data);
}

void RBBIAPITest::TestRoundtripRules() {
    RoundtripRule("word");
    RoundtripRule("title");
    RoundtripRule("sent");
    RoundtripRule("line");
    RoundtripRule("char");
    if (!quick) {
        RoundtripRule("word_th");
        RoundtripRule("line_th");
    }
}

//---------------------------------------------
// runIndexedTest
//---------------------------------------------

void RBBIAPITest::runIndexedTest( int32_t index, UBool exec, const char* &name, char* /*par*/ )
{
    if (exec) logln((UnicodeString)"TestSuite RuleBasedBreakIterator API ");
    switch (index) {
     //   case 0: name = "TestConstruction"; if (exec) TestConstruction(); break;
        case  0: name = "TestCloneEquals"; if (exec) TestCloneEquals(); break;
        case  1: name = "TestgetRules"; if (exec) TestgetRules(); break;
        case  2: name = "TestHashCode"; if (exec) TestHashCode(); break;
        case  3: name = "TestGetSetAdoptText"; if (exec) TestGetSetAdoptText(); break;
        case  4: name = "TestIteration"; if (exec) TestIteration(); break;
        case  5: name = ""; break;   /* Extra */
        case  6: name = ""; break;   /* Extra */
        case  7: name = "TestBuilder"; if (exec) TestBuilder(); break;
        case  8: name = "TestQuoteGrouping"; if (exec) TestQuoteGrouping(); break;
        case  9: name = "TestWordStatus"; if (exec) TestWordStatus(); break;
        case 10: name = "TestBug2190"; if (exec) TestBug2190(); break;
        case 11: name = "TestRegistration"; if (exec) TestRegistration(); break;
        case 12: name = "TestBoilerPlate"; if (exec) TestBoilerPlate(); break;
        case 13: name = "TestRoundtripRules"; if (exec) TestRoundtripRules(); break;

        default: name = ""; break; /*needed to end loop*/
    }
}

//---------------------------------------------
//Internal subroutines
//---------------------------------------------

void RBBIAPITest::doBoundaryTest(RuleBasedBreakIterator& bi, UnicodeString& text, int32_t *boundaries){
     logln((UnicodeString)"testIsBoundary():");
        int32_t p = 0;
        UBool isB;
        for (int32_t i = 0; i < text.length(); i++) {
            isB = bi.isBoundary(i);
            logln((UnicodeString)"bi.isBoundary(" + i + ") -> " + isB);

            if (i == boundaries[p]) {
                if (!isB)
                    errln((UnicodeString)"Wrong result from isBoundary() for " + i + (UnicodeString)": expected true, got false");
                p++;
            }
            else {
                if (isB)
                    errln((UnicodeString)"Wrong result from isBoundary() for " + i + (UnicodeString)": expected false, got true");
            }
        }
}
void RBBIAPITest::doTest(UnicodeString& testString, int32_t start, int32_t gotoffset, int32_t expectedOffset, const char* expectedString){
    UnicodeString selected;
    UnicodeString expected=CharsToUnicodeString(expectedString);

    if(gotoffset != expectedOffset)
         errln((UnicodeString)"ERROR:****returned #" + gotoffset + (UnicodeString)" instead of #" + expectedOffset);
    if(start <= gotoffset){
        testString.extractBetween(start, gotoffset, selected);  
    }
    else{
        testString.extractBetween(gotoffset, start, selected);
    }
    if(selected.compare(expected) != 0)
         errln(prettify((UnicodeString)"ERROR:****selected \"" + selected + "\" instead of \"" + expected + "\""));
    else
        logln(prettify("****selected \"" + selected + "\""));
}

#endif /* #if !UCONFIG_NO_BREAK_ITERATION */
