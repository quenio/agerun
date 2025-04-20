#ifndef AGERUN_QUEUE_H
#define AGERUN_QUEUE_H

#include <stdbool.h>

/**
 * A message queue structure for agent communication.
 */
typedef struct queue_s queue_t;

/**
 * Create a new heap-allocated empty queue.
 * @return Pointer to the new queue, or NULL on failure.
 */
queue_t* ar_queue_create(void);

/**
 * Destroy a queue and free all associated memory.
 * @param queue Queue to destroy.
 */
void ar_queue_destroy(queue_t *queue);

/**
 * Push a message to the queue.
 * @param queue Queue to push to.
 * @param message Message to push (reference only, not copied).
 * @return true if successful, false if queue is full or parameters are invalid.
 */
bool ar_queue_push(queue_t *queue, const char *message);

/**
 * Pop a message from the queue.
 * @param queue Queue to pop from.
 * @return Pointer to the next message, or NULL if queue is empty or invalid.
 */
const char* ar_queue_pop(queue_t *queue);

/**
 * Check if the queue is empty.
 * @param queue Queue to check.
 * @return true if empty or NULL, false otherwise.
 */
bool ar_queue_is_empty(const queue_t *queue);

#endif /* AGERUN_QUEUE_H */
