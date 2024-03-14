/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 *
 */

#ifndef __TFM_PERIPHERALS_DEF_H__
#define __TFM_PERIPHERALS_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

	/* TODO: NCSDK-22597: Define peripherals */

#include <nrfx.h>

#define TFM_TIMER0_IRQ         (NRFX_IRQ_NUMBER_GET(NRF_TIMER0))

extern struct platform_data_t tfm_peripheral_timer0;

#define TFM_PERIPHERAL_TIMER0       (&tfm_peripheral_timer0)

/*
 * Quantized default IRQ priority, the value is:
 * (Number of configurable priority) / 4: (1UL << __NVIC_PRIO_BITS) / 4
 */
#define DEFAULT_IRQ_PRIORITY    (1UL << (__NVIC_PRIO_BITS - 2))

#define TFM_PERIPHERAL_STD_UART     TFM_PERIPHERAL_UARTE1

#ifdef PSA_API_TEST_IPC
/* see other platforms when supporting this */
#error "Not supported yet"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PERIPHERALS_DEF_H__ */
