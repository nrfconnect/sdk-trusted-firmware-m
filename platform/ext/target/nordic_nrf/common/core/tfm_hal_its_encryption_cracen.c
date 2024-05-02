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

#include <stdint.h>
#include <string.h>

#include "config_tfm.h"
#include "platform/include/tfm_hal_its_encryption.h"
#include "platform/include/tfm_hal_its.h"
#include "psa/crypto.h"
#include "tfm_crypto_defs.h"

#define CHACHA20_KEY_SIZE 32
#define TFM_ITS_AEAD_ALG PSA_ALG_CHACHA20_POLY1305

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
 * It would still need 3.3x10^9 resets to get a collision with a probability of
 * 0.25.
 */
enum tfm_hal_status_t tfm_hal_its_aead_generate_nonce(uint8_t *nonce,
                                                      const size_t nonce_size)
{
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

    /* psa_generate_random is not using any key/its functions wo we can use it here*/
    if (g_enc_counter == 0) {
        psa_status_t status = psa_generate_random(g_enc_nonce_seed, sizeof(g_enc_nonce_seed));
        if (status != PSA_SUCCESS) {
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

static bool ctx_is_valid(struct tfm_hal_its_auth_crypt_ctx *ctx)
{
    bool ret;

    if (ctx == NULL) {
        return false;
    }

    ret = (ctx->deriv_label == NULL && ctx->deriv_label_size != 0) ||
          (ctx->aad == NULL && ctx->add_size != 0) ||
          (ctx->nonce == NULL && ctx->nonce_size != 0);

    return !ret;
}

/*
 * The cracen driver code doesn't use any persistent keys so no calls to its
 * therefore the PSA API's can be used directly.
 */
psa_status_t tfm_hal_its_get_aead(struct tfm_hal_its_auth_crypt_ctx *ctx,
                                  uint8_t *plaintext,
                                  const size_t plaintext_size,
                                  uint8_t *ciphertext,
                                  const size_t ciphertext_size,
                                  uint8_t *tag,
                                  const size_t tag_size,
                                  bool encrypt)
{
    psa_status_t status;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_derivation_operation_t op = PSA_KEY_DERIVATION_OPERATION_INIT;
    psa_key_id_t key;
    size_t ciphertext_length;
    size_t tag_length = PSA_AEAD_TAG_LENGTH(PSA_KEY_TYPE_CHACHA20,
                                            PSA_BYTES_TO_BITS(CHACHA20_KEY_SIZE),
                                            TFM_ITS_AEAD_ALG);

    if (!ctx_is_valid(ctx) || tag == NULL) {
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    if(tag_size < tag_length){
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    if (ciphertext_size < PSA_AEAD_ENCRYPT_OUTPUT_SIZE(PSA_KEY_TYPE_CHACHA20,
                                                       TFM_ITS_AEAD_ALG,
                                                       plaintext_size)){
        return TFM_HAL_ERROR_INVALID_INPUT;
    }

    /* Set the key attributes for the key */
    psa_set_key_usage_flags(&attributes, (PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT));
    psa_set_key_algorithm(&attributes, TFM_ITS_AEAD_ALG);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_CHACHA20);
    psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(CHACHA20_KEY_SIZE));

    status = psa_key_derivation_setup(&op, PSA_ALG_SP800_108_COUNTER_CMAC);
    if (status != PSA_SUCCESS) {
        return status;
    }

    /* Set up a key derivation operation with HUK  */
    status = psa_key_derivation_input_key(&op, PSA_KEY_DERIVATION_INPUT_SECRET,
                                          TFM_BUILTIN_KEY_ID_HUK);
    if (status != PSA_SUCCESS) {
        goto err_release_op;
    }

    /* Supply the PS key label as an input to the key derivation */
    status = psa_key_derivation_input_bytes(&op, PSA_KEY_DERIVATION_INPUT_LABEL,
                                            ctx->deriv_label,
                                            ctx->deriv_label_size);
    if (status != PSA_SUCCESS) {
        goto err_release_op;
    }

    /* Create the storage key from the key derivation operation */
    status = psa_key_derivation_output_key(&attributes, &op, &key);
    if (status != PSA_SUCCESS) {
        goto err_release_op;
    }

    /* Free resources associated with the key derivation operation */
    status = psa_key_derivation_abort(&op);
    if (status != PSA_SUCCESS) {
        goto err_release_key;
    }

    if (encrypt) {
        status = psa_aead_encrypt(key,
                                  TFM_ITS_AEAD_ALG,
                                  ctx->nonce,
                                  ctx->nonce_size,
                                  ctx->aad,
                                  ctx->add_size,
                                  plaintext,
                                  plaintext_size,
                                  ciphertext,
                                  ciphertext_size,
                                  &ciphertext_length);
    } else {
        status = psa_aead_decrypt(key,
                                  TFM_ITS_AEAD_ALG,
                                  ctx->nonce,
                                  ctx->nonce_size,
                                  ctx->aad,
                                  ctx->add_size,
                                  ciphertext,
                                  ciphertext_size,
                                  plaintext,
                                  plaintext_size,
                                  &ciphertext_length);
    }
    if(status != PSA_SUCCESS){
        goto err_release_key;
    }

    /* copy tag from ciphertext buffer to tag buffer */
    memcpy(tag, ciphertext + ciphertext_length - tag_length, tag_length);

err_release_key:
    (void)psa_destroy_key(key);

    return status;

err_release_op:
    (void)psa_key_derivation_abort(&op);

    return PSA_ERROR_GENERIC_ERROR;
}

enum tfm_hal_status_t tfm_hal_its_aead_encrypt(struct tfm_hal_its_auth_crypt_ctx *ctx,
                                               const uint8_t *plaintext,
                                               const size_t plaintext_size,
                                               uint8_t *ciphertext,
                                               const size_t ciphertext_size,
                                               uint8_t *tag,
                                               const size_t tag_size)
{
    psa_status_t status = tfm_hal_its_get_aead(ctx,
                                               plaintext,
                                               plaintext_size,
                                               ciphertext,
                                               ciphertext_size,
                                               tag,
                                               tag_size,
                                               true);
    if (status != PSA_SUCCESS) {
        return TFM_HAL_ERROR_GENERIC;
    }

    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_its_aead_decrypt(struct tfm_hal_its_auth_crypt_ctx *ctx,
                                               const uint8_t *ciphertext,
                                               const size_t ciphertext_size,
                                               uint8_t *tag,
                                               const size_t tag_size,
                                               uint8_t *plaintext,
                                               const size_t plaintext_size)
{
    psa_status_t status = tfm_hal_its_get_aead(ctx,
                                               ciphertext,
                                               ciphertext_size,
                                               plaintext,
                                               plaintext_size,
                                               tag,
                                               tag_size,
                                               false);

    return TFM_HAL_SUCCESS;
}

