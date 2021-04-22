/*
 * Copyright (c) 2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "psa/update.h"
#include "tfm_api.h"

#ifdef TFM_PSA_API
#include "psa/client.h"
#include "psa_manifest/sid.h"
#else
#include "tfm_veneers.h"
#endif

#define IOVEC_LEN(x) (uint32_t)(sizeof(x)/sizeof(x[0]))

psa_status_t psa_fwu_write(uint32_t image_id,
                           size_t block_offset,
                           const void *block,
                           size_t block_size)
{
    psa_status_t status;
#ifdef TFM_PSA_API
    psa_handle_t handle;
#endif

    psa_invec in_vec[] = {
        { .base = &image_id, .len = sizeof(image_id) },
        { .base = &block_offset, .len = sizeof(block_offset) },
        { .base = block, .len = block_size }
    };

#ifdef TFM_PSA_API
    handle = psa_connect(TFM_FWU_WRITE_SID, TFM_FWU_WRITE_VERSION);
    if (!PSA_HANDLE_IS_VALID(handle)) {
        return PSA_ERROR_GENERIC_ERROR;
    }

    status = psa_call(handle, PSA_IPC_CALL, in_vec, IOVEC_LEN(in_vec), NULL, 0);

    psa_close(handle);
#else
    status = tfm_tfm_fwu_write_req_veneer(in_vec,
                                          IOVEC_LEN(in_vec),
                                          NULL,
                                          0);
#endif

    /* A parameter with a buffer pointer where its data length is longer than
     * maximum permitted, it is treated as a secure violation.
     * TF-M framework rejects the request with TFM_ERROR_INVALID_PARAMETER.
     * The firmware update secure PSA implementation returns
     * PSA_ERROR_INVALID_ARGUMENT in that case.
     */
    if (status == (psa_status_t)TFM_ERROR_INVALID_PARAMETER) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    return status;
}

psa_status_t psa_fwu_install(psa_image_id_t image_id,
                             psa_image_id_t *dependency_uuid,
                             psa_image_version_t *dependency_version)
{
    psa_status_t status;
#ifdef TFM_PSA_API
    psa_handle_t handle;
#endif

    psa_invec in_vec[] = {
        { .base = &image_id, .len = sizeof(image_id) }
    };

    psa_outvec out_vec[] = {
        { .base = dependency_uuid, .len = sizeof(*dependency_uuid) },
        { .base = dependency_version, .len = sizeof(*dependency_version)}
    };

    if ((dependency_uuid == NULL) || (dependency_version == NULL)) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

#ifdef TFM_PSA_API
    handle = psa_connect(TFM_FWU_INSTALL_SID, TFM_FWU_INSTALL_VERSION);
    if (!PSA_HANDLE_IS_VALID(handle)) {
        return PSA_ERROR_GENERIC_ERROR;
    }

    status = psa_call(handle, PSA_IPC_CALL, in_vec, IOVEC_LEN(in_vec),
                      out_vec, IOVEC_LEN(out_vec));

    psa_close(handle);
#else
    status = tfm_tfm_fwu_install_req_veneer(in_vec, IOVEC_LEN(in_vec),
                                            out_vec, IOVEC_LEN(out_vec));
#endif

    /* A parameter with a buffer pointer where its data length is longer than
     * maximum permitted, it is treated as a secure violation.
     * TF-M framework rejects the request with TFM_ERROR_INVALID_PARAMETER.
     * The firmware update secure PSA implementation returns
     * PSA_ERROR_INVALID_ARGUMENT in that case.
     */
    if (status == (psa_status_t)TFM_ERROR_INVALID_PARAMETER) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    return status;
}

