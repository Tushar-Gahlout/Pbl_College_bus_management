#ifndef C_STACK_H
#define C_STACK_H

#include "c_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UNDO_STUDENT_ADDED = 1,
    UNDO_STUDENT_DELETED,
    UNDO_BUS_ASSIGNED,
    UNDO_SEAT_ALLOCATED,
    UNDO_SEAT_RELEASED
} CUndoActionType;

typedef struct {
    CUndoActionType action_type;
    char* target_id;
    char* target_name;
    char* previous_state;
    char* timestamp;
} CUndoAction;

typedef struct CStackNode {
    CUndoAction action;
    struct CStackNode* next;
} CStackNode;

typedef struct {
    CStackNode* top;
    size_t count;
    size_t max_capacity;
} CStack;

CStack* c_stack_create(size_t max_capacity);
bool c_stack_is_empty(const CStack* s);
bool c_stack_is_full(const CStack* s);
DS_Status c_stack_push(CStack* s, CUndoActionType action_type, const char* target_id, const char* target_name, const char* prev_state);
DS_Status c_stack_pop(CStack* s, CUndoAction* out_action);
DS_Status c_stack_peek(const CStack* s, CUndoAction* out_action);
size_t c_stack_size(const CStack* s);
void c_stack_print(const CStack* s);
void c_stack_free_action(CUndoAction* action);
void c_stack_destroy(CStack* s);

#ifdef __cplusplus
}
#endif

#endif /* C_STACK_H */
