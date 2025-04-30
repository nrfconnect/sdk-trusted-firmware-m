// #ifdef USE_PSA_CRYPTOCELL
// #include "psa/crypto.h"
// #include "psa_crypto_driver_wrappers.h"
// #endif


#include <stdint.h>
#include <stddef.h>
#include <cc3xx_pka.h>

/*Should I just throw an error or ignore this? If this is not defined
bigger problems probably exists*/
#ifndef PSA_MAX_RSA_KEY_BITS
    #define PSA_MAX_RSA_KEY_BITS 2048
#endif

/*There has to exist macro for this "8" --can use "#define PKA_WORD_SIZE  8"
but that's defined in a cc3xx_pka.c file? Should I move it to header file?*/
#define PSA_MAX_RSA_KEY_BYTES PSA_MAX_RSA_KEY_BITS/8
#define PSA_MAX_RSA_KEY_WORDS PSA_MAX_RSA_KEY_BYTES/sizeof(uint32_t)

cc3xx_err_t  cc3xx_lowlevel_rsa_pkcs1v15_encode(
    const uint8_t *input, /* Message   */
    size_t input_size, /* Lenght of the message to encode inn octets */
    uint32_t  encoded_msg_length, /* Intended length of the encoded msg*/
    psa_algorithm_t hash_alg, /*The hash alh used*/
    uint8_t *output_buf /* Pointer to the output buffer */

);

cc3xx_err_t  cc3xx_lowlevel_rsa_sign(
    const uint8_t *key,  /* Priv key id */
    const uint8_t *input, /* DataIn to encrypt */
    size_t input_size,
    uint32_t *signature /* Buffer for data out */
    );