psa_status_t psa_fwu_abort(psa_image_id_t image_id)
{
    psa_status_t status;
#ifdef TFM_PSA_API
    psa_handle_t handle;
#endif

    psa_invec in_vec[] = {
        { .base = &image_id, .len = sizeof(image_id) }
    };

#ifdef TFM_PSA_API
    handle = psa_connect(TFM_FWU_ABORT_SID, TFM_FWU_ABORT_VERSION);
    if (!PSA_HANDLE_IS_VALID(handle)) {
        return PSA_ERROR_GENERIC_ERROR;
    }

    status = psa_call(handle, PSA_IPC_CALL, in_vec, IOVEC_LEN(in_vec),
                      NULL, 0);

    psa_close(handle);
#else
    status = tfm_tfm_fwu_abort_req_veneer(in_vec, IOVEC_LEN(in_vec),
                                          NULL, 0);
#endif

    /* A parameter with a buffer pointer where its data length is longer than
     * maximum permitted, it is treated as a secure violation.
     * TF-M framework rejects the request with TFM_ERROR_INVALID_PARAMETER.
     * The firmware update secure PSA implementation returns
     * PSA_ERROR_INVALID_ARGUMENT in that case.
     */
    if (status == (psa_status_t)TFM_ERROR_INVALID_PARAMETER) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    return status;
}

psa_status_t psa_fwu_query(psa_image_id_t image_id, psa_image_info_t *info)
{
    psa_status_t status;
#ifdef TFM_PSA_API
    psa_handle_t handle;
#endif

    psa_invec in_vec[] = {
        { .base = &image_id, .len = sizeof(image_id) }
    };
    psa_outvec out_vec[] = {
        { .base = info, .len = sizeof(*info)}
    };

#ifdef TFM_PSA_API
    handle = psa_connect(TFM_FWU_QUERY_SID, TFM_FWU_QUERY_VERSION);
    if (!PSA_HANDLE_IS_VALID(handle)) {
        return PSA_ERROR_GENERIC_ERROR;
    }

    status = psa_call(handle, PSA_IPC_CALL, in_vec, IOVEC_LEN(in_vec),
                      out_vec, IOVEC_LEN(out_vec));

    psa_close(handle);
#else
    status = tfm_tfm_fwu_query_req_veneer(in_vec, IOVEC_LEN(in_vec),
                                          out_vec, IOVEC_LEN(out_vec));
#endif

    if (status == (psa_status_t)TFM_ERROR_INVALID_PARAMETER) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    return status;
}

psa_status_t psa_fwu_request_reboot(void)
{
    psa_status_t status;
#ifdef TFM_PSA_API
    psa_handle_t handle;
#endif

#ifdef TFM_PSA_API
    handle = psa_connect(TFM_FWU_REQUEST_REBOOT_SID,
                         TFM_FWU_REQUEST_REBOOT_VERSION);
    if (!PSA_HANDLE_IS_VALID(handle)) {
        return PSA_ERROR_GENERIC_ERROR;
    }

    status = psa_call(handle, PSA_IPC_CALL, NULL, 0, NULL, 0);

    psa_close(handle);
#else
    status = tfm_tfm_fwu_request_reboot_req_veneer(NULL, 0, NULL, 0);
#endif

    if (status == (psa_status_t)TFM_ERROR_INVALID_PARAMETER) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    return status;
}

psa_status_t psa_fwu_accept(void)
{
    psa_status_t status;
#ifdef TFM_PSA_API
    psa_handle_t handle;
#endif

#ifdef TFM_PSA_API
    handle = psa_connect(TFM_FWU_ACCEPT_SID,
                         TFM_FWU_ACCEPT_VERSION);
    if (!PSA_HANDLE_IS_VALID(handle)) {
        return PSA_ERROR_GENERIC_ERROR;
    }

    status = psa_call(handle, PSA_IPC_CALL, NULL, 0, NULL, 0);

    psa_close(handle);
#else
    status = tfm_tfm_fwu_accept_req_veneer(NULL, 0, NULL, 0);
#endif

    if (status == (psa_status_t)TFM_ERROR_INVALID_PARAMETER) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    return status;
}

psa_status_t psa_fwu_set_manifest(psa_image_id_t image_id,
                                  const void *manifest,
                                  size_t manifest_size,
                                  psa_hash_t *manifest_dependency)
{
    psa_status_t status;

    status = PSA_ERROR_NOT_SUPPORTED;

    return status;
}