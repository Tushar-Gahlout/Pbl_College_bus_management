#include "c_circular_queue.h"

CCircularQueue* c_queue_create(int capacity) {
    if (capacity <= 0) capacity = 20;

    CCircularQueue* q = (CCircularQueue*)malloc(sizeof(CCircularQueue));
    if (!q) return NULL;

    q->data = (CWaitingStudent*)calloc((size_t)capacity, sizeof(CWaitingStudent));
    if (!q->data) {
        free(q);
        return NULL;
    }

    q->front = 0;
    q->rear = -1;
    q->count = 0;
    q->capacity = capacity;
    return q;
}

bool c_queue_is_empty(const CCircularQueue* q) {
    return (!q || q->count == 0);
}

bool c_queue_is_full(const CCircularQueue* q) {
    return (q && q->count == q->capacity);
}

bool c_queue_contains_student(const CCircularQueue* q, const char* student_id) {
    if (!q || !student_id || q->count == 0) return false;

    for (int i = 0; i < q->count; ++i) {
        int idx = (q->front + i) % q->capacity;
        if (q->data[idx].student_id && strcmp(q->data[idx].student_id, student_id) == 0) {
            return true;
        }
    }
    return false;
}

DS_Status c_queue_enqueue(CCircularQueue* q, const char* student_id, const char* student_name, const char* bus_id, const char* request_time) {
    if (!q || !student_id || !student_name) return DS_ERR_NULL_PTR;
    if (c_queue_is_full(q)) return DS_ERR_FULL;
    if (c_queue_contains_student(q, student_id)) return DS_ERR_DUPLICATE;

    q->rear = (q->rear + 1) % q->capacity;
    q->data[q->rear].student_id = ds_strdup(student_id);
    q->data[q->rear].student_name = ds_strdup(student_name);
    q->data[q->rear].bus_id = bus_id ? ds_strdup(bus_id) : ds_strdup("None");
    q->data[q->rear].request_time = request_time ? ds_strdup(request_time) : ds_strdup("");

    q->count++;
    return DS_SUCCESS;
}

DS_Status c_queue_dequeue(CCircularQueue* q, CWaitingStudent* out_student) {
    if (!q) return DS_ERR_NULL_PTR;
    if (c_queue_is_empty(q)) return DS_ERR_EMPTY;

    if (out_student) {
        *out_student = q->data[q->front];
    } else {
        free(q->data[q->front].student_id);
        free(q->data[q->front].student_name);
        free(q->data[q->front].bus_id);
        free(q->data[q->front].request_time);
    }

    q->data[q->front].student_id = NULL;
    q->data[q->front].student_name = NULL;
    q->data[q->front].bus_id = NULL;
    q->data[q->front].request_time = NULL;

    q->front = (q->front + 1) % q->capacity;
    q->count--;
    return DS_SUCCESS;
}

DS_Status c_queue_peek(const CCircularQueue* q, CWaitingStudent* out_student) {
    if (!q || !out_student) return DS_ERR_NULL_PTR;
    if (c_queue_is_empty(q)) return DS_ERR_EMPTY;

    *out_student = q->data[q->front];
    return DS_SUCCESS;
}

int c_queue_size(const CCircularQueue* q) {
    return q ? q->count : 0;
}

void c_queue_print(const CCircularQueue* q) {
    if (!q) {
        printf("Circular Queue: (NULL)\n");
        return;
    }
    printf("Circular Queue [Count: %d / Cap: %d, Front: %d, Rear: %d]:\n", q->count, q->capacity, q->front, q->rear);
    for (int i = 0; i < q->count; ++i) {
        int idx = (q->front + i) % q->capacity;
        printf("  [%d] Student: %s (%s) -> Bus: %s\n", i + 1, q->data[idx].student_id, q->data[idx].student_name, q->data[idx].bus_id);
    }
}

void c_queue_destroy(CCircularQueue* q) {
    if (!q) return;
    if (q->data) {
        for (int i = 0; i < q->capacity; ++i) {
            free(q->data[i].student_id);
            free(q->data[i].student_name);
            free(q->data[i].bus_id);
            free(q->data[i].request_time);
        }
        free(q->data);
    }
    free(q);
}
