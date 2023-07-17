#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited. All rights reserved.
# Copyright (c) 2020, Nordic Semiconductor ASA.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

set(target nrf91)

add_subdirectory(../core nrf_common)

# Specify the location of platform specific build dependencies.
#========================= Platform Secure ====================================#

target_include_directories(platform_s
    PUBLIC
        .
)

target_sources(platform_s
    PRIVATE
        ${HAL_NORDIC_PATH}/nrfx/mdk/system_nrf91.c
)

target_compile_definitions(platform_s
    PUBLIC
        NRF_SKIP_FICR_NS_COPY_TO_RAM
)

#========================= Platform Non-Secure ================================#

target_include_directories(platform_ns
    PUBLIC
        .
)

target_sources(platform_ns
    PRIVATE
        ${HAL_NORDIC_PATH}/nrfx/mdk/system_nrf91.c
)

target_compile_definitions(platform_ns
    PUBLIC
        NRF_TRUSTZONE_NONSECURE
        DOMAIN_NS=1
)

#========================= Platform BL2 =======================================#

if(BL2)
    target_include_directories(platform_bl2
        PRIVATE
            .
    )

    target_sources(platform_bl2
        PRIVATE
            ${HAL_NORDIC_PATH}/nrfx/mdk/system_nrf91.c
    )

    target_compile_definitions(platform_bl2
        PUBLIC
            NRF_SKIP_FICR_NS_COPY_TO_RAM
    )
endif()

#========================= tfm_spm ============================================#

target_sources(tfm_spm
    PRIVATE
        $<$<OR:$<BOOL:${CONFIG_TFM_FLIH_API}>,$<BOOL:${CONFIG_TFM_SLIH_API}>>:${CMAKE_CURRENT_SOURCE_DIR}/tfm_interrupts.c>
)
