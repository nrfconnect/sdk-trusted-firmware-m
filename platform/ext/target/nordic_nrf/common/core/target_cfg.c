/*
 * Copyright (c) 2018-2020 Arm Limited. All rights reserved.
 * Copyright (c) 2020 Nordic Semiconductor ASA.
 * Copyright (c) 2021 Laird Connectivity.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "target_cfg.h"
#include "region_defs.h"
#include "tfm_plat_defs.h"
#include "tfm_peripherals_config.h"
#include "utilities.h"
#include "region.h"
#include "array.h"

#include <autoconf.h>

#include <spu.h>
#include <nrfx.h>

#include <hal/nrf_gpio.h>
#include <hal/nrf_spu.h>

#if CONFIG_NRF_RRAM_READYNEXT_TIMEOUT_VALUE > 0
#include <hal/nrf_rramc.h>
#endif

#ifdef CACHE_PRESENT
#include <hal/nrf_cache.h>
#endif

#ifdef NVMC_PRESENT
#include <nrfx_nvmc.h>
#include <hal/nrf_nvmc.h>
#endif

#ifdef MPC_PRESENT
#include <hal/nrf_mpc.h>
#endif

#define PIN_XL1 0
#define PIN_XL2 1

#if TFM_PERIPHERAL_DCNF_SECURE
struct platform_data_t tfm_peripheral_dcnf = {
    NRF_DCNF_S_BASE,
    NRF_DCNF_S_BASE + (sizeof(NRF_DCNF_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_FPU_SECURE
struct platform_data_t tfm_peripheral_fpu = {
    NRF_FPU_S_BASE,
    NRF_FPU_S_BASE + (sizeof(NRF_FPU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_OSCILLATORS_SECURE
struct platform_data_t tfm_peripheral_oscillators = {
    NRF_OSCILLATORS_S_BASE,
    NRF_OSCILLATORS_S_BASE + (sizeof(NRF_OSCILLATORS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_REGULATORS_SECURE
struct platform_data_t tfm_peripheral_regulators = {
    NRF_REGULATORS_S_BASE,
    NRF_REGULATORS_S_BASE + (sizeof(NRF_REGULATORS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_CLOCK_SECURE
struct platform_data_t tfm_peripheral_clock = {
    NRF_CLOCK_S_BASE,
    NRF_CLOCK_S_BASE + (sizeof(NRF_CLOCK_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_POWER_SECURE
struct platform_data_t tfm_peripheral_power = {
    NRF_POWER_S_BASE,
    NRF_POWER_S_BASE + (sizeof(NRF_POWER_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_RESET_SECURE
struct platform_data_t tfm_peripheral_reset = {
    NRF_RESET_S_BASE,
    NRF_RESET_S_BASE + (sizeof(NRF_RESET_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM0_SECURE
struct platform_data_t tfm_peripheral_spim0 = {
    NRF_SPIM0_S_BASE,
    NRF_SPIM0_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIS0_SECURE
struct platform_data_t tfm_peripheral_spis0 = {
    NRF_SPIS0_S_BASE,
    NRF_SPIS0_S_BASE + (sizeof(NRF_SPIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIM0_SECURE
struct platform_data_t tfm_peripheral_twim0 = {
    NRF_TWIM0_S_BASE,
    NRF_TWIM0_S_BASE + (sizeof(NRF_TWIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIS0_SECURE
struct platform_data_t tfm_peripheral_twis0 = {
    NRF_TWIS0_S_BASE,
    NRF_TWIS0_S_BASE + (sizeof(NRF_TWIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_UARTE0_SECURE
struct platform_data_t tfm_peripheral_uarte0 = {
    NRF_UARTE0_S_BASE,
    NRF_UARTE0_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM1_SECURE
struct platform_data_t tfm_peripheral_spim1 = {
    NRF_SPIM1_S_BASE,
    NRF_SPIM1_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIS1_SECURE
struct platform_data_t tfm_peripheral_spis1 = {
    NRF_SPIS1_S_BASE,
    NRF_SPIS1_S_BASE + (sizeof(NRF_SPIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIM1_SECURE
struct platform_data_t tfm_peripheral_twim1 = {
    NRF_TWIM1_S_BASE,
    NRF_TWIM1_S_BASE + (sizeof(NRF_TWIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIS1_SECURE
struct platform_data_t tfm_peripheral_twis1 = {
    NRF_TWIS1_S_BASE,
    NRF_TWIS1_S_BASE + (sizeof(NRF_TWIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_UARTE1_SECURE
struct platform_data_t tfm_peripheral_uarte1 = {
    NRF_UARTE1_S_BASE,
    NRF_UARTE1_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM4_SECURE
struct platform_data_t tfm_peripheral_spim4 = {
    NRF_SPIM4_S_BASE,
    NRF_SPIM4_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM2_SECURE
struct platform_data_t tfm_peripheral_spim2 = {
    NRF_SPIM2_S_BASE,
    NRF_SPIM2_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIS2_SECURE
struct platform_data_t tfm_peripheral_spis2 = {
    NRF_SPIS2_S_BASE,
    NRF_SPIS2_S_BASE + (sizeof(NRF_SPIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIM2_SECURE
struct platform_data_t tfm_peripheral_twim2 = {
    NRF_TWIM2_S_BASE,
    NRF_TWIM2_S_BASE + (sizeof(NRF_TWIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIS2_SECURE
struct platform_data_t tfm_peripheral_twis2 = {
    NRF_TWIS2_S_BASE,
    NRF_TWIS2_S_BASE + (sizeof(NRF_TWIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_UARTE2_SECURE
struct platform_data_t tfm_peripheral_uarte2 = {
    NRF_UARTE2_S_BASE,
    NRF_UARTE2_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIM3_SECURE
struct platform_data_t tfm_peripheral_spim3 = {
    NRF_SPIM3_S_BASE,
    NRF_SPIM3_S_BASE + (sizeof(NRF_SPIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SPIS3_SECURE
struct platform_data_t tfm_peripheral_spis3 = {
    NRF_SPIS3_S_BASE,
    NRF_SPIS3_S_BASE + (sizeof(NRF_SPIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIM3_SECURE
struct platform_data_t tfm_peripheral_twim3 = {
    NRF_TWIM3_S_BASE,
    NRF_TWIM3_S_BASE + (sizeof(NRF_TWIM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TWIS3_SECURE
struct platform_data_t tfm_peripheral_twis3 = {
    NRF_TWIS3_S_BASE,
    NRF_TWIS3_S_BASE + (sizeof(NRF_TWIS_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_UARTE3_SECURE
struct platform_data_t tfm_peripheral_uarte3 = {
    NRF_UARTE3_S_BASE,
    NRF_UARTE3_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_UARTE30_SECURE
struct platform_data_t tfm_peripheral_uarte30 = {
    NRF_UARTE30_S_BASE,
    NRF_UARTE30_S_BASE + (sizeof(NRF_UARTE_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_SAADC_SECURE
struct platform_data_t tfm_peripheral_saadc = {
    NRF_SAADC_S_BASE,
    NRF_SAADC_S_BASE + (sizeof(NRF_SAADC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TIMER0_SECURE
struct platform_data_t tfm_peripheral_timer0 = {
    NRF_TIMER0_S_BASE,
    NRF_TIMER0_S_BASE + (sizeof(NRF_TIMER_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TIMER1_SECURE
struct platform_data_t tfm_peripheral_timer1 = {
    NRF_TIMER1_S_BASE,
    NRF_TIMER1_S_BASE + (sizeof(NRF_TIMER_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_TIMER2_SECURE
struct platform_data_t tfm_peripheral_timer2 = {
    NRF_TIMER2_S_BASE,
    NRF_TIMER2_S_BASE + (sizeof(NRF_TIMER_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_RTC0_SECURE
struct platform_data_t tfm_peripheral_rtc0 = {
    NRF_RTC0_S_BASE,
    NRF_RTC0_S_BASE + (sizeof(NRF_RTC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_RTC1_SECURE
struct platform_data_t tfm_peripheral_rtc1 = {
    NRF_RTC1_S_BASE,
    NRF_RTC1_S_BASE + (sizeof(NRF_RTC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_DPPI_SECURE
struct platform_data_t tfm_peripheral_dppi = {
    NRF_DPPIC_S_BASE,
    NRF_DPPIC_S_BASE + (sizeof(NRF_DPPIC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_WDT_SECURE
struct platform_data_t tfm_peripheral_wdt = {
    NRF_WDT_S_BASE,
    NRF_WDT_S_BASE + (sizeof(NRF_WDT_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_WDT0_SECURE
struct platform_data_t tfm_peripheral_wdt0 = {
    NRF_WDT0_S_BASE,
    NRF_WDT0_S_BASE + (sizeof(NRF_WDT_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_WDT1_SECURE
struct platform_data_t tfm_peripheral_wdt1 = {
    NRF_WDT1_S_BASE,
    NRF_WDT1_S_BASE + (sizeof(NRF_WDT_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_COMP_SECURE
struct platform_data_t tfm_peripheral_comp = {
    NRF_COMP_S_BASE,
    NRF_COMP_S_BASE + (sizeof(NRF_COMP_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_LPCOMP_SECURE
struct platform_data_t tfm_peripheral_lpcomp = {
    NRF_LPCOMP_S_BASE,
    NRF_LPCOMP_S_BASE + (sizeof(NRF_LPCOMP_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU0_SECURE
struct platform_data_t tfm_peripheral_egu0 = {
    NRF_EGU0_S_BASE,
    NRF_EGU0_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU1_SECURE
struct platform_data_t tfm_peripheral_egu1 = {
    NRF_EGU1_S_BASE,
    NRF_EGU1_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU2_SECURE
struct platform_data_t tfm_peripheral_egu2 = {
    NRF_EGU2_S_BASE,
    NRF_EGU2_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU3_SECURE
struct platform_data_t tfm_peripheral_egu3 = {
    NRF_EGU3_S_BASE,
    NRF_EGU3_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU4_SECURE
struct platform_data_t tfm_peripheral_egu4 = {
    NRF_EGU4_S_BASE,
    NRF_EGU4_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_EGU5_SECURE
struct platform_data_t tfm_peripheral_egu5 = {
    NRF_EGU5_S_BASE,
    NRF_EGU5_S_BASE + (sizeof(NRF_EGU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PWM0_SECURE
struct platform_data_t tfm_peripheral_pwm0 = {
    NRF_PWM0_S_BASE,
    NRF_PWM0_S_BASE + (sizeof(NRF_PWM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PWM1_SECURE
struct platform_data_t tfm_peripheral_pwm1 = {
    NRF_PWM1_S_BASE,
    NRF_PWM1_S_BASE + (sizeof(NRF_PWM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PWM2_SECURE
struct platform_data_t tfm_peripheral_pwm2 = {
    NRF_PWM2_S_BASE,
    NRF_PWM2_S_BASE + (sizeof(NRF_PWM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PWM3_SECURE
struct platform_data_t tfm_peripheral_pwm3 = {
    NRF_PWM3_S_BASE,
    NRF_PWM3_S_BASE + (sizeof(NRF_PWM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PDM0_SECURE
struct platform_data_t tfm_peripheral_pdm0 = {
    NRF_PDM0_S_BASE,
    NRF_PDM0_S_BASE + (sizeof(NRF_PDM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_PDM_SECURE
struct platform_data_t tfm_peripheral_pdm = {
    NRF_PDM_S_BASE,
    NRF_PDM_S_BASE + (sizeof(NRF_PDM_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_I2S0_SECURE
struct platform_data_t tfm_peripheral_i2s0 = {
    NRF_I2S0_S_BASE,
    NRF_I2S0_S_BASE + (sizeof(NRF_I2S_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_I2S_SECURE
struct platform_data_t tfm_peripheral_i2s = {
    NRF_I2S_S_BASE,
    NRF_I2S_S_BASE + (sizeof(NRF_I2S_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_IPC_SECURE
struct platform_data_t tfm_peripheral_ipc = {
    NRF_IPC_S_BASE,
    NRF_IPC_S_BASE + (sizeof(NRF_IPC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_FPU_SECURE
struct platform_data_t tfm_peripheral_fpu = {
    NRF_FPU_S_BASE,
    NRF_FPU_S_BASE + (sizeof(NRF_FPU_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_QSPI_SECURE
struct platform_data_t tfm_peripheral_qspi = {
    NRF_QSPI_S_BASE,
    NRF_QSPI_S_BASE + (sizeof(NRF_QSPI_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_NFCT_SECURE
struct platform_data_t tfm_peripheral_nfct = {
    NRF_NFCT_S_BASE,
    NRF_NFCT_S_BASE + (sizeof(NRF_NFCT_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_MUTEX_SECURE
struct platform_data_t tfm_peripheral_mutex = {
    NRF_MUTEX_S_BASE,
    NRF_MUTEX_S_BASE + (sizeof(NRF_MUTEX_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_QDEC0_SECURE
struct platform_data_t tfm_peripheral_qdec0 = {
    NRF_QDEC0_S_BASE,
    NRF_QDEC0_S_BASE + (sizeof(NRF_QDEC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_QDEC1_SECURE
struct platform_data_t tfm_peripheral_qdec1 = {
    NRF_QDEC1_S_BASE,
    NRF_QDEC1_S_BASE + (sizeof(NRF_QDEC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_USBD_SECURE
struct platform_data_t tfm_peripheral_usbd = {
    NRF_USBD_S_BASE,
    NRF_USBD_S_BASE + (sizeof(NRF_USBD_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_USBREG_SECURE
struct platform_data_t tfm_peripheral_usbreg = {
    NRF_USBREGULATOR_S_BASE,
    NRF_USBREGULATOR_S_BASE + (sizeof(NRF_USBREG_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_NVMC_SECURE
struct platform_data_t tfm_peripheral_nvmc = {
    NRF_NVMC_S_BASE,
    NRF_NVMC_S_BASE + (sizeof(NRF_NVMC_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_GPIO0_SECURE
struct platform_data_t tfm_peripheral_gpio0 = {
    NRF_P0_S_BASE,
    NRF_P0_S_BASE + (sizeof(NRF_GPIO_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_GPIO1_SECURE
struct platform_data_t tfm_peripheral_gpio1 = {
    NRF_P1_S_BASE,
    NRF_P1_S_BASE + (sizeof(NRF_GPIO_Type) - 1),
};
#endif

#if TFM_PERIPHERAL_VMC_SECURE
struct platform_data_t tfm_peripheral_vmc = {
    NRF_VMC_S_BASE,
    NRF_VMC_S_BASE + (sizeof(NRF_VMC_Type) - 1),
};
#endif

#ifdef PSA_API_TEST_IPC
struct platform_data_t
    tfm_peripheral_FF_TEST_SERVER_PARTITION_MMIO = {
        FF_TEST_SERVER_PARTITION_MMIO_START,
        FF_TEST_SERVER_PARTITION_MMIO_END
};

struct platform_data_t
    tfm_peripheral_FF_TEST_DRIVER_PARTITION_MMIO = {
        FF_TEST_DRIVER_PARTITION_MMIO_START,
        FF_TEST_DRIVER_PARTITION_MMIO_END
};

/* This platform implementation uses PSA_TEST_SCRATCH_AREA for
 * storing the state between resets, but the FF_TEST_NVMEM_REGIONS
 * definitons are still needed for tests to compile.
 */
