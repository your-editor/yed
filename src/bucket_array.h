#ifndef __BUCKET_ARRAY_H__
#define __BUCKET_ARRAY_H__


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

bucket_array_t _bucket_array_make(int count, int elem_size);
void _bucket_array_free(bucket_array_t *array);
void * _bucket_array_item(bucket_array_t *array, int idx);
void * _bucket_array_last(bucket_array_t *array);
void * _bucket_array_insert(bucket_array_t *array, int idx, void *elem);
void * _bucket_array_push(bucket_array_t *array, void *elem);
void _bucket_array_delete(bucket_array_t *array, int idx);
void _bucket_array_pop(bucket_array_t *array);
void _bucket_array_pop(bucket_array_t *array);

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

#define bucket_array_clear(array) \
    (_bucket_array_clear(&(array)))


typedef struct {
    bucket_array_t *array;
    int             bucket_idx;
    int             elem_idx;
    int             direction; /* 0 is forwards, 1 is backwards */
} bucket_array_iter_t;


bucket_array_iter_t _bucket_array_iter_make_at(bucket_array_t *array, int idx, int dir);
bucket_array_iter_t _bucket_array_iter_make(bucket_array_t *array, int dir);
int _bucket_array_iter_is_end(bucket_array_iter_t *it);
void * _bucket_array_iter_item(bucket_array_iter_t *it);
void _bucket_array_iter_next(bucket_array_iter_t *it);

#define bucket_array_traverse(array, it)                                                   \
    for (bucket_array_iter_t __ba_iter##__LINE__ = _bucket_array_iter_make(&(array), 0);   \
         !_bucket_array_iter_is_end(&__ba_iter##__LINE__)                                  \
             && (((it) = (__typeof(it))_bucket_array_iter_item(&__ba_iter##__LINE__)), 1); \
         _bucket_array_iter_next(&__ba_iter##__LINE__))

#define bucket_array_traverse_from(array, it, starting_idx)                                                 \
    for (bucket_array_iter_t __ba_iter##__LINE__ = _bucket_array_iter_make_at(&(array), (starting_idx), 0); \
         !_bucket_array_iter_is_end(&__ba_iter##__LINE__)                                                   \
             && (((it) = (__typeof(it))_bucket_array_iter_item(&__ba_iter##__LINE__)), 1);                  \
         _bucket_array_iter_next(&__ba_iter##__LINE__))

#define bucket_array_rtraverse(array, it)                                                  \
    for (bucket_array_iter_t __ba_iter##__LINE__ = _bucket_array_iter_make(&(array), 1);   \
         !_bucket_array_iter_is_end(&__ba_iter##__LINE__)                                  \
             && (((it) = (__typeof(it))_bucket_array_iter_item(&__ba_iter##__LINE__)), 1); \
         _bucket_array_iter_next(&__ba_iter##__LINE__))

#define bucket_array_rtraverse_from(array, it, starting_idx)                                                \
    for (bucket_array_iter_t __ba_iter##__LINE__ = _bucket_array_iter_make_at(&(array), (starting_idx), 1); \
         !_bucket_array_iter_is_end(&__ba_iter##__LINE__)                                                   \
             && (((it) = (__typeof(it))_bucket_array_iter_item(&__ba_iter##__LINE__)), 1);                  \
         _bucket_array_iter_next(&__ba_iter##__LINE__))

#endif
