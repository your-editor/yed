#ifndef __BUCKET_ARRAY_H__
#define __BUCKET_ARRAY_H__

#include "internal.h"

typedef struct bucket_t {
    void            *data;
    uint32_t         used,
                     capacity;
} bucket_t;

typedef bucket_t *bucket_ptr_t;

typedef struct {
    array_t  buckets;
    uint32_t elem_size,
             n_fit,
             used;
} bucket_array_t;

static bucket_array_t _bucket_array_make(int count, int elem_size);
static void _bucket_array_free(bucket_array_t *array);
static void * _bucket_array_item(bucket_array_t *array, int idx);
static void * _bucket_array_last(bucket_array_t *array);
static void * _bucket_array_insert(bucket_array_t *array, int idx, void *elem);
static void * _bucket_array_push(bucket_array_t *array, void *elem);
static void _bucket_array_delete(bucket_array_t *array, int idx);
static void _bucket_array_pop(bucket_array_t *array);

#define bucket_array_make(n, T) \
    (_bucket_array_make(n, sizeof(T)))

#define bucket_array_free(array) \
    (_bucket_array_free(&(array)))

#define bucket_array_len(array) \
    ((array).used)

#define bucket_array_item(array, idx) \
    (_bucket_array_item(&(array), idx))

#define bucket_array_last(array) \
    (_bucket_array_last(&(array)))

#define bucket_array_insert(array, idx, elem) \
    (_bucket_array_insert(&(array), idx, &(elem)))

#define bucket_array_push(array, elem) \
    (_bucket_array_push(&(array), &(elem)))

#define bucket_array_delete(array, idx) \
    (_bucket_array_delete(&(array), idx))

#define bucket_array_pop(array) \
    (_bucket_array_pop(&(array)))

#define bucket_array_traverse(array, it)                                              \
    for (int __idx = 0;                                                               \
         (__idx < (array).used) && (((it) = _bucket_array_item(&(array), __idx)), 1); \
         __idx += 1)

#define bucket_array_traverse_from(array, it, starting_idx)                           \
    for (int __idx = starting_idx;                                                    \
         (__idx < (array).used) && (((it) = _bucket_array_item(&(array), __idx)), 1); \
         __idx += 1)

#endif
