/*
 * Copyright (c) 2023 Nordic Semiconductor ASA.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "platform/include/tfm_platform_system.h"
#include "platform/include/tfm_hal_its.h"
#include "psa/crypto.h"
#include <string.h>
#include <stdint.h>

/* The cc3xx driver is used directly in this file as the PSA APIs depends on
 * ITS, so using PSA APIs would result in circular dependencies
 */
#include "nrf_cc3xx_platform_derived_key.h"
#include "nrf_cc3xx_platform.h"
#include "nrf_cc3xx_platform_kmu.h"

#define ITS_ENCRYPTION_SUCCESS 0

#define HUK_KMU_SLOT 2
#define HUK_KMU_SIZE_BITS 128

/* Global encryption counter which resets per boot. The counter ensures that
 * the nonce will not be identical for consecutive file writes during the same
 * boot.
 */
static uint32_t g_enc_counter;

/* The global nonce seed which is fetched once in every boot. The seed is used
 * as part of the nonce and allows the platforms to diversify their nonces
 * across resets. Note that the way that this seed is generated is platform
 * specific, so the diversification is optional.
 */
static uint8_t g_enc_nonce_seed[TFM_ITS_ENC_NONCE_LENGTH -
                                sizeof(g_enc_counter)];

/* TFM_ITS_ENC_NONCE_LENGTH is configurable but this implementation expects
 * the seed to be 8 bytes and the nonce length to be 12.
 */
#if TFM_ITS_ENC_NONCE_LENGTH != 12
#error "This implementation only supports a ITS nonce of size 12"
#endif

