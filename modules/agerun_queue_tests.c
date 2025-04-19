#include "agerun_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_queue_init(void);
static void test_queue_push_pop_single(void);
static void test_queue_pop_empty(void);
static void test_queue_push_pop_multiple(void);
static void test_queue_wrap_around(void);
static void test_queue_full(void);

static void test_queue_init(void) {
    printf("Testing ar_queue_init()...\n");
    
    // Given an uninitialized queue structure
    queue_t queue;
    
    // When we initialize the queue
    bool result = ar_queue_init(&queue);
    
    // Then the initialization should succeed
    assert(result);
    
    // And the queue should be properly initialized with empty state
    assert(queue.head == 0);
    assert(queue.tail == 0);
    assert(queue.size == 0);
    
    printf("All ar_queue_init() tests passed!\n");
}

static void test_queue_push_pop_single(void) {
    printf("Testing ar_queue_push() and ar_queue_pop() with a single message...\n");
    
    // Given an initialized queue
    queue_t queue;
    ar_queue_init(&queue);
    
    // When we push a message to the queue
    bool push_result = ar_queue_push(&queue, "Hello, World!");
    
    // Then the push should succeed
    assert(push_result);
    
    // And the queue size should be incremented
    assert(queue.size == 1);
    
    // When we pop a message from the queue
    char message[MAX_MESSAGE_LENGTH];
    bool pop_result = ar_queue_pop(&queue, message);
    
    // Then the pop should succeed
    assert(pop_result);
    
    // And the message content should match what we pushed
    assert(strcmp(message, "Hello, World!") == 0);
    
    // And the queue should be empty
    assert(queue.size == 0);
    
    printf("Push and pop single message test passed!\n");
}

static void test_queue_pop_empty(void) {
    printf("Testing ar_queue_pop() from an empty queue...\n");
    
    // Given an initialized empty queue
    queue_t queue;
    ar_queue_init(&queue);
    
    // When we attempt to pop from the empty queue
    char message[MAX_MESSAGE_LENGTH];
    bool pop_result = ar_queue_pop(&queue, message);
    
    // Then the pop operation should fail
    assert(!pop_result);
    
    printf("Pop from empty queue test passed!\n");
}

static void test_queue_push_pop_multiple(void) {
    printf("Testing ar_queue_push() and ar_queue_pop() with multiple messages...\n");
    
    // Given an initialized queue
    queue_t queue;
    ar_queue_init(&queue);
    
    // When we push multiple messages to the queue
    bool push1 = ar_queue_push(&queue, "Message 1");
    bool push2 = ar_queue_push(&queue, "Message 2");
    bool push3 = ar_queue_push(&queue, "Message 3");
    
    // Then all pushes should succeed
    assert(push1);
    assert(push2);
    assert(push3);
    
    // And the queue size should be incremented correctly
    assert(queue.size == 3);
    
    // When we pop the first message
    char message[MAX_MESSAGE_LENGTH];
    bool pop1 = ar_queue_pop(&queue, message);
    
    // Then the pop should succeed and return the correct message
    assert(pop1);
    assert(strcmp(message, "Message 1") == 0);
    
    // When we pop the second message
    bool pop2 = ar_queue_pop(&queue, message);
    
    // Then the pop should succeed and return the correct message
    assert(pop2);
    assert(strcmp(message, "Message 2") == 0);
    
    // When we pop the third message
    bool pop3 = ar_queue_pop(&queue, message);
    
    // Then the pop should succeed and return the correct message
    assert(pop3);
    assert(strcmp(message, "Message 3") == 0);
    
    // And the queue should be empty
    assert(queue.size == 0);
    
    printf("Push and pop multiple messages test passed!\n");
}

