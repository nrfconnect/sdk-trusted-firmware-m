/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef TFM_PERIPHERALS_CONFIG_H__
#define TFM_PERIPHERALS_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SECURE_UART30
#define TFM_PERIPHERAL_UARTE30_SECURE 1
#endiff

#if TFM_PARTITION_SLIH_TEST || TFM_PARTITION_FLIH_TEST
#define TFM_PERIPHERAL_TIMER00_SECURE 1
#endif


#if defined(NRF54L15_ENGA_XXAA)
    #include <tfm_peripherals_config_nrf54l15.h>
#else
    #error "Unknown device."
#endif

#ifdef __cplusplus
}
#endif

#endif /* TFM_PERIPHERAL_CONFIG_H__ */
