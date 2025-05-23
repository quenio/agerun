# AgeRun Methods Directory

This directory contains AgeRun method definitions. Each file represents a single method that can be instantiated and executed by agents in the AgeRun system.

## Available Methods

| Method | Version | Description | Documentation |
|--------|---------|-------------|---------------|
| echo | 1.0.0 | Simple echo agent that returns messages to sender | [echo-1.0.0.md](echo-1.0.0.md) |
| calculator | 1.0.0 | Basic arithmetic calculator supporting add, subtract, multiply, divide | [calculator-1.0.0.md](calculator-1.0.0.md) |

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
send(sender, message)
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

**Agent Spawner Method** (`spawner-1.0.0.method`):
```
memory.child_id := agent(message.method_name, message.version, message.context)
send(memory.child_id, "__wake__")
memory.success := if(memory.child_id > 0, true, false)
send(message.sender, memory.success)
```

### Available Instructions

Methods can use the following system functions:

- **send(agent_id, message)**: Send a message to another agent
- **parse(template, input)**: Extract values from strings based on templates
- **build(template, values)**: Construct strings by replacing placeholders
- **method(name, instructions, version)**: Define a new method
- **agent(method_name, version, context)**: Create a new agent instance
- **destroy(agent_id)**: Destroy an agent
- **destroy(method_name, version)**: Destroy a method version
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
make test

# Run tests with memory sanitization
make test-sanitize

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
   #include "agerun_system.h"
   #include "agerun_agent.h"
   #include "agerun_methodology.h"
   #include "agerun_data.h"
   #include "agerun_io.h"
   #include "agerun_heap.h"
   ```

2. Implement a `read_method_file()` helper function to load the method

3. Create test functions for different scenarios:
   - Basic functionality tests
   - Edge case tests
   - Error handling tests

4. Use assertions to verify expected behavior

5. Clean up all resources to maintain zero memory leaks

### Example Test Pattern

```c
static void test_method_basic(void) {
    printf("Testing method basic functionality...\n");
    
    // Given the method file
    char *own_instructions = read_method_file("../methods/method-1.0.0.method");
    assert(own_instructions != NULL);
    
    // When we create the method and initialize the system
    bool method_created = ar_methodology_create_method("method", own_instructions, "1.0.0");
    assert(method_created);
    AR_HEAP_FREE(own_instructions);
    
    agent_id_t agent = ar_system_init("method", "1.0.0");
    assert(agent > 0);
    
    // Process wake message
    ar_system_process_next_message();
    
    // When we send a test message
    data_t *own_message = ar_data_create_string("test");
    bool sent = ar_agent_send(agent, own_message);
    assert(sent);
    
    // Process the message
    bool processed = ar_system_process_next_message();
    assert(processed);
    
    // Then verify expected behavior
    // ...
    
    // Cleanup
    ar_system_shutdown();
    ar_methodology_cleanup();
    
    printf("✓ Method basic test passed\n");
}
```

### Zero Memory Leaks

All method tests must maintain the project's zero memory leaks standard. Use the heap tracking macros (`AR_HEAP_MALLOC`, `AR_HEAP_FREE`) and verify with `make test-sanitize`.