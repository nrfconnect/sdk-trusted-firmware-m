/*
 * Copyright (c) 2021-2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NRFX_COMPAT_H__
#define NRFX_COMPAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(GPIO_PIN_CNF_MCUSEL_Msk) && defined(NRF_GPIO_HAS_SEL)
#include <hal/nrf_gpio.h>

typedef enum {
	NRF_GPIO_PIN_MCUSEL_APP         = GPIO_PIN_CNF_MCUSEL_AppMCU,
	NRF_GPIO_PIN_MCUSEL_NETWORK     = GPIO_PIN_CNF_MCUSEL_NetworkMCU,
	NRF_GPIO_PIN_MCUSEL_PERIPHERAL  = GPIO_PIN_CNF_MCUSEL_Peripheral,
	NRF_GPIO_PIN_MCUSEL_TND         = GPIO_PIN_CNF_MCUSEL_TND,
} nrf_gpio_pin_mcusel_t;

static inline void nrf_gpio_pin_mcu_select(uint32_t pin_number, nrf_gpio_pin_mcusel_t mcu)
{
    nrf_gpio_pin_control_select(pin_number, (nrf_gpio_pin_sel_t)mcu);
}
#endif /* defined(GPIO_PIN_CNF_MCUSEL_Msk) && defined(NRF_GPIO_HAS_SEL) */

#ifdef __cplusplus
}
#endif

#endif /* NRFX_COMPAT_H__ */
