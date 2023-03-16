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

/**
 * \brief Struct containing information required from the platform to perform
 *        encryption/decryption of ITS files.
 */
struct tfm_hal_its_auth_crypt_ctx{
    uint8_t *deriv_label;    /* The derivation label for AEAD */
    size_t deriv_label_size; /* Size of the deriv_label in bytes */
    uint8_t *add;            /* The additional authenticated data for AEAD */
    size_t add_size;         /* Size of the add in bytes */
    uint8_t *nonce;          /* The nonce for AEAD */
    size_t nonce_size;       /* Size of the nonce in bytes */
};

