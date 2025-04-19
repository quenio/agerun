#ifndef AGERUN_QUEUE_H
#define AGERUN_QUEUE_H

#include <stdbool.h>

/* Constants */
#define MAX_MESSAGE_LENGTH 1024
#define QUEUE_SIZE 256

/* Queue for Agent Communication */
typedef struct queue_s {
    char messages[QUEUE_SIZE][MAX_MESSAGE_LENGTH];
    int head;
    int tail;
    int size;
} queue_t;

/* Function declarations for queue operations */
bool ar_queue_init(queue_t *queue);
bool ar_queue_push(queue_t *queue, const char *message);
bool ar_queue_pop(queue_t *queue, char *message);

#endif /* AGERUN_QUEUE_H */
