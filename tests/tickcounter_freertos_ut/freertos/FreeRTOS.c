// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include "FreeRTOS.h"

static uint32_t fakeTick = FAKE_TICK_INIT_NO_OVERFLOW;

uint32_t xTaskGetTickCount()
{
	fakeTick += FAKE_TICK_INTERVAL;
	return fakeTick;
}

void initialize_fake_tick(bool doOverflow)
{
	fakeTick = FAKE_TICK_INIT_NO_OVERFLOW;
	if (doOverflow)
	{
		fakeTick = FAKE_TICK_INIT_WITH_OVERFLOW;
	}
}

