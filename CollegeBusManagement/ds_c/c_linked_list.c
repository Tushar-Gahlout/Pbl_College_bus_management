#include "c_linked_list.h"

CSinglyLinkedList* c_llist_create(void) {
    CSinglyLinkedList* list = (CSinglyLinkedList*)malloc(sizeof(CSinglyLinkedList));
    if (!list) return NULL;
    list->head = NULL;
    list->count = 0;
    return list;
}

static CListNode* c_llist_create_node(const char* stop_name, int stop_seq) {
    if (!stop_name) return NULL;
    CListNode* node = (CListNode*)malloc(sizeof(CListNode));
    if (!node) return NULL;

    node->stop_name = ds_strdup(stop_name);
    if (!node->stop_name) {
        free(node);
        return NULL;
    }
    node->stop_seq = stop_seq;
    node->next = NULL;
    return node;
}

DS_Status c_llist_insert_front(CSinglyLinkedList* list, const char* stop_name, int stop_seq) {
    if (!list || !stop_name) return DS_ERR_NULL_PTR;

    CListNode* node = c_llist_create_node(stop_name, stop_seq);
    if (!node) return DS_ERR_ALLOC_FAILED;

    node->next = list->head;
    list->head = node;
    list->count++;
    return DS_SUCCESS;
}

DS_Status c_llist_insert_back(CSinglyLinkedList* list, const char* stop_name, int stop_seq) {
    if (!list || !stop_name) return DS_ERR_NULL_PTR;

    CListNode* node = c_llist_create_node(stop_name, stop_seq);
    if (!node) return DS_ERR_ALLOC_FAILED;

    if (!list->head) {
        list->head = node;
    } else {
        CListNode* curr = list->head;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = node;
    }
    list->count++;
    return DS_SUCCESS;
}

DS_Status c_llist_delete_front(CSinglyLinkedList* list) {
    if (!list) return DS_ERR_NULL_PTR;
    if (!list->head) return DS_ERR_EMPTY;

    CListNode* temp = list->head;
    list->head = list->head->next;
    free(temp->stop_name);
    free(temp);
    list->count--;
    return DS_SUCCESS;
}

DS_Status c_llist_delete_by_name(CSinglyLinkedList* list, const char* stop_name) {
    if (!list || !stop_name) return DS_ERR_NULL_PTR;
    if (!list->head) return DS_ERR_EMPTY;

    CListNode* curr = list->head;
    CListNode* prev = NULL;

    while (curr) {
        if (curr->stop_name && strcmp(curr->stop_name, stop_name) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                list->head = curr->next;
            }
            free(curr->stop_name);
            free(curr);
            list->count--;
            return DS_SUCCESS;
        }
        prev = curr;
        curr = curr->next;
    }
    return DS_ERR_NOT_FOUND;
}

CListNode* c_llist_search(const CSinglyLinkedList* list, const char* stop_name) {
    if (!list || !stop_name) return NULL;
    CListNode* curr = list->head;
    while (curr) {
        if (curr->stop_name && strcmp(curr->stop_name, stop_name) == 0) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

size_t c_llist_size(const CSinglyLinkedList* list) {
    return list ? list->count : 0;
}

void c_llist_print(const CSinglyLinkedList* list) {
    if (!list) {
        printf("Linked List: (NULL)\n");
        return;
    }
    printf("Linked List Route Stops [%zu]: ", list->count);
    CListNode* curr = list->head;
    while (curr) {
        printf("[%d: %s]%s", curr->stop_seq, curr->stop_name, curr->next ? " -> " : "");
        curr = curr->next;
    }
    printf("\n");
}

char* c_llist_to_string(const CSinglyLinkedList* list, const char* delimiter) {
    if (!list || !list->head) return ds_strdup("");
    if (!delimiter) delimiter = ";";

    size_t total_len = 0;
    size_t delim_len = strlen(delimiter);

    CListNode* curr = list->head;
    while (curr) {
        total_len += (curr->stop_name ? strlen(curr->stop_name) : 0) + delim_len;
        curr = curr->next;
    }

    char* buffer = (char*)malloc(total_len + 1);
    if (!buffer) return NULL;
    buffer[0] = '\0';

    curr = list->head;
    bool first = true;
    while (curr) {
        if (!first) {
            strcat(buffer, delimiter);
        }
        if (curr->stop_name) {
            strcat(buffer, curr->stop_name);
        }
        first = false;
        curr = curr->next;
    }
    return buffer;
}

void c_llist_destroy(CSinglyLinkedList* list) {
    if (!list) return;
    CListNode* curr = list->head;
    while (curr) {
        CListNode* next = curr->next;
        free(curr->stop_name);
        free(curr);
        curr = next;
    }
    free(list);
}
