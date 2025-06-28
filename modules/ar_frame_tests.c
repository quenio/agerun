#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "ar_frame.h"
#include "ar_data.h"
#include "ar_heap.h"

// Test basic creation and destruction
static void test_frame__create_destroy(void) {
    printf("Testing ar_frame__create and ar_frame__destroy...\n");
    
    // Given valid memory, context, and message
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *context = ar__data__create_map();
    assert(context != NULL);
    
    data_t *message = ar__data__create_string("test message");
    assert(message != NULL);
    
    // When we create a frame
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    
    // Then it should be created successfully
    assert(frame != NULL);
    
    // And when we destroy it
    ar_frame__destroy(frame);
    
    // Cleanup the data objects (frame doesn't own them)
    ar__data__destroy(memory);
    ar__data__destroy(context);
    ar__data__destroy(message);
    
    printf("ar_frame__create and ar_frame__destroy test passed!\n");
}

// Test destruction of NULL frame
static void test_frame__destroy_null(void) {
    printf("Testing ar_frame__destroy with NULL...\n");
    
    // When we destroy a NULL frame
    ar_frame__destroy(NULL);
    
    // Then it should handle it gracefully (no crash)
    
    printf("ar_frame__destroy NULL test passed!\n");
}

// Test creation with NULL memory should fail
static void test_frame__create_with_null_memory(void) {
    printf("Testing ar_frame__create with NULL memory...\n");
    
    // Given valid context and message but NULL memory
    data_t *context = ar__data__create_map();
    assert(context != NULL);
    
    data_t *message = ar__data__create_string("test");
    assert(message != NULL);
    
    // When we try to create a frame with NULL memory
    ar_frame_t *frame = ar_frame__create(NULL, context, message);
    
    // Then it should fail
    assert(frame == NULL);
    
    // Cleanup
    ar__data__destroy(context);
    ar__data__destroy(message);
    
    printf("ar_frame__create NULL memory test passed!\n");
}

// Test creation with NULL context should fail
static void test_frame__create_with_null_context(void) {
    printf("Testing ar_frame__create with NULL context...\n");
    
    // Given valid memory and message but NULL context
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *message = ar__data__create_string("test");
    assert(message != NULL);
    
    // When we try to create a frame with NULL context
    ar_frame_t *frame = ar_frame__create(memory, NULL, message);
    
    // Then it should fail
    assert(frame == NULL);
    
    // Cleanup
    ar__data__destroy(memory);
    ar__data__destroy(message);
    
    printf("ar_frame__create NULL context test passed!\n");
}

// Test creation with NULL message should fail
static void test_frame__create_with_null_message(void) {
    printf("Testing ar_frame__create with NULL message...\n");
    
    // Given valid memory and context but NULL message
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *context = ar__data__create_map();
    assert(context != NULL);
    
    // When we try to create a frame with NULL message
    ar_frame_t *frame = ar_frame__create(memory, context, NULL);
    
    // Then it should fail
    assert(frame == NULL);
    
    // Cleanup
    ar__data__destroy(memory);
    ar__data__destroy(context);
    
    printf("ar_frame__create NULL message test passed!\n");
}

// Test getting memory from frame
static void test_frame__get_memory(void) {
    printf("Testing ar_frame__get_memory...\n");
    
    // Given a frame with known memory
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    ar__data__set_map_data(memory, "test", ar__data__create_integer(42));
    
    data_t *context = ar__data__create_map();
    assert(context != NULL);
    
    data_t *message = ar__data__create_string("test");
    assert(message != NULL);
    
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    assert(frame != NULL);
    
    // When we get the memory
    data_t *retrieved_memory = ar_frame__get_memory(frame);
    
    // Then it should be the same memory we passed in
    assert(retrieved_memory == memory);
    
    // And we should be able to access its contents
    data_t *value = ar__data__get_map_data(retrieved_memory, "test");
    assert(value != NULL);
    assert(ar__data__get_integer(value) == 42);
    
    // Cleanup
    ar_frame__destroy(frame);
    ar__data__destroy(memory);
    ar__data__destroy(context);
    ar__data__destroy(message);
    
    printf("ar_frame__get_memory test passed!\n");
}

// Test getting context from frame
static void test_frame__get_context(void) {
    printf("Testing ar_frame__get_context...\n");
    
    // Given a frame with known context
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *context = ar__data__create_map();
    assert(context != NULL);
    ar__data__set_map_data(context, "config", ar__data__create_string("value"));
    
    data_t *message = ar__data__create_string("test");
    assert(message != NULL);
    
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    assert(frame != NULL);
    
    // When we get the context
    const data_t *retrieved_context = ar_frame__get_context(frame);
    
    // Then it should be the same context we passed in
    assert(retrieved_context == context);
    
    // And we should be able to access its contents
    data_t *value = ar__data__get_map_data(context, "config");
    assert(value != NULL);
    assert(strcmp(ar__data__get_string(value), "value") == 0);
    
    // Cleanup
    ar_frame__destroy(frame);
    ar__data__destroy(memory);
    ar__data__destroy(context);
    ar__data__destroy(message);
    
    printf("ar_frame__get_context test passed!\n");
}

// Test getting message from frame
static void test_frame__get_message(void) {
    printf("Testing ar_frame__get_message...\n");
    
    // Given a frame with known message
    data_t *memory = ar__data__create_map();
    assert(memory != NULL);
    
    data_t *context = ar__data__create_map();
    assert(context != NULL);
    
    data_t *message = ar__data__create_string("hello world");
    assert(message != NULL);
    
    ar_frame_t *frame = ar_frame__create(memory, context, message);
    assert(frame != NULL);
    
    // When we get the message
    const data_t *retrieved_message = ar_frame__get_message(frame);
    
    // Then it should be the same message we passed in
    assert(retrieved_message == message);
    
    // And we should be able to access its contents
    assert(strcmp(ar__data__get_string(retrieved_message), "hello world") == 0);
    
    // Cleanup
    ar_frame__destroy(frame);
    ar__data__destroy(memory);
    ar__data__destroy(context);
    ar__data__destroy(message);
    
    printf("ar_frame__get_message test passed!\n");
}

int main(void) {
    printf("Starting frame module tests...\n\n");
    
    test_frame__create_destroy();
    test_frame__destroy_null();
    test_frame__create_with_null_memory();
    test_frame__create_with_null_context();
    test_frame__create_with_null_message();
    test_frame__get_memory();
    test_frame__get_context();
    test_frame__get_message();
    
    printf("\nAll frame tests passed!\n");
    return 0;
}