#ifndef CRYPTO_H
#define CRYPTO_H

#include <kernel/types.h>

// Şifreleme algoritma türleri
typedef enum {
    CRYPTO_NONE = 0,       // Şifreleme yok
    CRYPTO_XOR = 1,        // Basit XOR şifreleme
    CRYPTO_CAESAR = 2,     // Sezar şifreleme
    CRYPTO_AES = 3,        // AES şifreleme (ileride eklenecek)
    CRYPTO_RSA = 4,        // RSA şifreleme (ileride eklenecek)
    CRYPTO_SHA = 5         // SHA hash (ileride eklenecek)
} crypto_algorithm_t;

// Kriptografik anahtar
typedef struct {
    uint32_t id;                // Anahtar kimliği
    crypto_algorithm_t type;    // Şifreleme algoritması
    uint32_t length;            // Anahtar uzunluğu (bayt)
    uint8_t* key_data;          // Anahtar verisi
} crypto_key_t;

// Özet (hash) değeri
typedef struct {
    uint8_t data[32];           // 256-bit hash değeri
} crypto_hash_t;

// Kriptografik kontekst
typedef struct {
    crypto_algorithm_t algorithm;  // Kullanılan algoritma
    crypto_key_t* key;             // Kullanılan anahtar
    void* context_data;            // Algoritma-spesifik veri
} crypto_context_t;

// Şifreleme sistemi başlatma
void crypto_init(void);

// Şifreleme işlemi
int crypto_encrypt(crypto_context_t* ctx, const void* input, void* output, size_t length);

// Şifre çözme işlemi
int crypto_decrypt(crypto_context_t* ctx, const void* input, void* output, size_t length);

// Hash oluşturma
int crypto_hash(crypto_algorithm_t algorithm, const void* input, size_t length, crypto_hash_t* hash);

// Anahtar oluşturma
crypto_key_t* crypto_generate_key(crypto_algorithm_t algorithm, uint32_t key_length);

// Anahtar silme
void crypto_destroy_key(crypto_key_t* key);

// Kriptografik kontekst oluşturma
crypto_context_t* crypto_create_context(crypto_algorithm_t algorithm, crypto_key_t* key);

// Kriptografik kontekst silme
void crypto_destroy_context(crypto_context_t* ctx);

#endif // CRYPTO_H
