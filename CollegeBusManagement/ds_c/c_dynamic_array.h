#ifndef C_DYNAMIC_ARRAY_H
#define C_DYNAMIC_ARRAY_H

#include "c_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dynamic Array of string elements (e.g. Bus IDs, Student IDs, or string records).
 */
typedef struct {
    char** items;
    size_t size;
    size_t capacity;
} CDynamicArray;

CDynamicArray* c_darray_create(size_t initial_capacity);
DS_Status c_darray_append(CDynamicArray* arr, const char* value);
DS_Status c_darray_insert_at(CDynamicArray* arr, size_t index, const char* value);
DS_Status c_darray_delete_at(CDynamicArray* arr, size_t index);
DS_Status c_darray_remove_value(CDynamicArray* arr, const char* value);
int c_darray_find(const CDynamicArray* arr, const char* value);
const char* c_darray_get(const CDynamicArray* arr, size_t index);
size_t c_darray_size(const CDynamicArray* arr);
void c_darray_print(const CDynamicArray* arr);
void c_darray_destroy(CDynamicArray* arr);

#ifdef __cplusplus
}
#endif

#endif /* C_DYNAMIC_ARRAY_H */
