#-------------------------------------------------------------------------------
# Copyright (c) 2021-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# The 'TFM_VERSION_MANUAL' is used for fallback when Git tags are not available
set(TFM_VERSION_MANUAL "2.1.2")

set(TFM_VERSION_FULL v${TFM_VERSION_MANUAL})

string(REGEX REPLACE "TF-M" "" TFM_VERSION_FULL ${TFM_VERSION_FULL})
# remove a commit number
string(REGEX REPLACE "-[0-9]+-g" "+" TFM_VERSION_FULL ${TFM_VERSION_FULL})
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" TFM_VERSION ${TFM_VERSION_FULL})
