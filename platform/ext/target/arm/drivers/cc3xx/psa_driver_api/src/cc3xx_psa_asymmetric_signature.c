/*
 * Copyright (c) 2021-2024, The TrustedFirmware-M Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/** @file cc3xx_psa_asymmetric_signature.c
 *
 * This file contains the implementation of the entry points associated to the
 * asymmetric signature capability as described by the PSA Cryptoprocessor
 * Driver interface specification
 *
 */

#include "cc3xx_psa_asymmetric_signature.h"
#include "cc3xx_psa_hash.h"
#include "cc3xx_psa_key_generation.h"
#include "cc3xx_misc.h"

#include "cc3xx_stdlib.h"
#include "cc3xx_ecdsa.h"
#include "cc3xx_ec_curve_data.h"
#include "cc3xx_rsa.h"
#include <stdio.h>
#include <assert.h>
/* ToDo: This needs to be sorted out at TF-M level
 * To be able to include the PSA style configuration
 */

#include "mbedtls/build_info.h"

#if defined(PSA_WANT_ALG_RSA_PKCS1V15_SIGN)
/**
 * @brief wrapper for rsa, ifdef just to be cool and keep structure, needs to be
 * looked at in detail
 */

static psa_status_t rsa_sign(const uint8_t *key, size_t key_length, psa_algorithm_t alg,
			     const uint8_t *input, size_t input_length, uint8_t *signature,
			     size_t signature_size, size_t *signature_length, uint8_t is_input_hash)
{

	cc3xx_err_t err;
	psa_status_t status = PSA_SUCCESS;
	uint32_t encoded_msg_size = PSA_MAX_RSA_KEY_BYTES;

	psa_algorithm_t hash_alg;
	hash_alg = PSA_ALG_SIGN_GET_HASH(alg);
	assert(PSA_ALG_IS_HASH(hash_alg));

	printf("RSA SIGN\n");
	printf("--------------------HASHING--------------------\n");

    const size_t hash_length = PSA_HASH_LENGTH(hash_alg);
    uint32_t hash[CEIL_ALLOC_SZ(hash_length, sizeof(uint32_t))];
	
    
    if (!is_input_hash) {
		// assert(PSA_ALG_IS_HASH(hash_alg) && hash_alg != PSA_ALG_ANY_HASH);
		/* Compute the size of the local buffer to hold the hash, aligned to uint32_t */

		size_t hash_size;
		status = cc3xx_hash_compute(hash_alg, input, input_length, (uint8_t *)hash,
					    sizeof(hash), &hash_size);
	}else{
        memcpy(hash, (uint32_t*)input, hash_length); // Seems sketchy, is this allowed? 
    }
	// uint32_t key_buf[64];
	//  Initialize the engine
	//  TODO: Check lenght of key and init accordingly

	uint8_t encoded_msg[256] = {0};
	if (status != PSA_SUCCESS) {
		return status;
	}
	printf("--------------------ENCODING-------------------\n");
	err = cc3xx_lowlevel_rsa_pkcs1v15_encode(
		(uint8_t *)hash,	       /* Message */
		hash_length, encoded_msg_size, /* Intended length of the encoded msg*/
		hash_alg, encoded_msg);
	printf("--------------------SIGNING--------------------\n");

	err = cc3xx_lowlevel_rsa_sign(key,			 /* Priv key id */
				      encoded_msg,		 /* DataIn to encrypt */
				      256, (uint32_t *)signature /* Buffer for data out */
	);
	// Not random at all, this this the lenght of the
	*signature_length = encoded_msg_size;
	return status;
}

#endif

#if defined(PSA_WANT_ALG_ECDSA) && defined(PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC)
/**
 * @brief Wrapper function around the lowlevel ECDSA signing API that takes care
 *        of input/output formatting and hashing if required
 *
 * @param[in]  curve_id      Curve ID as defined by the CC3XX lowlevel driver
 * @param[in]  key           Buffer containing the private key
 * @param[in]  key_length    Size in bytes of the private key
 * @param[in]  input         Buffer containing the input, either a hash or message
 * @param[in]  input_length  Size in bytes of the input
 * @param[out] sig           Buffer containing the generated signature (r,s)
 * @param[in]  sig_length    Size in bytes of the \a sig buffer
 * @param[out] sig_size      Size in bytes of the generated signature in \a sig
 * @param[in]  is_input_hash Boolean indicating if the input is an hash
 * @param[in]  hash_alg      If \a is_input_hash is not true, then hashing will be performed
 *
 * @return psa_status_t
 */
