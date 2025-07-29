#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ar_method_fixture.h"
#include "ar_system.h"
#include "ar_agency.h"
#include "ar_data.h"

static void test_grade_evaluator_grades(void) {
    printf("Testing grade-evaluator method with grade evaluation...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("grade_evaluator_grades");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load and register grade-evaluator method
    assert(ar_method_fixture__load_method(own_fixture, "grade-evaluator", "../../methods/grade-evaluator-1.0.0.method", "1.0.0"));
    
    // Create initial memory for the agent
    ar_data_t *own_initial_memory = ar_data__create_map();
    assert(own_initial_memory != NULL);
    
    // Initialize memory fields used by the grade evaluator
    ar_data__set_map_integer(own_initial_memory, "is_grade", 0);
    ar_data__set_map_integer(own_initial_memory, "is_status", 0);
    ar_data__set_map_integer(own_initial_memory, "grade_a", 0);
    ar_data__set_map_integer(own_initial_memory, "grade_b", 0);
    ar_data__set_map_integer(own_initial_memory, "grade_c", 0);
    ar_data__set_map_string(own_initial_memory, "grade", "F");
    ar_data__set_map_string(own_initial_memory, "status", "");
    ar_data__set_map_string(own_initial_memory, "result", "");
    ar_data__set_map_string(own_initial_memory, "type", "");
    ar_data__set_map_integer(own_initial_memory, "value", 0);
    
    // Create grade-evaluator agent with initial memory
    int64_t evaluator_agent = ar_agency__create_agent("grade-evaluator", "1.0.0", own_initial_memory);
    assert(evaluator_agent > 0);
    
    // Process wake message
    printf("DEBUG: About to process wake message for agent %lld\n", (long long)evaluator_agent);
    printf("DEBUG: Agent exists: %s\n", ar_agency__agent_exists(evaluator_agent) ? "yes" : "no");
    
    // Check if agent has messages before processing
    bool has_messages = ar_agency__agent_has_messages(evaluator_agent);
    printf("DEBUG: Agent has messages: %s\n", has_messages ? "yes" : "no");
    
    bool wake_processed = ar_system__process_next_message();
    if (wake_processed) {
        printf("Wake message processed successfully\n");
    } else {
        printf("WARNING: Failed to process wake message\n");
    }
    
    // Verify agent memory was initialized
    const ar_data_t *agent_memory = ar_agency__get_agent_memory(evaluator_agent);
    assert(agent_memory != NULL);
    
    // Test case 1: Grade A (90+)
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    
    ar_data__set_map_integer(own_message, "sender", 0);
    ar_data__set_map_string(own_message, "type", "grade");
    ar_data__set_map_integer(own_message, "value", 95);
    
    bool sent = ar_agency__send_to_agent(evaluator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the grade message
    bool processed = ar_system__process_next_message();
    assert(processed);
    
    // Check the result
    const ar_data_t *type = ar_data__get_map_data(agent_memory, "type");
    if (type && ar_data__get_type(type) == AR_DATA_TYPE__STRING) {
        printf("SUCCESS: if() correctly identified type = \"%s\"\n", ar_data__get_string(type));
    }
    
    const ar_data_t *value = ar_data__get_map_data(agent_memory, "value");
    if (value && ar_data__get_type(value) == AR_DATA_TYPE__INTEGER && ar_data__get_integer(value) >= 90) {
        printf("SUCCESS: if() correctly identified value >= 90\n");
    }
    
    const ar_data_t *grade = ar_data__get_map_data(agent_memory, "grade");
    if (grade && ar_data__get_type(grade) == AR_DATA_TYPE__STRING) {
        const char *grade_str = ar_data__get_string(grade);
        printf("SUCCESS: Grade correctly set to \"%s\"\n", grade_str);
        assert(strcmp(grade_str, "A") == 0);
        printf("SUCCESS: Grade for 95: %s\n", grade_str);
    }
    
    // Test case 2: Grade B (80-89)
    ar_data_t *own_message2 = ar_data__create_map();
    assert(own_message2 != NULL);
    
    ar_data__set_map_integer(own_message2, "sender", 0);
    ar_data__set_map_string(own_message2, "type", "grade");
    ar_data__set_map_integer(own_message2, "value", 85);
    
    sent = ar_agency__send_to_agent(evaluator_agent, own_message2);
    assert(sent);
    own_message2 = NULL; // Ownership transferred
    
    processed = ar_system__process_next_message();
    assert(processed);
    
    grade = ar_data__get_map_data(agent_memory, "grade");
    assert(grade != NULL && ar_data__get_type(grade) == AR_DATA_TYPE__STRING);
    printf("Grade for 85: %s\n", ar_data__get_string(grade));
    assert(strcmp(ar_data__get_string(grade), "B") == 0);
    
    // Test case 3: Grade C (70-79)
    ar_data_t *own_message3 = ar_data__create_map();
    assert(own_message3 != NULL);
    
    ar_data__set_map_integer(own_message3, "sender", 0);
    ar_data__set_map_string(own_message3, "type", "grade");
    ar_data__set_map_integer(own_message3, "value", 75);
    
    sent = ar_agency__send_to_agent(evaluator_agent, own_message3);
    assert(sent);
    own_message3 = NULL; // Ownership transferred
    
    processed = ar_system__process_next_message();
    assert(processed);
    
    grade = ar_data__get_map_data(agent_memory, "grade");
    assert(grade != NULL && ar_data__get_type(grade) == AR_DATA_TYPE__STRING);
    printf("Grade for 75: %s\n", ar_data__get_string(grade));
    assert(strcmp(ar_data__get_string(grade), "C") == 0);
    
    // Test case 4: Grade F (below 70)
    ar_data_t *own_message4 = ar_data__create_map();
    assert(own_message4 != NULL);
    
    ar_data__set_map_integer(own_message4, "sender", 0);
    ar_data__set_map_string(own_message4, "type", "grade");
    ar_data__set_map_integer(own_message4, "value", 65);
    
    sent = ar_agency__send_to_agent(evaluator_agent, own_message4);
    assert(sent);
    own_message4 = NULL; // Ownership transferred
    
    processed = ar_system__process_next_message();
    assert(processed);
    
    grade = ar_data__get_map_data(agent_memory, "grade");
    assert(grade != NULL && ar_data__get_type(grade) == AR_DATA_TYPE__STRING);
    printf("Grade for 65: %s\n", ar_data__get_string(grade));
    assert(strcmp(ar_data__get_string(grade), "F") == 0);
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Grade evaluator grades test passed\n");
}

static void test_grade_evaluator_status(void) {
    printf("Testing grade-evaluator method with status evaluation...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("grade_evaluator_status");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load and register grade-evaluator method
    assert(ar_method_fixture__load_method(own_fixture, "grade-evaluator", "../../methods/grade-evaluator-1.0.0.method", "1.0.0"));
    
    // Create initial memory for the agent
    ar_data_t *own_initial_memory = ar_data__create_map();
    assert(own_initial_memory != NULL);
    
    // Initialize memory fields used by the grade evaluator
    ar_data__set_map_integer(own_initial_memory, "is_grade", 0);
    ar_data__set_map_integer(own_initial_memory, "is_status", 0);
    ar_data__set_map_integer(own_initial_memory, "grade_a", 0);
    ar_data__set_map_integer(own_initial_memory, "grade_b", 0);
    ar_data__set_map_integer(own_initial_memory, "grade_c", 0);
    ar_data__set_map_string(own_initial_memory, "grade", "F");
    ar_data__set_map_string(own_initial_memory, "status", "");
    ar_data__set_map_string(own_initial_memory, "result", "");
    ar_data__set_map_string(own_initial_memory, "type", "");
    ar_data__set_map_integer(own_initial_memory, "value", 0);
    
    // Create grade-evaluator agent with initial memory
    int64_t evaluator_agent = ar_agency__create_agent("grade-evaluator", "1.0.0", own_initial_memory);
    assert(evaluator_agent > 0);
    
    // Process wake message
    printf("DEBUG: About to process wake message for agent %lld\n", (long long)evaluator_agent);
    printf("DEBUG: Agent exists: %s\n", ar_agency__agent_exists(evaluator_agent) ? "yes" : "no");
    
    bool has_messages = ar_agency__agent_has_messages(evaluator_agent);
    printf("DEBUG: Agent has messages: %s\n", has_messages ? "yes" : "no");
    
    bool wake_processed = ar_system__process_next_message();
    if (wake_processed) {
        printf("Wake message processed successfully\n");
    } else {
        printf("WARNING: Failed to process wake message\n");
    }
    
    // Test case 1: Status active (value > 0)
    ar_data_t *own_message = ar_data__create_map();
    assert(own_message != NULL);
    
    ar_data__set_map_integer(own_message, "sender", 0);
    ar_data__set_map_string(own_message, "type", "status");
    ar_data__set_map_integer(own_message, "value", 5);
    
    bool sent = ar_agency__send_to_agent(evaluator_agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    bool processed = ar_system__process_next_message();
    assert(processed);
    
    const ar_data_t *agent_memory = ar_agency__get_agent_memory(evaluator_agent);
    assert(agent_memory != NULL);
    
    const ar_data_t *status = ar_data__get_map_data(agent_memory, "status");
    assert(status != NULL && ar_data__get_type(status) == AR_DATA_TYPE__STRING);
    printf("Status for value 5: %s\n", ar_data__get_string(status));
    assert(strcmp(ar_data__get_string(status), "active") == 0);
    
    // Test case 2: Status inactive (value <= 0)
    ar_data_t *own_message2 = ar_data__create_map();
    assert(own_message2 != NULL);
    
    ar_data__set_map_integer(own_message2, "sender", 0);
    ar_data__set_map_string(own_message2, "type", "status");
    ar_data__set_map_integer(own_message2, "value", 0);
    
    sent = ar_agency__send_to_agent(evaluator_agent, own_message2);
    assert(sent);
    own_message2 = NULL; // Ownership transferred
    
    processed = ar_system__process_next_message();
    assert(processed);
    
    status = ar_data__get_map_data(agent_memory, "status");
    assert(status != NULL && ar_data__get_type(status) == AR_DATA_TYPE__STRING);
    printf("Status for value 0: %s\n", ar_data__get_string(status));
    assert(strcmp(ar_data__get_string(status), "inactive") == 0);
    
    // Test case 3: Unknown type
    ar_data_t *own_message3 = ar_data__create_map();
    assert(own_message3 != NULL);
    
    ar_data__set_map_integer(own_message3, "sender", 0);
    ar_data__set_map_string(own_message3, "type", "unknown");
    ar_data__set_map_integer(own_message3, "value", 100);
    
    sent = ar_agency__send_to_agent(evaluator_agent, own_message3);
    assert(sent);
    own_message3 = NULL; // Ownership transferred
    
    processed = ar_system__process_next_message();
    assert(processed);
    
    const ar_data_t *result = ar_data__get_map_data(agent_memory, "result");
    assert(result != NULL && ar_data__get_type(result) == AR_DATA_TYPE__STRING);
    printf("Result for unknown type: %s\n", ar_data__get_string(result));
    assert(strcmp(ar_data__get_string(result), "unknown") == 0);
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Grade evaluator status test passed\n");
}

int main(void) {
    printf("Running grade-evaluator method tests...\n\n");
    
    test_grade_evaluator_grades();
    test_grade_evaluator_status();
    
    printf("\nAll grade-evaluator method tests passed!\n");
    return 0;
}