#define FF_TEST_NVMEM_REGION_START  0xFFFFFFFF
#define FF_TEST_NVMEM_REGION_END    0xFFFFFFFF
struct platform_data_t
    tfm_peripheral_FF_TEST_NVMEM_REGION = {
        FF_TEST_NVMEM_REGION_START,
        FF_TEST_NVMEM_REGION_END
};
#endif /* PSA_API_TEST_IPC */


/* The section names come from the scatter file */
REGION_DECLARE(Load$$LR$$, LR_NS_PARTITION, $$Base);
REGION_DECLARE(Image$$, ER_VENEER, $$Base);
REGION_DECLARE(Image$$, VENEER_ALIGN, $$Limit);

const struct memory_region_limits memory_regions = {
    .non_secure_code_start =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
        BL2_HEADER_SIZE,

    .non_secure_partition_base =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base),

    .non_secure_partition_limit =
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_NS_PARTITION, $$Base) +
        NS_PARTITION_SIZE - 1,

    .veneer_base =
        (uint32_t)&REGION_NAME(Image$$, ER_VENEER, $$Base),

    .veneer_limit =
        (uint32_t)&REGION_NAME(Image$$, VENEER_ALIGN, $$Limit),

#ifdef NRF_NS_SECONDARY
    .secondary_partition_base = SECONDARY_PARTITION_START,
    .secondary_partition_limit = SECONDARY_PARTITION_START +
        SECONDARY_PARTITION_SIZE - 1,
