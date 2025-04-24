/*
 * Probably a lot of defines and ifdefs needed in order to run anything
 */

/* #ifdef USE_PSA_CRYPTOCELL */ 
#include "psa/crypto.h"
#include "psa_crypto_driver_wrappers.h"
/*  #endif  */
#include "cc3xx_rsa.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h> // To be removed when I find correct error handling for printf

#define MODSIZE_IN_BYTES 256
#define PRIV_EXP_IN_BYTES 256


typedef enum {
    VERSION = 0,
    MODULUS,
    PUBLIC_EXPONENT,
    PRIVATE_EXPONENT,
    PRIME1,
    PRIME2,
    EXPONENT1,
    EXPONENT2,
    COEFFICIENT,
    
    OTHER
} rsa_key_compotents ;
#define HASH_DER_CODE_MAX_SIZE_BYTES 20
typedef struct HashDerCode_t {
    uint32_t algIdSizeBytes;
    psa_algorithm_t hashMode;
    uint8_t algId[HASH_DER_CODE_MAX_SIZE_BYTES];
}HashDerCode_t;


static const HashDerCode_t gHashDerCodes[] = {
    {15,PSA_ALG_SHA_1 ,      {0x30,0x21,0x30,0x09,0x06,0x05,0x2b,0x0e,0x03,0x02,0x1a,0x05,0x00,0x04,0x14}},                     /*SHA1*/
    {19,PSA_ALG_SHA_224,     {0x30,0x2D,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x04,0x05,0x00,0x04,0x1C}}, /*SHA224*/
    {19,PSA_ALG_SHA_256 ,    {0x30,0x31,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x01,0x05,0x00,0x04,0x20}}, /*SHA256*/
    {19,PSA_ALG_SHA_384,     {0x30,0x41,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x02,0x05,0x00,0x04,0x30}}, /*SHA384*/
    {19,PSA_ALG_SHA_512 ,    {0x30,0x51,0x30,0x0D,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x03,0x05,0x00,0x04,0x40}}, /*SHA512*/
    {18,PSA_ALG_MD5 ,        {0x30,0x20,0x30,0x0c,0x06,0x08,0x2a,0x86,0x48,0x86,0xf7,0x0d,0x02,0x05,0x05,0x00,0x04,0x10}},      /*MD5*/
    {19,PSA_ALG_SHA_512_224, {0x30,0x2d,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x05,0x05,0x00,0x04,0x1c}}, /*SHA512/224*/
    {19,PSA_ALG_SHA_512_256, {0x30,0x31,0x30,0x0d,0x06,0x09,0x60,0x86,0x48,0x01,0x65,0x03,0x04,0x02,0x06,0x05,0x00,0x04,0x20}}, /*SHA512/256*/



};
int cc3xx_get_rsa_key_component(const uint8_t *key_buffer, uint32_t *output_buf, rsa_key_compotents component);

// Swap  in a buffer
cc3xx_err_t cc3xx_word_swap(uint32_t *input_buf, uint32_t *output_buf, uint8_t buf_size);

cc3xx_err_t cc3xx_get_der_encoding(psa_algorithm_t hash_alg, uint8_t **pHashAlgId, uint32_t *hashAlgIdSize );

cc3xx_err_t cc3xx_get_der_encoding(psa_algorithm_t hash_alg, uint8_t **pHashAlgId, uint32_t *hashAlgIdSize ){

    switch (hash_alg)
    {
    case PSA_ALG_SHA_1:
        *pHashAlgId = gHashDerCodes[0].algId;
        *hashAlgIdSize = gHashDerCodes[0].algIdSizeBytes;
        return CC3XX_ERR_SUCCESS;
    case PSA_ALG_SHA_224: 
        *pHashAlgId = gHashDerCodes[1].algId;
        *hashAlgIdSize = gHashDerCodes[1].algIdSizeBytes;
        return CC3XX_ERR_SUCCESS;
    case PSA_ALG_SHA_256:
        *pHashAlgId = gHashDerCodes[2].algId;
        *hashAlgIdSize = gHashDerCodes[2].algIdSizeBytes;
        return CC3XX_ERR_SUCCESS;
    case PSA_ALG_SHA_384: 
        *pHashAlgId = gHashDerCodes[3].algId;
        *hashAlgIdSize = gHashDerCodes[3].algIdSizeBytes;
        return CC3XX_ERR_SUCCESS;
    case PSA_ALG_SHA_512: 
        *pHashAlgId = gHashDerCodes[4].algId;
        *hashAlgIdSize = gHashDerCodes[4].algIdSizeBytes;
        return CC3XX_ERR_SUCCESS;
    case PSA_ALG_MD5: 
        *pHashAlgId = gHashDerCodes[5].algId;
        *hashAlgIdSize = gHashDerCodes[5].algIdSizeBytes;
        return CC3XX_ERR_SUCCESS;
    case PSA_ALG_SHA_512_224:
        *pHashAlgId = gHashDerCodes[6].algId;
        *hashAlgIdSize = gHashDerCodes[6].algIdSizeBytes;
        return CC3XX_ERR_SUCCESS;
    case PSA_ALG_SHA_512_256: 
        *pHashAlgId = gHashDerCodes[7].algId;
        *hashAlgIdSize = gHashDerCodes[7].algIdSizeBytes;
        return CC3XX_ERR_SUCCESS;
    default:
        return CC3XX_ERR_INVALID_ALGORITHM;
    }
}


