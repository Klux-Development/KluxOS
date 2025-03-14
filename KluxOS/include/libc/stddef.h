#ifndef _STDDEF_H
#define _STDDEF_H

/* C standart kütüphane tiplerini tanımlama */

#if !defined(NULL)
#define NULL ((void*)0)
#endif

typedef unsigned int size_t;
typedef int ptrdiff_t;

#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _STDDEF_H */ 