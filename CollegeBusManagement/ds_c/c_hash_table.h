#ifndef C_HASH_TABLE_H
#define C_HASH_TABLE_H

#include "c_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CHashEntry {
    char* key;              /* Student ID, e.g. "STU101" */
    char* name;
    char* department;
    char* bus_number;
    int seat_number;
    char* phone;
    struct CHashEntry* next; /* Separate chaining link */
} CHashEntry;

typedef struct {
    CHashEntry** buckets;
    size_t num_buckets;
    size_t count;
} CHashTable;

CHashTable* c_htable_create(size_t num_buckets);
DS_Status c_htable_insert(CHashTable* ht, const char* key, const char* name, const char* dept, const char* bus, int seat, const char* phone);
const CHashEntry* c_htable_get(const CHashTable* ht, const char* key);
DS_Status c_htable_delete(CHashTable* ht, const char* key);
bool c_htable_contains(const CHashTable* ht, const char* key);
size_t c_htable_size(const CHashTable* ht);
void c_htable_print(const CHashTable* ht);
void c_htable_destroy(CHashTable* ht);

#ifdef __cplusplus
}
#endif

#endif /* C_HASH_TABLE_H */
