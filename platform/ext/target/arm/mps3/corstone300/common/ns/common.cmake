#-------------------------------------------------------------------------------
# Copyright (c) 2023-2024, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)

set(CONFIG_TFM_FP_ARCH "fpv5-d16")
set(CONFIG_TFM_FP_ARCH_ASM "FPv5_D16")

#========================= Platform region defs ===============================#

target_include_directories(platform_region_defs
    INTERFACE
        ${CORSTONE300_COMMON_DIR}/partition
        ${PLATFORM_DIR}/partition
        ${PLATFORM_DIR}
)

target_sources(${NS_TARGET_NAME}
    PRIVATE
        ${CORSTONE300_COMMON_DIR}/device/source/startup_corstone300.c
        ${CORSTONE300_COMMON_DIR}/device/source/system_core_init.c
)

#========================= Device definition lib ===============================#

add_library(device_definition INTERFACE)
target_include_directories(device_definition
    INTERFACE
        ${CORSTONE300_COMMON_DIR}
        ${CORSTONE300_COMMON_DIR}/device/config
        ${CORSTONE300_COMMON_DIR}/device/include
        ${CORSTONE300_COMMON_DIR}/native_drivers
        ${CORSTONE300_COMMON_DIR}/partition
        ${CMAKE_CURRENT_SOURCE_DIR}/partition
        ${PLATFORM_DIR}/ext/target/arm/drivers/flash/common
        ${PLATFORM_DIR}/ext/target/arm/drivers/usart/cmsdk
        ${PLATFORM_DIR}/ext/target/arm/drivers/usart/common
        ${PLATFORM_DIR}/ext/target/arm/drivers/mpc_sie
        ${PLATFORM_DIR}/ext/target/arm/drivers/mpu/armv8m
        ${PLATFORM_DIR}/ext/target/arm/drivers/counter/armv8m
        ${PLATFORM_DIR}/ext/target/arm/drivers/timer/armv8m
        ${ETHOS_DRIVER_PATH}/src
        ${ETHOS_DRIVER_PATH}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/device/config
)

#========================= Platform Non-Secure ================================#

target_sources(platform_ns
    PRIVATE
        ${CORSTONE300_COMMON_DIR}/device/source/platform_ns_device_definition.c
        ${CORSTONE300_COMMON_DIR}/cmsis_drivers/Driver_USART.c
        ${PLATFORM_DIR}/ext/target/arm/drivers/usart/cmsdk/uart_cmsdk_drv.c
    INTERFACE
        $<$<BOOL:${TEST_NS_FPU}>:${CORSTONE300_COMMON_DIR}/device/source/corstone300_ns_init.c>
        $<$<BOOL:${TEST_NS_FPU}>:${PLATFORM_DIR}/ext/common/test_interrupt.c>
)

target_include_directories(platform_ns
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/device/config
        ${CORSTONE300_COMMON_DIR}/device/include
        ${CORSTONE300_COMMON_DIR}/include
        ${CORSTONE300_COMMON_DIR}/cmsis_drivers
        ${PLATFORM_DIR}/ext/cmsis/Include
        ${PLATFORM_DIR}/ext/cmsis/Include/m-profile
        ${CORSTONE300_COMMON_DIR}/partition
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/common
        ${CORSTONE300_COMMON_DIR}/cmsis_drivers/config/non_secure
)

target_compile_definitions(platform_ns
    PUBLIC
        $<$<BOOL:${TEST_NS_FPU}>:TEST_NS_FPU>
)

target_link_libraries(platform_ns
    PUBLIC
        device_definition
)
