// #ifdef USE_PSA_CRYPTOCELL
// #include "psa/crypto.h"
// #include "psa_crypto_driver_wrappers.h"
// #endif


#include <stdint.h>
#include <stddef.h>
#include <cc3xx_pka.h>
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