#endif /* NRF_NS_SECONDARY */

#ifdef NRF_NS_STORAGE_PARTITION_START
    .non_secure_storage_partition_base = NRF_NS_STORAGE_PARTITION_START,
    .non_secure_storage_partition_limit = NRF_NS_STORAGE_PARTITION_START +
        NRF_NS_STORAGE_PARTITION_SIZE - 1,
#endif /* NRF_NS_STORAGE_PARTITION_START */
};

/* To write into AIRCR register, 0x5FA value must be write to the VECTKEY field,
 * otherwise the processor ignores the write.
 */
#define SCB_AIRCR_WRITE_MASK ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos))

enum tfm_plat_err_t enable_fault_handlers(void)
{
    /* Explicitly set secure fault priority to the highest */
    NVIC_SetPriority(SecureFault_IRQn, 0);

    /* Enables BUS, MEM, USG and Secure faults */
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk |
                  SCB_SHCSR_BUSFAULTENA_Msk |
                  SCB_SHCSR_MEMFAULTENA_Msk |
                  SCB_SHCSR_SECUREFAULTENA_Msk;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t system_reset_cfg(void)
{
    uint32_t reg_value = SCB->AIRCR;

    /* Clear SCB_AIRCR_VECTKEY value */
    reg_value &= ~(uint32_t)(SCB_AIRCR_VECTKEY_Msk);

    /* Enable system reset request only to the secure world */
    reg_value |= (uint32_t)(SCB_AIRCR_WRITE_MASK | SCB_AIRCR_SYSRESETREQS_Msk);

    SCB->AIRCR = reg_value;

    return TFM_PLAT_ERR_SUCCESS;
}

enum tfm_plat_err_t init_debug(void)
{
#if defined(NRF91_SERIES)

#if !defined(DAUTH_CHIP_DEFAULT)
#error "Debug access on this platform can only be configured by programming the corresponding registers in UICR."
#endif

#elif defined(NRF53_SERIES)

#if defined(DAUTH_NONE)
    /* Disable debugging */
    NRF_CTRLAP->APPROTECT.DISABLE = 0;
    NRF_CTRLAP->SECUREAPPROTECT.DISABLE = 0;
#elif defined(DAUTH_NS_ONLY)
    /* Allow debugging Non-Secure only */
    NRF_CTRLAP->APPROTECT.DISABLE = NRF_UICR->APPROTECT;
    NRF_CTRLAP->SECUREAPPROTECT.DISABLE = 0;
#elif defined(DAUTH_FULL) || defined(DAUTH_CHIP_DEFAULT)
    /* Allow debugging */
    /* Use the configuration in UICR. */
    NRF_CTRLAP->APPROTECT.DISABLE = NRF_UICR->APPROTECT;
    NRF_CTRLAP->SECUREAPPROTECT.DISABLE = NRF_UICR->SECUREAPPROTECT;
#else
#error "No debug authentication setting is provided."
#endif

