
/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
/**
 * \file psa/crypto_platform.h
 *
 * \brief PSA cryptography module: TF-M platform definitions
 *
 * \note This file may not be included directly. Applications must
 * include psa/crypto.h.
 *
 * This file contains platform-dependent type definitions.
 *
 * In implementations with isolation between the application and the
 * cryptography module, implementers should take care to ensure that
 * the definitions that are exposed to applications match what the
 * module implements.
 */

#ifndef PSA_CRYPTO_PLATFORM_H
#define PSA_CRYPTO_PLATFORM_H

/* PSA requires several types which C99 provides in stdint.h. */
#include <stdint.h>

/* No particular platform definition is currently required for the
 * TF-M client view of the PSA Crytpo APIs, but we keep this header
 * available for reference and future compatibility
 */

#if defined(__NRF_TFM__) && defined(MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER)
/*
 * This header file is provided by TF-M and is only intended to be
 * used by clients of PSA, not implementations of PSA. So secure
 * partitions in the TF-M image (other than the crypto partition
 * itself), and non-secure images.
 *
 * In NCS, the TF-M crypto partition should be using PSA header files
 * from Oberon as it is Oberon that implements PSA.
 *
 * We want to detect that a source file is in the crypto partition,
 * but has accidentally used this TF-M header instead of headers
 * provided by Oberon. To do this we would ideally have a
 * IS_IN_CRYPTO_PARTION define, but since there is no such define at
 * time of writing we check MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER
 * instead, which should only be defined by the PSA implementation,
 * not by PSA clients.
 */
#error "The TF-M image included the TF-M PSA headers but should have included Oberon PSA headers"
#endif

#endif /* PSA_CRYPTO_PLATFORM_H */
