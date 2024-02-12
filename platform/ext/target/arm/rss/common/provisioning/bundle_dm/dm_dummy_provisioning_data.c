/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "config_tfm.h"
#include "rss_provisioning_bundle.h"

#if (MCUBOOT_SIGNATURE_TYPE == RSA-2048)
#define ASSEMBLY_AND_TEST_PROV_DATA_KIND_0 {             \
        0xfc, 0x57, 0x01, 0xdc, 0x61, 0x35, 0xe1, 0x32,  \
        0x38, 0x47, 0xbd, 0xc4, 0x0f, 0x04, 0xd2, 0xe5,  \
        0xbe, 0xe5, 0x83, 0x3b, 0x23, 0xc2, 0x9f, 0x93,  \
        0x59, 0x3d, 0x00, 0x01, 0x8c, 0xfa, 0x99, 0x94,  \
    }
#define ASSEMBLY_AND_TEST_PROV_DATA_KIND_1 {             \
        0xe1, 0x80, 0x15, 0x99, 0x3d, 0x6d, 0x27, 0x60,  \
        0xb4, 0x99, 0x27, 0x4b, 0xae, 0xf2, 0x64, 0xb8,  \
        0x3a, 0xf2, 0x29, 0xe9, 0xa7, 0x85, 0xf3, 0xd5,  \
        0xbf, 0x00, 0xb9, 0xd3, 0x2c, 0x1f, 0x03, 0x96,  \
    }
#elif (MCUBOOT_SIGNATURE_TYPE == RSA-3072)
#define ASSEMBLY_AND_TEST_PROV_DATA_KIND_0 {             \
        0xbf, 0xe6, 0xd8, 0x6f, 0x88, 0x26, 0xf4, 0xff,  \
        0x97, 0xfb, 0x96, 0xc4, 0xe6, 0xfb, 0xc4, 0x99,  \
        0x3e, 0x46, 0x19, 0xfc, 0x56, 0x5d, 0xa2, 0x6a,  \
        0xdf, 0x34, 0xc3, 0x29, 0x48, 0x9a, 0xdc, 0x38,  \
    }
#define ASSEMBLY_AND_TEST_PROV_DATA_KIND_1 {             \
        0xb3, 0x60, 0xca, 0xf5, 0xc9, 0x8c, 0x6b, 0x94,  \
        0x2a, 0x48, 0x82, 0xfa, 0x9d, 0x48, 0x23, 0xef,  \
        0xb1, 0x66, 0xa9, 0xef, 0x6a, 0x6e, 0x4a, 0xa3,  \
        0x7c, 0x19, 0x19, 0xed, 0x1f, 0xcc, 0xc0, 0x49,  \
    }
#elif defined(MCUBOOT_SIGN_EC256)
#define ASSEMBLY_AND_TEST_PROV_DATA_KIND_0 {             \
        0xe3, 0x04, 0x66, 0xf6, 0xb8, 0x47, 0x0c, 0x1f,  \
        0x29, 0x07, 0x0b, 0x17, 0xf1, 0xe2, 0xd3, 0xe9,  \
        0x4d, 0x44, 0x5e, 0x3f, 0x60, 0x80, 0x87, 0xfd,  \
        0xc7, 0x11, 0xe4, 0x38, 0x2b, 0xb5, 0x38, 0xb6,  \
    }
#define ASSEMBLY_AND_TEST_PROV_DATA_KIND_1 {             \
        0x82, 0xa5, 0xb4, 0x43, 0x59, 0x48, 0x53, 0xd4,  \
        0xbf, 0x0f, 0xdd, 0x89, 0xa9, 0x14, 0xa5, 0xdc,  \
        0x16, 0xf8, 0x67, 0x54, 0x82, 0x07, 0xd7, 0x07,  \
        0x7e, 0x74, 0xd8, 0x0c, 0x06, 0x3e, 0xfd, 0xa9,  \
    }
#elif defined(MCUBOOT_SIGN_EC384)
#define ASSEMBLY_AND_TEST_PROV_DATA_KIND_0 {             \
        0x85, 0xb7, 0xbd, 0x5f, 0x5d, 0xff, 0x9a, 0x03,  \
        0xa9, 0x99, 0x27, 0xad, 0xaf, 0x6c, 0xa6, 0xfe,  \
        0xbd, 0xe8, 0x22, 0xc1, 0xa4, 0x80, 0x92, 0x83,  \
        0x24, 0xa8, 0xe6, 0x03, 0x23, 0x71, 0x5c, 0x57,  \
        0x79, 0x46, 0x1c, 0x49, 0x6a, 0x95, 0xae, 0xe8,  \
        0xc4, 0xf9, 0x0b, 0x99, 0x77, 0x9f, 0x84, 0x8a,  \
    }
