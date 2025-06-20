/*
 * Copyright (c) 2017-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "build_config_check.h"
#include "internal_status_code.h"
#include "fih.h"
#include "tfm_boot_data.h"
#include "memory_symbols.h"
#include "spm.h"
#include "tfm_hal_isolation.h"
#include "tfm_hal_platform.h"
#include "tfm_spm_log.h"
#include "tfm_version.h"
#include "tfm_plat_otp.h"
#include "tfm_plat_provisioning.h"
#include "ffm/backend.h"

#ifdef CONFIG_TFM_ENABLE_PROFILING
#include "prof_intf_s.h"
#endif

uintptr_t spm_boundary = (uintptr_t)NULL;

static fih_int tfm_core_init(void)
{
    enum tfm_plat_err_t plat_err = TFM_PLAT_ERR_SYSTEM_ERR;
    fih_int fih_rc = FIH_FAILURE;
    bool provisioning_required;

    /*
     * Access to any peripheral should be performed after programming
     * the necessary security components such as PPC/SAU.
     */
    FIH_CALL(tfm_hal_set_up_static_boundaries, fih_rc, &spm_boundary);
    if (fih_not_eq(fih_rc, fih_int_encode(TFM_HAL_SUCCESS))) {
        FIH_RET(fih_int_encode(SPM_ERROR_GENERIC));
    }
#ifdef TFM_FIH_PROFILE_ON
    FIH_CALL(tfm_hal_verify_static_boundaries, fih_rc);
    if (fih_not_eq(fih_rc, fih_int_encode(TFM_HAL_SUCCESS))) {
        tfm_core_panic();
    }
#endif

    FIH_CALL(tfm_hal_platform_init, fih_rc);
    if (fih_not_eq(fih_rc, fih_int_encode(TFM_HAL_SUCCESS))) {
        FIH_RET(fih_int_encode(SPM_ERROR_GENERIC));
    }

    /*
     * Print the TF-M version now that the platform has initialized
     * the logging backend.
     */
    SPMLOG_INFMSG("\033[1;34mBooting TF-M "VERSION_FULLSTR"\033[0m\r\n");

    plat_err = tfm_plat_otp_init();
    if (plat_err != TFM_PLAT_ERR_SUCCESS) {
        FIH_RET(fih_int_encode(SPM_ERROR_GENERIC));
    }

    /* Perform provisioning. */
    plat_err = tfm_plat_provisioning_is_required(&provisioning_required);
    if (plat_err != TFM_PLAT_ERR_SUCCESS) {
        FIH_RET(fih_int_encode(SPM_ERROR_GENERIC));
    }

    if (provisioning_required) {
        plat_err = tfm_plat_provisioning_perform();
        if (plat_err != TFM_PLAT_ERR_SUCCESS) {
            FIH_RET(fih_int_encode(SPM_ERROR_GENERIC));
        }
    }

    tfm_plat_provisioning_check_for_dummy_keys();

    /* Configures architecture */
    tfm_arch_config_extensions();

    SPMLOG_INFMSG("\033[1;34m[Sec Thread] Secure image initializing!\033[0m\r\n");

    SPMLOG_DBGMSGVAL("TF-M isolation level is: ", TFM_ISOLATION_LEVEL);

#if (CONFIG_TFM_FLOAT_ABI == 2)
    SPMLOG_INFMSG("TF-M Float ABI: Hard\r\n");
#ifdef CONFIG_TFM_LAZY_STACKING
    SPMLOG_INFMSG("Lazy stacking enabled\r\n");
#else
    SPMLOG_INFMSG("Lazy stacking disabled\r\n");
#endif
#endif

    tfm_core_validate_boot_data();

    FIH_RET(fih_int_encode(SPM_SUCCESS));
}

int main(void)
{
#ifdef CONFIG_TFM_ENABLE_PROFILING
    PROFILING_INIT();
#endif

    fih_int fih_rc = FIH_FAILURE;

    /* set Main Stack Pointer limit */
    tfm_arch_set_msplim(SPM_BOOT_STACK_TOP);

    fih_delay_init();

    FIH_CALL(tfm_core_init, fih_rc);
    if (fih_not_eq(fih_rc, fih_int_encode(SPM_SUCCESS))) {
        tfm_core_panic();
    }

    /* All isolation should have been set up at this point */
    FIH_LABEL_CRITICAL_POINT();

    /*
     * Prioritise secure exceptions to avoid NS being able to pre-empt
     * secure SVC or SecureFault. Do it before PSA API initialization.
     */
    tfm_arch_set_secure_exception_priorities();

#ifdef TFM_FIH_PROFILE_ON
    /* Check secure exception priority */
    FIH_CALL(tfm_arch_verify_secure_exception_priorities, fih_rc);
    if (fih_not_eq(fih_rc, FIH_SUCCESS)) {
         tfm_core_panic();
    }
#endif

    /* Further SPM initialization. */
    BACKEND_SPM_INIT();

    return 0;
}
