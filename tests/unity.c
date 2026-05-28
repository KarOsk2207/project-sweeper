#include "unity.h"

static int tests_passed = 0;
static int tests_failed = 0;

void unity_fail(const char* file, int line, const char* msg) {
    printf("FAIL: %s:%d: %s\n", file, line, msg);
    tests_failed++;
}

void unity_assert_equal_int(int expected, int actual, const char* file, int line) {
    if (expected != actual) {
        printf("FAIL: %s:%d: Expected %d, got %d\n", file, line, expected, actual);
        tests_failed++;
    } else {
        tests_passed++;
    }
}

void unity_assert_equal_float(float expected, float actual, float delta, const char* file, int line) {
    if ((expected - actual > delta) || (actual - expected > delta)) {
        printf("FAIL: %s:%d: Expected %f, got %f (delta %f)\n", file, line, expected, actual, delta);
        tests_failed++;
    } else {
        tests_passed++;
    }
}

void unity_assert_true(int condition, const char* file, int line) {
    if (!condition) {
        printf("FAIL: %s:%d: Condition is false\n", file, line);
        tests_failed++;
    } else {
        tests_passed++;
    }
}

void unity_print_summary(void) {
    printf("\n----------------------\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("----------------------\n");
}
