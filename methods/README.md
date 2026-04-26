# AgeRun Methods Directory

This directory contains AgeRun method definitions. Each file represents a single method that can be instantiated and executed by agents in the AgeRun system.

## Available Methods

| Method | Version | Description | Documentation |
|--------|---------|-------------|---------------|
| echo | 1.0.0 | Simple echo agent that returns messages to sender | [echo-1.0.0.md](echo-1.0.0.md) |
| boot-echo | 1.0.0 | Boot-safe wrapper that handles `"__boot__"`, spawns `echo`, and queues a demo payload | [boot-echo-1.0.0.md](boot-echo-1.0.0.md) |
| calculator | 1.0.0 | Basic arithmetic calculator supporting add, subtract, multiply, divide | [calculator-1.0.0.md](calculator-1.0.0.md) |
| chat-session | 1.0.0 | Stateful chat/session backend that tracks conversation state and escalation | [chat-session-1.0.0.md](chat-session-1.0.0.md) |
| shell | 1.0.0 | Built-in shell method used by `arsh` for restricted shell parsing, session storage, and runtime interaction | [shell-1.0.0.md](shell-1.0.0.md) |
| bootstrap | 1.0.0 | Startup demo that spawns the workflow coordinator and queues the bundled workflow run | [bootstrap.md](bootstrap.md) |
| workflow-coordinator | 1.0.0 | Boot-time orchestrator that stages workflow definition readiness and startup failure handling | [workflow-coordinator-1.0.0.md](workflow-coordinator-1.0.0.md) |
| workflow-definition | 1.0.0 | Workflow definition loader/evaluator that passes item context to `complete(...)` and normalizes decisions | [workflow-definition-1.0.0.md](workflow-definition-1.0.0.md) |
| workflow-item | 1.0.0 | Stateful workflow item method that tracks lifecycle progress and summaries | [workflow-item-1.0.0.md](workflow-item-1.0.0.md) |
| workflow-reporter | 1.0.0 | Reporter method that emits user-visible progress, startup failure, and summary logs | [workflow-reporter-1.0.0.md](workflow-reporter-1.0.0.md) |

## File Naming Convention

Method files follow a strict naming convention:

```
<method-name>-<semver>.method
```

Where:
- `<method-name>` is the name of the method (alphanumeric characters and underscores)
- `<semver>` is the semantic version in the format `MAJOR.MINOR.PATCH` (e.g., `1.0.0`)
- `.method` is the required file extension

### Examples

- `echo-1.0.0.method` - Echo method version 1.0.0
- `boot-echo-1.0.0.method` - Boot-safe echo wrapper method version 1.0.0
- `calculator-2.1.3.method` - Calculator method version 2.1.3
- `data_processor-0.5.0.method` - Data processor method version 0.5.0

## Method File Format

Each `.method` file is a plain text file containing AgeRun instruction code. A method definition consists of a sequence of instructions, with each instruction on a separate line.

### Structure

According to the AgeRun grammar:
- Each instruction must be on its own line
- Instructions cannot be combined on the same line
- Even the last instruction must be followed by a newline
- Empty lines are ignored

### Instruction Types

Instructions can be one of two types:

1. **Assignment**: Stores a value in memory
   ```
   memory.variable := expression
   ```

2. **Function Instruction**: Calls a system function (optionally storing the result)
   ```
   function(parameters)
   memory.result := function(parameters)
   ```

### Example Method Files

**Simple Echo Method** (`echo-1.0.0.method`):
```
send(message.sender, message.content)
```

**Boot Echo Method** (`boot-echo-1.0.0.method`):
```
memory.is_boot := if(message = "__boot__", 1, 0)
memory.method_name := if(memory.is_boot = 1, "echo", "")
memory.method_version := "1.0.0"
memory.sender := 0
memory.content := "boot_echo_ready"
memory.echo_spawn_result := spawn(memory.method_name, memory.method_version, context)
memory.echo_id := if(memory.is_boot = 1, memory.echo_spawn_result, memory.echo_id)
memory.echo_input := build("sender={sender} content={content}", memory)
memory.echo_payload := parse("sender={sender} content={content}", memory.echo_input)
memory.echo_sent := send(memory.echo_id * memory.is_boot, memory.echo_payload)
memory.demo_status := if(memory.is_boot = 1, "Boot echo queued", "Boot echo skipped")
```

**Calculator Method** (`calculator-1.0.0.method`):
```
memory.result := 0
memory.result := if(message.operation = "add", message.a + message.b, memory.result)
memory.result := if(message.operation = "multiply", message.a * message.b, memory.result)
memory.result := if(message.operation = "subtract", message.a - message.b, memory.result)
memory.result := if(message.operation = "divide", message.a / message.b, memory.result)
send(message.sender, memory.result)
```

**Workflow Bootstrap Method** (`bootstrap-1.0.0.method`):
```
memory.is_boot := if(message = "__boot__", 1, 0)
memory.coordinator_spawn_method := if(memory.is_boot = 1, "workflow-coordinator", 0)
memory.coordinator_spawn_result := spawn(memory.coordinator_spawn_method, "1.0.0", context)
memory.coordinator_id := if(memory.is_boot = 1, memory.coordinator_spawn_result, memory.coordinator_id)
memory.start_input := build("action=start definition_method_name=workflow-definition definition_method_version=1.0.0 definition_path=workflows/default.workflow reporter_method_name=workflow-reporter reporter_method_version=1.0.0 item_id=demo-item-1 title=demo_work_item priority=high owner=workflow_owner review_status=approved", memory)
memory.start_message := parse("action={action} definition_method_name={definition_method_name} definition_method_version={definition_method_version} definition_path={definition_path} reporter_method_name={reporter_method_name} reporter_method_version={reporter_method_version} item_id={item_id} title={title} priority={priority} owner={owner} review_status={review_status}", memory.start_input)
send(memory.coordinator_id, memory.start_message)
```

