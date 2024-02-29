/*
 * Copyright (c) 2024, Nordic Semiconductor ASA. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

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
#if CRYPTO_PAKE_MODULE_ENABLED
psa_status_t tfm_crypto_pake_interface(psa_invec in_vec[],
                                       psa_outvec out_vec[],
                                       struct tfm_crypto_key_id_s *encoded_key)
{
        const struct tfm_crypto_pack_iovec *iov = in_vec[0].base;
        psa_status_t status = PSA_ERROR_NOT_SUPPORTED;
        psa_pake_operation_t *operation = NULL;
        uint32_t *p_handle = NULL;
        uint16_t sid = iov->function_id;

        tfm_crypto_library_key_id_t library_key = tfm_crypto_library_key_id_init(
                                                  encoded_key->owner, encoded_key->key_id);
        if(sid == TFM_CRYPTO_PAKE_SETUP_SID) {
                p_handle = out_vec[0].base;
                *p_handle = iov->op_handle;
                status = tfm_crypto_operation_alloc(TFM_CRYPTO_PAKE_OPERATION,
                                                    out_vec[0].base,
                                                    (void **)&operation);
        } else {
                status = tfm_crypto_operation_lookup(TFM_CRYPTO_PAKE_OPERATION,
                                                iov->op_handle,
                                                (void **)&operation);
                if ((sid == TFM_CRYPTO_PAKE_ABORT_SID) || sid == TFM_CRYPTO_PAKE_GET_SHARED_KEY_SID){
                /*
                * finish()/abort() interface put handle in out_vec[0].
                * Therefore, out_vec[0] shall be specially set to original handle
                * value. Otherwise, the garbage data in message out_vec[0] may
                * override the original handle value in client, after lookup fails.
                */
                p_handle = out_vec[0].base;
                *p_handle = iov->op_handle;
                }
        }
        if (status != PSA_SUCCESS) {
                if (sid == TFM_CRYPTO_PAKE_ABORT_SID) {
                /*
                * Mbed TLS psa_pake_abort() will return a misleading error code
                * if it is called with invalid operation content, since it
                * doesn't validate the operation handle.
                * It is neither necessary to call tfm_crypto_operation_release()
                * with an invalid handle.
                * Therefore return PSA_SUCCESS directly as psa_cipher_abort() can
                * be called multiple times.
                */
                return PSA_SUCCESS;
                }
                return status;
        }

        switch (sid)
        {
        case TFM_CRYPTO_PAKE_SETUP_SID:
        {
                status = psa_pake_setup(operation, library_key, in_vec[1].base);
                if (status != PSA_SUCCESS) {
                        goto release_operation_and_return;
                }
        }
        break;
        case TFM_CRYPTO_PAKE_SET_ROLE_SID:
        {
                status = psa_pake_set_role(operation, iov->role);
        }
        break;
        case TFM_CRYPTO_PAKE_SET_USER_SID:
        {
                status = psa_pake_set_user(operation, in_vec[1].base, in_vec[1].len);
        }
        break;
        case TFM_CRYPTO_PAKE_SET_PEER_SID:
        {
                status = psa_pake_set_peer(operation, in_vec[1].base, in_vec[1].len);
        }
        break;
        case TFM_CRYPTO_PAKE_SET_CONTEXT_SID:
        {
                status = psa_pake_set_context(operation, in_vec[1].base, in_vec[1].len);
        }
        break;
        case TFM_CRYPTO_PAKE_OUTPUT_SID:
        {
                psa_pake_step_t step = (psa_pake_step_t)iov->step;
                uint8_t *output = (uint8_t *)out_vec[0].base;
                size_t output_size = out_vec[0].len;
                size_t *output_length = &out_vec[0].len;
                status = psa_pake_output(operation, step, output, output_size, output_length);
                if (status != PSA_SUCCESS) {
                        out_vec[0].len = 0;
                }
        }
        break;
        case TFM_CRYPTO_PAKE_INPUT_SID:
        {
                status = psa_pake_input(operation, (psa_pake_step_t)iov->step, in_vec[1].base, in_vec[1].len);
        }
        break;
        case TFM_CRYPTO_PAKE_GET_SHARED_KEY_SID:
        {
                psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;
                const struct psa_client_key_attributes_s *client_key_attr =
                                                                 in_vec[1].base;
                psa_key_id_t *key_handle = out_vec[1].base;

                status = tfm_crypto_core_library_key_attributes_from_client(
                                                       client_key_attr,
                                                       encoded_key->owner,
                                                       &key_attributes);

                if (status != PSA_SUCCESS) {
                        break;
                }

                status = psa_pake_get_shared_key(operation, &key_attributes, &library_key);
                if (status == PSA_SUCCESS) {
                        *key_handle = CRYPTO_LIBRARY_GET_KEY_ID(library_key);
                        /* In case of success automatically release the operation */
                        goto release_operation_and_return;
                }
        }
        break;
        case TFM_CRYPTO_PAKE_ABORT_SID:
        {
                status = psa_pake_abort(operation);
                goto release_operation_and_return;
        }
        default:
                return PSA_ERROR_NOT_SUPPORTED;
        }

        return status;

release_operation_and_return:
        /* Release the operation context, ignore if the operation fails. */
        (void)tfm_crypto_operation_release(p_handle);
        return status;
}
#else /* CRYPTO_PAKE_MODULE_ENABLED */
psa_status_t tfm_crypto_pake_interface(psa_invec in_vec[],
                                       psa_outvec out_vec[],
                                       struct tfm_crypto_key_id_s *encoded_key)
{
    (void)in_vec;
    (void)out_vec;
    (void)encoded_key;

    return PSA_ERROR_NOT_SUPPORTED;
}
#endif /* CRYPTO_PAKE_MODULE_ENABLED */
/*!@}*/