static psa_status_t ecdsa_sign(cc3xx_ec_curve_id_t curve_id, const uint8_t *key, size_t key_length,
			       const uint8_t *input, size_t input_length, uint8_t *sig,
			       size_t sig_length, size_t *sig_size, bool is_input_hash,
			       psa_algorithm_t hash_alg)
{
	cc3xx_err_t err;
	psa_status_t status;

	if (!is_input_hash) {
		assert(PSA_ALG_IS_HASH(hash_alg) && hash_alg != PSA_ALG_ANY_HASH);
	}

	const size_t modulus_sz = cc3xx_lowlevel_ec_get_modulus_size_from_curve(curve_id);

	uint32_t scratch_r[modulus_sz / sizeof(uint32_t)];
	uint32_t scratch_s[modulus_sz / sizeof(uint32_t)];
	size_t sig_r_sz, sig_s_sz;
	uint32_t key_buf[CEIL_ALLOC_SZ(key_length, sizeof(uint32_t))];
	/* Compute the size of the local buffer to hold the hash, aligned to uint32_t */
	const size_t hash_length = (is_input_hash) ? input_length : PSA_HASH_LENGTH(hash_alg);
	uint32_t hash[CEIL_ALLOC_SZ(hash_length, sizeof(uint32_t))];
	size_t hash_size;

	if (is_input_hash) {
		memcpy(hash, input, input_length);
		hash_size = input_length;
	} else {
		status = cc3xx_hash_compute(hash_alg, input, input_length, (uint8_t *)hash,
					    sizeof(hash), &hash_size);
		if (status != PSA_SUCCESS) {
			return status;
		}
	}

	cc3xx_dpa_hardened_word_copy(key_buf, (uint32_t *)key, key_length / sizeof(uint32_t));

	err = cc3xx_lowlevel_ecdsa_sign(curve_id, key_buf, key_length, hash, hash_size, scratch_r,
					sizeof(scratch_r), &sig_r_sz, scratch_s, sizeof(scratch_s),
					&sig_s_sz);

	cc3xx_secure_erase_buffer(key_buf, sizeof(key_buf) / sizeof(uint32_t));

	if (err != CC3XX_ERR_SUCCESS) {
		return cc3xx_to_psa_err(err);
	}

	/* Copy the result in the correct output buffer */
	memcpy(sig, scratch_r, sig_r_sz);
	memcpy(&sig[sig_r_sz], scratch_s, sig_s_sz);
	*sig_size = sig_r_sz + sig_s_sz;

	return PSA_SUCCESS;
}
#endif /* PSA_WANT_ALG_ECDSA && PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC */

#if defined(PSA_WANT_ALG_ECDSA) && defined(PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY)
/**
 * @brief Wrapper function around the lowlevel ECDSA verification API that takes care
 *        of input/output formatting and hashing if required
 *
 * @param[in] attributes     Attributes of the key, required in case of public key export
 * @param[in] curve_id       Curve ID as defined by the CC3XX lowlevel driver
 * @param[in] key            Buffer containing the private or public key
 * @param[in] key_length     Size in bytes of the key buffer
 * @param[in] is_key_private Boolean indicating if the key is private
 * @param[in] input          Buffer containing the input, either a hash or message
 * @param[in] input_length   Size in bytes of the input
 * @param[in] sig            Buffer containing the signature to be verified (r,s)
 * @param[in] sig_length     Size in bytes of the \a sig buffer
 * @param[in] is_input_hash  Boolean indicating if the input is an hash
 * @param[in] hash_alg       If \a is_input_hash is not true, then hashing will be performed
 *
 * @return psa_status_t
 */