### Available Instructions

Methods can use the following system functions:

- **send(agent_id, message)**: Send a message to another agent
- **parse(template, input)**: Extract values from strings based on templates
- **build(template, values)**: Construct strings by replacing placeholders
- **method(name, instructions, version)**: Define a new method
- **spawn(method_name, version, context)**: Spawn a new agent instance
- **exit(agent_id)**: Exit an agent
- **deprecate(method_name, version)**: Deprecate a method version
- **if(condition, true_value, false_value)**: Conditional evaluation

### Expressions

Expressions can include:
- **String literals**: `"Hello, World"`
- **Number literals**: `42`, `3.14`
- **Memory access**: `memory.variable`, `message.field`, `context.setting`
- **Arithmetic**: `a + b`, `x * y`, `m - n`, `p / q`
- **Comparisons**: `a = b`, `x <> y`, `m < n`, `p <= q`, `r > s`, `t >= u`
- **String concatenation**: `"Hello, " + name`

### Special Messages

Methods should handle these lifecycle messages:
- `__wake__`: Sent when an agent is created or resumed
- `__sleep__`: Sent before an agent is paused or destroyed
- `__boot__`: Sent by the executable to the selected fresh-start boot method; only boot-capable methods such as `bootstrap-1.0.0` or `boot-echo-1.0.0` should be used with executable boot overrides

## Version Management

Methods follow semantic versioning:
- **MAJOR**: Incompatible changes to method behavior
- **MINOR**: New functionality added in a backward-compatible manner
- **PATCH**: Backward-compatible bug fixes

Multiple versions of the same method can coexist, allowing agents to use specific versions as needed.

## Usage

Methods in this directory are loaded by the AgeRun methodology system and can be instantiated by agents using:

```
agent("method_name", "version", context)
```

The system will look for the corresponding file in this directory based on the method name and version specified.

## Testing

Each method in this directory has corresponding test files that verify the method's behavior. Tests follow the same naming convention but with a `_tests.c` suffix.

### Test Files

Test files are written in C and are located in the same directory as the method files:
- `echo_tests.c` - Tests for the echo method
- `calculator_tests.c` - Tests for the calculator method

### Running Tests

Method tests are integrated into the main test suite and can be run using:

```bash
# Run all tests including method tests
make run-tests

# Run tests with memory sanitization
make sanitize-tests

# Run static analysis on test code
make analyze-tests
```

### Test Structure

Each test file:
1. Reads the method definition from the `.method` file
2. Creates the method in the methodology system
3. Instantiates an agent with the method
4. Sends test messages to the agent
5. Verifies the agent behaves correctly
6. Cleans up resources

### Writing New Tests

When creating a new method, also create a corresponding test file. The test should:

1. Include necessary headers:
   ```c
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <assert.h>
   #include "ar_method_fixture.h"
   #include "ar_system.h"
   #include "ar_agent.h"
   #include "ar_data.h"
   ```

2. Use the test fixture module for setup and teardown (no helper functions needed)

3. Create test functions for different scenarios:
   - Basic functionality tests
   - Edge case tests
   - Error handling tests

4. Use assertions to verify expected behavior

5. Use `ar_method_fixture__check_memory()` to ensure zero memory leaks

### Example Test Pattern

```c
static void test_method_basic(void) {
    printf("Testing method basic functionality...\n");
    
    // Create test fixture
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("method_basic");
    assert(own_fixture != NULL);
    
    // Initialize test environment
    assert(ar_method_fixture__initialize(own_fixture));
    
    // Verify correct directory
    assert(ar_method_fixture__verify_directory(own_fixture));
    
    // Load method
    assert(ar_method_fixture__load_method(own_fixture, "method-name", 
                                       "../methods/method-name-1.0.0.method", "1.0.0"));
    
    // Create agent
    int64_t agent = ar_agent__create("method-name", "1.0.0", NULL);
    assert(agent > 0);
    
    // Process wake message
    ar_system__process_next_message(own_system));
    
    // When we send a test message
    ar_data_t *own_message = ar_data__create_string("test");
    bool sent = ar_agent__send(agent, own_message);
    assert(sent);
    own_message = NULL; // Ownership transferred
    
    // Process the message
    bool processed = ar_system__process_next_message(own_system));
    assert(processed);
    
    // Then verify expected behavior
    // ...
    
    // Check for memory leaks
    assert(ar_method_fixture__check_memory(own_fixture));
    
    // Destroy fixture (handles all cleanup)
    ar_method_fixture__destroy(own_fixture);
    
    printf("✓ Method basic test passed\n");
}

int main(void) {
    printf("Running method-name tests...\n\n");
    
    test_method_basic();
    
    printf("\nAll tests passed!\n");
    return 0;
}
```

### Zero Memory Leaks

All method tests must maintain the project's zero memory leaks standard. Use the heap tracking macros (`AR_HEAP_MALLOC`, `AR_HEAP_FREE`) and verify with `make sanitize-tests`.
