#include "agerun_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations
static void test_queue_create_destroy(void);
static void test_queue_push_pop_single(void);
static void test_queue_pop_empty(void);
static void test_queue_push_pop_multiple(void);
static void test_queue_wrap_around(void);
static void test_queue_full(void);

// We need static messages for tests since queue now only stores references
static char g_message_hello[] = "Hello, World!";
static char g_message_1[] = "Message 1";
static char g_message_2[] = "Message 2";
static char g_message_3[] = "Message 3";
static char g_message_replacement[] = "Replacement";
static char g_message_overflow[] = "Overflow";
static char g_message_buffer[512][32]; // For wrap-around tests

static void test_queue_create_destroy(void) {
    printf("Testing ar_queue_create() and ar_queue_destroy()...\n");
    
    // Given we need a new queue
    
    // When we create a queue
    queue_t *queue = ar_queue_create();
    
    // Then the creation should succeed
    assert(queue != NULL);
    
    // And the queue should be empty
    assert(ar_queue_is_empty(queue));
    
    // When we destroy the queue
    ar_queue_destroy(queue);
    
    // Then no assertion failures should occur
    printf("Queue creation and destruction tests passed!\n");
}

static void test_queue_push_pop_single(void) {
    printf("Testing ar_queue_push() and ar_queue_pop() with a single message...\n");
    
    // Given an initialized queue
    queue_t *queue = ar_queue_create();
    
    // When we push a message to the queue
    bool push_result = ar_queue_push(queue, g_message_hello);
    
    // Then the push should succeed
    assert(push_result);
    
    // And the queue should not be empty
    assert(!ar_queue_is_empty(queue));
    
    // When we pop a message from the queue
    const char *message = ar_queue_pop(queue);
    
    // Then the pop should return a valid message pointer
    assert(message != NULL);
    
    // And the message content should match what we pushed
    assert(strcmp(message, "Hello, World!") == 0);
    
    // And it should be the exact same pointer we pushed
    assert(message == g_message_hello);
    
    // And the queue should be empty
    assert(ar_queue_is_empty(queue));
    
    // Clean up
    ar_queue_destroy(queue);
    
    printf("Push and pop single message test passed!\n");
}

static void test_queue_pop_empty(void) {
    printf("Testing ar_queue_pop() from an empty queue...\n");
    
    // Given an initialized empty queue
    queue_t *queue = ar_queue_create();
    
    // When we attempt to pop from the empty queue
    const char *message = ar_queue_pop(queue);
    
    // Then the pop operation should return NULL
    assert(message == NULL);
    
    // Clean up
    ar_queue_destroy(queue);
    
    printf("Pop from empty queue test passed!\n");
}

static void test_queue_push_pop_multiple(void) {
    printf("Testing ar_queue_push() and ar_queue_pop() with multiple messages...\n");
    
    // Given an initialized queue
    queue_t *queue = ar_queue_create();
    
    // When we push multiple messages to the queue
    bool push1 = ar_queue_push(queue, g_message_1);
    bool push2 = ar_queue_push(queue, g_message_2);
    bool push3 = ar_queue_push(queue, g_message_3);
    
    // Then all pushes should succeed
    assert(push1);
    assert(push2);
    assert(push3);
    
    // And the queue should not be empty
    assert(!ar_queue_is_empty(queue));
    
    // When we pop the first message
    const char *message1 = ar_queue_pop(queue);
    
    // Then the pop should return a valid message pointer
    assert(message1 != NULL);
    
    // And the message content should match what we pushed
    assert(strcmp(message1, "Message 1") == 0);
    
    // And it should be the exact same pointer we pushed
    assert(message1 == g_message_1);
    
    // When we pop the second message
    const char *message2 = ar_queue_pop(queue);
    
    // Then the pop should return a valid message pointer
    assert(message2 != NULL);
    
    // And the message content should match what we pushed
    assert(strcmp(message2, "Message 2") == 0);
    
    // And it should be the exact same pointer we pushed
    assert(message2 == g_message_2);
    
    // When we pop the third message
    const char *message3 = ar_queue_pop(queue);
    
    // Then the pop should return a valid message pointer
    assert(message3 != NULL);
    
    // And the message content should match what we pushed
    assert(strcmp(message3, "Message 3") == 0);
    
    // And it should be the exact same pointer we pushed
    assert(message3 == g_message_3);
    
    // And the queue should be empty
    assert(ar_queue_is_empty(queue));
    
    // Clean up
    ar_queue_destroy(queue);
    
    printf("Push and pop multiple messages test passed!\n");
}

