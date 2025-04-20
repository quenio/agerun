#include "agerun_queue.h"
#include <stdlib.h>

/* Constants */
#define QUEUE_SIZE 256           // Maximum number of messages in the queue

/**
 * Queue structure implementation.
 * Uses a circular buffer for efficient message storage.
 */
struct queue_s {
    const char *messages[QUEUE_SIZE];  // Message storage (references only)
    int head;                          // Index of the oldest message
    int tail;                          // Index for inserting new messages
    int size;                          // Current number of messages
};

/**
 * Creates a new queue structure and initializes it.
 */
queue_t* ar_queue_create(void) {
    queue_t *queue = (queue_t *)malloc(sizeof(queue_t));
    if (!queue) {
        return NULL;
    }
    
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    
    return queue;
}

/**
 * Frees all resources allocated for the queue.
 */
void ar_queue_destroy(queue_t *queue) {
    if (queue) {
        free(queue);
    }
}

/**
 * Adds a message reference to the queue.
 * The message is not copied, only its reference is stored.
 */
bool ar_queue_push(queue_t *queue, const char *message) {
    if (!queue || !message || queue->size >= QUEUE_SIZE) {
        return false;
    }
    
    queue->messages[queue->tail] = message;
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->size++;
    
    return true;
}

/**
 * Removes and returns a reference to the oldest message from the queue.
 */
const char* ar_queue_pop(queue_t *queue) {
    if (!queue || queue->size == 0) {
        return NULL;
    }
    
    const char *message = queue->messages[queue->head];
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    
    return message;
}

/**
 * Checks if the queue is empty.
 */
bool ar_queue_is_empty(const queue_t *queue) {
    return queue ? queue->size == 0 : true;
}

