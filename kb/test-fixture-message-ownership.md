# Test Fixture Message Ownership

## Learning
Test fixtures that directly call `ar_interpreter__execute_method` must manage message ownership themselves. Unlike the system which automatically handles ownership, fixtures bypass this flow and can expose messages to being claimed by the expression evaluator, causing corruption.

## Importance
- Prevents hard-to-debug message corruption where MAP messages become INTEGER 0
- Ensures test fixtures accurately replicate production behavior
- Avoids ownership gaps that allow expression evaluator to claim messages
- Critical for any test code that bypasses normal system message flow

## Example
```c
// WRONG: Fixture passes message without ownership management
ar_data_t *own_message = ar_data__create_map();
ar_data__set_map_string(own_message, "text", "Hello");

// This allows expression evaluator to claim the message during wake handling
bool result = ar_interpreter__execute_method(
    fixture->own_interpreter,
    agent_id,
    own_message  // No owner - vulnerable to being claimed!
);
ar_data__destroy(own_message);  // May crash if evaluator claimed it

// CORRECT: Test takes ownership before passing to fixture
ar_data_t *own_message = ar_data__create_map();
ar_data__set_map_string(own_message, "text", "Hello");

// Take ownership to prevent evaluator from claiming it
ar_data__take_ownership(own_message, own_fixture);

// Now safe to execute
int64_t temp_agent_id = ar_interpreter_fixture__execute_with_message(
    own_fixture,
    "memory.greeting := \"Message says: \" + message.text",
    own_message
);

// Clean up with ownership check
ar_data__destroy_if_owned(own_message, own_fixture);
```

## Generalization
1. **System flow**: System takes ownership → passes to interpreter → destroys after
2. **Fixture requirement**: Must replicate system's ownership management
3. **Ownership pattern**: `take_ownership` before execution, `destroy_if_owned` after
4. **Applies to**: Any test code calling interpreter methods directly
5. **Debug sign**: "Claimed ownership of MAP" in logs indicates missing ownership

## Implementation
```c
// Pattern for test fixtures with messages
void test_with_message(ar_interpreter_fixture_t *own_fixture) {
    // Create message
    ar_data_t *own_message = ar_data__create_map();
    ar_data__set_map_string(own_message, "field", "value");
    
    // CRITICAL: Take ownership before execution
    ar_data__take_ownership(own_message, own_fixture);
    
    // Execute through fixture
    int64_t agent_id = fixture_execute_with_message(
        own_fixture, 
        instruction, 
        own_message
    );
    
    // CRITICAL: Destroy with ownership check
    ar_data__destroy_if_owned(own_message, own_fixture);
    
    // Continue with verification...
}

// System pattern shown in ar_system__process_next_message:
// 1. Dequeue message from agent (agent drops ownership)
// 2. System takes ownership of the dequeued message
ar_data__take_ownership(own_message, mut_system);

// 3. Execute with owned message
ar_interpreter__execute_method(mut_system->own_interpreter, agent_id, own_message);

// 4. System destroys the message
ar_data__destroy_if_owned(own_message, mut_system);
```

## Related Patterns
- [Message Ownership Flow](message-ownership-flow.md)
- [Test Fixture API Adaptation](test-fixture-api-adaptation.md)
- [Expression Evaluator Claim Behavior](expression-evaluator-claim-behavior.md)
- [Ownership Gap Vulnerability](ownership-gap-vulnerability.md)