static void test_queue_wrap_around(void) {
    printf("Testing queue wrap-around behavior...\n");
    
    // Given an initialized queue
    queue_t *queue = ar_queue_create();
    
    // For test purposes, we'll define a reasonable queue size
    const int TEST_QUEUE_SIZE = 256;
    
    // Initialize message buffers
    for (int i = 0; i < TEST_QUEUE_SIZE - 1; i++) {
        sprintf(g_message_buffer[i], "Message %d", i);
    }
    
    for (int i = 0; i < (TEST_QUEUE_SIZE - 1) / 2; i++) {
        sprintf(g_message_buffer[i + TEST_QUEUE_SIZE - 1], "Wrap %d", i);
    }
    
    // And the queue is filled to near capacity
    for (int i = 0; i < TEST_QUEUE_SIZE - 1; i++) {
        assert(ar_queue_push(queue, g_message_buffer[i]));
    }
    
    // When we pop half the messages
    for (int i = 0; i < (TEST_QUEUE_SIZE - 1) / 2; i++) {
        const char *message = ar_queue_pop(queue);
        
        // Then each pop should return a valid message pointer
        assert(message != NULL);
        
        // And the messages should match what we pushed
        char expected[32];
        sprintf(expected, "Message %d", i);
        assert(strcmp(message, expected) == 0);
        
        // And it should be the exact same pointer we pushed
        assert(message == g_message_buffer[i]);
    }
    
    // When we push more messages to force wrap-around
    int start_idx = (TEST_QUEUE_SIZE - 1) / 2;
    for (int i = 0; i < (TEST_QUEUE_SIZE - 1) / 2; i++) {
        bool push_result = ar_queue_push(queue, g_message_buffer[i + TEST_QUEUE_SIZE - 1]);
        
        // Then each push should succeed
        assert(push_result);
    }
    
    // When we pop the remaining original messages
    for (int i = start_idx; i < TEST_QUEUE_SIZE - 1; i++) {
        const char *message = ar_queue_pop(queue);
        
        // Then each pop should return a valid message pointer
        assert(message != NULL);
        
        // And the messages should match the original ones
        char expected[32];
        sprintf(expected, "Message %d", i);
        assert(strcmp(message, expected) == 0);
        
        // And it should be the exact same pointer we pushed
        assert(message == g_message_buffer[i]);
    }
    
    // When we pop the wrap-around messages
    for (int i = 0; i < (TEST_QUEUE_SIZE - 1) / 2; i++) {
        const char *message = ar_queue_pop(queue);
        
        // Then each pop should return a valid message pointer
        assert(message != NULL);
        
        // And the messages should match the wrap-around ones
        char expected[32];
        sprintf(expected, "Wrap %d", i);
        assert(strcmp(message, expected) == 0);
        
        // And it should be the exact same pointer we pushed
        assert(message == g_message_buffer[i + TEST_QUEUE_SIZE - 1]);
    }
    
    // Then the queue should be empty 
    assert(ar_queue_is_empty(queue));
    
    // And popping from the empty queue should return NULL
    assert(ar_queue_pop(queue) == NULL);
    
    // Clean up
    ar_queue_destroy(queue);
    
    printf("Queue wrap-around tests passed!\n");
}

static void test_queue_full(void) {
    printf("Testing queue full behavior...\n");
    
    // Given an initialized queue
    queue_t *queue = ar_queue_create();
    
    // For test purposes, we'll define a reasonable queue size
    const int TEST_QUEUE_SIZE = 256;
    
    // Initialize message buffers if not already initialized
    for (int i = 0; i < TEST_QUEUE_SIZE; i++) {
        if (g_message_buffer[i][0] == '\0') {
            sprintf(g_message_buffer[i], "Message %d", i);
        }
    }
    
    // When we fill the queue to capacity
    for (int i = 0; i < TEST_QUEUE_SIZE; i++) {
        bool push_result = ar_queue_push(queue, g_message_buffer[i]);
        
        // Then each push should succeed
        assert(push_result);
    }
    
    // When we try to push one more message to a full queue
    bool overflow_result = ar_queue_push(queue, g_message_overflow);
    
    // Then the push operation should fail
    assert(!overflow_result);
    
    // When we pop one message
    const char *message = ar_queue_pop(queue);
    
    // Then the pop should return a valid message pointer
    assert(message != NULL);
    
    // And the message should match the first one we pushed
    assert(strcmp(message, "Message 0") == 0);
    
    // And it should be the exact same pointer we pushed
    assert(message == g_message_buffer[0]);
    
    // When we push a new message after making space
    bool replacement_result = ar_queue_push(queue, g_message_replacement);
    
    // Then the push should succeed
    assert(replacement_result);
    
    // Clean up
    ar_queue_destroy(queue);
    
    printf("Queue full tests passed!\n");
}

int main(void) {
    printf("Starting Queue Module Tests...\n");
    
    test_queue_create_destroy();
    test_queue_push_pop_single();
    test_queue_pop_empty();
    test_queue_push_pop_multiple();
    test_queue_wrap_around();
    test_queue_full();
    
    printf("All queue tests passed!\n");
    return 0;
}
