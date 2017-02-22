// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/threadapi.h"

#include "freertos/FreeRTOS.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

void* my_gballoc_malloc(size_t size)
{
    return malloc(size);
}

void my_gballoc_free(void* ptr)
{
    free(ptr);
}


#include "azure_c_shared_utility/tickcounter.h"

#define ENABLE_MOCKS

#include "azure_c_shared_utility/gballoc.h"

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

BEGIN_TEST_SUITE(tickcounter_unittests)

TEST_SUITE_INITIALIZE(suite_init)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

    REGISTER_UMOCK_ALIAS_TYPE(TICK_COUNTER_HANDLE, void*);

    REGISTER_GLOBAL_MOCK_HOOK(gballoc_malloc, my_gballoc_malloc);
    REGISTER_GLOBAL_MOCK_HOOK(gballoc_free, my_gballoc_free);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_010: [ tickcounter_get_current_ms shall set *current_ms to the number of milliseconds elapsed since the tickcounter_create call for the specified tick_counter and return 0 to indicate success in situations where the FreeRTOS call xTaskGetTickCount has experienced overflow between the calls to tickcounter_create and tickcounter_get_current_ms. (In FreeRTOS this call has no failure case.) ] */
TEST_FUNCTION(tickcounter_freertos_create_fails)
{
    ///arrange
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1)
        .SetReturn((void*)NULL);

    ///act
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
    ASSERT_IS_NULL(tickHandle);
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_003: [ tickcounter_create shall allocate and initialize an internally-defined TICK_COUNTER_INSTANCE structure and return its pointer on success. ] */
TEST_FUNCTION(tickcounter_freertos_create_succeed)
{
    ///arrange
    STRICT_EXPECTED_CALL(gballoc_malloc(IGNORED_NUM_ARG))
        .IgnoreArgument(1);

    ///act
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();

    ///assert
    ASSERT_IS_NOT_NULL(tickHandle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    tickcounter_destroy(tickHandle);
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_006: [ If the tick_counter parameter is NULL, tickcounter_destroy shall do nothing. ] */
TEST_FUNCTION(tickcounter_freertos_destroy_tick_counter_NULL_succeed)
{
    ///arrange

    ///act
    tickcounter_destroy(NULL);

    ///assert
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_005: [ tickcounter_destroy shall delete the internally-defined TICK_COUNTER_INSTANCE structure specified by the tick_counter parameter. (This call has no failure case.) ] */
TEST_FUNCTION(tickcounter_freertos_destroy_succeed)
{
    ///arrange
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
    umock_c_reset_all_calls();

    STRICT_EXPECTED_CALL(gballoc_free(IGNORED_PTR_ARG))
        .IgnoreArgument(1);

    ///act
    tickcounter_destroy(tickHandle);

    ///assert
    ASSERT_IS_NOT_NULL(tickHandle);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_007: [ If the tick_counter parameter is NULL, tickcounter_get_current_ms shall return a non-zero value to indicate error. ] */
TEST_FUNCTION(tickcounter_freertos_get_current_ms_tick_counter_NULL_fail)
{
    ///arrange
    tickcounter_ms_t current_ms = 0;

    ///act
    int result = tickcounter_get_current_ms(NULL, &current_ms);

    ///assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_008: [ If the current_ms parameter is NULL, tickcounter_get_current_ms shall return a non-zero value to indicate error. ] */
TEST_FUNCTION(tickcounter_freertos_get_current_ms_current_ms_NULL_fail)
{
    ///arrange
    TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
    umock_c_reset_all_calls();

    ///act
    int result = tickcounter_get_current_ms(tickHandle, NULL);

    ///assert
    ASSERT_ARE_NOT_EQUAL(int, 0, result);
    ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

    tickcounter_destroy(tickHandle);
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_009: [ tickcounter_get_current_ms shall set *current_ms to the number of milliseconds elapsed since the tickcounter_create call for the specified tick_counter and return 0 to indicate success in situations where the FreeRTOS call xTaskGetTickCount has not experienced overflow between the calls to tickcounter_create and tickcounter_get_current_ms. (In FreeRTOS this call has no failure case.) ] */
/* Tests_SRS_TICKCOUNTER_FREERTOS_30_001: [ The tickcounter_freertos adapter shall use the following data types as defined in tickcounter.h. */
/* Tests_SRS_TICKCOUNTER_FREERTOS_30_002: [ The tickcounter_freertos adapter shall implement the API calls defined in tickcounter.h:

TICK_COUNTER_HANDLE tickcounter_create(void);
void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter);
int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms);
 
] */
TEST_FUNCTION(tickcounter_freertos_get_current_ms_succeed)
{
	///arrange
	initialize_fake_tick(false);
	TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
	umock_c_reset_all_calls();


	tickcounter_ms_t current_ms = 0;
	tickcounter_ms_t expected_ms = FAKE_TICK_SCALED_INTERVAL;

	///act
	int result = tickcounter_get_current_ms(tickHandle, &current_ms);

	///assert
	ASSERT_ARE_EQUAL(int, 0, result);
	ASSERT_ARE_EQUAL(uint32_t, expected_ms, current_ms);
	ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

	/// clean
	tickcounter_destroy(tickHandle);
}

/* Tests_SRS_TICKCOUNTER_FREERTOS_30_010: [ tickcounter_get_current_ms shall set *current_ms to the number of milliseconds elapsed since the tickcounter_create call for the specified tick_counter and return 0 to indicate success in situations where the FreeRTOS call xTaskGetTickCount has experienced overflow between the calls to tickcounter_create and tickcounter_get_current_ms. (In FreeRTOS this call has no failure case.) ] */
TEST_FUNCTION(tickcounter_freertos_get_current_ms_succeed_despite_overflow)
{
	///arrange
	initialize_fake_tick(true);
	TICK_COUNTER_HANDLE tickHandle = tickcounter_create();
	umock_c_reset_all_calls();


	tickcounter_ms_t current_ms = 0;
	tickcounter_ms_t expected_ms = FAKE_TICK_SCALED_INTERVAL;

	///act
	int result = tickcounter_get_current_ms(tickHandle, &current_ms);

	///assert
	ASSERT_ARE_EQUAL(int, 0, result);
	ASSERT_ARE_EQUAL(uint32_t, expected_ms, current_ms);
	ASSERT_ARE_EQUAL(char_ptr, umock_c_get_expected_calls(), umock_c_get_actual_calls());

	/// clean
	tickcounter_destroy(tickHandle);
}

END_TEST_SUITE(tickcounter_unittests)