    /* Lock access to APPROTECT, SECUREAPPROTECT */
    NRF_CTRLAP->APPROTECT.LOCK = CTRLAPPERI_APPROTECT_LOCK_LOCK_Locked <<
        CTRLAPPERI_APPROTECT_LOCK_LOCK_Msk;
    NRF_CTRLAP->SECUREAPPROTECT.LOCK = CTRLAPPERI_SECUREAPPROTECT_LOCK_LOCK_Locked <<
        CTRLAPPERI_SECUREAPPROTECT_LOCK_LOCK_Msk;

#elif defined(NRF54L15_ENGA_XXAA)
	// TODO: NCSDK-25047: Support nRF54L
#else

#error "Unrecognized platform"

#endif

    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt target state to NS configuration ----------*/
enum tfm_plat_err_t nvic_interrupt_target_state_cfg(void)
{
    /* Target every interrupt to NS; unimplemented interrupts will be Write-Ignored */
    for (uint8_t i = 0; i < sizeof(NVIC->ITNS) / sizeof(NVIC->ITNS[0]); i++) {
        NVIC->ITNS[i] = 0xFFFFFFFF;
    }

    /* Make sure that the SPU instance(s) are targeted to S state */
	for(int i = 0; i < ARRAY_SIZE(spu_instances); i++) {
		NVIC_ClearTargetState(NRFX_IRQ_NUMBER_GET(spu_instances[i]));
	}

#ifdef NRF_CRACEN
	NVIC_ClearTargetState(NRFX_IRQ_NUMBER_GET(NRF_CRACEN));
#endif

#ifdef SECURE_UART1
#if NRF_SECURE_UART_INSTANCE == 0
    /* UARTE0 is a secure peripheral, so its IRQ has to target S state */
    NVIC_ClearTargetState(NRFX_IRQ_NUMBER_GET(NRF_UARTE0));
#elif NRF_SECURE_UART_INSTANCE == 1
    /* UARTE1 is a secure peripheral, so its IRQ has to target S state */
    NVIC_ClearTargetState(NRFX_IRQ_NUMBER_GET(NRF_UARTE1));
#elif NRF_SECURE_UART_INSTANCE == 30
    /* UARTE30 is a secure peripheral, so its IRQ has to target S state */
    NVIC_ClearTargetState(NRFX_IRQ_NUMBER_GET(NRF_UARTE30));
#endif
#endif

    return TFM_PLAT_ERR_SUCCESS;
}

/*----------------- NVIC interrupt enabling for S peripherals ----------------*/
enum tfm_plat_err_t nvic_interrupt_enable(void)
{
    /* SPU interrupt enabling */
    spu_enable_interrupts();

	for(int i = 0; i < ARRAY_SIZE(spu_instances); i++) {
		NVIC_ClearPendingIRQ(NRFX_IRQ_NUMBER_GET(spu_instances[i]));
		NVIC_EnableIRQ(NRFX_IRQ_NUMBER_GET(spu_instances[i]));
	}

	/* The CRACEN driver configures the NVIC for CRACEN and is
	 * therefore omitted here.
	 */

