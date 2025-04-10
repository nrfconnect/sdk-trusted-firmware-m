#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0079 NEW)

# TODO: Verify that this works for both minimal and normal configuration
target_compile_definitions(tfm_config
    INTERFACE
        CRYPTO_HW_ACCELERATOR
        CRYPTO_EXT_RNG
)

# When using nrf_security we don't need these build scripts
return()

if(BL2)
    add_library(bl2_crypto_hw STATIC)
endif()

if (TFM_PARTITION_CRYPTO)
    add_library(crypto_service_crypto_hw STATIC)
endif()

message(STATUS "[Platform] Crypto HW accelerator type: ${CRYPTO_HW_ACCELERATOR_TYPE}")
add_subdirectory(${CRYPTO_HW_ACCELERATOR_TYPE})

############################ Project Config ####################################

target_compile_definitions(tfm_config
    INTERFACE
        CRYPTO_HW_ACCELERATOR
)

################################ BL2 ###########################################

if(BL2)
    target_include_directories(bl2_crypto_hw
        PUBLIC
            interface
    )

    target_compile_definitions(bl2_crypto_hw
        PUBLIC
            CRYPTO_HW_ACCELERATOR
    )
endif()

############################ Crypto Service ####################################

if (TFM_PARTITION_CRYPTO)
    target_include_directories(crypto_service_crypto_hw
        PUBLIC
            interface
    )

    target_compile_definitions(crypto_service_crypto_hw
        PUBLIC
            CRYPTO_HW_ACCELERATOR
    )
endif()
