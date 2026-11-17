#ifndef C_CIRCULAR_QUEUE_H
#define C_CIRCULAR_QUEUE_H

#include "c_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* student_id;
    char* student_name;
    char* bus_id;
    char* request_time;
} CWaitingStudent;

typedef struct {
    CWaitingStudent* data;
    int front;
    int rear;
    int count;
    int capacity;
} CCircularQueue;

CCircularQueue* c_queue_create(int capacity);
bool c_queue_is_empty(const CCircularQueue* q);
bool c_queue_is_full(const CCircularQueue* q);
bool c_queue_contains_student(const CCircularQueue* q, const char* student_id);
DS_Status c_queue_enqueue(CCircularQueue* q, const char* student_id, const char* student_name, const char* bus_id, const char* request_time);
DS_Status c_queue_dequeue(CCircularQueue* q, CWaitingStudent* out_student);
DS_Status c_queue_peek(const CCircularQueue* q, CWaitingStudent* out_student);
int c_queue_size(const CCircularQueue* q);
void c_queue_print(const CCircularQueue* q);
void c_queue_destroy(CCircularQueue* q);

#ifdef __cplusplus
}
#endif

#endif /* C_CIRCULAR_QUEUE_H */
