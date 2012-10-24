#include <stdio.h>
#include <stdlib.h>

#include "intltest.h"
#include "unicode/compactdecimalformat.h"
#include "unicode/unum.h"

#define LENGTHOF(array) (int32_t)(sizeof(array) / sizeof((array)[0]))

typedef struct ExpectedResult {
  double value;
  const char *expected;
} ExpectedResult;

static const char *kShortStr = "Short";
static const char *kLongStr = "Long";

static ExpectedResult kEnglishShort[] = {
  {0.0, "0.0"},
  {0.17, "0.17"},
  {1, "1"},
  {1234, "1.2K"},
  {12345, "12K"},
  {123456, "120K"},
  {1234567, "1.2M"},
  {12345678, "12M"},
  {123456789, "120M"},
  {1234567890, "1.2B"},
  {12345678901, "12B"},
  {123456789012, "120B"},
  {1234567890123, "1.2T"},
  {12345678901234, "12T"},
  {123456789012345, "120T"},
  {1234567890123456, "1200T"}};

static ExpectedResult kSerbianShort[] = {
  {1234, "1200"},
  {12345, "12K"},
  {20789, "21\\u00a0\\u0445\\u0438\\u0459"},
  {123456, "120\\u00a0\\u0445\\u0438\\u0459"},
  {1234567, "1,2\\u00A0\\u043C\\u0438\\u043B"},
  {12345678, "12\\u00A0\\u043C\\u0438\\u043B"},
  {123456789, "120\\u00A0\\u043C\\u0438\\u043B"},
  {1234567890, "1,2\\u00A0\\u043C\\u043B\\u0440\\u0434"},
  {12345678901, "12\\u00A0\\u043C\\u043B\\u0440\\u0434"},
  {123456789012, "120\\u00A0\\u043C\\u043B\\u0440\\u0434"},
  {1234567890123, "1,2\\u00A0\\u0431\\u0438\\u043B"},
  {12345678901234, "12\\u00A0\\u0431\\u0438\\u043B"},
  {123456789012345, "120\\u00A0\\u0431\\u0438\\u043B"},
  {1234567890123456, "1200\\u00A0\\u0431\\u0438\\u043B"}};

static ExpectedResult kSerbianLong[] = {
  {1234, "1,2 \\u0445\\u0438\\u0459\\u0430\\u0434\\u0430"},
  {12345, "12 \\u0445\\u0438\\u0459\\u0430\\u0434\\u0430"},
  {21789, "22 \\u0445\\u0438\\u0459\\u0430\\u0434\\u0435"},
  {123456, "120 \\u0445\\u0438\\u0459\\u0430\\u0434\\u0430"},
  {999999, "1 \\u043C\\u0438\\u043B\\u0438\\u043E\\u043D"},
  {1234567, "1,2 \\u043C\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {12345678, "12 \\u043C\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {123456789, "120 \\u043C\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {1234567890, "1,2 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0438"},
  {12345678901, "12 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0438"},
  {20890123456, "21 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0430"},
  {21890123456, "22 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0435"},
  {123456789012, "120 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0438"},
  {1234567890123, "1,2 \\u0442\\u0440\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {12345678901234, "12 \\u0442\\u0440\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {123456789012345, "120 \\u0442\\u0440\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {1234567890123456, "1200 \\u0442\\u0440\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"}};

static ExpectedResult kSerbianLongNegative[] = {
  {-1234, "-1,2 \\u0445\\u0438\\u0459\\u0430\\u0434\\u0430"},
  {-12345, "-12 \\u0445\\u0438\\u0459\\u0430\\u0434\\u0430"},
  {-21789, "-22 \\u0445\\u0438\\u0459\\u0430\\u0434\\u0435"},
  {-123456, "-120 \\u0445\\u0438\\u0459\\u0430\\u0434\\u0430"},
  {-999999, "-1 \\u043C\\u0438\\u043B\\u0438\\u043E\\u043D"},
  {-1234567, "-1,2 \\u043C\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {-12345678, "-12 \\u043C\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {-123456789, "-120 \\u043C\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {-1234567890, "-1,2 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0438"},
  {-12345678901, "-12 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0438"},
  {-20890123456, "-21 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0430"},
  {-21890123456, "-22 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0435"},
  {-123456789012, "-120 \\u043C\\u0438\\u043B\\u0438\\u0458\\u0430\\u0440\\u0434\\u0438"},
  {-1234567890123, "-1,2 \\u0442\\u0440\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {-12345678901234, "-12 \\u0442\\u0440\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {-123456789012345, "-120 \\u0442\\u0440\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"},
  {-1234567890123456, "-1200 \\u0442\\u0440\\u0438\\u043B\\u0438\\u043E\\u043D\\u0430"}};

static ExpectedResult kJapaneseShort[] = {
  {1234, "1.2\\u5343"},
  {12345, "1.2\\u4E07"},
  {123456, "12\\u4E07"},
  {1234567, "120\\u4E07"},
  {12345678, "1200\\u4E07"},
  {123456789, "1.2\\u5104"},
  {1234567890, "12\\u5104"},
  {12345678901, "120\\u5104"},
  {123456789012, "1200\\u5104"},
  {1234567890123, "1.2\\u5146"},
  {12345678901234, "12\\u5146"},
  {123456789012345, "120\\u5146"}};

static ExpectedResult kSwahiliShort[] = {
  {1234, "elfu\\u00a01.2"},
  {12345, "elfu\\u00a012"},
  {123456, "laki1.2"},
  {1234567, "M1.2"},
  {12345678, "M12"},
  {123456789, "M120"},
  {1234567890, "B1.2"},
  {12345678901, "B12"},
  {123456789012, "B120"},
  {1234567890123, "T1.2"},
  {12345678901234, "T12"},
  {1234567890123456, "T1200"}};

static ExpectedResult kCsShort[] = {
  {1000, "1\\u00a0tis."},
  {1500, "1,5\\u00a0tis."},
  {5000, "5\\u00a0tis."},
  {23000, "23\\u00a0tis."},
  {127123, "130\\u00a0tis."},
  {1271234, "1,3\\u00a0mil."},
  {12712345, "13\\u00a0mil."},
  {127123456, "130\\u00a0mil."},
  {1271234567, "1,3\\u00a0mld."},
  {12712345678, "13\\u00a0mld."},
  {127123456789, "130\\u00a0mld."},
  {1271234567890, "1,3\\u00a0bil."},
  {12712345678901, "13\\u00a0bil."},
  {127123456789012, "130\\u00a0bil."}};

static ExpectedResult kSkLong[] = {
  {1000, "1 tis\\u00edc"},
  {1572, "1,6 tis\\u00edc"},
  {5184, "5,2 tis\\u00edc"}};

static ExpectedResult kSwahiliShortNegative[] = {
  {-1234, "elfu\\u00a0-1.2"},
  {-12345, "elfu\\u00a0-12"},
  {-123456, "laki-1.2"},
  {-1234567, "M-1.2"},
  {-12345678, "M-12"},
  {-123456789, "M-120"},
  {-1234567890, "B-1.2"},
  {-12345678901, "B-12"},
  {-123456789012, "B-120"},
  {-1234567890123, "T-1.2"},
  {-12345678901234, "T-12"},
  {-1234567890123456, "T-1200"}};

static ExpectedResult kArabicLong[] = {
  {-5300, "\\u0665\\u066B\\u0663- \\u0623\\u0644\\u0641"}};


class CompactDecimalFormatTest : public IntlTest {
public:
    CompactDecimalFormatTest() {
    }

    void runIndexedTest(int32_t index, UBool exec, const char *&name, char *par=0);
private:
    void TestEnglishShort();
    void TestSerbianShort();
    void TestSerbianLong();
    void TestSerbianLongNegative();
    void TestJapaneseShort();
    void TestSwahiliShort();
    void TestCsShort();
    void TestSkLong();
    void TestSwahiliShortNegative();
    void TestArabicLong();
    void TestFieldPosition();
    void CheckLocale(
        const Locale& locale, UNumberCompactStyle style,
        const ExpectedResult* expectedResult, int32_t expectedResultLength);
    void CheckExpectedResult(
        const CompactDecimalFormat* cdf, const ExpectedResult* expectedResult,
        const char* description);
    static const char *StyleStr(UNumberCompactStyle style);
};

void CompactDecimalFormatTest::runIndexedTest(
    int32_t index, UBool exec, const char *&name, char *) {
  if (exec) {
    logln("TestSuite CompactDecimalFormatTest: ");
  }
  TESTCASE_AUTO_BEGIN;
  TESTCASE_AUTO(TestEnglishShort);
  TESTCASE_AUTO(TestSerbianShort);
  TESTCASE_AUTO(TestSerbianLong);
  TESTCASE_AUTO(TestSerbianLongNegative);
  TESTCASE_AUTO(TestJapaneseShort);
  TESTCASE_AUTO(TestSwahiliShort);
  TESTCASE_AUTO(TestCsShort);
  TESTCASE_AUTO(TestSkLong);
  TESTCASE_AUTO(TestSwahiliShortNegative);
  TESTCASE_AUTO(TestArabicLong);
  TESTCASE_AUTO(TestFieldPosition);
  TESTCASE_AUTO_END;
}

void CompactDecimalFormatTest::TestEnglishShort() {
  CheckLocale("en", UNUM_SHORT, kEnglishShort, LENGTHOF(kEnglishShort));
}

void CompactDecimalFormatTest::TestSerbianShort() {
  CheckLocale("sr", UNUM_SHORT, kSerbianShort, LENGTHOF(kSerbianShort));
}

void CompactDecimalFormatTest::TestSerbianLong() {
  CheckLocale("sr", UNUM_LONG, kSerbianLong, LENGTHOF(kSerbianLong));
}

void CompactDecimalFormatTest::TestSerbianLongNegative() {
  CheckLocale("sr", UNUM_LONG, kSerbianLongNegative, LENGTHOF(kSerbianLongNegative));
}

void CompactDecimalFormatTest::TestJapaneseShort() {
  CheckLocale(Locale::getJapan(), UNUM_SHORT, kJapaneseShort, LENGTHOF(kJapaneseShort));
}

void CompactDecimalFormatTest::TestSwahiliShort() {
  CheckLocale("sw", UNUM_SHORT, kSwahiliShort, LENGTHOF(kSwahiliShort));
}

void CompactDecimalFormatTest::TestFieldPosition() {
  // Swahili uses prefixes which forces offsets in field position to change
  UErrorCode status = U_ZERO_ERROR;
  CompactDecimalFormat* cdf = CompactDecimalFormat::createInstance("sw", UNUM_SHORT, status);
  if (U_FAILURE(status)) {
    errln("Unable to create format object - %s", u_errorName(status));
  }
  FieldPosition fp(UNUM_INTEGER_FIELD);
  UnicodeString result;
  cdf->format(1234567.0, result, fp);
  UnicodeString subString = result.tempSubString(fp.getBeginIndex(), fp.getEndIndex() - fp.getBeginIndex());
  if (subString != UnicodeString("1", -1, US_INV)) {
    errln(UnicodeString("Expected 1, got ") + subString);
  }
  delete cdf;
}

void CompactDecimalFormatTest::TestCsShort() {
  CheckLocale("cs", UNUM_SHORT, kCsShort, LENGTHOF(kCsShort));
}

void CompactDecimalFormatTest::TestSkLong() {
  // In CLDR we have:
  // 1000 {
  //   few{"0"}
  //   one{"0"}
  //   other{"0"}
  CheckLocale("sk", UNUM_LONG, kSkLong, LENGTHOF(kSkLong));
}

void CompactDecimalFormatTest::TestSwahiliShortNegative() {
  CheckLocale("sw", UNUM_SHORT, kSwahiliShortNegative, LENGTHOF(kSwahiliShortNegative));
}

void CompactDecimalFormatTest::TestArabicLong() {
  CheckLocale("ar", UNUM_LONG, kArabicLong, LENGTHOF(kArabicLong));
}

void CompactDecimalFormatTest::CheckLocale(const Locale& locale, UNumberCompactStyle style, const ExpectedResult* expectedResults, int32_t expectedResultLength) {
  UErrorCode status = U_ZERO_ERROR;
  CompactDecimalFormat* cdf = CompactDecimalFormat::createInstance(locale, style, status);
  if (U_FAILURE(status)) {
    errln("Unable to create format object - %s", u_errorName(status));
    return;
  }
  if (cdf == NULL) {
    errln("Got NULL pointer back for format object.");
    return;
  }
  char description[256];
  sprintf(description,"%s - %s", locale.getName(), StyleStr(style));
  for (int32_t i = 0; i < expectedResultLength; i++) {
    CheckExpectedResult(cdf, &expectedResults[i], description);
  }
  delete cdf;
}

void CompactDecimalFormatTest::CheckExpectedResult(
    const CompactDecimalFormat* cdf, const ExpectedResult* expectedResult, const char* description) {
  UnicodeString actual;
  cdf->format(expectedResult->value, actual);
  UnicodeString expected(expectedResult->expected, -1, US_INV);
  expected = expected.unescape();
  if (actual != expected) {
    errln(UnicodeString("Fail: Expected: ") + expected
          + UnicodeString(" Got: ") + actual
          + UnicodeString(" for: ") + UnicodeString(description));
  }
}

const char *CompactDecimalFormatTest::StyleStr(UNumberCompactStyle style) {
  if (style == UNUM_SHORT) {
    return kShortStr;
  }
  return kLongStr;
}

extern IntlTest *createCompactDecimalFormatTest() {
  return new CompactDecimalFormatTest();
}
