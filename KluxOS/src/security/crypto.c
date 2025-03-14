#include <security/crypto.h>
#include <kernel/types.h>
#include <drivers/terminal.h>
#include "../kernel/mm/memory.h"

#define MAX_CRYPTO_KEYS 16

static crypto_key_t* key_store[MAX_CRYPTO_KEYS];
static uint32_t key_count = 0;
static uint32_t next_key_id = 1;

void crypto_init(void) {
    terminal_writestring("crypto init started\n");
    
    for (int i = 0; i < MAX_CRYPTO_KEYS; i++) {
        key_store[i] = NULL;
    }
    key_count = 0;
    
    terminal_writestring("crypto init completed\n");
}

static int crypto_xor_process(const void* input, void* output, size_t length, const uint8_t* key, size_t key_length) {
    const uint8_t* in = (const uint8_t*)input;
    uint8_t* out = (uint8_t*)output;
    
    for (size_t i = 0; i < length; i++) {
        out[i] = in[i] ^ key[i % key_length];
    }
    
    return 0;
}

//shitty encryption
static int crypto_caesar_encrypt(const void* input, void* output, size_t length, const uint8_t* key, size_t key_length) {
    const uint8_t* in = (const uint8_t*)input;
    uint8_t* out = (uint8_t*)output;
    uint8_t shift = key[0];
    
    for (size_t i = 0; i < length; i++) {
        out[i] = in[i] + shift;
    }
    
    return 0;
}

//shitty encryption
static int crypto_caesar_decrypt(const void* input, void* output, size_t length, const uint8_t* key, size_t key_length) {
    const uint8_t* in = (const uint8_t*)input;
    uint8_t* out = (uint8_t*)output;
    uint8_t shift = key[0];
    
    for (size_t i = 0; i < length; i++) {
        out[i] = in[i] - shift;
    }
    
    return 0;
}

int crypto_encrypt(crypto_context_t* ctx, const void* input, void* output, size_t length) {
    if (!ctx || !ctx->key || !input || !output) {
        return -1;
    }
    
    switch (ctx->algorithm) {
        case CRYPTO_XOR:
            return crypto_xor_process(input, output, length, ctx->key->key_data, ctx->key->length);
            
        case CRYPTO_CAESAR:
            return crypto_caesar_encrypt(input, output, length, ctx->key->key_data, ctx->key->length);
            
        case CRYPTO_AES:
        case CRYPTO_RSA:
            // İleride eklenecek
            terminal_writestring("ERROR: AES and RSA not supported yet.\n");
            return -2;
            
        default:
            return -3;
    }
}

int crypto_decrypt(crypto_context_t* ctx, const void* input, void* output, size_t length) {
    if (!ctx || !ctx->key || !input || !output) {
        return -1;
    }
    
    switch (ctx->algorithm) {
        case CRYPTO_XOR:
            return crypto_xor_process(input, output, length, ctx->key->key_data, ctx->key->length);
            
        case CRYPTO_CAESAR:
            return crypto_caesar_decrypt(input, output, length, ctx->key->key_data, ctx->key->length);
            
        case CRYPTO_AES:
        case CRYPTO_RSA:
            // İleride eklenecek
            terminal_writestring("ERROR: AES and RSA not supported yet.\n");
            return -2;
            
        default:
            return -3;
    }
}

static int crypto_simple_hash(const void* input, size_t length, crypto_hash_t* hash) {
    if (!input || !hash) {
        return -1;
    }
    
    const uint8_t* data = (const uint8_t*)input;
    
    for (int i = 0; i < 32; i++) {
        hash->data[i] = 0;
    }
    
    for (size_t i = 0; i < length; i++) {
        hash->data[i % 32] ^= data[i];
        
        if (i % 8 == 7) {
            for (int j = 0; j < 31; j++) {
                hash->data[j] = ((hash->data[j] << 1) | (hash->data[j+1] >> 7)) & 0xFF;
            }
            hash->data[31] = ((hash->data[31] << 1) | (hash->data[0] >> 7)) & 0xFF;
        }
    }
    
    return 0;
}

int crypto_hash(crypto_algorithm_t algorithm, const void* input, size_t length, crypto_hash_t* hash) {
    if (!input || !hash) {
        return -1;
    }
    
    switch (algorithm) {
        case CRYPTO_SHA:
            // İleride eklenecek
            terminal_writestring("WARNING: SHA not supported yet, using simple hash.\n");
            return crypto_simple_hash(input, length, hash);
            
        default:
            return crypto_simple_hash(input, length, hash);
    }
}

crypto_key_t* crypto_generate_key(crypto_algorithm_t algorithm, uint32_t key_length) {
    if (algorithm == CRYPTO_NONE || 
        (algorithm >= CRYPTO_AES && algorithm != CRYPTO_SHA)) {
        return NULL;
    }
    
    if (algorithm == CRYPTO_XOR) {
        if (key_length < 4) key_length = 4;
        if (key_length > 32) key_length = 32;
    }
    
    if (algorithm == CRYPTO_CAESAR) {
        key_length = 1;
    }
    
    crypto_key_t* key = (crypto_key_t*)kmalloc(sizeof(crypto_key_t));
    if (!key) return NULL;
    
    key->key_data = (uint8_t*)kmalloc(key_length);
    if (!key->key_data) {
        kfree(key);
        return NULL;
    }
    
    key->id = next_key_id++;
    key->type = algorithm;
    key->length = key_length;
    
    for (uint32_t i = 0; i < key_length; i++) {
        key->key_data[i] = (uint8_t)((i * 29 + key->id * 13) % 256);
    }
    
    if (key_count < MAX_CRYPTO_KEYS) {
        key_store[key_count++] = key;
    }
    
    return key;
}

void crypto_destroy_key(crypto_key_t* key) {
    if (!key) return;
    
    for (uint32_t i = 0; i < key_count; i++) {
        if (key_store[i] == key) {
            kfree(key->key_data);
            kfree(key);
            
            key_store[i] = key_store[--key_count];
            key_store[key_count] = NULL;
            return;
        }
    }
    
    if (key->key_data) {
        kfree(key->key_data);
    }
    kfree(key);
}

crypto_context_t* crypto_create_context(crypto_algorithm_t algorithm, crypto_key_t* key) {
    if (!key || key->type != algorithm) {
        return NULL;
    }
    
    crypto_context_t* ctx = (crypto_context_t*)kmalloc(sizeof(crypto_context_t));
    if (!ctx) return NULL;
    
    ctx->algorithm = algorithm;
    ctx->key = key;
    ctx->context_data = NULL; 
    
    return ctx;
}

void crypto_destroy_context(crypto_context_t* ctx) {
    if (!ctx) return;
    
    if (ctx->context_data) {
        kfree(ctx->context_data);
    }
    
    kfree(ctx);
}
