/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_PLAT_PROVISIONING_H__
#define __TFM_PLAT_PROVISIONING_H__

#include "tfm_plat_defs.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief                               Check the contents of OTP memory for the
 *                                      presence of keys provisioned by
 *                                      TFM_DUMMY_PROVISIONING
 *
 * \note                                This function should print a warning to
 *                                      the UART, or otherwise notify the device
 *                                      user, if dummy keys are detected.
 */
void tfm_plat_provisioning_check_for_dummy_keys(void);

/**
 * \brief           Check if a provisioning operation is required.
 *
 * \param[out] provisioning_required Whether or not provisioning is required.
 *
 * \return          TFM_PLAT_ERR_SUCCESS on success, non-zero on error.
 */
enum tfm_plat_err_t tfm_plat_provisioning_is_required(bool *provisioning_required);

/**
 * \brief                               Performs a provisioning operation.
 *
 * \retval TFM_PLAT_ERR_SUCCESS         The provisioning operation completed
 *                                      successfully.
 * \retval TFM_PLAT_ERR_INVALID_INPUT   The provisioning data that was input to
 *                                      the device is invalid
 * \retval TFM_PLAT_ERR_SYSTEM_ERR      An unspecified error occurred.
 */
enum tfm_plat_err_t tfm_plat_provisioning_perform(void);

#ifdef __cplusplus
}
#endif

#endif /* __TFM_PLAT_PROVISIONING_H__ */
