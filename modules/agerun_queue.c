#include "agerun_queue.h"
#include <string.h>
#include <stdlib.h>

/* Constants */
#define MAX_MESSAGE_LENGTH 1024  // Maximum length of a message
#define QUEUE_SIZE 256           // Maximum number of messages in the queue

/**
 * Queue structure implementation.
 * Uses a circular buffer for efficient message storage.
 */
struct queue_s {
    char messages[QUEUE_SIZE][MAX_MESSAGE_LENGTH];  // Message storage
    int head;                                       // Index of the oldest message
    int tail;                                       // Index for inserting new messages
    int size;                                       // Current number of messages
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
 * Adds a message to the queue.
 * The message is copied, so the original can be safely modified after this call.
 */
bool ar_queue_push(queue_t *queue, const char *message) {
    if (!queue || !message || queue->size >= QUEUE_SIZE) {
        return false;
    }
    
    size_t message_length = strlen(message);
    size_t copy_length = message_length < MAX_MESSAGE_LENGTH - 1 ? message_length : MAX_MESSAGE_LENGTH - 1;
    
    memcpy(queue->messages[queue->tail], message, copy_length);
    queue->messages[queue->tail][copy_length] = '\0';
    
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->size++;
    
    return true;
}

/**
 * Removes and returns the oldest message from the queue.
 */
bool ar_queue_pop(queue_t *queue, char *message) {
    if (!queue || !message || queue->size == 0) {
        return false;
    }
    
    strcpy(message, queue->messages[queue->head]);
    
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    
    return true;
}

/**
 * Checks if the queue is empty.
 */
bool ar_queue_is_empty(const queue_t *queue) {
    return queue ? queue->size == 0 : true;
}

/**
 * Returns the current number of messages in the queue.
 */
int ar_queue_size(const queue_t *queue) {
    return queue ? queue->size : 0;
}
