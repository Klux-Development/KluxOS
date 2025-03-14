#ifndef _STDBOOL_H
#define _STDBOOL_H

/* C99-compliant stdbool.h implementation */

// Halihazırda bool tipinin tanımlı olup olmadığını kontrol et
#ifndef __bool_defined
#define __bool_defined 1

#ifndef __cplusplus
// types.h'da bool zaten tanımlanmışsa, burada yeniden tanımlamayalım
#ifndef _KERNEL_TYPES_H
typedef enum { false = 0, true = 1 } bool;
#endif
#endif

// Sabitler
#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#endif /* __bool_defined */

#endif /* _STDBOOL_H */ 