    return TFM_PLAT_ERR_SUCCESS;
}

/*------------------- SAU/IDAU configuration functions -----------------------*/

void sau_and_idau_cfg(void)
{
	/*
	 * SAU and IDAU configuration is very different between old
	 * (53/91) and new (54++) platforms. New platforms have a proper SAU
	 * and IDAU, whereas old platforms do not.
	 */
#ifdef NRF54L15_ENGA_XXAA
	/*
	 * This SAU configuration aligns with ARM's RSS implementation of
	 * sau_and_idau_cfg when possible.
	 */

	/* Enables SAU */
    TZ_SAU_Enable();

    /* Configures SAU regions to be non-secure */

	/* Note that this SAU configuration assumes that there is only one
	 * secure NVM partition and one non-secure NVM partition. Meaning,
	 * memory_regions.non_secure_partition_limit is at the end of
	 * NVM.
	 */

	/* Configure the end of NVM, and the FICR, to be non-secure using
	   a single region. Note that the FICR is placed after the
	   non-secure NVM and before the UICR.*/
    SAU->RNR  = 0;
    SAU->RBAR = (memory_regions.non_secure_partition_base
                 & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (NRF_UICR_S_BASE & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

	/* Leave SAU region 1 disabled until we find a use for it */

    /* Configures veneers region to be non-secure callable */
    SAU->RNR  = 2;
    SAU->RBAR = (memory_regions.veneer_base & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (memory_regions.veneer_limit & SAU_RLAR_LADDR_Msk)
                 | SAU_RLAR_ENABLE_Msk | SAU_RLAR_NSC_Msk;

	/* Configures SAU region 3 to cover both the end of SRAM and
	 * regions above it as shown in the "Example memory map" in the
	 * "Product Specification" */
    SAU->RNR  = 3;
    SAU->RBAR = (NS_DATA_START & SAU_RBAR_BADDR_Msk);
    SAU->RLAR = (0xFFFFFFFFul & SAU_RLAR_LADDR_Msk) | SAU_RLAR_ENABLE_Msk;

#else
    /* IDAU (SPU) is always enabled. SAU is non-existent.
     * Allow SPU to have precedence over (non-existing) ARMv8-M SAU.
     */
    TZ_SAU_Disable();
    SAU->CTRL |= SAU_CTRL_ALLNS_Msk;
#endif
}

#if NRF_SPU_HAS_MEMORY
enum tfm_plat_err_t spu_init_cfg(void)
{
    /*
     * Configure SPU Regions for Non-Secure Code and SRAM (Data)
     * Configure SPU for Peripheral Security
     * Configure Non-Secure Callable Regions
     * Configure Secondary Image Partition
     * Configure Non-Secure Storage Partition
     */

    /* Reset Flash and SRAM configuration of regions that are not owned by
     * the bootloader(s) to all-Secure.
     */
    spu_regions_reset_unlocked_secure();

    uint32_t perm;

    /* Configure Secure Code to be secure and RX */
    perm = 0;
    perm |= NRF_SPU_MEM_PERM_READ;
    /* Do not permit writes to secure flash */
    perm |= NRF_SPU_MEM_PERM_EXECUTE;

    spu_regions_flash_config(S_CODE_START, S_CODE_LIMIT, SPU_SECURE_ATTR_SECURE, perm,
			     SPU_LOCK_CONF_LOCKED);

    /* Configure Secure RAM to be secure and RWX */
    perm = 0;
    perm |= NRF_SPU_MEM_PERM_READ;
    perm |= NRF_SPU_MEM_PERM_WRITE;
    /* Permit execute from Secure RAM because otherwise Crypto fails
     * to initialize. */
    perm |= NRF_SPU_MEM_PERM_EXECUTE;

    spu_regions_sram_config(S_DATA_START, S_DATA_LIMIT, SPU_SECURE_ATTR_SECURE, perm,
			    SPU_LOCK_CONF_LOCKED);

    /* Configures SPU Code and Data regions to be non-secure */
    perm = 0;
    perm |= NRF_SPU_MEM_PERM_READ;
    perm |= NRF_SPU_MEM_PERM_WRITE;
    perm |= NRF_SPU_MEM_PERM_EXECUTE;

    spu_regions_flash_config(memory_regions.non_secure_partition_base,
			     memory_regions.non_secure_partition_limit, SPU_SECURE_ATTR_NONSECURE,
			     perm, SPU_LOCK_CONF_LOCKED);

    spu_regions_sram_config(NS_DATA_START, NS_DATA_LIMIT, SPU_SECURE_ATTR_NONSECURE, perm,
			    SPU_LOCK_CONF_LOCKED);

    /* Configures veneers region to be non-secure callable */
    spu_regions_flash_config_non_secure_callable(memory_regions.veneer_base,
						 memory_regions.veneer_limit - 1);

#ifdef NRF_NS_SECONDARY
	perm = 0;
	perm |= NRF_SPU_MEM_PERM_READ;
	perm |= NRF_SPU_MEM_PERM_WRITE;

    /* Secondary image partition */
    spu_regions_flash_config(memory_regions.secondary_partition_base,
			     memory_regions.secondary_partition_limit, SPU_SECURE_ATTR_NONSECURE,
			     perm, SPU_LOCK_CONF_LOCKED);
#endif /* NRF_NS_SECONDARY */

#ifdef NRF_NS_STORAGE_PARTITION_START
    /* Configures storage partition to be non-secure */
    perm = 0;
    perm |= NRF_SPU_MEM_PERM_READ;
    perm |= NRF_SPU_MEM_PERM_WRITE;

    spu_regions_flash_config(memory_regions.non_secure_storage_partition_base,
			     memory_regions.non_secure_storage_partition_limit,
			     SPU_SECURE_ATTR_NONSECURE, perm, SPU_LOCK_CONF_LOCKED);
#endif /* NRF_NS_STORAGE_PARTITION_START */

    return TFM_PLAT_ERR_SUCCESS;
}
#endif /* NRF_SPU_HAS_MEMORY */


#ifdef MPC_PRESENT
struct mpc_region_override {
	nrf_mpc_override_config_t config;
	nrf_owner_t owner_id;
	uintptr_t start_address;
	size_t endaddr;
	uint32_t perm;
	uint32_t permmask;
	size_t index;
};

static void mpc_configure_override(NRF_MPC_Type *mpc, struct mpc_region_override *override)
{
	nrf_mpc_override_startaddr_set(mpc, override->index, override->start_address);
	nrf_mpc_override_endaddr_set(mpc, override->index, override->endaddr);
	nrf_mpc_override_perm_set(mpc, override->index, override->perm);
	nrf_mpc_override_permmask_set(mpc, override->index, override->permmask);
	nrf_mpc_override_ownerid_set(mpc, override->index, override->owner_id);
	nrf_mpc_override_config_set(mpc, override->index, &override->config);
}

/*
 * Configure the override struct with reasonable defaults. This includes:
 *
 * Use a slave number of 0 to avoid redirecting bus transactions from
 * one slave to another.
 *
 * Lock the override to prevent the code that follows from tampering
 * with the configuration.
 *
 * Enable the override so it takes effect.
 *
 * Indicate that secdom is not enabled as this driver is not used on
 * platforms with secdom.
 */
static void init_mpc_region_override(struct mpc_region_override * override)
{
	*override = (struct mpc_region_override){
		.config =
		(nrf_mpc_override_config_t){
			.slave_number = 0,
			.lock = true,
			.enable = true,
			.secdom_enable = false,
			.secure_mask = true,
		},
		.perm = 0, /* 0 for non-secure */
		.owner_id = 0,
	};

	override->permmask = MPC_OVERRIDE_PERM_SECATTR_Msk;
}

enum tfm_plat_err_t nrf_mpc_init_cfg(void)
{
	/* On 54l the NRF_MPC00->REGION[]'s are fixed in HW and the
	 * OVERRIDE indexes (that are useful to us) start at 0 and end
	 * (inclusive) at 4.
	 */
	uint32_t index = 0;

	/* Configure the non-secure partition of the non-volatile
	 * memory. This MPC region is intended to cover both the
	 * non-secure partition in the NVM and also the FICR. The FICR
	 * starts after the NVM and ends just before the UICR.
	 */
	{
		struct mpc_region_override override;

		init_mpc_region_override(&override);

		override.start_address = memory_regions.non_secure_partition_base;
		override.endaddr = NRF_UICR_S_BASE;
		override.index = index++;

		mpc_configure_override(NRF_MPC00, &override);
	}

	/* Configure the non-secure partition of the volatile memory */
	{
		struct mpc_region_override override;

		init_mpc_region_override(&override);

		override.start_address = NS_DATA_START;
		override.endaddr = 1 + NS_DATA_LIMIT;
		override.index = index++;

		mpc_configure_override(NRF_MPC00, &override);
	}

	if(index > 4) {
		/* Used more overrides than are available */
		tfm_core_panic();
	}

	/* TODO: NCSDK-25050: Review configuration. Any other addresses we need to override? */

	/* Note that we don't configure the NSC region to be NS because it is secure */

	/* Note that OVERRIDE[n].MASTERPORT has a reasonable reset value
	 * so it is left unconfigured.
	 */

	return TFM_PLAT_ERR_SUCCESS;
}

#endif /* MPC_PRESENT */

static void dppi_channel_configuration(void)
{
	/* The SPU HW and corresponding NRFX HAL API have two different
	 * API's for DPPI security configuration. The defines
	 * NRF_SPU_HAS_OWNERSHIP and NRF_SPU_HAS_MEMORY identify which of the two API's
	 * are present.
	 *
	 * TFM_PERIPHERAL_DPPI_CHANNEL_MASK_SECURE is configurable, but
	 * usually defaults to 0, which results in all DPPI channels being
	 * non-secure.
	 */
#if NRF_SPU_HAS_MEMORY
    /* There is only one dppi_id */
    uint8_t dppi_id = 0;
    nrf_spu_dppi_config_set(NRF_SPU, dppi_id, TFM_PERIPHERAL_DPPI_CHANNEL_MASK_SECURE,
			    SPU_LOCK_CONF_LOCKED);
#else
	/* TODO_NRF54L15: Use the nrf_spu_feature API to configure DPPI
	   channels according to a user-controllable config similar to
	   TFM_PERIPHERAL_DPPI_CHANNEL_MASK_SECURE. */
#endif
}

enum tfm_plat_err_t spu_periph_init_cfg(void)
{
    /* Peripheral configuration */
#ifdef NRF54L15_ENGA_XXAA
	/* Configure features to be non-secure */

	/*
	 * Due to MLT-7600, many SPU HW reset values are wrong. The docs
	 * generally features being non-secure when coming out of HW
	 * reset, but the HW has a good mix of both.
	 *
	 * When configuring NRF_SPU 0 will indicate non-secure and 1 will
	 * indicate secure.
	 *
	 * Most of the chip should be non-secure so to simplify and be
	 * consistent, we memset the entire memory map of each SPU
	 * peripheral to 0.
	 *
	 * Just after memsetting to 0 we explicitly configure the
	 * peripherals that should be secure back to secure again.
	 *
	 * At the moment we also have some redundant code that is
	 * configuring things to 0/NonSecure because it is not clear if
	 * this strategy is safe and we want to keep this code in case we
	 * need it later.
	 */
	// TODO: NCSDK-22597: Evaluate if it is safe to memset everything
	// in NRF_SPU to 0.
	memset(NRF_SPU00, 0, sizeof(NRF_SPU_Type));
	memset(NRF_SPU10, 0, sizeof(NRF_SPU_Type));
	memset(NRF_SPU20, 0, sizeof(NRF_SPU_Type));
	memset(NRF_SPU30, 0, sizeof(NRF_SPU_Type));

	/* Configure peripherals to be non-secure */
	for(int i = 0; i < ARRAY_SIZE(spu_instances); i++) {
		NRF_SPU_Type * spu_instance = spu_instances[i];

		/* Configure all pins as non-secure */
		bool spu_has_GPIO = i != 1;
		if(spu_has_GPIO) {
			for(int j = 0; j < ARRAY_SIZE(spu_instance->FEATURE.GPIO); j++) {
				for(int pin = 0; pin < 32; pin++) {
					spu_instance->FEATURE.GPIO[j].PIN[pin] = 0;
				}
			}
		}

		/* TODO: NCSDK-22597: Configure UART30 pins as secure */

		for(uint8_t index = 0; index < ARRAY_SIZE(spu_instance->PERIPH); index++) {
			if(!nrf_spu_periph_perm_present_get(spu_instance, index)) {
				/* Peripheral is not present, nothing to configure */
				continue;
			}

			nrf_spu_securemapping_t securemapping = nrf_spu_periph_perm_securemapping_get(spu_instance, index);

			bool secattr_has_effect =
				securemapping == NRF_SPU_SECUREMAPPING_USERSELECTABLE ||
				securemapping == NRF_SPU_SECUREMAPPING_SPLIT;

			if(secattr_has_effect) {
				bool enable = false;  /* false means non-secure */

				nrf_spu_periph_perm_secattr_set(spu_instance, index, enable);
			}

			/* Note that we don't configure dmasec because it has no effect when secattr is non-secure */

			/* nrf_spu_periph_perm_lock_enable TODO: NCSDK-25009: Lock it down without breaking TF-M UART */
		}
	}

	/* Configure TF-M's UART30 peripheral to be secure with secure DMA */
	bool enable = true; /* true means secure */
	uint32_t UART30_SLAVE_INDEX = (NRF_UARTE30_S_BASE & 0x0003F000) >> 12;
	nrf_spu_periph_perm_secattr_set(NRF_SPU30, UART30_SLAVE_INDEX, enable);
	nrf_spu_periph_perm_dmasec_set(NRF_SPU30, UART30_SLAVE_INDEX, enable);

#else
static const uint8_t target_peripherals[] = {
    /* The following peripherals share ID:
     * - FPU (FPU cannot be configured in NRF91 series, it's always NS)
     * - DCNF (On 53, but not 91)
     */
#ifndef NRF91_SERIES
    NRFX_PERIPHERAL_ID_GET(NRF_FPU),
#endif
    /* The following peripherals share ID:
     * - REGULATORS
     * - OSCILLATORS
     */
    NRFX_PERIPHERAL_ID_GET(NRF_REGULATORS),
    /* The following peripherals share ID:
     * - CLOCK
     * - POWER
     * - RESET (On 53, but not 91)
     */
    NRFX_PERIPHERAL_ID_GET(NRF_CLOCK),
    /* The following peripherals share ID: (referred to as Serial-Box)
     * - SPIMx
     * - SPISx
     * - TWIMx
     * - TWISx
     * - UARTEx
     */

    /* When UART0 is a secure peripheral we need to leave Serial-Box 0 as Secure.
     * The UART Driver will configure it as non-secure when it uninitializes.
     */
#if !(defined(SECURE_UART1) && NRF_SECURE_UART_INSTANCE == 0)
    NRFX_PERIPHERAL_ID_GET(NRF_SPIM0),
#endif

    /* When UART1 is a secure peripheral we need to leave Serial-Box 1 as Secure */
#if !(defined(SECURE_UART1) && NRF_SECURE_UART_INSTANCE == 1)
    NRFX_PERIPHERAL_ID_GET(NRF_SPIM1),
#endif
    NRFX_PERIPHERAL_ID_GET(NRF_SPIM2),
    NRFX_PERIPHERAL_ID_GET(NRF_SPIM3),
    /* When UART30 is a secure peripheral we need to leave Serial-Box 30 as Secure */
#if !(defined(SECURE_UART1) && NRF_SECURE_UART_INSTANCE == 30)
    // TODO: NCSDK-25009: spu_peripheral_config_non_secure((uint32_t)NRF_SPIM30, false);
#endif

#ifdef NRF_SPIM4
    NRFX_PERIPHERAL_ID_GET(NRF_SPIM4),
#endif
    NRFX_PERIPHERAL_ID_GET(NRF_SAADC),
    NRFX_PERIPHERAL_ID_GET(NRF_TIMER0),
    NRFX_PERIPHERAL_ID_GET(NRF_TIMER1),
    NRFX_PERIPHERAL_ID_GET(NRF_TIMER2),
    NRFX_PERIPHERAL_ID_GET(NRF_RTC0),
    NRFX_PERIPHERAL_ID_GET(NRF_RTC1),
    NRFX_PERIPHERAL_ID_GET(NRF_DPPIC),
#ifndef PSA_API_TEST_IPC
#ifdef NRF_WDT0
    /* WDT0 is used as a secure peripheral in PSA FF tests */
    NRFX_PERIPHERAL_ID_GET(NRF_WDT0),
#endif
#ifdef NRF_WDT
    NRFX_PERIPHERAL_ID_GET(NRF_WDT),
#endif
#endif /* PSA_API_TEST_IPC */
#ifdef NRF_WDT1
    NRFX_PERIPHERAL_ID_GET(NRF_WDT1),
#endif
    /* The following peripherals share ID:
     * - COMP
     * - LPCOMP
     */
#ifdef NRF_COMP
    NRFX_PERIPHERAL_ID_GET(NRF_COMP),
#endif
    NRFX_PERIPHERAL_ID_GET(NRF_EGU0),
    NRFX_PERIPHERAL_ID_GET(NRF_EGU1),
    NRFX_PERIPHERAL_ID_GET(NRF_EGU2),
    NRFX_PERIPHERAL_ID_GET(NRF_EGU3),
    NRFX_PERIPHERAL_ID_GET(NRF_EGU4),
#ifndef PSA_API_TEST_IPC
    /* EGU5 is used as a secure peripheral in PSA FF tests */
    NRFX_PERIPHERAL_ID_GET(NRF_EGU5),
#endif
    NRFX_PERIPHERAL_ID_GET(NRF_PWM0),
    NRFX_PERIPHERAL_ID_GET(NRF_PWM1),
    NRFX_PERIPHERAL_ID_GET(NRF_PWM2),
    NRFX_PERIPHERAL_ID_GET(NRF_PWM3),
#ifdef NRF_PDM
    NRFX_PERIPHERAL_ID_GET(NRF_PDM),
#endif
#ifdef NRF_PDM0
    NRFX_PERIPHERAL_ID_GET(NRF_PDM0),
#endif
#ifdef NRF_I2S
    NRFX_PERIPHERAL_ID_GET(NRF_I2S),
#endif
#ifdef NRF_I2S0
    NRFX_PERIPHERAL_ID_GET(NRF_I2S0),
#endif
    NRFX_PERIPHERAL_ID_GET(NRF_IPC),
#ifndef SECURE_QSPI
#ifdef NRF_QSPI
    NRFX_PERIPHERAL_ID_GET(NRF_QSPI),
#endif
#endif
#ifdef NRF_NFCT
    NRFX_PERIPHERAL_ID_GET(NRF_NFCT),
#endif
#ifdef NRF_MUTEX
    NRFX_PERIPHERAL_ID_GET(NRF_MUTEX),
#endif
#ifdef NRF_QDEC0
    NRFX_PERIPHERAL_ID_GET(NRF_QDEC0),
#endif
#ifdef NRF_QDEC1
    NRFX_PERIPHERAL_ID_GET(NRF_QDEC1),
#endif
#ifdef NRF_USBD
    NRFX_PERIPHERAL_ID_GET(NRF_USBD),
#endif
#ifdef NRF_USBREGULATOR
    NRFX_PERIPHERAL_ID_GET(NRF_USBREGULATOR),
#endif
    NRFX_PERIPHERAL_ID_GET(NRF_NVMC),
    NRFX_PERIPHERAL_ID_GET(NRF_P0),
#ifdef NRF_P1
    NRFX_PERIPHERAL_ID_GET(NRF_P1),
#endif
    NRFX_PERIPHERAL_ID_GET(NRF_VMC),
};

    for (int i = 0; i < ARRAY_SIZE(target_peripherals); i++) {
        spu_peripheral_config_non_secure(target_peripherals[i], SPU_LOCK_CONF_UNLOCKED);
    }

#endif /* Moonlight */

    /* DPPI channel configuration */
	dppi_channel_configuration();

    /* GPIO pin configuration */
#ifdef NRF_SPU

	nrf_spu_gpio_config_set(NRF_SPU, 0, TFM_PERIPHERAL_GPIO0_PIN_MASK_SECURE, SPU_LOCK_CONF_LOCKED);
#ifdef TFM_PERIPHERAL_GPIO1_PIN_MASK_SECURE
	nrf_spu_gpio_config_set(NRF_SPU, 1, TFM_PERIPHERAL_GPIO1_PIN_MASK_SECURE, SPU_LOCK_CONF_LOCKED);
#endif

#else
	/* TODO: NCSDK-22597: Support configuring pins as secure or non-secure on nrf54L */
#endif

#ifdef NRF53_SERIES
    /* Configure properly the XL1 and XL2 pins so that the low-frequency crystal
     * oscillator (LFXO) can be used.
     * This configuration can be done only from secure code, as otherwise those
     * register fields are not accessible. That's why it is placed here.
     */
    nrf_gpio_pin_control_select(PIN_XL1, NRF_GPIO_PIN_SEL_PERIPHERAL);
    nrf_gpio_pin_control_select(PIN_XL2, NRF_GPIO_PIN_SEL_PERIPHERAL);
#endif

	/*
	 * 91 has an instruction cache.
	 * 53 has both instruction cache and a data cache.
	 *
	 * 53's instruction cache has an nrfx driver, but 91's cache is
	 * not supported by nrfx at time of writing.
	 *
	 * We enable all caches available here because non-secure cannot
	 * configure caches.
	 */
#if defined(NVMC_FEATURE_CACHE_PRESENT) // From MDK
	nrfx_nvmc_icache_enable();
#elif defined(CACHE_PRESENT) // From MDK

#ifdef NRF_CACHE
	nrf_cache_enable(NRF_CACHE);
#endif
#ifdef NRF_ICACHE
	nrf_cache_enable(NRF_ICACHE);
#endif
#ifdef NRF_DCACHE
	nrf_cache_enable(NRF_DCACHE);
#endif

#endif

#if CONFIG_NRF_RRAM_READYNEXT_TIMEOUT_VALUE > 0
	/*
	 * The NRF_RRAMC peripheral is hardware-fixed to S so the non-secure
	 * image cannot configure the peripheral without costly TF-M system
	 * calls.
	 *
	 * To fix this we do static configuration of the NRF_RRAMC_S peripheral
	 * for the NRF_RRAMC_S->CONFIG value during TF-M boot.
	 */
	nrf_rramc_ready_next_timeout_t params = {
		.value = CONFIG_NRF_RRAM_READYNEXT_TIMEOUT_VALUE,
		.enable = true,
	};
	nrf_rramc_ready_next_timeout_set(NRF_RRAMC_S, &params);
#endif

#if NRF_SPU_HAS_MEMORY
    /* Enforce that the nRF5340 Network MCU is in the Non-Secure
     * domain. Non-secure is the HW reset value for the network core
     * so configuring this should not be necessary, but we want to
     * make sure that the bootloader has not accidentally configured
     * it to be secure. Additionally we lock the register to make sure
     * it doesn't get changed by accident.
     */
    nrf_spu_extdomain_set(NRF_SPU, 0, false, true);
#else
	/* TODO: NCSDK-22597: Configure VPR to be non-secure on nrf54L */
#endif

    return TFM_PLAT_ERR_SUCCESS;
}