static psa_status_t ecdsa_verify(const psa_key_attributes_t *attributes,
				 cc3xx_ec_curve_id_t curve_id, const uint8_t *key,
				 size_t key_length, bool is_key_private, const uint8_t *input,
				 size_t input_length, const uint8_t *sig, size_t sig_length,
				 bool is_input_hash, psa_algorithm_t hash_alg)
{
	cc3xx_err_t err;
	psa_status_t status;

#if !defined(PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC)
	assert(is_key_private == false);
#endif

	if (!is_input_hash) {
		assert(PSA_ALG_IS_HASH(hash_alg) && hash_alg != PSA_ALG_ANY_HASH);
	}

	const size_t modulus_sz = cc3xx_lowlevel_ec_get_modulus_size_from_curve(curve_id);

	uint32_t sig_r[modulus_sz / sizeof(uint32_t)];
	uint32_t sig_s[modulus_sz / sizeof(uint32_t)];

	memcpy(sig_r, sig, sig_length / 2);
	memcpy(sig_s, &sig[sig_length / 2], sig_length / 2);

	/* Compute the size of the local buffer to hold the hash, aligned to uint32_t */
	const size_t hash_length = (is_input_hash) ? input_length : PSA_HASH_LENGTH(hash_alg);
	uint32_t hash[CEIL_ALLOC_SZ(hash_length, sizeof(uint32_t))];
	size_t hash_size;

	if (is_input_hash) {
		memcpy(hash, input, input_length);
		hash_size = input_length;
	} else {
		status = cc3xx_hash_compute(hash_alg, input, input_length, (uint8_t *)hash,
					    sizeof(hash), &hash_size);
		if (status != PSA_SUCCESS) {
			return status;
		}
	}

	/* Public keys are in uncompressed format, i.e. 0x04 X Y */
#if !defined(PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC)
	const size_t modulus_size_in_bytes = (key_length - 1) / 2;
#else
	const size_t modulus_size_in_bytes = (is_key_private) ? key_length : (key_length - 1) / 2;
#endif
	uint32_t pubkey[1 + 2 * CEIL_ALLOC_SZ(modulus_size_in_bytes, sizeof(uint32_t))];
	uint32_t *key_x = &pubkey[1];
	uint32_t *key_y = &pubkey[1 + CEIL_ALLOC_SZ(modulus_size_in_bytes, sizeof(uint32_t))];

	const uint8_t *p_key;
	size_t key_length_public;
#if defined(PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC)
	if (is_key_private) {
		p_key = &((uint8_t *)pubkey)[3];
		/* In this case we need to extract the public key from the private first */
		psa_status_t status =
			cc3xx_export_public_key(attributes, key, key_length, (uint8_t *)p_key,
						sizeof(pubkey) - 3, &key_length_public);
		if (status != PSA_SUCCESS) {
			return status;
		}
	} else
#endif /* PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC */
	{
		/* Just copy the public key points in the aligned buffers */
		p_key = key;
		key_length_public = key_length;
	}

	memcpy(key_x, &p_key[1], (key_length_public - 1) / 2);
	memcpy(key_y, &p_key[1 + (key_length_public - 1) / 2], (key_length_public - 1) / 2);

	err = cc3xx_lowlevel_ecdsa_verify(curve_id, (const uint32_t *)key_x, modulus_size_in_bytes,
					  (const uint32_t *)key_y, modulus_size_in_bytes, hash,
					  hash_size, (const uint32_t *)sig_r, sig_length / 2,
					  (const uint32_t *)sig_s, sig_length / 2);

	return cc3xx_to_psa_err(err);
}
#endif /* PSA_WANT_ALG_ECDSA && PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY */

/** @defgroup psa_asym_sign PSA driver entry points for asymmetric sign/verify
 *
 *  Entry points for asymmetric message signing and signature verification as
 *  described by the PSA Cryptoprocessor Driver interface specification
 *
 *  @{
 */
