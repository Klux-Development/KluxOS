#ifndef _STRING_H
#define _STRING_H

#include <kernel/types.h>

/* C standart kütüphane string fonksiyonları */

// Belleği doldur
void* memset(void* s, int c, size_t n);

// Belleği kopyala
void* memcpy(void* dest, const void* src, size_t n);

// Belleği taşı (örtüşen bölgeler için güvenli)
void* memmove(void* dest, const void* src, size_t n);

// Bellek karşılaştır
int memcmp(const void* s1, const void* s2, size_t n);

// String uzunluğu
size_t strlen(const char* s);

// String kopyala
char* strcpy(char* dest, const char* src);

// String karşılaştır
int strcmp(const char* s1, const char* s2);

// String birleştir
char* strcat(char* dest, const char* src);

// String içinde karakter ara
char* strchr(const char* s, int c);

// String içindeki karakterleri ara
char* strpbrk(const char* s, const char* accept);

// String içinde alt dizi ara
char* strstr(const char* haystack, const char* needle);

#endif /* _STRING_H */ 