cc3xx_err_t  cc3xx_lowlevel_rsa_pkcs1v15_encode(
    const uint8_t *input, /* Message */
    size_t input_size, /* Lenght of the message to encode inn octets */
    uint32_t  encoded_msg_length, /* Intended length of the encoded msg*/
    psa_algorithm_t hash_alg,
    uint8_t *output_buf /* Pointer to the output buffer */

)
{
    /*----------------IGNORING HASHING--------------*/

    /* Error to success, ask if this is something that is supposed to be one */
    cc3xx_err_t err = CC3XX_ERR_SUCCESS;
    /* Assume RAW MODE, remember to edit and add guards later
     * Check if intended length in octets of the encoded message, at
               least tLen + 11, where tLen is the octet length of the
               Distinguished Encoding Rules (DER) encoding T of
               a certain value computed during the encoding operation (per def)
     */
     /* The pointer to Hash Alg. ID (DER code) and its size */



    int32_t  PSSize;
    if(encoded_msg_length < input_size + 11){ 
        return CC3XX_ERR_RSA_ENCODE_MSG_TOO_LONG;
    }
        /*---------------------------------------------------*/
        /*  Encryption block formating for EMSA-PKCS1-v1_5:  */
        /*          00 || 01 || PS || 00 || T            */
        /*         MSB                      LSB          */
        /* Note:  BT=02, PS=FF...FF, T=DER||Hash(M)          */
        /*---------------------------------------------------*/
    /* The pointer to Hash Alg. ID (DER code) and its size */
    uint8_t *pHashAlgId = NULL;
    uint32_t hashAlgIdSize = 0;
    // To be fixed, won't return correct value, dict would be perfect here, think of another implementation, maybe switch statement? 
    // hashAlgIdSize = gHashDerCodes[hash_alg].algIdSizeBytes;  // Returns size in bytes  
    // *pHashAlgId = gHashDerCodes[hash_alg].algId;
    err = cc3xx_get_der_encoding(hash_alg, &pHashAlgId, &hashAlgIdSize);

    if(err != CC3XX_ERR_SUCCESS){
        return err;
    }


    PSSize = encoded_msg_length - input_size - 3 - hashAlgIdSize; 
    output_buf[0] = 0x00;
    output_buf[1] = 0x01;
    memset(output_buf + 2, 0xFF, PSSize);
    output_buf[PSSize + 2] = 0x00;

    // If statement for later is_hash ignore as of now
    if(1){
        memcpy(&output_buf[PSSize + 3], pHashAlgId, hashAlgIdSize);
        memcpy(&output_buf[PSSize + 3 + hashAlgIdSize], input, input_size);
    }else {
        memcpy(&output_buf[PSSize + 3], input, input_size); 
    }

    return err;
} 