static void test_queue_wrap_around(void) {
    printf("Testing queue wrap-around behavior...\n");
    
    // Given an initialized queue
    queue_t queue;
    ar_queue_init(&queue);
    char message[MAX_MESSAGE_LENGTH];
    
    // And the queue is filled to near capacity
    for (int i = 0; i < QUEUE_SIZE - 1; i++) {
        char temp[MAX_MESSAGE_LENGTH];
        sprintf(temp, "Message %d", i);
        assert(ar_queue_push(&queue, temp));
    }
    
    // When we pop half the messages
    for (int i = 0; i < (QUEUE_SIZE - 1) / 2; i++) {
        bool pop_result = ar_queue_pop(&queue, message);
        
        // Then each pop should succeed
        assert(pop_result);
        
        // And the messages should match what we pushed
        char expected[MAX_MESSAGE_LENGTH];
        sprintf(expected, "Message %d", i);
        assert(strcmp(message, expected) == 0);
    }
    
    // When we push more messages to force wrap-around
    int start_idx = (QUEUE_SIZE - 1) / 2;
    for (int i = 0; i < (QUEUE_SIZE - 1) / 2; i++) {
        char temp[MAX_MESSAGE_LENGTH];
        sprintf(temp, "Wrap %d", i);
        bool push_result = ar_queue_push(&queue, temp);
        
        // Then each push should succeed
        assert(push_result);
    }
    
    // When we pop the remaining original messages
    for (int i = start_idx; i < QUEUE_SIZE - 1; i++) {
        bool pop_result = ar_queue_pop(&queue, message);
        
        // Then each pop should succeed
        assert(pop_result);
        
        // And the messages should match the original ones
        char expected[MAX_MESSAGE_LENGTH];
        sprintf(expected, "Message %d", i);
        assert(strcmp(message, expected) == 0);
    }
    
    // When we pop the wrap-around messages
    for (int i = 0; i < (QUEUE_SIZE - 1) / 2; i++) {
        bool pop_result = ar_queue_pop(&queue, message);
        
        // Then each pop should succeed
        assert(pop_result);
        
        // And the messages should match the wrap-around ones
        char expected[MAX_MESSAGE_LENGTH];
        sprintf(expected, "Wrap %d", i);
        assert(strcmp(message, expected) == 0);
    }
    
    // Then the queue should be empty 
    assert(queue.size == 0);
    
    // And popping from the empty queue should fail
    assert(!ar_queue_pop(&queue, message));
    
    printf("Queue wrap-around tests passed!\n");
}

static void test_queue_full(void) {
    printf("Testing queue full behavior...\n");
    
    // Given an initialized queue
    queue_t queue;
    ar_queue_init(&queue);
    
    // When we fill the queue to capacity
    for (int i = 0; i < QUEUE_SIZE; i++) {
        char temp[MAX_MESSAGE_LENGTH];
        sprintf(temp, "Message %d", i);
        bool push_result = ar_queue_push(&queue, temp);
        
        // Then each push should succeed
        assert(push_result);
    }
    
    // When we try to push one more message to a full queue
    bool overflow_result = ar_queue_push(&queue, "Overflow");
    
    // Then the push operation should fail
    assert(!overflow_result);
    
    // When we pop one message
    char message[MAX_MESSAGE_LENGTH];
    bool pop_result = ar_queue_pop(&queue, message);
    
    // Then the pop should succeed
    assert(pop_result);
    
    // And the message should match the first one we pushed
    assert(strcmp(message, "Message 0") == 0);
    
    // When we push a new message after making space
    bool replacement_result = ar_queue_push(&queue, "Replacement");
    
    // Then the push should succeed
    assert(replacement_result);
    
    printf("Queue full tests passed!\n");
}

int main(void) {
    printf("Starting Queue Module Tests...\n");
    
    test_queue_init();
    test_queue_push_pop_single();
    test_queue_pop_empty();
    test_queue_push_pop_multiple();
    test_queue_wrap_around();
    test_queue_full();
    
    printf("All queue tests passed!\n");
    return 0;
}
