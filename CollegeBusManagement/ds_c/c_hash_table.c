#include "c_hash_table.h"

#define DEFAULT_BUCKETS 31

/* Custom polynomial rolling hash (DJB2 algorithm with unsigned char cast) */
static size_t c_htable_hash(const char* str, size_t num_buckets) {
    if (!str || num_buckets == 0) return 0;
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + (unsigned long)c; /* hash * 33 + c */
    }
    return (size_t)(hash % num_buckets);
}

CHashTable* c_htable_create(size_t num_buckets) {
    if (num_buckets == 0) num_buckets = DEFAULT_BUCKETS;

    CHashTable* ht = (CHashTable*)malloc(sizeof(CHashTable));
    if (!ht) return NULL;

    ht->buckets = (CHashEntry**)calloc(num_buckets, sizeof(CHashEntry*));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }

    ht->num_buckets = num_buckets;
    ht->count = 0;
    return ht;
}

static CHashEntry* c_htable_create_entry(const char* key, const char* name, const char* dept, const char* bus, int seat, const char* phone) {
    CHashEntry* entry = (CHashEntry*)malloc(sizeof(CHashEntry));
    if (!entry) return NULL;

    entry->key = ds_strdup(key);
    entry->name = ds_strdup(name);
    entry->department = dept ? ds_strdup(dept) : ds_strdup("General");
    entry->bus_number = bus ? ds_strdup(bus) : ds_strdup("None");
    entry->seat_number = seat;
    entry->phone = phone ? ds_strdup(phone) : ds_strdup("");
    entry->next = NULL;
    return entry;
}

static void c_htable_free_entry(CHashEntry* entry) {
    if (!entry) return;
    free(entry->key);
    free(entry->name);
    free(entry->department);
    free(entry->bus_number);
    free(entry->phone);
    free(entry);
}

DS_Status c_htable_insert(CHashTable* ht, const char* key, const char* name, const char* dept, const char* bus, int seat, const char* phone) {
    if (!ht || !key || !name) return DS_ERR_NULL_PTR;

    size_t index = c_htable_hash(key, ht->num_buckets);
    CHashEntry* curr = ht->buckets[index];

    /* Update existing key if found */
    while (curr) {
        if (curr->key && strcmp(curr->key, key) == 0) {
            free(curr->name);
            free(curr->department);
            free(curr->bus_number);
            free(curr->phone);

            curr->name = ds_strdup(name);
            curr->department = dept ? ds_strdup(dept) : ds_strdup("General");
            curr->bus_number = bus ? ds_strdup(bus) : ds_strdup("None");
            curr->seat_number = seat;
            curr->phone = phone ? ds_strdup(phone) : ds_strdup("");
            return DS_SUCCESS;
        }
        curr = curr->next;
    }

    /* Otherwise insert new node at the head of the chain */
    CHashEntry* new_entry = c_htable_create_entry(key, name, dept, bus, seat, phone);
    if (!new_entry) return DS_ERR_ALLOC_FAILED;

    new_entry->next = ht->buckets[index];
    ht->buckets[index] = new_entry;
    ht->count++;
    return DS_SUCCESS;
}

const CHashEntry* c_htable_get(const CHashTable* ht, const char* key) {
    if (!ht || !key) return NULL;

    size_t index = c_htable_hash(key, ht->num_buckets);
    CHashEntry* curr = ht->buckets[index];

    while (curr) {
        if (curr->key && strcmp(curr->key, key) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

bool c_htable_contains(const CHashTable* ht, const char* key) {
    return (c_htable_get(ht, key) != NULL);
}

DS_Status c_htable_delete(CHashTable* ht, const char* key) {
    if (!ht || !key) return DS_ERR_NULL_PTR;

    size_t index = c_htable_hash(key, ht->num_buckets);
    CHashEntry* curr = ht->buckets[index];
    CHashEntry* prev = NULL;

    while (curr) {
        if (curr->key && strcmp(curr->key, key) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                ht->buckets[index] = curr->next;
            }
            c_htable_free_entry(curr);
            ht->count--;
            return DS_SUCCESS;
        }
        prev = curr;
        curr = curr->next;
    }
    return DS_ERR_NOT_FOUND;
}

size_t c_htable_size(const CHashTable* ht) {
    return ht ? ht->count : 0;
}

void c_htable_print(const CHashTable* ht) {
    if (!ht) {
        printf("Hash Table: (NULL)\n");
        return;
    }
    printf("Hash Table [Buckets: %zu, Total Count: %zu]:\n", ht->num_buckets, ht->count);
    for (size_t b = 0; b < ht->num_buckets; ++b) {
        CHashEntry* curr = ht->buckets[b];
        if (curr) {
            printf("  Bucket %2zu: ", b);
            while (curr) {
                printf("[%s -> %s]%s", curr->key, curr->name, curr->next ? " -> " : "");
                curr = curr->next;
            }
            printf("\n");
        }
    }
}

void c_htable_destroy(CHashTable* ht) {
    if (!ht) return;
    if (ht->buckets) {
        for (size_t b = 0; b < ht->num_buckets; ++b) {
            CHashEntry* curr = ht->buckets[b];
            while (curr) {
                CHashEntry* next = curr->next;
                c_htable_free_entry(curr);
                curr = next;
            }
        }
        free(ht->buckets);
    }
    free(ht);
}