/*
 * This implementation doesn't use monotonic counters, but therfore a 64 bit
 * seed combined with a counter, that gets reset on each reboot.
 * This still has the risk of getting a collision on the seed resulting in
 * nonce's beeing the same after a reboot.
*/
enum tfm_hal_status_t tfm_hal_its_aead_generate_nonce(uint8_t *nonce,
                                                      size_t nonce_size)
{
    int err;

    if(nonce == NULL){
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    if(nonce_size < sizeof(g_enc_nonce_seed) + sizeof(g_enc_counter)){
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    /* To avoid wrap-around of the g_enc_counter and subsequent re-use of the
     * nonce we check the counter value for its max value
     */
    if(g_enc_counter ==  UINT32_MAX) {
        return TFM_HAL_ERROR_GENERIC;
    }

    if (g_enc_counter == 0) {
        err = nrf_cc3xx_platform_get_nonce_seed(g_enc_nonce_seed);
        if (err != 0) {
            return TFM_HAL_ERROR_GENERIC;
        }
    }

    memcpy(nonce, g_enc_nonce_seed, sizeof(g_enc_nonce_seed));
    memcpy(nonce + sizeof(g_enc_nonce_seed),
               &g_enc_counter,
               sizeof(g_enc_counter));

    g_enc_counter++;

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_its_aead_set_deriv_label(
                                        struct tfm_hal_its_auth_crypt_ctx *ctx,
                                        uint8_t *deriv_label,
                                        size_t deriv_label_size)
{
    if (ctx == NULL || deriv_label == NULL) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    ctx->deriv_label = deriv_label;
    ctx->deriv_label_size = deriv_label_size;

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_its_aead_set_nonce(
                                        struct tfm_hal_its_auth_crypt_ctx *ctx,
                                        uint8_t *nonce,
                                        size_t nonce_size)
{
    if (ctx == NULL || nonce == NULL) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    ctx->nonce = nonce;
    ctx->nonce_size = nonce_size;

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_its_aead_set_ad(
                                        struct tfm_hal_its_auth_crypt_ctx *ctx,
                                        uint8_t *ad,
                                        size_t ad_size) {

    if (ctx == NULL) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    if (ad == NULL && ad_size != 0) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    ctx->add = ad;
    ctx->add_size = ad_size;

    return TFM_HAL_SUCCESS;
}


static bool ctx_is_valid(struct tfm_hal_its_auth_crypt_ctx *ctx)
{
    bool ret;

    if (ctx == NULL) {
        return false;
    }

    ret = (ctx->deriv_label == NULL && ctx->deriv_label_size != 0) ||
          (ctx->add == NULL && ctx->add_size != 0) ||
          (ctx->nonce == NULL && ctx->nonce_size != 0);

    return !ret;
}

static enum tfm_hal_status_t tfm_hal_its_aead_init(
                            struct tfm_hal_its_auth_crypt_ctx *ctx,
                            nrf_cc3xx_platform_derived_key_ctx_t *platform_ctx,
                            uint8_t *tag,
                            size_t tag_size)
{

    int err = NRF_CC3XX_PLATFORM_ERROR_INTERNAL;


    err = nrf_cc3xx_platform_derived_key_init(platform_ctx);
    if (err != NRF_CC3XX_PLATFORM_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    err = nrf_cc3xx_platform_derived_key_set_info(platform_ctx,
                                                  HUK_KMU_SLOT,
                                                  HUK_KMU_SIZE_BITS,
                                                  ctx->deriv_label,
                                                  ctx->deriv_label_size);
    if (err != NRF_CC3XX_PLATFORM_SUCCESS) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    err = nrf_cc3xx_platform_derived_key_set_cipher(platform_ctx,
                                                    ALG_CHACHAPOLY_256_BIT);

    if (err != NRF_CC3XX_PLATFORM_SUCCESS) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    err = nrf_cc3xx_platform_derived_key_set_auth_info(platform_ctx,
                                                       ctx->nonce,
                                                       ctx->nonce_size,
                                                       ctx->add,
                                                       ctx->add_size,
                                                       tag,
                                                       tag_size);
    if (err != NRF_CC3XX_PLATFORM_SUCCESS) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    return TFM_HAL_SUCCESS;
}

static void tfm_hal_its_aead_cleanup(
                            nrf_cc3xx_platform_derived_key_ctx_t *platform_ctx)
{
    if(platform_ctx != NULL){
        memset(platform_ctx,
                   0x0,
                   sizeof(nrf_cc3xx_platform_derived_key_ctx_t));
    }
}

enum tfm_hal_status_t tfm_hal_its_aead_encrypt(
                                        struct tfm_hal_its_auth_crypt_ctx *ctx,
                                        uint8_t *plaintext,
                                        size_t plaintext_size,
                                        uint8_t *ciphertext,
                                        size_t ciphertext_size,
                                        uint8_t *tag,
                                        size_t tag_size)
{
    nrf_cc3xx_platform_derived_key_ctx_t platform_ctx = {0};
    enum tfm_hal_status_t err = TFM_HAL_ERROR_GENERIC;
    int plat_err = NRF_CC3XX_PLATFORM_ERROR_INTERNAL;

    if (!ctx_is_valid(ctx) || tag == NULL) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    if (plaintext_size > ciphertext_size) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    err = tfm_hal_its_aead_init(ctx,
                                &platform_ctx,
                                tag,
                                tag_size);
    if (err !=  TFM_HAL_SUCCESS) {
        tfm_hal_its_aead_cleanup(&platform_ctx);
        return err;
    }


    plat_err = nrf_cc3xx_platform_derived_key_encrypt(&platform_ctx,
                                                      ciphertext,
                                                      plaintext_size,
                                                      plaintext);

    tfm_hal_its_aead_cleanup(&platform_ctx);

    if (plat_err != NRF_CC3XX_PLATFORM_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_its_aead_decrypt(
                                        struct tfm_hal_its_auth_crypt_ctx *ctx,
                                        uint8_t *ciphertext,
                                        size_t ciphertext_size,
                                        uint8_t *tag,
                                        size_t tag_size,
                                        uint8_t *plaintext,
                                        size_t plaintext_size)
{
    nrf_cc3xx_platform_derived_key_ctx_t platform_ctx = {0};
    enum tfm_hal_status_t err = TFM_HAL_ERROR_GENERIC;
    int plat_err = NRF_CC3XX_PLATFORM_ERROR_INTERNAL;

    if (!ctx_is_valid(ctx) || tag == NULL) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    if (plaintext_size < ciphertext_size) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    err = tfm_hal_its_aead_init(ctx,
                                &platform_ctx,
                                tag,
                                tag_size);
    if (err != TFM_HAL_SUCCESS) {
        tfm_hal_its_aead_cleanup(&platform_ctx);
        return err;
    }


    plat_err = nrf_cc3xx_platform_derived_key_decrypt(&platform_ctx,
                                                      plaintext,
                                                      ciphertext_size,
                                                      ciphertext);
    tfm_hal_its_aead_cleanup(&platform_ctx);

    if (plat_err != NRF_CC3XX_PLATFORM_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    return TFM_HAL_SUCCESS;
}

