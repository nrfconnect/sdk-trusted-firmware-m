#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# In the new split build this file defines a platform specific parameters
# like mcpu core, arch etc and to be included by NS toolchain file.
# A platform owner is free to configure toolchain here for building NS side.

# Set architecture and CPU
set(TFM_SYSTEM_PROCESSOR cortex-m33)
set(TFM_SYSTEM_ARCHITECTURE armv8-m.main)
set(CONFIG_TFM_FP_ARCH "fpv5-sp-d16")

# The MUSCA_S1 has a CryptoCell-312 as an accelerator.
set(CRYPTO_HW_ACCELERATOR_TYPE cc312)
