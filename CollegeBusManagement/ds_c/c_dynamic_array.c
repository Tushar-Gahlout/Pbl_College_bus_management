#include "c_dynamic_array.h"

#define DEFAULT_CAPACITY 8
#define GROWTH_FACTOR 2

CDynamicArray* c_darray_create(size_t initial_capacity) {
    if (initial_capacity == 0) initial_capacity = DEFAULT_CAPACITY;

    CDynamicArray* arr = (CDynamicArray*)malloc(sizeof(CDynamicArray));
    if (!arr) return NULL;

    arr->items = (char**)malloc(sizeof(char*) * initial_capacity);
    if (!arr->items) {
        free(arr);
        return NULL;
    }

    arr->size = 0;
    arr->capacity = initial_capacity;
    return arr;
}

static DS_Status c_darray_ensure_capacity(CDynamicArray* arr, size_t needed) {
    if (!arr) return DS_ERR_NULL_PTR;
    if (needed <= arr->capacity) return DS_SUCCESS;

    size_t new_cap = arr->capacity * GROWTH_FACTOR;
    if (new_cap < needed) new_cap = needed;

    char** new_items = (char**)realloc(arr->items, sizeof(char*) * new_cap);
    if (!new_items) return DS_ERR_ALLOC_FAILED;

    arr->items = new_items;
    arr->capacity = new_cap;
    return DS_SUCCESS;
}

DS_Status c_darray_append(CDynamicArray* arr, const char* value) {
    if (!arr || !value) return DS_ERR_NULL_PTR;

    DS_Status st = c_darray_ensure_capacity(arr, arr->size + 1);
    if (st != DS_SUCCESS) return st;

    char* copy = ds_strdup(value);
    if (!copy) return DS_ERR_ALLOC_FAILED;

    arr->items[arr->size++] = copy;
    return DS_SUCCESS;
}

DS_Status c_darray_insert_at(CDynamicArray* arr, size_t index, const char* value) {
    if (!arr || !value) return DS_ERR_NULL_PTR;
    if (index > arr->size) return DS_ERR_OUT_OF_BOUNDS;

    DS_Status st = c_darray_ensure_capacity(arr, arr->size + 1);
    if (st != DS_SUCCESS) return st;

    char* copy = ds_strdup(value);
    if (!copy) return DS_ERR_ALLOC_FAILED;

    for (size_t i = arr->size; i > index; --i) {
        arr->items[i] = arr->items[i - 1];
    }
    arr->items[index] = copy;
    arr->size++;
    return DS_SUCCESS;
}

DS_Status c_darray_delete_at(CDynamicArray* arr, size_t index) {
    if (!arr) return DS_ERR_NULL_PTR;
    if (index >= arr->size) return DS_ERR_OUT_OF_BOUNDS;

    free(arr->items[index]);
    for (size_t i = index; i < arr->size - 1; ++i) {
        arr->items[i] = arr->items[i + 1];
    }
    arr->size--;
    arr->items[arr->size] = NULL;
    return DS_SUCCESS;
}

DS_Status c_darray_remove_value(CDynamicArray* arr, const char* value) {
    if (!arr || !value) return DS_ERR_NULL_PTR;
    int idx = c_darray_find(arr, value);
    if (idx < 0) return DS_ERR_NOT_FOUND;
    return c_darray_delete_at(arr, (size_t)idx);
}

int c_darray_find(const CDynamicArray* arr, const char* value) {
    if (!arr || !value) return -1;
    for (size_t i = 0; i < arr->size; ++i) {
        if (arr->items[i] && strcmp(arr->items[i], value) == 0) {
            return (int)i;
        }
    }
    return -1;
}

const char* c_darray_get(const CDynamicArray* arr, size_t index) {
    if (!arr || index >= arr->size) return NULL;
    return arr->items[index];
}

size_t c_darray_size(const CDynamicArray* arr) {
    return arr ? arr->size : 0;
}

void c_darray_print(const CDynamicArray* arr) {
    if (!arr) {
        printf("Dynamic Array: (NULL)\n");
        return;
    }
    printf("Dynamic Array [Size: %zu, Capacity: %zu]: ", arr->size, arr->capacity);
    for (size_t i = 0; i < arr->size; ++i) {
        printf("%s%s", arr->items[i], (i + 1 < arr->size) ? ", " : "");
    }
    printf("\n");
}

void c_darray_destroy(CDynamicArray* arr) {
    if (!arr) return;
    if (arr->items) {
        for (size_t i = 0; i < arr->size; ++i) {
            free(arr->items[i]);
        }
        free(arr->items);
    }
    free(arr);
}
