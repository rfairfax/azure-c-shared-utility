// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef FREERTOS_H
#define FREERTOS_H

#ifdef __cplusplus
#include <cstdbool>
#else
#include <stdbool.h>
#endif


#define CONFIG_FREERTOS_HZ 100


#define FAKE_TICK_INIT_NO_OVERFLOW 333
#define FAKE_TICK_INTERVAL 1200
#define FAKE_TICK_INIT_WITH_OVERFLOW (UINT32_MAX - FAKE_TICK_INTERVAL - 100)
#define FAKE_TICK_SCALED_INTERVAL FAKE_TICK_INTERVAL * 1000  / CONFIG_FREERTOS_HZ

#ifdef __cplusplus
extern "C"
{
#endif
	// The fake call into FreeRTOS
	uint32_t xTaskGetTickCount();

	// Set up the fake call for unit testing
	void initialize_fake_tick(bool doOverflow);
#ifdef __cplusplus
}
#endif

#endif // FREERTOS_H