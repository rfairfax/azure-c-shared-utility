// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/gballoc.h"
#include <stdint.h>
#include <time.h>
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"


#ifndef CONFIG_FREERTOS_HZ
#define CONFIG_FREERTOS_HZ 100
static uint32_t fakeTick = 0;
static uint32_t xTaskGetTickCount()
{
	fakeTick += 100;
	return fakeTick;
}
#endif

typedef struct TICK_COUNTER_INSTANCE_TAG
{
    uint32_t original_tick_count;
} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    TICK_COUNTER_INSTANCE* result = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (result == NULL)
    {
        LogError("Failed creating tick counter");
    }
	// xTaskGetTickCount has no failure path
	result->original_tick_count = xTaskGetTickCount();
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (tick_counter != NULL)
    {
        free(tick_counter);
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t * current_ms)
{
    int result;

    if (tick_counter == NULL || current_ms == NULL)
    {
        LogError("tickcounter failed: Invalid Arguments.\r\n");
        result = __FAILURE__;
    }
    else
    {
        *current_ms = (tickcounter_ms_t)(
			// Subtraction of two uint32_t's followed by a cast to uint32_t
			// ensures that the result remains valid until the real difference exceeds 32 bits.
			((uint32_t)(xTaskGetTickCount() - tick_counter->original_tick_count))
			// Now that overflow behavior is ensured it is safe to scale. CONFIG_FREERTOS_HZ is typically
			// equal to 1000 or less, so overflow won't happen until the 49.7 day limit
			// of this call's uint32_t return value.
			* 1000.0 / CONFIG_FREERTOS_HZ
			);
        result = 0;
    }

    return result;
}
