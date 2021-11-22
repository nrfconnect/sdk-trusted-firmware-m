/*
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_internal_trusted_storage.h"

#include "tfm_hal_its.h"
#include "tfm_hal_ps.h"
#include "flash/its_flash.h"
#include "flash_fs/its_flash_fs.h"
#include "psa_manifest/pid.h"
#include "tfm_memory_utils.h"
#include "tfm_its_defs.h"
#include "tfm_its_req_mngr.h"
#include "its_utils.h"

#ifdef TFM_PARTITION_PROTECTED_STORAGE
#include "ps_object_defs.h"
#endif

#ifndef ITS_BUF_SIZE
/* By default, set the ITS buffer size to the max asset size so that all
 * requests can be handled in one iteration.
 */
#define ITS_BUF_SIZE ITS_MAX_ASSET_SIZE
#endif

/* Buffer to store asset data from the caller.
 * Note: size must be aligned to the max flash program unit to meet the
 * alignment requirement of the filesystem.
 */
static uint8_t asset_data[ITS_UTILS_ALIGN(ITS_BUF_SIZE,
                                          ITS_FLASH_MAX_ALIGNMENT)];

#ifdef TFM_ITS_ENCRYPT

#if ITS_BUF_SIZE != ITS_MAX_ASSET_SIZE
#error "ITS_BUF_SIZE needs to be equal to the ITS_MAX_ASSET_SIZE when ITS encryption is enabled"
#endif
/* Buffer to store the encrypted asset data before it is stored in the filesystem.
 * Note: size must be aligned to the max flash program unit to meet the
 * alignment requirement of the filesystem.
 */
static uint8_t enc_asset_data[ITS_UTILS_ALIGN(ITS_MAX_ASSET_SIZE,
                                              ITS_FLASH_MAX_ALIGNMENT)];

/* The aad consist of the file id, the flags (uint32_t) and the data length(size_t) */
static uint8_t g_enc_aad[ITS_FILE_ID_SIZE + sizeof(uint32_t) + sizeof(size_t)];
static uint32_t g_enc_counter = 0;
#endif

static uint8_t g_fid[ITS_FILE_ID_SIZE];
static struct its_file_info_t g_file_info;

static its_flash_fs_ctx_t fs_ctx_its;
static struct its_flash_fs_config_t fs_cfg_its = {
    .flash_dev = &ITS_FLASH_DEV,
    .program_unit = ITS_FLASH_ALIGNMENT,
    .max_file_size = ITS_UTILS_ALIGN(ITS_MAX_ASSET_SIZE, ITS_FLASH_ALIGNMENT),
    .max_num_files = ITS_NUM_ASSETS + 1, /* Extra file for atomic replacement */
};

#ifdef TFM_PARTITION_PROTECTED_STORAGE
static its_flash_fs_ctx_t fs_ctx_ps;
static struct its_flash_fs_config_t fs_cfg_ps = {
    .flash_dev = &PS_FLASH_DEV,
    .program_unit = PS_FLASH_ALIGNMENT,
    .max_file_size = ITS_UTILS_ALIGN(PS_MAX_OBJECT_SIZE, PS_FLASH_ALIGNMENT),
    .max_num_files = PS_MAX_NUM_OBJECTS,
};
#endif

static its_flash_fs_ctx_t *get_fs_ctx(int32_t client_id)
{
#ifdef TFM_PARTITION_PROTECTED_STORAGE
    return (client_id == TFM_SP_PS) ? &fs_ctx_ps : &fs_ctx_its;
#else
    (void)client_id;
    return &fs_ctx_its;
#endif
}

#ifdef TFM_ITS_ENCRYPT
static psa_status_t tfm_its_fill_enc_add(uint8_t *add,
                                         size_t add_size,
                                         uint8_t *fid,
                                         size_t fid_size,
                                         uint32_t flags,
                                         size_t data_size)

