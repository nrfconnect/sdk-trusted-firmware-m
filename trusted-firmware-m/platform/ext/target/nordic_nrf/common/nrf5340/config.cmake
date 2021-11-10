#-------------------------------------------------------------------------------
# Copyright (c) 2020, Nordic Semiconductor ASA.
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(SECURE_UART1                        ON         CACHE BOOL      "Enable secure UART1")
set(PSA_API_TEST_TARGET                 "nrf"      CACHE STRING    "PSA API test target")
set(ITS_NUM_ASSETS                      "5"        CACHE STRING    "The maximum number of assets to be stored in the Internal Trusted Storage area")
set(NRF_NS_STORAGE                      OFF        CACHE BOOL      "Enable non-secure storage partition")
set(TFM_ITS_ENCRYPT                     ON         CACHE BOOL      "Enable encryption of ITS objects using platform specific APIs")
set(TFM_ITS_ENC_NONCE_SIZE              "16"       CACHE STRING    "The size of the nonce used in ITS encryption in bytes")
set(TFM_ITS_ENC_TAG_SIZE                "16"       CACHE STRING    "The size of the tag used in ITS encryption in bytes")
