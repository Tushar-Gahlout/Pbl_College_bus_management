#ifndef C_COMMON_H
#define C_COMMON_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Status Return Codes */
typedef enum {
    DS_SUCCESS = 0,
    DS_ERR_NULL_PTR = -1,
    DS_ERR_ALLOC_FAILED = -2,
    DS_ERR_NOT_FOUND = -3,
    DS_ERR_DUPLICATE = -4,
    DS_ERR_FULL = -5,
    DS_ERR_EMPTY = -6,
    DS_ERR_OUT_OF_BOUNDS = -7
} DS_Status;

/* Safe C String duplication helper */
static inline char* ds_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* copy = (char*)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

#ifdef __cplusplus
}
#endif

#endif /* C_COMMON_H */
