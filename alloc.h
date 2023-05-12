#ifndef ALLOC_H
#define ALLOC_H

typedef void*(*malloc_t)(unsigned long long, void*);
typedef void (*free_t)(void*,void*);

typedef struct {
    void* meta;
    malloc_t malloc;
    free_t free;
} alloc;

#ifndef NO_STD

#include <stdlib.h>

void* std_malloc(unsigned long long size, void* _meta) {
    return malloc(size);
}

void std_free(void* ptr, void* _meta) {
    free(ptr);
}

alloc std_alloc = { (void*)0, std_malloc, std_free };

#endif

#endif // ALLOC_H