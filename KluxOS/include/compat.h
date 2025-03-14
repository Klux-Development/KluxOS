#ifndef COMPAT_H
#define COMPAT_H

/**
 * Derleyici uyumluluk makroları
 * Bu dosya farklı derleyiciler arasında kod taşınabilirliğini sağlar
 */

// Önce ana tip tanımlamaları
#include <kernel/types.h>  // bool ve size_t temel tipleri

// Ardından gerektiğinde libc uyumluluğu için header'lar
// stdbool.h türünü sağla - types.h'dan sonra dahil edilmeli
#include <libc/stdbool.h>

// İşletim sistemi platformunu algıla
#if defined(__linux__) || defined(__linux) || defined(linux)
    #define OS_LINUX
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #define OS_WINDOWS
#endif

// Derleyici türünü algıla - GCC zaten kendi tanımını yapıyor
#if defined(__GNUC__) || defined(__GNUG__)
    #define COMPILER_GCC
#elif defined(_MSC_VER)
    #define COMPILER_MSVC
#endif

// Inline Assembly makroları
#if defined(COMPILER_GCC)
    // For simple assembly commands
    #define ASM_INLINE(x) __asm__ volatile(x)
    // For assembly with output or input parameters
    #define ASM_INLINE_PARAMS(code, outputs, inputs) __asm__ volatile(code : outputs : inputs)
    // For assembly with one input parameter
    #define ASM_INLINE_INPUT1(code, input1) __asm__ volatile(code : : input1)
    // For assembly with two input parameters
    #define ASM_INLINE_INPUT2(code, input1, input2) __asm__ volatile(code : : input1, input2)
    #define HAVE_INLINE_ASM 1
#elif defined(COMPILER_MSVC)
    #define ASM_INLINE(x) __asm x
    #define ASM_INLINE_PARAMS(code, outputs, inputs) /* not supported */
    #define ASM_INLINE_INPUT1(code, input1) /* not supported */
    #define ASM_INLINE_INPUT2(code, input1, input2) /* not supported */
    #define HAVE_INLINE_ASM 1
#else
    #define ASM_INLINE(x) /* assembly disabled */
    #define ASM_INLINE_PARAMS(code, outputs, inputs) /* not supported */
    #define ASM_INLINE_INPUT1(code, input1) /* not supported */
    #define ASM_INLINE_INPUT2(code, input1, input2) /* not supported */
    #define HAVE_INLINE_ASM 0
#endif

// Bit fields compatibility
#if defined(COMPILER_GCC)
    // GCC supports bit fields directly
#elif defined(COMPILER_MSVC)
    // MSVC may behave differently in some cases
#endif

// Alignment attributes
#if defined(COMPILER_GCC)
    #define ALIGNED(x) __attribute__((aligned(x)))
#elif defined(COMPILER_MSVC)
    #define ALIGNED(x) __declspec(align(x))
#else
    #define ALIGNED(x)
#endif

// Attributes for packed structures
#if defined(COMPILER_GCC)
    #define PACKED __attribute__((packed))
#elif defined(COMPILER_MSVC)
    #define PACKED
#else
    #define PACKED
#endif

#endif // COMPAT_H 