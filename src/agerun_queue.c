#include "agerun_queue.h"
#include <string.h>

bool ar_queue_init(queue_t *queue) {
    if (!queue) {
        return false;
    }
    
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    
    return true;
}

bool ar_queue_push(queue_t *queue, const char *message) {
    if (!queue || !message || queue->size >= QUEUE_SIZE) {
        return false;
    }
    
    strncpy(queue->messages[queue->tail], message, MAX_MESSAGE_LENGTH - 1);
    queue->messages[queue->tail][MAX_MESSAGE_LENGTH - 1] = '\0';
    
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->size++;
    
    return true;
}

bool ar_queue_pop(queue_t *queue, char *message) {
    if (!queue || !message || queue->size == 0) {
        return false;
    }
    
    strncpy(message, queue->messages[queue->head], MAX_MESSAGE_LENGTH);
    
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    
    return true;
}