psa_status_t cc3xx_sign_hash(const psa_key_attributes_t *attributes, const uint8_t *key,
			     size_t key_length, psa_algorithm_t alg, const uint8_t *hash,
			     size_t hash_length, uint8_t *signature, size_t signature_size,
			     size_t *signature_length)
{
	CC3XX_ASSERT(attributes != NULL);
	CC3XX_ASSERT(key != NULL);
	CC3XX_ASSERT(hash != NULL);
	CC3XX_ASSERT(signature != NULL);
	CC3XX_ASSERT(signature_length != NULL);

	psa_algorithm_t hash_alg = PSA_ALG_SIGN_GET_HASH(alg);

	psa_key_type_t key_type = psa_get_key_type(attributes);
	psa_key_bits_t key_bits = psa_get_key_bits(attributes);

	/* Initialise the return value to 0 */
	*signature_length = 0;

#if defined(PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC)
	if (PSA_KEY_TYPE_IS_ECC(key_type)) {

		if (!PSA_ALG_IS_HASH_AND_SIGN(alg) || PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(key_type)) {
			return PSA_ERROR_INVALID_ARGUMENT;
		}

		if (hash_alg != PSA_ALG_ANY_HASH) {
			/* We can check that the input length is compliant if the hash is specified
			 */
			if (hash_length != PSA_HASH_LENGTH(hash_alg)) {
				return PSA_ERROR_INVALID_ARGUMENT;
			}
		}

		if (PSA_ALG_ECDSA_IS_DETERMINISTIC(alg)) {
			/* The lowlevel driver does not implement Deterministic ECDSA RFC 6979
			 * because the algorithm to compute the value of K would not be hardenend
			 * against side channel attacks
			 */
			return PSA_ERROR_NOT_SUPPORTED;
		}

		/* Translate from PSA curve ID to CC3XX curve ID*/
		const cc3xx_ec_curve_id_t curve_id =
			cc3xx_to_curve_id(PSA_KEY_TYPE_ECC_GET_FAMILY(key_type), key_bits);

		if (CC3XX_IS_CURVE_ID_INVALID(curve_id)) {
			return PSA_ERROR_NOT_SUPPORTED;
		}

		return ecdsa_sign(curve_id, key, key_length, hash, hash_length, signature,
				  signature_size, signature_length, true, hash_alg);
	} else
#endif /* PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC */
#if defined(PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC)

		if (PSA_WANT_ALG_RSA_PKCS1V15_SIGN) {
		return rsa_sign(key, key_length, alg, hash, hash_length, signature, signature_size,
				signature_length, 1);
	} else {
		return PSA_ERROR_NOT_SUPPORTED;
	}

#endif /* PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC */
	{
		(void)hash_alg;
		(void)key_bits;
		(void)key_type;
		return PSA_ERROR_INVALID_ARGUMENT;
	}
}

psa_status_t cc3xx_verify_hash(const psa_key_attributes_t *attributes, const uint8_t *key,
			       size_t key_length, psa_algorithm_t alg, const uint8_t *hash,
			       size_t hash_length, const uint8_t *signature,
			       size_t signature_length)
{
	CC3XX_ASSERT(attributes != NULL);
	CC3XX_ASSERT(key != NULL);
	CC3XX_ASSERT(hash != NULL);
	CC3XX_ASSERT(signature != NULL);

	psa_algorithm_t hash_alg = PSA_ALG_SIGN_GET_HASH(alg);

	psa_key_type_t key_type = psa_get_key_type(attributes);
	psa_key_bits_t key_bits = psa_get_key_bits(attributes);

#if defined(PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC) || defined(PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY)
	if (PSA_KEY_TYPE_IS_ECC(key_type)) {

		if (!PSA_ALG_IS_HASH_AND_SIGN(alg)) {
			return PSA_ERROR_INVALID_ARGUMENT;
		}

		if (hash_alg != PSA_ALG_ANY_HASH) {
			/* We can check that the input length is compliant if the hash is specified
			 */
			if (hash_length != PSA_HASH_LENGTH(hash_alg)) {
				return PSA_ERROR_INVALID_ARGUMENT;
			}
		}

		/* Translate from PSA curve ID to CC3XX curve ID*/
		const cc3xx_ec_curve_id_t curve_id =
			cc3xx_to_curve_id(PSA_KEY_TYPE_ECC_GET_FAMILY(key_type), key_bits);

		if (CC3XX_IS_CURVE_ID_INVALID(curve_id)) {
			return PSA_ERROR_NOT_SUPPORTED;
		}

		return ecdsa_verify(attributes, curve_id, key, key_length,
				    !PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(key_type), hash, hash_length,
				    signature, signature_length, true, hash_alg);
	} else
#endif /* PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC || defined(PSA_WANT_KEY_TYPE_ECC_PUBLIC) */
#if defined(PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC) || defined(PSA_WANT_KEY_TYPE_RSA_PUBLIC_KEY)
		if (PSA_KEY_TYPE_IS_RSA(key_type)) {

		return PSA_ERROR_NOT_SUPPORTED;

	} else
#endif /* PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC || PSA_WANT_KEY_TYPE_RSA_PUBLIC_KEY */
	{
		(void)hash_alg;
		(void)key_bits;
		(void)key_type;
		return PSA_ERROR_INVALID_ARGUMENT;
	}
}

