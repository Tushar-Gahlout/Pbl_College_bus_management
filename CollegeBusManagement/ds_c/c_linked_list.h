#ifndef C_LINKED_LIST_H
#define C_LINKED_LIST_H

#include "c_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CListNode {
    char* stop_name;
    int stop_seq;
    struct CListNode* next;
} CListNode;

typedef struct {
    CListNode* head;
    size_t count;
} CSinglyLinkedList;

CSinglyLinkedList* c_llist_create(void);
DS_Status c_llist_insert_front(CSinglyLinkedList* list, const char* stop_name, int stop_seq);
DS_Status c_llist_insert_back(CSinglyLinkedList* list, const char* stop_name, int stop_seq);
DS_Status c_llist_delete_by_name(CSinglyLinkedList* list, const char* stop_name);
DS_Status c_llist_delete_front(CSinglyLinkedList* list);
CListNode* c_llist_search(const CSinglyLinkedList* list, const char* stop_name);
size_t c_llist_size(const CSinglyLinkedList* list);
void c_llist_print(const CSinglyLinkedList* list);
char* c_llist_to_string(const CSinglyLinkedList* list, const char* delimiter);
void c_llist_destroy(CSinglyLinkedList* list);

#ifdef __cplusplus
}
#endif

#endif /* C_LINKED_LIST_H */
