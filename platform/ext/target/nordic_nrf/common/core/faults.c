/*
 * Copyright (c) 2018-2022, Arm Limited. All rights reserved.
 * Copyright (c) 2020, Nordic Semiconductor ASA. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "cmsis.h"
#include "spu.h"
#include "utilities.h"
#include "nrf_exception_info.h"
/* "exception_info.h" must be the last include because of the IAR pragma */
#include "exception_info.h"

void SPU_Handler(void)
{
#ifdef TFM_EXCEPTION_INFO_DUMP
    nrf_exception_info_store_context();
#endif
    /* Clear SPU interrupt flag and pending SPU IRQ */
    spu_clear_events();

    NVIC_ClearPendingIRQ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) - NVIC_USER_IRQ_OFFSET);

    tfm_core_panic();
}

__attribute__((naked)) void SPU_IRQHandler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "BL        SPU_Handler             \n"
        "B         .                       \n"
    );
}

#ifdef NRF_SPU00
__attribute__((naked)) void SPU00_IRQHandler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "BL        SPU_Handler             \n"
        "B         .                       \n"
    );
}
#endif

#ifdef NRF_SPU10
__attribute__((naked)) void SPU10_IRQHandler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "BL        SPU_Handler             \n"
        "B         .                       \n"
    );
}
#endif

#ifdef NRF_SPU20
__attribute__((naked)) void SPU20_IRQHandler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "BL        SPU_Handler             \n"
        "B         .                       \n"
    );
}
#endif

#ifdef NRF_SPU30
__attribute__((naked)) void SPU30_IRQHandler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "BL        SPU_Handler             \n"
        "B         .                       \n"
    );
}
#endif

#ifdef NRF_MPC00
void MPC_Handler(void)
{
#ifdef TFM_EXCEPTION_INFO_DUMP
    nrf_exception_info_store_context();
#endif

    /* Clear MPC interrupt flag and pending MPC IRQ */
    mpc_clear_events();

    NVIC_ClearPendingIRQ(MPC00_IRQn);

    tfm_core_panic();
}

__attribute__((naked)) void MPC00_IRQHandler(void)
{
    EXCEPTION_INFO();

    __ASM volatile(
        "BL        MPC_Handler             \n"
        "B         .                       \n"
    );
}
#endif
