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
    
    size_t message_length = strlen(message);
    size_t copy_length = message_length < MAX_MESSAGE_LENGTH - 1 ? message_length : MAX_MESSAGE_LENGTH - 1;
    
    memcpy(queue->messages[queue->tail], message, copy_length);
    queue->messages[queue->tail][copy_length] = '\0';
    
    queue->tail = (queue->tail + 1) % QUEUE_SIZE;
    queue->size++;
    
    return true;
}

bool ar_queue_pop(queue_t *queue, char *message) {
    if (!queue || !message || queue->size == 0) {
        return false;
    }
    
    strcpy(message, queue->messages[queue->head]);
    
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    
    return true;
}