#define ASSEMBLY_AND_TEST_PROV_DATA_KIND_1 {             \
        0xae, 0x13, 0xb7, 0x1d, 0xae, 0x49, 0xa7, 0xb8,  \
        0x9d, 0x4d, 0x8e, 0xe5, 0x09, 0x5c, 0xb8, 0xd4,  \
        0x5a, 0x32, 0xbd, 0x9c, 0x7d, 0x50, 0x1c, 0xd3,  \
        0xb8, 0xf8, 0x6c, 0xbc, 0x8b, 0x41, 0x43, 0x9b,  \
        0x1b, 0x22, 0x5c, 0xc3, 0x6a, 0x5b, 0xa8, 0x08,  \
        0x1d, 0xf0, 0x71, 0xe0, 0xcb, 0xbc, 0x61, 0x92,  \
    }
#else
#error "No public key available for given signing algorithm."
#endif /* MCUBOOT_SIGNATURE_TYPE */

const struct dm_provisioning_data data = {
    /* BL1 ROTPK 0 */
    {
        0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x04,
        0xb6, 0xb4, 0x44, 0xcb, 0x92, 0x58, 0x4e, 0xe0,
        0x81, 0xb6, 0x78, 0xf3, 0x9a, 0x0d, 0xd7, 0x4e,
        0x0c, 0xfd, 0x7e, 0x45, 0x01, 0x76, 0xdd, 0x0c,
        0xb6, 0x54, 0xc9, 0xbb, 0xbf, 0x63, 0x2b, 0x14,
        0xb2, 0xee, 0x00, 0x89, 0x63, 0xe0, 0x43, 0xaf,
        0xeb, 0xe8, 0xac, 0x6c, 0x1d, 0x60, 0x5e, 0xbc,
    },
    /* BL2 Encryption key */
    {
        0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45,
        0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01,
        0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67,
        0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23
    },
    /* bl2_rotpk */
    {
        ASSEMBLY_AND_TEST_PROV_DATA_KIND_0,
        ASSEMBLY_AND_TEST_PROV_DATA_KIND_1,
#if (MCUBOOT_IMAGE_NUMBER > 2)
        ASSEMBLY_AND_TEST_PROV_DATA_KIND_0,
#endif
#if (MCUBOOT_IMAGE_NUMBER > 3)
        ASSEMBLY_AND_TEST_PROV_DATA_KIND_0,
#endif
#if (MCUBOOT_IMAGE_NUMBER > 4)
        ASSEMBLY_AND_TEST_PROV_DATA_KIND_0,
#endif
#if (MCUBOOT_IMAGE_NUMBER > 5)
        ASSEMBLY_AND_TEST_PROV_DATA_KIND_0,
#endif
#if (MCUBOOT_IMAGE_NUMBER > 6)
        ASSEMBLY_AND_TEST_PROV_DATA_KIND_0,
#endif
#if (MCUBOOT_IMAGE_NUMBER > 7)
        ASSEMBLY_AND_TEST_PROV_DATA_KIND_0,
#endif
#if (MCUBOOT_IMAGE_NUMBER > 8)
        ASSEMBLY_AND_TEST_PROV_DATA_KIND_0,
#endif
    },

    /* Secure image encryption key; */
    {
        0xfc, 0x57, 0x01, 0xdc, 0x61, 0x35, 0xe1, 0x32,
        0x38, 0x47, 0xbd, 0xc4, 0x0f, 0x04, 0xd2, 0xe5,
        0xbe, 0xe5, 0x83, 0x3b, 0x23, 0xc2, 0x9f, 0x93,
        0x59, 0x3d, 0x00, 0x01, 0x8c, 0xfa, 0x99, 0x94,
    },
    /* Non-secure image encryption key; */
    {
        0xfc, 0x57, 0x01, 0xdc, 0x61, 0x35, 0xe1, 0x32,
        0x38, 0x47, 0xbd, 0xc4, 0x0f, 0x04, 0xd2, 0xe5,
        0xbe, 0xe5, 0x83, 0x3b, 0x23, 0xc2, 0x9f, 0x93,
        0x59, 0x3d, 0x00, 0x01, 0x8c, 0xfa, 0x99, 0x94,
    },
    /* HOST_ROTPK_S */
    {
        0x09, 0x20, 0x59, 0xde, 0xc5, 0x1b, 0xe2, 0x96,
        0xfe, 0x4b, 0xa0, 0x16, 0x20, 0xac, 0xd7, 0xce,
        0xe2, 0x1e, 0xd5, 0xbf, 0x74, 0x4f, 0xe4, 0x47,
        0xab, 0x1f, 0xe4, 0xcb, 0x91, 0x52, 0x94, 0xb2,
        0xf2, 0xff, 0xaf, 0x3a, 0x47, 0x26, 0x0e, 0x13,
        0x4f, 0x8f, 0x2c, 0x1b, 0x5e, 0xde, 0xe8, 0x9e,
        0xdd, 0x2e, 0x1c, 0xf1, 0x0d, 0x3c, 0xc1, 0xee,
        0x32, 0x92, 0x9d, 0x05, 0xca, 0x57, 0x0d, 0x0e,
        0xbc, 0xd1, 0x72, 0x32, 0xf4, 0x1f, 0x1c, 0xe4,
        0x48, 0xd8, 0x79, 0x87, 0xfc, 0x3b, 0x2f, 0xf4,
        0x79, 0xe2, 0xf1, 0x03, 0x1f, 0xf3, 0x4d, 0xbc,
        0x76, 0x8a, 0x81, 0x19, 0x4a, 0x95, 0x4d, 0xac
    },
    /* HOST_ROTPK_NS */
    {
        0x09, 0x20, 0x59, 0xde, 0xc5, 0x1b, 0xe2, 0x96,
        0xfe, 0x4b, 0xa0, 0x16, 0x20, 0xac, 0xd7, 0xce,
        0xe2, 0x1e, 0xd5, 0xbf, 0x74, 0x4f, 0xe4, 0x47,
        0xab, 0x1f, 0xe4, 0xcb, 0x91, 0x52, 0x94, 0xb2,
        0xf2, 0xff, 0xaf, 0x3a, 0x47, 0x26, 0x0e, 0x13,
        0x4f, 0x8f, 0x2c, 0x1b, 0x5e, 0xde, 0xe8, 0x9e,
        0xdd, 0x2e, 0x1c, 0xf1, 0x0d, 0x3c, 0xc1, 0xee,
        0x32, 0x92, 0x9d, 0x05, 0xca, 0x57, 0x0d, 0x0e,
        0xbc, 0xd1, 0x72, 0x32, 0xf4, 0x1f, 0x1c, 0xe4,
        0x48, 0xd8, 0x79, 0x87, 0xfc, 0x3b, 0x2f, 0xf4,
        0x79, 0xe2, 0xf1, 0x03, 0x1f, 0xf3, 0x4d, 0xbc,
        0x76, 0x8a, 0x81, 0x19, 0x4a, 0x95, 0x4d, 0xac
    },
    /* HOST_ROTPK_CCA */
    {
        0x09, 0x20, 0x59, 0xde, 0xc5, 0x1b, 0xe2, 0x96,
        0xfe, 0x4b, 0xa0, 0x16, 0x20, 0xac, 0xd7, 0xce,
        0xe2, 0x1e, 0xd5, 0xbf, 0x74, 0x4f, 0xe4, 0x47,
        0xab, 0x1f, 0xe4, 0xcb, 0x91, 0x52, 0x94, 0xb2,
        0xf2, 0xff, 0xaf, 0x3a, 0x47, 0x26, 0x0e, 0x13,
        0x4f, 0x8f, 0x2c, 0x1b, 0x5e, 0xde, 0xe8, 0x9e,
        0xdd, 0x2e, 0x1c, 0xf1, 0x0d, 0x3c, 0xc1, 0xee,
        0x32, 0x92, 0x9d, 0x05, 0xca, 0x57, 0x0d, 0x0e,
        0xbc, 0xd1, 0x72, 0x32, 0xf4, 0x1f, 0x1c, 0xe4,
        0x48, 0xd8, 0x79, 0x87, 0xfc, 0x3b, 0x2f, 0xf4,
        0x79, 0xe2, 0xf1, 0x03, 0x1f, 0xf3, 0x4d, 0xbc,
        0x76, 0x8a, 0x81, 0x19, 0x4a, 0x95, 0x4d, 0xac
    },
    /* implementation id */
    {
        0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
        0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
        0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
        0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
    },
    /* verification_service_url */
    "www.trustedfirmware.org",
    /* attestation_profile_definition */
#if ATTEST_TOKEN_PROFILE_PSA_IOT_1
    "PSA_IOT_PROFILE_1",
#elif ATTEST_TOKEN_PROFILE_PSA_2_0_0
    "http://arm.com/psa/2.0.0",
#elif ATTEST_TOKEN_PROFILE_ARM_CCA
    "http://arm.com/CCA-SSD/1.0.0",
#else
#ifdef TFM_PARTITION_INITIAL_ATTESTATION
#error "Attestation token profile is incorrect"
#else
    "UNDEFINED",
#endif /* TFM_PARTITION_INITIAL_ATTESTATION */
#endif
    /* Secure debug public key */
    {
        0xf4, 0x0c, 0x8f, 0xbf, 0x12, 0xdb, 0x78, 0x2a,
        0xfd, 0xf4, 0x75, 0x96, 0x6a, 0x06, 0x82, 0x36,
        0xe0, 0x32, 0xab, 0x80, 0xd1, 0xb7, 0xf1, 0xbc,
        0x9f, 0xe7, 0xd8, 0x7a, 0x88, 0xcb, 0x26, 0xd0,
    },
};
