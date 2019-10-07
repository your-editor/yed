#include "bucket_array.h"


static bucket_t new_bucket(bucket_array_t *array) {
    bucket_t bucket;

    bucket.data     = malloc(array->n_fit * array->elem_size);
    bucket.used     = 0;
    bucket.capacity = array->n_fit;

    return bucket;
}

static bucket_array_t _bucket_array_make(int count, int elem_size) {
    bucket_array_t array;

    array.buckets   = array_make(bucket_t);
    array.elem_size = elem_size;
    array.n_fit     = count;
    array.used      = 0;

    return array;
}

static bucket_t * bucket_array_add_new_bucket(bucket_array_t *array) {
    bucket_t  new_b,
             *b;

    new_b = new_bucket(array);
    b     = array_push(array->buckets, new_b);

    return b;
}

static void _bucket_array_free(bucket_array_t *array) {
    bucket_t *bucket_it;

    array_traverse(array->buckets, bucket_it) {
        free(bucket_it->data);
    }

    array_free(array->buckets);
}

#define BUCKET_ITEM(b, idx, elem_size) \
    ((b)->data + ((elem_size) * (idx)))

static int get_bucket_and_elem_idx_for_idx(bucket_array_t *array, int *idx) {
    int       b_idx;
    bucket_t *b;

    if (array_len(array->buckets) == 0) {
        return -1;
    }

    b_idx = 0;

    while (b_idx < array_len(array->buckets)) {
        b = array_item(array->buckets, b_idx);

        if (*idx < b->used) {
            return b_idx;
        } else {
            *idx   -= b->used;
            b_idx  += 1;
        }
    }

    return -1;
}

static int get_bucket_and_slot_idx_for_idx(bucket_array_t *array, int *idx) {
    int       b_idx;
    bucket_t *b, new_b;

    b_idx = 0;

    while (b_idx < array_len(array->buckets)) {
        b = array_item(array->buckets, b_idx);

        if (*idx < b->capacity) {
            return b_idx;
        } else {
            *idx  -= b->capacity;
            b_idx += 1;
        }
    }

    if (*idx == 0) {
        new_b = new_bucket(array);
        array_push(array->buckets, new_b);
        return array_len(array->buckets) - 1;
    }

    return -1;
}

static void * _bucket_array_item(bucket_array_t *array, int idx) {
    bucket_t *b;
    int       b_idx;

    b_idx = get_bucket_and_elem_idx_for_idx(array, &idx);
    ASSERT(b_idx >= 0, "index out of bounds in _bucket_array_item()");

    b = array_item(array->buckets, b_idx);

    return BUCKET_ITEM(b, idx, array->elem_size);
}

static void *_bucket_array_last(bucket_array_t *array) {
    bucket_t *b;

    if (array_len(array->buckets) == 0) {
        return NULL;
    }

    b = array_item(array->buckets, array_len(array->buckets) - 1);

    ASSERT(b->used, "why is there an empty bucket?");

    return BUCKET_ITEM(b, b->used - 1, array->elem_size);
}

static void bucket_delete(bucket_array_t *array, int b_idx, int idx, int elem_size) {
    void     *split;
    bucket_t *b;

    b = array_item(array->buckets, b_idx);

    ASSERT(idx < b->used, "can't delete from this index into bucket");

    if (b->used == 1) {
        free(b->data);
        array_delete(array->buckets, b_idx);
    } else {
        if (idx != b->used - 1) {
            split = b->data + (elem_size * idx);
            memmove(split,
                    split + elem_size,
                    elem_size * (b->used - idx - 1));
        }

        b->used -= 1;
    }

    array->used -= 1;
}

static void _bucket_array_delete(bucket_array_t *array, int idx) {
    int b_idx;

    if (idx == array->used - 1) {
        _bucket_array_pop(array);
        return;
    }

    b_idx = get_bucket_and_elem_idx_for_idx(array, &idx);
    ASSERT(b_idx >= 0, "index out of bounds in _bucket_array_delete()");

    bucket_delete(array, b_idx, idx, array->elem_size);
}

static void * bucket_insert(bucket_array_t *array, int b_idx, int idx, void *elem, int elem_size) {
    void     *elem_slot;
    bucket_t *b, *spill_b, *next_b, new_b;

    b = array_item(array->buckets, b_idx);

    if (b->used == b->capacity) {
        next_b = NULL;
        /*
         * If there's a next bucket that isn't full, we can spill there.
         * Otherwise, we need to make a new bucket.
         */
        if (b_idx < array_len(array->buckets) - 1) {
            next_b = array_item(array->buckets, b_idx + 1);
        }

        if (next_b && next_b->used < next_b->capacity) {
            spill_b = next_b;
        } else {
            /* Make a new empty bucket. */
            new_b   = new_bucket(array);
            spill_b = array_insert(array->buckets, b_idx + 1, new_b);
        }

        if (spill_b->used) {
            /* Shift everything in the spill bucket down an element. */
            memmove(spill_b->data + elem_size,
                    spill_b->data,
                    elem_size * spill_b->used);
        }

        spill_b->used += 1;

        /* Move the last element in the current bucket
         * to be the first element in the next bucket. */
        memcpy(spill_b->data,
               b->data + (elem_size * (b->used - 1)),
               elem_size);

        b->used -= 1;
    }

    /*
     * By this point, we (better) have room in the current bucket
     * to insert an element.
     */
    elem_slot = b->data + (elem_size * idx);

    if (idx < b->used) {
        memmove(elem_slot + elem_size,
                elem_slot,
                elem_size * (b->used - idx));
    }

    memcpy(elem_slot, elem, elem_size);

    b->used     += 1;
    array->used += 1;

    return elem_slot;
}

static void * _bucket_array_insert(bucket_array_t *array, int idx, void *elem) {
    int   b_idx;
    void *new_elem;

    if (idx == array->used) {
        return _bucket_array_push(array, elem);
    }

    b_idx = get_bucket_and_slot_idx_for_idx(array, &idx);

    ASSERT(b_idx >= 0, "index out of bounds in _bucket_array_insert()");

    new_elem = bucket_insert(array, b_idx, idx, elem, array->elem_size);

    return new_elem;
}

static void * _bucket_array_push(bucket_array_t *array, void *elem) {
    int       elem_size,
              created_new_bucket;
    bucket_t *b;
    void     *elem_slot;

    created_new_bucket = 0;
    elem_size          = array->elem_size;

    if (array_len(array->buckets) == 0) {
            b = bucket_array_add_new_bucket(array);
    } else {
        b = array_last(array->buckets);
        if (b->used == b->capacity) {
            b = bucket_array_add_new_bucket(array);
        }
    }

    if (created_new_bucket) {
    }

    elem_slot = b->data + (elem_size * b->used);

    memcpy(elem_slot, elem, elem_size);

    b->used     += 1;
    array->used += 1;

    return elem_slot;
}

static void _bucket_array_pop(bucket_array_t *array) {
    int       b_idx;
    bucket_t *b;

    ASSERT(array_len(array->buckets) > 0, "can't pop from an empty bucket array");

    b_idx = array_len(array->buckets) - 1;
    b     = array_item(array->buckets, b_idx);

    bucket_delete(array, b_idx, b->used - 1, array->elem_size);
}
