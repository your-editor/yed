#include "array.h"

static array_t _array_make(int elem_size) {
    array_t a;

    memset(&a, 0, sizeof(a));
    a.elem_size = elem_size;

    return a;
}

static void array_grow_if_needed(array_t *array) {
    void *data_save;

    if (!array->data) {
        array->capacity = ARRAY_DEFAULT_CAP;
        array->data     = calloc(array->capacity, array->elem_size);
    } else if (array->used == array->capacity) {
        array->capacity <<= 1;
        data_save         = array->data;
        array->data       = calloc(array->capacity, array->elem_size);
        memcpy(array->data, data_save, array->used * array->elem_size);
    }
}

static void * _array_next_elem(array_t *array) {
    void *elem_slot;

    array_grow_if_needed(array);
    elem_slot = array->data + (array->elem_size * array->used++);
    return elem_slot;
}

static void * _array_push(array_t *array, void *elem) {
    void *elem_slot;

    array_grow_if_needed(array);

    elem_slot = _array_next_elem(array);
    memcpy(elem_slot, elem, array->elem_size);

    return elem_slot;
}

static void * _array_insert(array_t *array, int idx, void *elem) {
    void *elem_slot;

    if (idx == array->used) {
        return _array_push(array, elem);
    }

    array_grow_if_needed(array);

    elem_slot = array->data + (array->elem_size * idx);

    memmove(elem_slot + array->elem_size,
            elem_slot,
            array->elem_size * (array->used - idx));

    memcpy(elem_slot, elem, array->elem_size);

    array->used += 1;

    return elem_slot;
}

static void _array_delete(array_t *array, int idx) {
    void *split;

    if (array->used == 0) {
        return;
    }

    if (idx != array->used - 1) {
        split = array->data + (array->elem_size * idx);
        memmove(split,
                split + array->elem_size,
                array->elem_size * (array->used - idx - 1));
    }

    array->used -= 1;
}

static void _array_zero_term(array_t *array) {
    array_grow_if_needed(array);
    memset(array->data + (array->used * array->elem_size),
           0,
           array->elem_size);
}
