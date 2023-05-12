/*
MIT License

Copyright (c) 2023 m1cha1s

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef VEC_H
#define VEC_H

#include "alloc.h"

typedef struct {
    alloc vec_alloc;
    
    unsigned int capacity; // The bytes available in the current buffer
    unsigned int count; // The bytes used by the current buffer
    unsigned int growth_factor;
    void** data;
} vec;

/*
Create a new vec with custom alloc and growth factor
Returns:
    NULL             -> Fail
    <something else> -> Ok

*/
vec* vec_new(alloc vec_alloc, unsigned int growth_factor) {
    vec* v = (vec *)vec_alloc.malloc(sizeof(vec), vec_alloc.meta);
    if (!v) 
        return (vec*)0;
    
    v->vec_alloc = vec_alloc;
    v->growth_factor = growth_factor;
    v->data = (void**)0;
    v->capacity = 0;
    v->count = 0;
    
    return v;
}

/*
Expand the vec internal data buffer
Returns:
    0  -> Ok
    -1 -> Alloc error
*/
static int vec_grow(vec* v) {
    if (v->capacity == 0) {
        v->capacity = 8;
        v->data = (void**)v->vec_alloc.malloc(v->capacity*sizeof(void*), v->vec_alloc.meta);
        return 0;
    }

    v->capacity*=v->growth_factor;
    
    void** new_data = (void**)v->vec_alloc.malloc(v->capacity*sizeof(void*), v->vec_alloc.meta);
    if (!new_data)
        return -1;

    for (unsigned int i = 0; i < v->count-1; i++)
        new_data[i] = v->data[i];

    v->vec_alloc.free(v->data, v->vec_alloc.meta);
    v->data = new_data;

    return 0;
}

/*
Attemt to shrink the existing vec data.
Returns:
    0  -> Ok
    -1 -> Alloc error
    -2 -> Cannot shrink
*/
int vec_try_shrink(vec* v) {
    if (v->count > v->capacity && v->capacity >= v->growth_factor)
        return -2; // There is no need to shrink the vec

    unsigned int new_capacity = v->capacity/v->growth_factor;
    void** new_data = (void**)v->vec_alloc.malloc(new_capacity*sizeof(void*), v->vec_alloc.meta);
    if (!new_data)
        return -1;

    for (int i = 0; i < v->count; i++)
        new_data[i] = v->data[i];

    v->vec_alloc.free(v->data, v->vec_alloc.meta);
    v->data = new_data;
    v->capacity = new_capacity;

    return 0;
}

/*
Insert value into the vec.
Returns:
    0  -> Ok
    -1 -> Alloc error
*/
int vec_add(vec* v, void* item) {
    v->count++;
    if (v->capacity < v->count)
        if (vec_grow(v)) {
            v->count--;
            return -1;
        }

    v->data[v->count-1] = item;

    return 0;
}

/*
Get data from vec.
Returns:
    NULL             -> Out of range
    <something else> -> Ok
*/
void* vec_get(vec* v, unsigned int idx) {
    if (idx+1 > v->count)
        return (void*)0;

    return v->data[idx];
}

/*
Get vec lenght. THIS IS NOT IT'S SIZE IN MEMORY!!!
*/
unsigned int vec_len(vec* v) {
    return v->count;
}

/*
Remove value from vec at idx.
Returns:
    NULL             -> Out of range
    <something else> -> The value that has been removed from vec
*/
void* vec_remove(vec* v, unsigned int idx) {
    if (idx+1 > v->count)
        return (void*)0;

    void* val = v->data[idx];

    v->count--;

    for (int i = idx; i < v->count; i++)
        v->data[i] = v->data[i+1];

    // This may fail byt we don't care :)
    vec_try_shrink(v);

    return val;
}

/*
Deallocate all items from vector with alloc that was used to allocate them.
*/
void vec_clear(vec* v, alloc item_alloc) {
    for (int i = 0; i < v->count; i++)
        item_alloc.free(v->data[i], item_alloc.meta);
    
    v->count = 0;
    while (vec_try_shrink(v) != -2) {}
}

/*
Free the vec data strcture.
*/
void vec_free(vec* v) {
    v->vec_alloc.free(v->data, v->vec_alloc.meta);
    v->vec_alloc.free(v, v->vec_alloc.meta);
}

/*
vec_clean and vec_free combined.
*/
void vec_clear_free(vec*v, alloc item_alloc) {
    vec_clear(v, item_alloc);
    vec_free(v);
}

#ifndef NO_STD

/*
Same as vec_new but uses stdlib based alloc
*/
vec* vec_new_default() {
    return vec_new(std_alloc, 8);
}

/*
Same as vec_clear but uses stdlib based alloc
*/
void vec_clear_default(vec* v) {
    for (int i = 0; i < v->count; i++)
        std_alloc.free(v->data[i], v->vec_alloc.meta);
}

/*
Same as vec_clear_free but uses stdlib based alloc
*/
void vec_clear_free_default(vec*v) {
    vec_clear_default(v);
    vec_free(v);
}

#endif // NO_STD

#endif //VEC_H
