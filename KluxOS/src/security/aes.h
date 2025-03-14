#ifndef AES_H
#define AES_H

#include <kernel/types.h>

#define AES_BLOCK_SIZE 16


typedef enum {
    AES_128 = 16,  // 128 bit / 16 bayt
    AES_192 = 24,  // 192 bit / 24 bayt
    AES_256 = 32   // 256 bit / 32 bayt
} aes_key_size_t;


typedef struct {
    uint8_t expanded_key[240];
    int rounds;
} aes_context_t;


int aes_init(aes_context_t* ctx, const uint8_t* key, aes_key_size_t key_size);


void aes_encrypt_block(const aes_context_t* ctx, const uint8_t* input, uint8_t* output);


void aes_decrypt_block(const aes_context_t* ctx, const uint8_t* input, uint8_t* output);


int aes_encrypt_cbc(const aes_context_t* ctx, const uint8_t* iv, 
                    const uint8_t* input, uint8_t* output, size_t length);


int aes_decrypt_cbc(const aes_context_t* ctx, const uint8_t* iv, 
                    const uint8_t* input, uint8_t* output, size_t length);

#endif // AES_H
