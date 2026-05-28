#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>

void unity_fail(const char* file, int line, const char* msg);
void unity_assert_equal_int(int expected, int actual, const char* file, int line);
void unity_assert_equal_float(float expected, float actual, float delta, const char* file, int line);
void unity_assert_true(int condition, const char* file, int line);
void unity_print_summary(void);

#define TEST_ASSERT_EQUAL_INT(expected, actual)   unity_assert_equal_int((expected), (actual), __FILE__, __LINE__)
#define TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual) unity_assert_equal_float((expected), (actual), (delta), __FILE__, __LINE__)
#define TEST_ASSERT_TRUE(condition)               unity_assert_true((condition), __FILE__, __LINE__)
#define TEST_ASSERT_FALSE(condition)              unity_assert_true(!(condition), __FILE__, __LINE__)

#endif
