#include "agerun_queue.h"
#include <stdlib.h>

/* Constants */
#define QUEUE_SIZE 256           // Maximum number of references in the queue

/**
 * Queue structure implementation.
 * Uses a circular buffer for efficient reference storage.
 */
struct queue_s {
    const void *refs[QUEUE_SIZE];  // Reference storage (not copied)
    int head;                       // Index of the oldest reference
    int tail;                       // Index for inserting new references
    int size;                       // Current number of references
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
 * Adds a reference to the queue.
 * The reference is not copied, only stored.
 */
bool ar_queue_push(queue_t *queue, const void *ref) {
    if (!queue || !ref || queue->size >= QUEUE_SIZE) {
        return false;
    }
    
    queue->refs[queue->tail] = ref;
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->size++;
    
    return true;
}

/**
 * Removes and returns the oldest reference from the queue.
 */
const void* ar_queue_pop(queue_t *queue) {
    if (!queue || queue->size == 0) {
        return NULL;
    }
    
    const void *ref = queue->refs[queue->head];
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    
    return ref;
}

/**
 * Checks if the queue is empty.
 */
bool ar_queue_is_empty(const queue_t *queue) {
    return queue ? queue->size == 0 : true;
}

