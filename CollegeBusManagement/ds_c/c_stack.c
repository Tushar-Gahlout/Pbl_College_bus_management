#include "c_stack.h"
#include <time.h>

CStack* c_stack_create(size_t max_capacity) {
    if (max_capacity == 0) max_capacity = 50;

    CStack* s = (CStack*)malloc(sizeof(CStack));
    if (!s) return NULL;

    s->top = NULL;
    s->count = 0;
    s->max_capacity = max_capacity;
    return s;
}

bool c_stack_is_empty(const CStack* s) {
    return (!s || s->count == 0);
}

bool c_stack_is_full(const CStack* s) {
    return (s && s->count >= s->max_capacity);
}

void c_stack_free_action(CUndoAction* action) {
    if (!action) return;
    free(action->target_id);
    free(action->target_name);
    free(action->previous_state);
    free(action->timestamp);
    action->target_id = NULL;
    action->target_name = NULL;
    action->previous_state = NULL;
    action->timestamp = NULL;
}

DS_Status c_stack_push(CStack* s, CUndoActionType action_type, const char* target_id, const char* target_name, const char* prev_state) {
    if (!s || !target_id) return DS_ERR_NULL_PTR;
    if (c_stack_is_full(s)) return DS_ERR_FULL;

    CStackNode* node = (CStackNode*)malloc(sizeof(CStackNode));
    if (!node) return DS_ERR_ALLOC_FAILED;

    node->action.action_type = action_type;
    node->action.target_id = ds_strdup(target_id);
    node->action.target_name = target_name ? ds_strdup(target_name) : ds_strdup("");
    node->action.previous_state = prev_state ? ds_strdup(prev_state) : ds_strdup("");

    time_t now = time(NULL);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", localtime(&now));
    node->action.timestamp = ds_strdup(time_str);

    node->next = s->top;
    s->top = node;
    s->count++;
    return DS_SUCCESS;
}

DS_Status c_stack_pop(CStack* s, CUndoAction* out_action) {
    if (!s) return DS_ERR_NULL_PTR;
    if (c_stack_is_empty(s)) return DS_ERR_EMPTY;

    CStackNode* popped = s->top;
    s->top = popped->next;
    s->count--;

    if (out_action) {
        *out_action = popped->action;
    } else {
        c_stack_free_action(&popped->action);
    }

    free(popped);
    return DS_SUCCESS;
}

DS_Status c_stack_peek(const CStack* s, CUndoAction* out_action) {
    if (!s || !out_action) return DS_ERR_NULL_PTR;
    if (c_stack_is_empty(s)) return DS_ERR_EMPTY;

    *out_action = s->top->action;
    return DS_SUCCESS;
}

size_t c_stack_size(const CStack* s) {
    return s ? s->count : 0;
}

void c_stack_print(const CStack* s) {
    if (!s) {
        printf("Stack: (NULL)\n");
        return;
    }
    printf("Undo Stack [%zu / max %zu]:\n", s->count, s->max_capacity);
    CStackNode* curr = s->top;
    int idx = 1;
    while (curr) {
        printf("  [%d] Action: %d | Target: %s (%s) @ %s\n",
               idx++, curr->action.action_type, curr->action.target_id,
               curr->action.target_name, curr->action.timestamp);
        curr = curr->next;
    }
}

void c_stack_destroy(CStack* s) {
    if (!s) return;
    CStackNode* curr = s->top;
    while (curr) {
        CStackNode* next = curr->next;
        c_stack_free_action(&curr->action);
        free(curr);
        curr = next;
    }
    free(s);
}
