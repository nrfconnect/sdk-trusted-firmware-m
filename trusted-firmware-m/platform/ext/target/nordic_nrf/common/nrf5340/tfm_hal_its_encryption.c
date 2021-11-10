#include "platform/include/tfm_platform_system.h"
#include "platform/include/tfm_hal_its.h"
#include "psa/crypto.h"

#include "nrf_cc3xx_platform_derived_key.h"
#include "nrf_cc3xx_platform.h"
/* TODO: Remove this after testing */
#include "nrf_cc3xx_platform_kmu.h"
#include <string.h>

#define ITS_ENCRYPTION_SUCCESS 0

#define HUK_KMU_SLOT 2
#define HUK_KMU_SIZE_BITS 256

static int32_t tfm_hal_its_aead_crypt(uint8_t *plaintext,
                                      size_t plaintext_size,
                                      uint8_t *ciphertext,
                                      size_t ciphertext_size,
                                      uint8_t *deriv_label,
                                      size_t deriv_label_size,
                                      uint8_t *nonce,
                                      size_t nonce_size,
                                      uint8_t *aad,
                                      size_t aad_size,
                                      uint8_t *tag,
                                      size_t tag_size,
                                      bool isEncrypt)
{
    int err = NRF_CC3XX_PLATFORM_ERROR_INTERNAL;
    nrf_cc3xx_platform_derived_key_ctx_t deriv_ctx = {0};

    err = nrf_cc3xx_platform_derived_key_init(&deriv_ctx);
    if (err != NRF_CC3XX_PLATFORM_SUCCESS){
        return PSA_ERROR_HARDWARE_FAILURE;
    }

    err = nrf_cc3xx_platform_derived_key_set_info(&deriv_ctx,
                                                  HUK_KMU_SLOT,
                                                  HUK_KMU_SIZE_BITS,
                                                  deriv_label,
                                                  deriv_label_size);
    if (err != NRF_CC3XX_PLATFORM_SUCCESS){
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    err = nrf_cc3xx_platform_derived_key_set_cipher(&deriv_ctx,
                                                    ALG_CHACHAPOLY_256_BIT);

    if (err != NRF_CC3XX_PLATFORM_SUCCESS){
        return PSA_ERROR_HARDWARE_FAILURE;
    }

    err = nrf_cc3xx_platform_derived_key_set_auth_info(&deriv_ctx,
                                                       nonce,
                                                       nonce_size,
                                                       aad,
                                                       aad_size,
                                                       tag,
                                                       tag_size);
    if (err != NRF_CC3XX_PLATFORM_SUCCESS){
        return PSA_ERROR_INVALID_ARGUMENT;
    }


    if(isEncrypt){
        err = nrf_cc3xx_platform_derived_key_encrypt(&deriv_ctx,
                                                     ciphertext,
                                                     plaintext_size,
                                                     plaintext);

    } else
    {
        err = nrf_cc3xx_platform_derived_key_decrypt(&deriv_ctx,
                                                     plaintext,
                                                     ciphertext_size,
                                                     ciphertext);
    }

    if (err != NRF_CC3XX_PLATFORM_SUCCESS){
        return PSA_ERROR_HARDWARE_FAILURE;
    }

    return PSA_SUCCESS;
}

psa_status_t tfm_hal_its_aead_set_deriv_label(struct tfm_hal_its_aead_ctx *ctx,
                                              uint8_t *deriv_label,
                                              size_t deriv_label_size)
{
    if(ctx == NULL || deriv_label == NULL){
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    ctx->deriv_label = deriv_label;
    ctx->deriv_label_size = deriv_label_size;

    return PSA_SUCCESS;
}



psa_status_t tfm_hal_its_aead_set_nonce(struct tfm_hal_its_aead_ctx *ctx,
                                        uint8_t *nonce,
                                        size_t nonce_size)
{
    if(ctx == NULL || nonce == NULL){
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    ctx->nonce = nonce;
    ctx->nonce_size = nonce_size;

    return PSA_SUCCESS;
}

psa_status_t tfm_hal_its_aead_set_aad(struct tfm_hal_its_aead_ctx *ctx,
                                      uint8_t *aad,
                                      size_t aad_size)
{
    if(ctx == NULL){
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    if(aad == NULL && aad_size != 0){
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    ctx->aad = aad;
    ctx->aad_size = aad_size;

    return PSA_SUCCESS;
}

psa_status_t tfm_hal_its_aead_set_tag(struct tfm_hal_its_aead_ctx *ctx,
                                      uint8_t *tag,
                                      size_t tag_size)
{
    if(ctx == NULL || tag == NULL){
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    ctx->tag = tag;
    ctx->tag_size = tag_size;

    return PSA_SUCCESS;
}

psa_status_t tfm_hal_its_aead_encrypt(struct tfm_hal_its_aead_ctx *ctx,
                                      uint8_t *plaintext,
                                      size_t plaintext_size,
                                      uint8_t *ciphertext,
                                      size_t ciphertext_size,
                                      size_t *ciphertext_output)
{
    if(ciphertext_output == NULL){
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    *ciphertext_output = plaintext_size;

    return tfm_hal_its_aead_crypt(plaintext,
                                  plaintext_size,
                                  ciphertext,
                                  ciphertext_size,
                                  ctx->deriv_label,
                                  ctx->deriv_label_size,
                                  ctx->nonce,
                                  ctx->nonce_size,
                                  ctx->aad,
                                  ctx->aad_size,
                                  ctx->tag,
                                  ctx->tag_size,
                                  true);
}

psa_status_t tfm_hal_its_aead_decrypt(struct tfm_hal_its_aead_ctx *ctx,
                                      uint8_t *ciphertext,
                                      size_t ciphertext_size,
                                      uint8_t *plaintext,
                                      size_t plaintext_size,
                                      size_t *plaintext_output)
{
    if(plaintext_output == NULL){
        return PSA_ERROR_INVALID_ARGUMENT;
    }

    *plaintext_output = plaintext_size;

    return tfm_hal_its_aead_crypt(plaintext,
                                  plaintext_size,
                                  ciphertext,
                                  ciphertext_size,
                                  ctx->deriv_label,
                                  ctx->deriv_label_size,
                                  ctx->nonce,
                                  ctx->nonce_size,
                                  ctx->aad,
                                  ctx->aad_size,
                                  ctx->tag,
                                  ctx->tag_size,
                                  false);

}