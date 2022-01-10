/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include "zcbor_decode.h"
#include "zcbor_encode.h"
#include "zcbor_debug.h"

void test_main(void)
{
	ztest_test_suite(zcbor_unit_tests,
	);
	ztest_run_test_suite(zcbor_unit_tests);
}