{
    size_t  offset = 0;
    if(add_size < fid_size + sizeof(flags) + sizeof(data_size)){
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    tfm_memset(add, 0x0, add_size);
    tfm_memcpy(add, fid, fid_size);
    offset+= fid_size;
    tfm_memcpy(add + offset, &flags, sizeof(flags));
    offset+= sizeof(flags);
    tfm_memcpy(add + offset, &data_size, sizeof(data_size));

    return PSA_SUCCESS;
}

static psa_status_t tfm_its_fill_enc_nonce(uint8_t *nonce,
                                           size_t nonce_size,
                                           uint8_t *fid,
                                           size_t fid_size,
                                           uint32_t counter)

{
    if( nonce_size < fid_size + sizeof(counter)){
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    tfm_memset(nonce, 0x0, nonce_size);
    tfm_memcpy(nonce, fid, fid_size);
    tfm_memcpy(nonce + fid_size, &counter, sizeof(counter));

    return PSA_SUCCESS;
}

#endif

/**
 * \brief Maps a pair of client id and uid to a file id.
 *
 * \param[in]  client_id  Identifier of the asset's owner (client)
 * \param[in]  uid        Identifier for the data
 * \param[out] fid        Identifier of the file
 */
static void tfm_its_get_fid(int32_t client_id,
                            psa_storage_uid_t uid,
                            uint8_t *fid)
{
    tfm_memcpy(fid, (const void *)&client_id, sizeof(client_id));
    tfm_memcpy(fid + sizeof(client_id), (const void *)&uid, sizeof(uid));
}

/**
 * \brief Initialise the static filesystem configurations.
 *
 * \return Returns PSA_ERROR_PROGRAMMER_ERROR if there is a configuration error,
 *         and PSA_SUCCESS otherwise.
 */
static psa_status_t init_fs_cfg(void)
{
    struct tfm_hal_its_fs_info_t its_fs_info;

    /* Check the compile-time program unit matches the runtime value */
    if (TFM_HAL_ITS_FLASH_DRIVER.GetInfo()->program_unit
        != TFM_HAL_ITS_PROGRAM_UNIT) {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Retrieve flash properties from the ITS flash driver */
    fs_cfg_its.sector_size = TFM_HAL_ITS_FLASH_DRIVER.GetInfo()->sector_size;
    fs_cfg_its.erase_val = TFM_HAL_ITS_FLASH_DRIVER.GetInfo()->erased_value;

    /* Retrieve FS parameters from the ITS HAL */
    if (tfm_hal_its_fs_info(&its_fs_info) != TFM_HAL_SUCCESS) {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Derive address, block_size and num_blocks from the HAL parameters */
    fs_cfg_its.flash_area_addr = its_fs_info.flash_area_addr;
    fs_cfg_its.block_size = fs_cfg_its.sector_size
                            * its_fs_info.sectors_per_block;
    fs_cfg_its.num_blocks = its_fs_info.flash_area_size / fs_cfg_its.block_size;

#ifdef TFM_PARTITION_PROTECTED_STORAGE
    struct tfm_hal_ps_fs_info_t ps_fs_info;

    /* Check the compile-time program unit matches the runtime value */
    if (TFM_HAL_PS_FLASH_DRIVER.GetInfo()->program_unit
        != TFM_HAL_PS_PROGRAM_UNIT) {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Retrieve flash properties from the PS flash driver */
    fs_cfg_ps.sector_size = TFM_HAL_PS_FLASH_DRIVER.GetInfo()->sector_size;
    fs_cfg_ps.erase_val = TFM_HAL_PS_FLASH_DRIVER.GetInfo()->erased_value;

    /* Retrieve FS parameters from the PS HAL */
    if (tfm_hal_ps_fs_info(&ps_fs_info) != TFM_HAL_SUCCESS) {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    /* Derive address, block_size and num_blocks from the HAL parameters */
    fs_cfg_ps.flash_area_addr = ps_fs_info.flash_area_addr;
    fs_cfg_ps.block_size = fs_cfg_ps.sector_size * ps_fs_info.sectors_per_block;
    fs_cfg_ps.num_blocks = ps_fs_info.flash_area_size / fs_cfg_ps.block_size;
#endif

    return PSA_SUCCESS;
}

psa_status_t tfm_its_init(void)
{
    psa_status_t status;

    status = init_fs_cfg();
    if (status != PSA_SUCCESS) {
        return status;
    }

    /* Initialise the ITS filesystem context */
    status = its_flash_fs_init_ctx(&fs_ctx_its, &fs_cfg_its, &ITS_FLASH_OPS);
    if (status != PSA_SUCCESS) {
        return status;
    }

    /* Prepare the ITS filesystem */
    status = its_flash_fs_prepare(&fs_ctx_its);
#ifdef ITS_CREATE_FLASH_LAYOUT
    /* If ITS_CREATE_FLASH_LAYOUT is set, it indicates that it is required to
     * create a ITS flash layout. ITS service will generate an empty and valid
     * ITS flash layout to store assets. It will erase all data located in the
     * assigned ITS memory area before generating the ITS layout.
     * This flag is required to be set if the ITS memory area is located in
     * non-persistent memory.
     * This flag can be set if the ITS memory area is located in persistent
     * memory without a previous valid ITS flash layout in it. That is the case
     * when it is the first time in the device life that the ITS service is
     * executed.
     */
     if (status != PSA_SUCCESS) {
        /* Remove all data in the ITS memory area and create a valid ITS flash
         * layout in that area.
         */
        status = its_flash_fs_wipe_all(&fs_ctx_its);
        if (status != PSA_SUCCESS) {
            return status;
        }

        /* Attempt to prepare again */
        status = its_flash_fs_prepare(&fs_ctx_its);
    }
#endif /* ITS_CREATE_FLASH_LAYOUT */

#ifdef TFM_PARTITION_PROTECTED_STORAGE
    /* Check status of ITS initialisation before continuing with PS */
    if (status != PSA_SUCCESS) {
        return status;
    }

    /* Initialise the PS filesystem context */
    status = its_flash_fs_init_ctx(&fs_ctx_ps, &fs_cfg_ps, &PS_FLASH_OPS);
    if (status != PSA_SUCCESS) {
        return status;
    }

    /* Prepare the PS filesystem */
    status = its_flash_fs_prepare(&fs_ctx_ps);
#ifdef PS_CREATE_FLASH_LAYOUT
    /* If PS_CREATE_FLASH_LAYOUT is set, it indicates that it is required to
     * create a PS flash layout. PS service will generate an empty and valid
     * PS flash layout to store assets. It will erase all data located in the
     * assigned PS memory area before generating the PS layout.
     * This flag is required to be set if the PS memory area is located in
     * non-persistent memory.
     * This flag can be set if the PS memory area is located in persistent
     * memory without a previous valid PS flash layout in it. That is the case
     * when it is the first time in the device life that the PS service is
     * executed.
     */
     if (status != PSA_SUCCESS) {
        /* Remove all data in the PS memory area and create a valid PS flash
         * layout in that area.
         */
        status = its_flash_fs_wipe_all(&fs_ctx_ps);
        if (status != PSA_SUCCESS) {
            return status;
        }

        /* Attempt to prepare again */
        status = its_flash_fs_prepare(&fs_ctx_ps);
    }
#endif /* PS_CREATE_FLASH_LAYOUT */
#endif /* TFM_PARTITION_PROTECTED_STORAGE */

    return status;
}

psa_status_t tfm_its_set(int32_t client_id,
                         psa_storage_uid_t uid,
                         size_t data_length,
                         psa_storage_create_flags_t create_flags)
{
    psa_status_t status;
    size_t write_size;
    size_t offset;
    uint32_t flags;
    uint8_t *buffer_pnt = asset_data;
    struct its_file_info_t f_info = {0};

    /* Check that the UID is valid */
    if (uid == TFM_ITS_INVALID_UID) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    /* Check that the create_flags does not contain any unsupported flags */
    if (create_flags & ~(PSA_STORAGE_FLAG_WRITE_ONCE |
                         PSA_STORAGE_FLAG_NO_CONFIDENTIALITY |
                         PSA_STORAGE_FLAG_NO_REPLAY_PROTECTION)) {
        return PSA_ERROR_NOT_SUPPORTED;
    }

#ifdef TFM_ITS_ENCRYPT
        struct tfm_hal_its_aead_ctx aead_ctx = {0};

        if(data_length > sizeof(asset_data)){
            return PSA_ERROR_BUFFER_TOO_SMALL;
        }
#endif

    /* Set file id */
    tfm_its_get_fid(client_id, uid, g_fid);

    /* Read file info */
    status = its_flash_fs_file_get_info(get_fs_ctx(client_id), g_fid,
                                        &g_file_info);
    if (status == PSA_SUCCESS) {
        /* If the object exists and has the write once flag set, then it
         * cannot be modified.
         */
        if (g_file_info.flags & PSA_STORAGE_FLAG_WRITE_ONCE) {
            return PSA_ERROR_NOT_PERMITTED;
        }
    } else if (status != PSA_ERROR_DOES_NOT_EXIST) {
        /* If the file does not exist, then do nothing.
         * If other error occurred, return it
         */
        return status;
    }

    offset = 0;
    flags = (uint32_t)create_flags |
            ITS_FLASH_FS_FLAG_CREATE | ITS_FLASH_FS_FLAG_TRUNCATE;

    /* Populate the file info for the new file */
    f_info.size_max = data_length;
    f_info.flags = flags;

    /* Iteratively read data from the caller and write it to the filesystem, in
     * chunks no larger than the size of the asset_data buffer.
     */
    do {
        write_size = ITS_UTILS_MIN(data_length, sizeof(asset_data));

        /* Read asset data from the caller */
        (void)its_req_mngr_read(asset_data, write_size);

#ifdef TFM_ITS_ENCRYPT
        status = tfm_its_fill_enc_add(g_enc_aad,
                                      sizeof(g_enc_aad),
                                      g_fid,
                                      sizeof(g_fid),
                                      flags,
                                      data_length);
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        status = tfm_its_fill_enc_nonce(f_info.nonce,
                                        sizeof(f_info.nonce),
                                        g_fid,
                                        sizeof(g_fid),
                                        g_enc_counter);
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        g_enc_counter++;

        status = tfm_hal_its_aead_set_deriv_label(&aead_ctx,
                                                  g_fid,
                                                  sizeof(g_fid));
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        status = tfm_hal_its_aead_set_nonce(&aead_ctx,
                                            f_info.nonce,
                                            sizeof(f_info.nonce));
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        status = tfm_hal_its_aead_set_aad(&aead_ctx,
                                          g_enc_aad,
                                          sizeof(g_enc_aad));
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        status = tfm_hal_its_aead_set_tag(&aead_ctx,
                                          f_info.tag,
                                          sizeof(f_info.tag));
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        size_t  output_size = 0;
        status = tfm_hal_its_aead_encrypt(&aead_ctx,
                                          asset_data,
                                          sizeof(asset_data),
                                          enc_asset_data,
                                          sizeof(enc_asset_data),
                                          &output_size);

        if( status != PSA_SUCCESS || output_size < data_length){
            return PSA_ERROR_GENERIC_ERROR;
        }

        f_info.plaintext_size = data_length;
        f_info.size_max = output_size;
        buffer_pnt = enc_asset_data;

#endif

        /* Write to the file in the file system */
        status = its_flash_fs_file_write(get_fs_ctx(client_id), g_fid,
                                         &f_info, write_size, offset,
                                         buffer_pnt);
        if (status != PSA_SUCCESS) {
            return status;
        }

        /* Do not create or truncate after the first iteration */
        flags &= ~(ITS_FLASH_FS_FLAG_CREATE | ITS_FLASH_FS_FLAG_TRUNCATE);

        offset += write_size;
        data_length -= write_size;
    } while (data_length > 0);

    return PSA_SUCCESS;
}

psa_status_t tfm_its_get(int32_t client_id,
                         psa_storage_uid_t uid,
                         size_t data_offset,
                         size_t data_size,
                         size_t *p_data_length)
{
    psa_status_t status;
    size_t read_size;
    /* The data size of the required file, this will be equal to the data_size
     * argument when ITS encryption is not enabled */
    size_t f_data_size;

#ifdef TFM_PARTITION_TEST_PS
    /* The PS test partition can call tfm_its_get() through PS code. Treat it
     * as if it were PS.
     */
    if (client_id == TFM_SP_PS_TEST) {
        client_id = TFM_SP_PS;
    }
#endif

    /* Check that the UID is valid */
    if (uid == TFM_ITS_INVALID_UID) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    /* Set file id */
    tfm_its_get_fid(client_id, uid, g_fid);

#ifdef TFM_ITS_ENCRYPT
        struct tfm_hal_its_aead_ctx aead_ctx = {0};

        if(data_size > sizeof(asset_data)){
            return PSA_ERROR_BUFFER_TOO_SMALL;
        }
#endif

    /* Read file info */
    status = its_flash_fs_file_get_info(get_fs_ctx(client_id), g_fid,
                                        &g_file_info);
    if (status != PSA_SUCCESS) {
        return status;
    }

#ifdef TFM_ITS_ENCRYPT
    f_data_size = g_file_info.plaintext_size;
#else
    f_data_size = g_file_info.size_current;
#endif

    /* Boundary check the incoming request */
    if (data_offset > f_data_size) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    /* Copy the object data only from within the file boundary */
    data_size = ITS_UTILS_MIN(data_size,
                              f_data_size - data_offset);

    /* Update the size of the output data */
    *p_data_length = data_size;

    /* Iteratively read data from the filesystem and write it to the caller, in
     * chunks no larger than the size of the asset_data buffer.
     */
    do {
        /* Read as much of the data as will fit in the asset_data buffer */
        read_size = ITS_UTILS_MIN(data_size, sizeof(asset_data));

#ifdef TFM_ITS_ENCRYPT
        if ( g_file_info.size_max < sizeof(enc_asset_data) ){
            return PSA_ERROR_BUFFER_TOO_SMALL;
        }

        /* When encryption is enabled we need to read the whole file */
        status = its_flash_fs_file_read(get_fs_ctx(client_id), g_fid, g_file_info.size_max,
                                        0, enc_asset_data);
        if (status != PSA_SUCCESS) {
            *p_data_length = 0;
            return status;
        }

        status = tfm_its_fill_enc_add(g_enc_aad,
                                      sizeof(g_enc_aad),
                                      g_fid,
                                      sizeof(g_fid),
                                      g_file_info.flags,
                                      g_file_info.size_max);
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        status = tfm_hal_its_aead_set_deriv_label(&aead_ctx,
                                                  g_fid,
                                                  sizeof(g_fid));
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        status = tfm_hal_its_aead_set_nonce(&aead_ctx,
                                            g_file_info.nonce,
                                            sizeof(g_file_info.nonce));
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        status = tfm_hal_its_aead_set_aad(&aead_ctx,
                                          g_enc_aad,
                                          sizeof(g_enc_aad));
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        status = tfm_hal_its_aead_set_tag(&aead_ctx,
                                          g_file_info.tag,
                                          sizeof(g_file_info.tag));
        if( status != PSA_SUCCESS ){
            return PSA_ERROR_GENERIC_ERROR;
        }

        size_t output_size = 0;
        status = tfm_hal_its_aead_decrypt(&aead_ctx,
                                          enc_asset_data,
                                          g_file_info.size_max,
                                          asset_data,
                                          sizeof(asset_data),
                                          &output_size);

        if( status != PSA_SUCCESS || output_size != g_file_info.plaintext_size){
            return PSA_ERROR_GENERIC_ERROR;
        }


        tfm_memcpy(asset_data, asset_data + data_offset, data_size);
        tfm_memset(asset_data + data_size, 0x0, g_file_info.plaintext_size - data_size);

#else
        /* Read file data from the filesystem */
        status = its_flash_fs_file_read(get_fs_ctx(client_id), g_fid, read_size,
                                        data_offset, asset_data);
        if (status != PSA_SUCCESS) {
            *p_data_length = 0;
            return status;
        }
#endif

        /* Write asset data to the caller */
        its_req_mngr_write(asset_data, read_size);

        data_offset += read_size;
        data_size -= read_size;
    } while (data_size > 0);

    return PSA_SUCCESS;
}

psa_status_t tfm_its_get_info(int32_t client_id, psa_storage_uid_t uid,
                              struct psa_storage_info_t *p_info)
{
    psa_status_t status;

    /* Check that the UID is valid */
    if (uid == TFM_ITS_INVALID_UID) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    /* Set file id */
    tfm_its_get_fid(client_id, uid, g_fid);

    /* Read file info */
    status = its_flash_fs_file_get_info(get_fs_ctx(client_id), g_fid,
                                        &g_file_info);
    if (status != PSA_SUCCESS) {
        return status;
    }

    /* Copy file info to the PSA info struct */
    p_info->capacity = g_file_info.size_current;
    p_info->size = g_file_info.size_current;
    p_info->flags = g_file_info.flags;

    return PSA_SUCCESS;
}

psa_status_t tfm_its_remove(int32_t client_id, psa_storage_uid_t uid)
{
    psa_status_t status;

#ifdef TFM_PARTITION_TEST_PS
    /* The PS test partition can call tfm_its_remove() through PS code. Treat
     * it as if it were PS.
     */
    if (client_id == TFM_SP_PS_TEST) {
        client_id = TFM_SP_PS;
    }
#endif

    /* Check that the UID is valid */
    if (uid == TFM_ITS_INVALID_UID) {
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    /* Set file id */
    tfm_its_get_fid(client_id, uid, g_fid);

    status = its_flash_fs_file_get_info(get_fs_ctx(client_id), g_fid,
                                        &g_file_info);
    if (status != PSA_SUCCESS) {
        return status;
    }

    /* If the object exists and has the write once flag set, then it
     * cannot be deleted.
     */
    if (g_file_info.flags & PSA_STORAGE_FLAG_WRITE_ONCE) {
        return PSA_ERROR_NOT_PERMITTED;
    }

    /* Delete old file from the persistent area */
    return its_flash_fs_file_delete(get_fs_ctx(client_id), g_fid);
}
