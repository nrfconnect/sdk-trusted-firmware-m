/*
 * Copyright (c) 2026, Nordic Semiconductor ASA. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdint.h>

#include "config_tfm.h"
#include "tfm_mbedcrypto_include.h"

#include "tfm_crypto_api.h"
#include "tfm_crypto_key.h"
#include "tfm_crypto_defs.h"

#include "crypto_library.h"

/*!
 * \addtogroup tfm_crypto_api_shim_layer
 *
 */

/*!@{*/
#if CRYPTO_KEY_WRAPPING_MODULE_ENABLED
psa_status_t tfm_crypto_key_wrapping_interface(psa_invec in_vec[],
                                               psa_outvec out_vec[],
                                               struct tfm_crypto_key_id_s *encoded_key)
{
    const struct tfm_crypto_pack_iovec *iov = in_vec[0].base;
    psa_status_t status = PSA_ERROR_NOT_SUPPORTED;
    enum tfm_crypto_func_sid_t sid = (enum tfm_crypto_func_sid_t)iov->function_id;

    tfm_crypto_library_key_id_t library_wrapping_key = tfm_crypto_library_key_id_init(
                                                  encoded_key->owner, encoded_key->key_id);
    tfm_crypto_library_key_id_t library_plain_key = tfm_crypto_library_key_id_init_default();

    if (sid == TFM_CRYPTO_KEY_WRAPPING_WRAP_SID) {
        if (out_vec[0].base == NULL ||
            in_vec[1].base == NULL ||
            in_vec[1].len != sizeof(psa_key_id_t)) {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        psa_key_id_t plain_key = *((psa_key_id_t *)in_vec[1].base);
        library_plain_key = tfm_crypto_library_key_id_init(encoded_key->owner, plain_key);

        uint8_t *output = out_vec[0].base;
        size_t output_size = out_vec[0].len;

        status = psa_wrap_key(library_wrapping_key, iov->alg, library_plain_key, output,
                              output_size, &out_vec[0].len);

        if (status != PSA_SUCCESS) {
            out_vec[0].len = 0;
        }
    }

    if (sid == TFM_CRYPTO_KEY_WRAPPING_UNWRAP_SID) {
        psa_key_attributes_t key_attrs;

        if (in_vec[1].base == NULL ||
            in_vec[1].len != sizeof(psa_key_attributes_t) ||
            out_vec[0].base == NULL ||
            out_vec[0].len != sizeof(psa_key_id_t)) {
            return PSA_ERROR_PROGRAMMER_ERROR;
        }

        memcpy(&key_attrs, in_vec[1].base, in_vec[1].len);
        tfm_crypto_library_get_library_key_id_set_owner(encoded_key->owner, &key_attrs);

        const uint8_t *input = in_vec[2].base;
        size_t input_length = in_vec[2].len;
        psa_key_id_t *key_id = out_vec[0].base;

        status = psa_unwrap_key(&key_attrs, library_wrapping_key, iov->alg, input, input_length,
                                &library_plain_key);
        if (status == PSA_SUCCESS) {
            *key_id = CRYPTO_LIBRARY_GET_KEY_ID(library_plain_key);
        }
    }

    return status;
}
#else /* CRYPTO_KEY_WRAPPING_MODULE_ENABLED */
psa_status_t tfm_crypto_key_wrapping_interface(psa_invec in_vec[],
                                               psa_outvec out_vec[],
                                               struct tfm_crypto_key_id_s *encoded_key)
{
    (void)in_vec;
    (void)out_vec;
    (void)encoded_key;

    return PSA_ERROR_NOT_SUPPORTED;
}
#endif /* CRYPTO_KEY_WRAPPING_MODULE_ENABLED */
/*!@}*/
