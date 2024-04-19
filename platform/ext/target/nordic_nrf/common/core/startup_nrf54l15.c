/*
 * Copyright (c) 2022 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * This file is derivative of CMSIS V5.9.0 startup_ARMCM33.c
 * Git SHA: 2b7495b8535bdcb306dac29b9ded4cfb679d7e5c
 */

// TODO: NCSDK-25033: Support interrupt handling in TF-M. The IRQs
// below correspond to nrf53, not nrf54L.

/*
 * Define __VECTOR_TABLE_ATTRIBUTE (which can be provided by cmsis.h)
 * before including cmsis.h because TF-M's linker script
 * tfm_common_s.ld assumes the vector table section is called .vectors
 * while cmsis.h will sometimes (e.g. when cmsis is provided by nrfx)
 * default to using the name .isr_vector.
 */
#define __VECTOR_TABLE_ATTRIBUTE  __attribute__((used, section(".vectors")))

#include "cmsis.h"
#include "startup.h"
#include "exception_info.h"

__NO_RETURN __attribute__((naked)) void default_tfm_IRQHandler(void) {
	EXCEPTION_INFO();

	__ASM volatile(
        "BL        default_irq_handler     \n"
        "B         .                       \n"
    );
}

DEFAULT_IRQ_HANDLER(NMI_Handler)
DEFAULT_IRQ_HANDLER(HardFault_Handler)
DEFAULT_IRQ_HANDLER(MemManage_Handler)
DEFAULT_IRQ_HANDLER(BusFault_Handler)
DEFAULT_IRQ_HANDLER(UsageFault_Handler)
DEFAULT_IRQ_HANDLER(SecureFault_Handler)
DEFAULT_IRQ_HANDLER(SVC_Handler)
DEFAULT_IRQ_HANDLER(DebugMon_Handler)
DEFAULT_IRQ_HANDLER(PendSV_Handler)
DEFAULT_IRQ_HANDLER(SysTick_Handler)

DEFAULT_IRQ_HANDLER(SWI00_Handler)
DEFAULT_IRQ_HANDLER(SWI01_Handler)
DEFAULT_IRQ_HANDLER(SWI02_Handler)
DEFAULT_IRQ_HANDLER(SWI03_Handler)
DEFAULT_IRQ_HANDLER(MPC00_Handler)
DEFAULT_IRQ_HANDLER(AAR00_CCM00_Handler)
DEFAULT_IRQ_HANDLER(ECB00_Handler)
DEFAULT_IRQ_HANDLER(SERIAL00_Handler)
DEFAULT_IRQ_HANDLER(RRAMC_Handler)
DEFAULT_IRQ_HANDLER(VPR00_Handler)
DEFAULT_IRQ_HANDLER(CTRLAP_Handler)
DEFAULT_IRQ_HANDLER(CM33SS_Handler)
DEFAULT_IRQ_HANDLER(TIMER00_Handler)
DEFAULT_IRQ_HANDLER(TIMER10_Handler)
DEFAULT_IRQ_HANDLER(RTC10_Handler)
DEFAULT_IRQ_HANDLER(EGU10_Handler)
DEFAULT_IRQ_HANDLER(AAR10_CCM10_Handler)
DEFAULT_IRQ_HANDLER(ECB10_Handler)
DEFAULT_IRQ_HANDLER(RADIO_0_Handler)
DEFAULT_IRQ_HANDLER(RADIO_1_Handler)
DEFAULT_IRQ_HANDLER(SERIAL20_Handler)
DEFAULT_IRQ_HANDLER(SERIAL21_Handler)
DEFAULT_IRQ_HANDLER(SERIAL22_Handler)
DEFAULT_IRQ_HANDLER(EGU20_Handler)
DEFAULT_IRQ_HANDLER(TIMER20_Handler)
DEFAULT_IRQ_HANDLER(TIMER21_Handler)
DEFAULT_IRQ_HANDLER(TIMER22_Handler)
DEFAULT_IRQ_HANDLER(TIMER23_Handler)
DEFAULT_IRQ_HANDLER(TIMER24_Handler)
DEFAULT_IRQ_HANDLER(PWM20_Handler)
DEFAULT_IRQ_HANDLER(PWM21_Handler)
DEFAULT_IRQ_HANDLER(PWM22_Handler)
DEFAULT_IRQ_HANDLER(SAADC_Handler)
DEFAULT_IRQ_HANDLER(NFCT_Handler)
DEFAULT_IRQ_HANDLER(TEMP_Handler)
DEFAULT_IRQ_HANDLER(GPIOTE20_0_Handler)
DEFAULT_IRQ_HANDLER(GPIOTE20_1_Handler)
DEFAULT_IRQ_HANDLER(TAMPC_Handler)
DEFAULT_IRQ_HANDLER(I2S20_Handler)
DEFAULT_IRQ_HANDLER(QDEC20_Handler)
DEFAULT_IRQ_HANDLER(QDEC21_Handler)
DEFAULT_IRQ_HANDLER(GRTC_0_Handler)
DEFAULT_IRQ_HANDLER(GRTC_1_Handler)
DEFAULT_IRQ_HANDLER(GRTC_2_Handler)
DEFAULT_IRQ_HANDLER(GRTC_3_Handler)
DEFAULT_IRQ_HANDLER(SERIAL30_Handler)
DEFAULT_IRQ_HANDLER(RTC30_Handler)
DEFAULT_IRQ_HANDLER(COMP_LPCOMP_Handler)
DEFAULT_IRQ_HANDLER(WDT30_Handler)
DEFAULT_IRQ_HANDLER(WDT31_Handler)
DEFAULT_IRQ_HANDLER(GPIOTE30_0_Handler)
DEFAULT_IRQ_HANDLER(GPIOTE30_1_Handler)
DEFAULT_IRQ_HANDLER(CLOCK_POWER_Handler)

