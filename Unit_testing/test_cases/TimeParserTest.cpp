#include <gtest/gtest.h>
#include "../TimeParser.h"

// Test suite: TimeParserTest

// 1 TIME STRING TESTS
TEST(TimeParserTest, TestCaseCorrectTime) {

    // Note that this test fails on purpose!!

    // Test with correct time string
    char time_test[] = "000005";
    ASSERT_EQ(time_parse(time_test), 5);

    char time_test2[] = "000120";
    ASSERT_EQ(time_parse(time_test2), 80);
}

// 2 BOUNDARY TESTS
TEST(TimeParserTest, TestInvalidHour) {
    // Hours
    char time_test[] = "240000";
    ASSERT_EQ(time_parse(time_test), TIME_VALUE_ERROR);

    // Minutes
    char time_test2[] = "126000";
    ASSERT_EQ(time_parse(time_test2), TIME_VALUE_ERROR);

    // seconds
    char time_test3[] = "120060";
    ASSERT_EQ(time_parse(time_test3), TIME_VALUE_ERROR);
}

// 3 NULL VALUE TEST
TEST(TimeParserTest, TestNullPointer) {
    ASSERT_EQ(time_parse(NULL), TIME_LEN_ERROR);
}

// 4 MSG LENGHT TEST
TEST(TimeParserTest, TestInvalidLenght) {
    char short_time[] = "12345";
    ASSERT_EQ(time_parse(short_time), TIME_LEN_ERROR);

    char long_time[] = "1234567";
    ASSERT_EQ(time_parse(long_time), TIME_LEN_ERROR);
}

// 5 ZERO TIME TEST
TEST(TimeParserTest, TestZeroTime) {
    char zero_time[] = "000000";
    ASSERT_EQ(time_parse(zero_time), TIME_ZERO_ERROR);
}

// 6 IS NUMBER TEST
TEST(TimeParserTest, TestNotNumber) {
    char invalid_time[] = "12345A";
    ASSERT_EQ(time_parse(invalid_time), TIME_ARRAY_ERROR);
}

// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html
