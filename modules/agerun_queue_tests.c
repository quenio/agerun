#include "agerun_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_queue_init(void);
static void test_queue_push_pop(void);
static void test_queue_wrap_around(void);
static void test_queue_full(void);

static void test_queue_init(void) {
    printf("Testing ar_queue_init()...\n");
    
    queue_t queue;
    bool result = ar_queue_init(&queue);
    
    assert(result);
    assert(queue.head == 0);
    assert(queue.tail == 0);
    assert(queue.size == 0);
    
    printf("All ar_queue_init() tests passed!\n");
}

static void test_queue_push_pop(void) {
    printf("Testing ar_queue_push() and ar_queue_pop()...\n");
    
    queue_t queue;
    ar_queue_init(&queue);
    
    // Test 1: Push and pop a single message
    bool push_result = ar_queue_push(&queue, "Hello, World!");
    assert(push_result);
    assert(queue.size == 1);
    
    char message[MAX_MESSAGE_LENGTH];
    bool pop_result = ar_queue_pop(&queue, message);
    
    assert(pop_result);
    assert(strcmp(message, "Hello, World!") == 0);
    assert(queue.size == 0);
    
    // Test 2: Pop from empty queue
    pop_result = ar_queue_pop(&queue, message);
    assert(!pop_result);
    
    // Test 3: Push multiple messages
    assert(ar_queue_push(&queue, "Message 1"));
    assert(ar_queue_push(&queue, "Message 2"));
    assert(ar_queue_push(&queue, "Message 3"));
    assert(queue.size == 3);
    
    // Pop all messages
    assert(ar_queue_pop(&queue, message));
    assert(strcmp(message, "Message 1") == 0);
    
    assert(ar_queue_pop(&queue, message));
    assert(strcmp(message, "Message 2") == 0);
    
    assert(ar_queue_pop(&queue, message));
    assert(strcmp(message, "Message 3") == 0);
    
    assert(queue.size == 0);
    
    printf("Push and pop tests passed!\n");
}

static void test_queue_wrap_around(void) {
    printf("Testing queue wrap-around behavior...\n");
    
    queue_t queue;
    ar_queue_init(&queue);
    char message[MAX_MESSAGE_LENGTH];
    
    // Fill the queue to capacity minus 1
    for (int i = 0; i < QUEUE_SIZE - 1; i++) {
        char temp[MAX_MESSAGE_LENGTH];
        sprintf(temp, "Message %d", i);
        assert(ar_queue_push(&queue, temp));
    }
    
    // Pop half the messages
    for (int i = 0; i < (QUEUE_SIZE - 1) / 2; i++) {
        assert(ar_queue_pop(&queue, message));
        char expected[MAX_MESSAGE_LENGTH];
        sprintf(expected, "Message %d", i);
        assert(strcmp(message, expected) == 0);
    }
    
    // Push more messages to force wrap-around
    int start_idx = (QUEUE_SIZE - 1) / 2;
    for (int i = 0; i < (QUEUE_SIZE - 1) / 2; i++) {
        char temp[MAX_MESSAGE_LENGTH];
        sprintf(temp, "Wrap %d", i);
        assert(ar_queue_push(&queue, temp));
    }
    
    // Pop remaining original messages
    for (int i = start_idx; i < QUEUE_SIZE - 1; i++) {
        assert(ar_queue_pop(&queue, message));
        char expected[MAX_MESSAGE_LENGTH];
        sprintf(expected, "Message %d", i);
        assert(strcmp(message, expected) == 0);
    }
    
    // Pop wrap-around messages
    for (int i = 0; i < (QUEUE_SIZE - 1) / 2; i++) {
        assert(ar_queue_pop(&queue, message));
        char expected[MAX_MESSAGE_LENGTH];
        sprintf(expected, "Wrap %d", i);
        assert(strcmp(message, expected) == 0);
    }
    
    // Queue should be empty now
    assert(queue.size == 0);
    assert(!ar_queue_pop(&queue, message));
    
    printf("Queue wrap-around tests passed!\n");
}

static void test_queue_full(void) {
    printf("Testing queue full behavior...\n");
    
    queue_t queue;
    ar_queue_init(&queue);
    
    // Fill the queue to capacity
    for (int i = 0; i < QUEUE_SIZE; i++) {
        char temp[MAX_MESSAGE_LENGTH];
        sprintf(temp, "Message %d", i);
        assert(ar_queue_push(&queue, temp));
    }
    
    // Try to push one more (should fail)
    bool result = ar_queue_push(&queue, "Overflow");
    assert(!result);
    
    // Pop one message
    char message[MAX_MESSAGE_LENGTH];
    assert(ar_queue_pop(&queue, message));
    assert(strcmp(message, "Message 0") == 0);
    
    // Now we should be able to push again
    result = ar_queue_push(&queue, "Replacement");
    assert(result);
    
    printf("Queue full tests passed!\n");
}

int main(void) {
    printf("Starting Queue Module Tests...\n");
    
    test_queue_init();
    test_queue_push_pop();
    test_queue_wrap_around();
    test_queue_full();
    
    printf("All queue tests passed!\n");
    return 0;
}
