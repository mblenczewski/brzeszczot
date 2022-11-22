#ifndef TEST_H
#define TEST_H

#include "common.h"

#define TEST_PASS() return 1;
#define TEST_FAIL() return 0;

#define TESTS_BEGIN() s32 __test_suite_result = 0;
#define TESTS_END() return __test_suite_result;

#define TEST_RUN(name)								\
{										\
	errlog("%s:%s:\n", __FILE__, #name);					\
	if (name()) {								\
		errlog("%s:%s: OK\n", __FILE__, #name);				\
	} else {								\
		errlog("%s:%s: FAILED\n", __FILE__, #name);			\
		__test_suite_result = 1;					\
	}									\
}

#define _TEST_ASSERT_IMPL(cond, msg)						\
errlog("[%s:%d] %s: %s\n", __func__, __LINE__, #cond, msg)

#define TEST_ASSERT(cond, msg)							\
if (!(cond)) { _TEST_ASSERT_IMPL(cond, msg); TEST_FAIL() }

#define TEST_EXPECT(cond, msg)							\
if (!(cond)) { _TEST_ASSERT_IMPL(cond, msg); }

#endif /* TEST_H */