psa_status_t cc3xx_sign_message(const psa_key_attributes_t *attributes, const uint8_t *key,
				size_t key_length, psa_algorithm_t alg, const uint8_t *input,
				size_t input_length, uint8_t *signature, size_t signature_size,
				size_t *signature_length)
{
	CC3XX_ASSERT(attributes != NULL);
	CC3XX_ASSERT(key != NULL);
	CC3XX_ASSERT(input != NULL);
	CC3XX_ASSERT(signature != NULL);
	CC3XX_ASSERT(signature_length != NULL);

	psa_algorithm_t hash_alg = PSA_ALG_SIGN_GET_HASH(alg);

	psa_key_type_t key_type = psa_get_key_type(attributes);
	psa_key_bits_t key_bits = psa_get_key_bits(attributes);

	/* Initialise the return value to 0 */
	*signature_length = 0;

#if defined(PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC)
	if (PSA_KEY_TYPE_IS_ECC(key_type)) {

		if (!PSA_ALG_IS_HASH_AND_SIGN(alg) || PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(key_type) ||
		    hash_alg == PSA_ALG_ANY_HASH) {
			return PSA_ERROR_INVALID_ARGUMENT;
		}

		if (PSA_ALG_ECDSA_IS_DETERMINISTIC(alg)) {
			/* The lowlevel driver does not implement Deterministic ECDSA RFC 6979
			 * because the algorithm to compute the value of K would not be hardenend
			 * against side channel attacks
			 */
			return PSA_ERROR_NOT_SUPPORTED;
		}

		/* Translate from PSA curve ID to CC3XX curve ID*/
		const cc3xx_ec_curve_id_t curve_id =
			cc3xx_to_curve_id(PSA_KEY_TYPE_ECC_GET_FAMILY(key_type), key_bits);

		if (CC3XX_IS_CURVE_ID_INVALID(curve_id)) {
			return PSA_ERROR_NOT_SUPPORTED;
		}

		return ecdsa_sign(curve_id, key, key_length, input, input_length, signature,
				  signature_size, signature_length, false, hash_alg);
	} else
#endif /* PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC */
#if defined(PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC)
		if (PSA_KEY_TYPE_IS_RSA(key_type)) {
		/*Check somewhere if the algorithm is correct*/
		if (PSA_WANT_ALG_RSA_PKCS1V15_SIGN) {
			return rsa_sign(key, key_length, alg, input, input_length, signature,
					signature_size, signature_length, 0);
		} else {
			return PSA_ERROR_NOT_SUPPORTED;
		}

		return PSA_ERROR_NOT_SUPPORTED;
	} else
#endif /* PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC */
	{
		(void)key_type;
		(void)key_bits;
		(void)hash_alg;
		return PSA_ERROR_INVALID_ARGUMENT;
	}
}

psa_status_t cc3xx_verify_message(const psa_key_attributes_t *attributes, const uint8_t *key,
				  size_t key_length, psa_algorithm_t alg, const uint8_t *input,
				  size_t input_length, const uint8_t *signature,
				  size_t signature_length)
{
	CC3XX_ASSERT(attributes != NULL);
	CC3XX_ASSERT(key != NULL);
	CC3XX_ASSERT(input != NULL);
	CC3XX_ASSERT(signature != NULL);

	psa_algorithm_t hash_alg = PSA_ALG_SIGN_GET_HASH(alg);

	psa_key_type_t key_type = psa_get_key_type(attributes);
	psa_key_bits_t key_bits = psa_get_key_bits(attributes);

#if defined(PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC)
	if (PSA_KEY_TYPE_IS_ECC(key_type)) {

		if (!PSA_ALG_IS_HASH_AND_SIGN(alg) || hash_alg == PSA_ALG_ANY_HASH) {
			return PSA_ERROR_INVALID_ARGUMENT;
		}

		/* Translate from PSA curve ID to CC3XX curve ID*/
		const cc3xx_ec_curve_id_t curve_id =
			cc3xx_to_curve_id(PSA_KEY_TYPE_ECC_GET_FAMILY(key_type), key_bits);

		if (CC3XX_IS_CURVE_ID_INVALID(curve_id)) {
			return PSA_ERROR_NOT_SUPPORTED;
		}

		return ecdsa_verify(attributes, curve_id, key, key_length,
				    !PSA_KEY_TYPE_IS_ECC_PUBLIC_KEY(key_type), input, input_length,
				    signature, signature_length, false, hash_alg);
	} else
#endif /* PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC */
#if defined(PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC)
		if (PSA_KEY_TYPE_IS_RSA(key_type)) {

		return PSA_ERROR_NOT_SUPPORTED;

	} else
#endif /* PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC */
	{
		(void)key_type;
		(void)key_bits;
		(void)hash_alg;
		return PSA_ERROR_INVALID_ARGUMENT;
	}
}
/** @} */ // end of psa_asym_sign
