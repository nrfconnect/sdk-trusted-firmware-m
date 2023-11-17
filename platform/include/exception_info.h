/*
 * Copyright (c) 2021, Nordic Semiconductor ASA. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __EXCEPTION_INFO_H__
#define __EXCEPTION_INFO_H__

#include <stdint.h>

#if defined(__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
#define TRUSTZONE_PRESENT
#endif

#if defined(__ARM_ARCH_8_1M_MAIN__) || defined(__ARM_ARCH_8M_MAIN__) \
    || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
#define FAULT_STATUS_PRESENT
#endif


/* This level of indirection is needed to fully resolve exception info when it's
 * a macro
 */
#define _STRINGIFY(exception_info) #exception_info

/* Store context for an exception, and print an error message with the context.
 *
 * @param[in]  exception_type  One of the EXCEPTION_TYPE_* values defined above. Any
 *                             other value will result in printing "Unknown".
 */
#ifdef TFM_EXCEPTION_INFO_DUMP

struct exception_info_t {
    uint32_t VECTACTIVE;        /* Active exception number. */
    uint32_t EXC_RETURN;        /* EXC_RETURN value in LR. */
    uint32_t MSP;               /* (Secure) MSP. */
    uint32_t PSP;               /* (Secure) PSP. */
    uint32_t *EXC_FRAME;        /* Exception frame on stack. */
    uint32_t EXC_FRAME_COPY[8]; /* Copy of the basic exception frame. */
    uint32_t xPSR;              /* Program Status Registers. */

#ifdef FAULT_STATUS_PRESENT
    uint32_t CFSR;              /* Configurable Fault Status Register. */
    uint32_t HFSR;              /* Hard Fault Status Register. */
    uint32_t BFAR;              /* Bus Fault address register. */
    uint32_t BFARVALID;         /* Whether BFAR contains a valid address. */
    uint32_t MMFAR;             /* MemManage Fault address register. */
    uint32_t MMARVALID;         /* Whether MMFAR contains a valid address. */
#ifdef TRUSTZONE_PRESENT
    uint32_t SFSR;              /* SecureFault Status Register. */
    uint32_t SFAR;              /* SecureFault Address Register. */
    uint32_t SFARVALID;         /* Whether SFAR contains a valid address. */
#endif
#endif
};

#define EXCEPTION_INFO()                                \
    __ASM volatile(                                     \
        "MOV     r0, lr\n"                              \
        "MRS     r1, MSP\n"                             \
        "MRS     r2, PSP\n"                             \
        "BL      store_and_dump_context\n"              \
    )

/* Store context for an exception, then print the info.
 * Call EXCEPTION_INFO() instead of calling this directly.
 */
void store_and_dump_context(uint32_t LR_in, uint32_t MSP_in, uint32_t PSP_in);


/**
 * \brief Get a pointer to the current exception_info_t context
 *
 * \return  A pointer to the exception_info_t context or NULL if no context
 *          has been stored
 */
void tfm_exception_info_get_context(struct exception_info_t *ctx);

#else
#define EXCEPTION_INFO()
#endif

#endif /* __EXCEPTION_INFO_H__ */