cc3xx_err_t  cc3xx_lowlevel_rsa_sign(
    const uint8_t *key,  /* Priv key id */
    const uint8_t *input, /* DataIn to encrypt */
    size_t input_size, /* Size of the input msg*/
    uint32_t *signature/* Buffer for data out */
    )
    {

    /* BEFORE ANYTHING ELSE remember to check if pointers are valid */
    cc3xx_err_t err;

    //psa_status_t status;
    
    /* ASSUME NON CTR---- CTR TO BE ADDED */
    /*s = m ^d mod n.*/
   cc3xx_pka_reg_id_t sig_reg;
   cc3xx_pka_reg_id_t input_reg;
   cc3xx_pka_reg_id_t mod_reg;
   cc3xx_pka_reg_id_t priv_exp_reg;
   cc3xx_pka_reg_id_t barrett_reg;
   /*REMEMBER TO INITIALIZE PKA*/
   
   /* Initializing registers */
   
   sig_reg = cc3xx_lowlevel_pka_allocate_reg();
   input_reg = cc3xx_lowlevel_pka_allocate_reg();
   mod_reg = cc3xx_lowlevel_pka_allocate_reg();
   priv_exp_reg = cc3xx_lowlevel_pka_allocate_reg();
   barrett_reg = cc3xx_lowlevel_pka_allocate_reg();
   
    uint32_t mod[64] = {0};
    uint32_t priv_exp[64] = {0};

    // Error handling needs to be added here
    err = cc3xx_get_rsa_key_component(key, priv_exp, PRIVATE_EXPONENT);
    err = cc3xx_get_rsa_key_component(key, mod, MODULUS);

    cc3xx_lowlevel_pka_write_reg_swap_endian(mod_reg, mod, MODSIZE_IN_BYTES);
    cc3xx_lowlevel_pka_write_reg_swap_endian(priv_exp_reg, priv_exp, PRIV_EXP_IN_BYTES);
    cc3xx_lowlevel_pka_write_reg_swap_endian(input_reg,(uint32_t*) input, input_size);

    /* It is necessary to set the barret tag true and allocate a register for it, otherwise,
    only the first byte will be correct, and the rest will be ?random? */
    
    cc3xx_lowlevel_pka_set_modulus(mod_reg, true, barrett_reg);
    
    memset(priv_exp, 0, 64*sizeof(uint32_t));
    cc3xx_lowlevel_pka_mod_exp(input_reg, priv_exp_reg, sig_reg);

    cc3xx_lowlevel_pka_read_reg_swap_endian(sig_reg, signature, input_size);

    return err;

}

#define PUBLIC_RSA_KEY_TYPE 1
#define PRIVATE_RSA_KEY_TYPE 2

/* This implementation as it stands is suboptimal, it only takes the standard format into consideration and will breake as soon
 * as someone adds custom fields and if there is a sequence at the beginning too.... 
 */

size_t cc3xx_get_val_length(uint8_t **key_buffer);

int cc3xx_get_rsa_key_component(const uint8_t *key_buffer, uint32_t *output_buf, rsa_key_compotents component){
    
    // TODO check that the lenght field indicates the correct length of the key? Otherwise something went wrong? 
    uint8_t *key_p = (uint8_t*)key_buffer; // Casts so I can manipulate the new ptr, (key_buffer is const)

    size_t val_length;
    key_p += 4; // For the sake of skipping the introducing bits
    // Now I iterate throught the data which won't be needed and should end up behind the correct 
    for (uint8_t i = 0; i < component; i++)
    {
        val_length = cc3xx_get_val_length(&key_p);
        // Remove or change, bad practice, since uint8 is returned so it can actidentally hit true?? Maybe? 
        if(val_length == -1){
            return -1;
        } 
        // TODO, needs to skip the length of the length field itself
        key_p += (val_length + 1);

        //Checks if the next field is an int and returns errors since we should only handle integers for now
        if(*key_p != 0x02){
            return -1; // Something something, then something other then int is met, need to be changed,(1 week went by and I almost did not understand my own comment)
        }
            }
    val_length = cc3xx_get_val_length(&key_p);
    // Ugly ugly, increasing the buffer past the length field itself
    // Need to skip 0x00 if that's the first byte. Also need a better impd obviously but let me check if this works       
    if(component == MODULUS){
        key_p ++;
        key_p ++;

    }else{

        key_p ++;
    }
    // Copy the value from the key_bufer to the output_buffer 
    // Maybe rather use cc3xx hardened copy function? 
    memcpy(output_buf, key_p, val_length);
    return 0;
};

size_t cc3xx_get_val_length(uint8_t **key_buffer){
    // Stolen from old driver, not quite sure if the syntax is valid
    size_t length;
    *key_buffer += 1;
    if ((**key_buffer & 0x80) == 0) {    
        return **key_buffer;
    }else{
        switch (**key_buffer & 0x7F) {
        case 1:
            *key_buffer += 1;
            return **key_buffer;    
        case 2:
            length = ((size_t)(*key_buffer)[1] << 8) | (*key_buffer)[2];
            *key_buffer += 2; 
            return length;  
        case 3:
            length = ((size_t)(*key_buffer)[1] << 16) | ((size_t)(*key_buffer)[2] << 8) | (*key_buffer)[3];
            *key_buffer += 3;
            return length;
        case 4: // Up to case 4 since the old driver did this in this way ... anything past 2 is insane so maybe just have 2 cases? 
            length = ((size_t)(*key_buffer)[1] << 24) | ((size_t)(*key_buffer)[2] << 16) |
                   ((size_t)(*key_buffer)[3] << 8) | (*key_buffer)[4];
            *key_buffer += 4;
            return length;
        default:
            return -1; //ADD some error code to let the func know it didn't find the lenght
        }
    }

}