#if defined(DOMAIN_NS) || defined(BL2)
DEFAULT_IRQ_HANDLER(MPC00_IRQHandler)
DEFAULT_IRQ_HANDLER(SPU00_IRQHandler)
DEFAULT_IRQ_HANDLER(SPU10_IRQHandler)
DEFAULT_IRQ_HANDLER(SPU20_IRQHandler)
DEFAULT_IRQ_HANDLER(SPU30_IRQHandler)
DEFAULT_IRQ_HANDLER(CRACEN_IRQHandler)
#else
// TODO: Move into the SPU error handling code
DEFAULT_IRQ_HANDLER(SPU00_IRQHandler)
DEFAULT_IRQ_HANDLER(SPU10_IRQHandler)
DEFAULT_IRQ_HANDLER(SPU20_IRQHandler)
DEFAULT_IRQ_HANDLER(SPU30_IRQHandler)
#endif

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

const VECTOR_TABLE_Type __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE = {
  (VECTOR_TABLE_Type)(&__INITIAL_SP),     /*      Initial Stack Pointer */
/* Exceptions */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,           /* MPU Fault Handler */
    BusFault_Handler,
    UsageFault_Handler,
    SecureFault_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    SVC_Handler,
    DebugMon_Handler,
    default_tfm_IRQHandler,
    PendSV_Handler,
    SysTick_Handler,
/* Device specific interrupt handlers */
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    SWI00_Handler,
    SWI01_Handler,
    SWI02_Handler,
    SWI03_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    SPU00_IRQHandler,
    MPC00_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    AAR00_CCM00_Handler,
    ECB00_Handler,
    CRACEN_IRQHandler,
    default_tfm_IRQHandler,
    SERIAL00_Handler,
    RRAMC_Handler,
    VPR00_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    CTRLAP_Handler,
    CM33SS_Handler,
    default_tfm_IRQHandler,
    TIMER00_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    SPU10_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    TIMER10_Handler,
    RTC10_Handler,
    EGU10_Handler,
    AAR10_CCM10_Handler,
    ECB10_Handler,
    RADIO_0_Handler,
    RADIO_1_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    SPU20_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    SERIAL20_Handler,
    SERIAL21_Handler,
    SERIAL22_Handler,
    EGU20_Handler,
    TIMER20_Handler,
    TIMER21_Handler,
    TIMER22_Handler,
    TIMER23_Handler,
    TIMER24_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    PWM20_Handler,
    PWM21_Handler,
    PWM22_Handler,
    SAADC_Handler,
    NFCT_Handler,
    TEMP_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    GPIOTE20_0_Handler,
    GPIOTE20_1_Handler,
    TAMPC_Handler,
    I2S20_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    QDEC20_Handler,
    QDEC21_Handler,
    GRTC_0_Handler,
    GRTC_1_Handler,
    GRTC_2_Handler,
    GRTC_3_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    SPU30_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    SERIAL30_Handler,
    RTC30_Handler,
    COMP_LPCOMP_Handler,
    default_tfm_IRQHandler,
    WDT30_Handler,
    WDT31_Handler,
    default_tfm_IRQHandler,
    default_tfm_IRQHandler,
    GPIOTE30_0_Handler,
    GPIOTE30_1_Handler,
    CLOCK_POWER_Handler,
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif
