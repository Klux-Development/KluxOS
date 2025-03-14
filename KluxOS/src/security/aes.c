#include "../security/aes.h"
#include <kernel/types.h>
#include <drivers/terminal.h>
#include "../kernel/mm/memory.h"

// AES S-box
static const uint8_t sbox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

// Ters AES S-box (şifre çözme için)
static const uint8_t rsbox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

static const uint8_t rcon[10] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

static uint8_t xtime(uint8_t x) {
    return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
}

static void sub_bytes(uint8_t* state) {
    for (int i = 0; i < 16; i++) {
        state[i] = sbox[state[i]];
    }
}

static void inv_sub_bytes(uint8_t* state) {
    for (int i = 0; i < 16; i++) {
        state[i] = rsbox[state[i]];
    }
}

static void shift_rows(uint8_t* state) {
    uint8_t temp;
    
    temp = state[1];
    state[1] = state[5];
    state[5] = state[9];
    state[9] = state[13];
    state[13] = temp;
    
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    
    temp = state[15];
    state[15] = state[11];
    state[11] = state[7];
    state[7] = state[3];
    state[3] = temp;
}

static void inv_shift_rows(uint8_t* state) {
    uint8_t temp;
    
    temp = state[13];
    state[13] = state[9];
    state[9] = state[5];
    state[5] = state[1];
    state[1] = temp;
    
    temp = state[2];
    state[2] = state[10];
    state[10] = temp;
    temp = state[6];
    state[6] = state[14];
    state[14] = temp;
    
    temp = state[3];
    state[3] = state[7];
    state[7] = state[11];
    state[11] = state[15];
    state[15] = temp;
}

static void mix_columns(uint8_t* state) {
    uint8_t a, b, c, d, temp[4];
    
    for (int i = 0; i < 4; i++) {
        a = state[i * 4];
        b = state[i * 4 + 1];
        c = state[i * 4 + 2];
        d = state[i * 4 + 3];
        
        temp[0] = (uint8_t)(xtime(a) ^ xtime(b) ^ b ^ c ^ d);
        temp[1] = (uint8_t)(a ^ xtime(b) ^ xtime(c) ^ c ^ d);
        temp[2] = (uint8_t)(a ^ b ^ xtime(c) ^ xtime(d) ^ d);
        temp[3] = (uint8_t)(xtime(a) ^ a ^ b ^ c ^ xtime(d));
        
        state[i * 4] = temp[0];
        state[i * 4 + 1] = temp[1];
        state[i * 4 + 2] = temp[2];
        state[i * 4 + 3] = temp[3];
    }
}

static void inv_mix_columns(uint8_t* state) {
    uint8_t a, b, c, d, e, f, g, h, temp[4];
    
    for (int i = 0; i < 4; i++) {
        a = state[i * 4];
        b = state[i * 4 + 1];
        c = state[i * 4 + 2];
        d = state[i * 4 + 3];
        
        e = xtime(a);
        f = xtime(b);
        g = xtime(c);
        h = xtime(d);
        
        uint8_t ea = xtime(e);
        uint8_t fb = xtime(f);
        uint8_t gc = xtime(g);
        uint8_t hd = xtime(h);
        
        uint8_t eea = xtime(ea);
        uint8_t ffb = xtime(fb);
        uint8_t ggc = xtime(gc);
        uint8_t hhd = xtime(hd);
        
        temp[0] = (uint8_t)(eea ^ eea ^ a ^ ffb ^ b ^ gc ^ c ^ hd ^ d);
        temp[1] = (uint8_t)(ea ^ a ^ ffb ^ ffb ^ b ^ ggc ^ c ^ hd ^ d);
        temp[2] = (uint8_t)(ea ^ a ^ fb ^ b ^ ggc ^ ggc ^ c ^ hhd ^ d);
        temp[3] = (uint8_t)(eea ^ a ^ fb ^ b ^ gc ^ c ^ hhd ^ hhd ^ d);
        
        state[i * 4] = temp[0];
        state[i * 4 + 1] = temp[1];
        state[i * 4 + 2] = temp[2];
        state[i * 4 + 3] = temp[3];
    }
}

static void add_round_key(uint8_t* state, const uint8_t* round_key) {
    for (int i = 0; i < 16; i++) {
        state[i] ^= round_key[i];
    }
}

static void expand_key(const uint8_t* key, uint8_t* expanded_key, int key_size, int round_count) {
    int i, j, k;
    uint8_t temp[4], t;
    
    for (i = 0; i < key_size; i++) {
        expanded_key[i] = key[i];
    }
    
    i = key_size;
    
    while (i < 16 * (round_count + 1)) {
        for (j = 0; j < 4; j++) {
            temp[j] = expanded_key[i - 4 + j];
        }
        
        if (i % key_size == 0) {
            t = temp[0];
            temp[0] = temp[1];
            temp[1] = temp[2];
            temp[2] = temp[3];
            temp[3] = t;
            
            for (j = 0; j < 4; j++) {
                temp[j] = sbox[temp[j]];
            }
            
            temp[0] ^= rcon[i / key_size - 1];
        } 
        else if (key_size > 24 && i % key_size == 16) {
            for (j = 0; j < 4; j++) {
                temp[j] = sbox[temp[j]];
            }
        }
        
        for (j = 0; j < 4; j++) {
            expanded_key[i + j] = expanded_key[i - key_size + j] ^ temp[j];
        }
        
        i += 4;
    }
}

// AES başlatma (anahtar genişletme)
int aes_init(aes_context_t* ctx, const uint8_t* key, aes_key_size_t key_size) {
    if (!ctx || !key) {
        return -1;
    }
    
    int rounds;
    
    switch (key_size) {
        case AES_128:
            rounds = 10;
            break;
        case AES_192:
            rounds = 12;
            break;
        case AES_256:
            rounds = 14;
            break;
        default:
            return -2;
    }
    
    ctx->rounds = rounds;
    
    expand_key(key, ctx->expanded_key, key_size, rounds);
    
    return 0;
}

void aes_encrypt_block(const aes_context_t* ctx, const uint8_t* input, uint8_t* output) {
    uint8_t state[16];
    
    for (int i = 0; i < 16; i++) {
        state[i] = input[i];
    }
    
    add_round_key(state, ctx->expanded_key);
    
    for (int round = 1; round < ctx->rounds; round++) {
        sub_bytes(state);
        shift_rows(state);
        mix_columns(state);
        add_round_key(state, ctx->expanded_key + round * 16);
    }
    
    sub_bytes(state);
    shift_rows(state);
    add_round_key(state, ctx->expanded_key + ctx->rounds * 16);
    
    for (int i = 0; i < 16; i++) {
        output[i] = state[i];
    }
}

void aes_decrypt_block(const aes_context_t* ctx, const uint8_t* input, uint8_t* output) {
    uint8_t state[16];
    
    for (int i = 0; i < 16; i++) {
        state[i] = input[i];
    }
    
    add_round_key(state, ctx->expanded_key + ctx->rounds * 16);
    

    for (int round = ctx->rounds - 1; round > 0; round--) {
        inv_shift_rows(state);
        inv_sub_bytes(state);
        add_round_key(state, ctx->expanded_key + round * 16);
        inv_mix_columns(state);
    }
    
    inv_shift_rows(state);
    inv_sub_bytes(state);
    add_round_key(state, ctx->expanded_key);
    
    for (int i = 0; i < 16; i++) {
        output[i] = state[i];
    }
}


int aes_encrypt_cbc(const aes_context_t* ctx, const uint8_t* iv, 
                    const uint8_t* input, uint8_t* output, size_t length) {
    if (!ctx || !iv || !input || !output) {
        return -1;
    }
    
    if (length % AES_BLOCK_SIZE != 0) {
        return -2;
    }
    
    uint8_t iv_copy[AES_BLOCK_SIZE];
    uint8_t block[AES_BLOCK_SIZE];
    
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        iv_copy[i] = iv[i];
    }
    
    for (size_t i = 0; i < length; i += AES_BLOCK_SIZE) {
        for (int j = 0; j < AES_BLOCK_SIZE; j++) {
            block[j] = input[i + j] ^ iv_copy[j];
        }
        
        aes_encrypt_block(ctx, block, &output[i]);
        
        for (int j = 0; j < AES_BLOCK_SIZE; j++) {
            iv_copy[j] = output[i + j];
        }
    }
    
    return 0;
}


int aes_decrypt_cbc(const aes_context_t* ctx, const uint8_t* iv, 
                    const uint8_t* input, uint8_t* output, size_t length) {
    if (!ctx || !iv || !input || !output) {
        return -1;
    }
    
    if (length % AES_BLOCK_SIZE != 0) {
        return -2;
    }
    
    uint8_t iv_copy[AES_BLOCK_SIZE];
    uint8_t temp[AES_BLOCK_SIZE];
    
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        iv_copy[i] = iv[i];
    }
    
    for (size_t i = 0; i < length; i += AES_BLOCK_SIZE) {
        aes_decrypt_block(ctx, &input[i], temp);
        
        for (int j = 0; j < AES_BLOCK_SIZE; j++) {
            output[i + j] = temp[j] ^ iv_copy[j];
        }
        
        for (int j = 0; j < AES_BLOCK_SIZE; j++) {
            iv_copy[j] = input[i + j];
        }
    }
    
    return 0;
}
