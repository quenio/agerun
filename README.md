# AgeRun

AgeRun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Features

- **Single-Method Agents**: Each agent is defined by a single method that processes all messages.
- **Message-Driven Architecture**: Agents communicate exclusively through asynchronous message passing.
- **Versioning System**: Method definitions are versioned with backward compatibility.
- **Persistence**: Methodology (method definitions) and agency (agents with their state) can be persisted and restored.
- **Dynamic Agent Creation**: Agents can create other agents at runtime.
- **Minimal Memory Footprint**: Designed to be lightweight and efficient.

## Building from Source

### Prerequisites

- C compiler (gcc-13)
- Make

### Build Steps

1. Clone the repository:
   ```
   git clone https://github.com/quenio/agerun.git
   cd agerun
   ```

2. Build the library and executable:
   ```
   make
   ```

3. Run tests:
   ```
   make run-tests
   ```

4. Run the executable:
   ```
   make run-exec
   ```

### Memory Safety Testing

AgeRun includes built-in support for comprehensive memory error detection:

#### Runtime Detection with Address Sanitizer (ASan)

Note: On macOS, the build system automatically uses clang for sanitizer builds due to better AddressSanitizer support. On Linux, gcc-13 is used for sanitizer builds.

1. Build with Address Sanitizer enabled:
   ```
   make sanitize
   ```

2. Run tests with memory error detection:
   ```
   make sanitize-tests
   ```

3. Run the executable with memory error detection:
   ```
   make sanitize-exec
   ```

#### Static Analysis with Clang Static Analyzer

1. Run static analysis on executable code:
   ```
   make analyze-exec
   ```

2. Run static analysis on tests too:
   ```
   make analyze-tests
   ```

3. View results:
   - If scan-build is installed: HTML reports in `bin/scan-build-results`
   - Without scan-build: Direct analyzer warnings in the console output

Note: The makefile automatically detects if scan-build is available and falls back to direct clang analysis if needed.

These tools help detect memory issues such as:
- Memory leaks
- Use-after-free errors
- Buffer overflows
- Double-free errors
- Memory ownership violations
- Null pointer dereferences

Developers are encouraged to run both dynamic (ASan) and static analysis regularly during development.

## Usage Examples

### Basic Usage

```c
#include "ar_system.h"
#include <stdio.h>

int main(void) {
    // Define a simple echo method with semantic version "1.0.0"
    const char *echo_version = ar_method__create("echo", "send(0, message)", "1.0.0");

    // Initialize the runtime with the echo method
    int64_t initial_agent = ar_system__init("echo", echo_version);

    // Send a message to the echo agent
    ar_agent__send(initial_agent, "Hello, AgeRun!");

    // Process all messages
    ar_system__process_all_messages();

    // Shutdown the runtime
    ar_system__shutdown();

    return 0;
}
```

### Creating Agents

```c
// Define a counter method
const char *counter_version = ar_method__create("counter",
    "if(message = \"increment\", memory.count := memory.count + 1, \"\")\n"
    "if(message = \"get\", send(0, build(\"Count: {}\", memory.count)), \"\")",
    "1.0.0");

// Create a counter agent
int64_t counter_id = ar_agent__create("counter", counter_version, NULL);

// Send messages to the counter agent
ar_agent__send(counter_id, "increment");
ar_agent__send(counter_id, "increment");
ar_agent__send(counter_id, "get");

// Process all messages
ar_system__process_all_messages();
```

### Persistence

```c
// Save agents and methods to disk
ar_agency__save_agents();
ar_methodology__save_methods();

// Shutdown the runtime
ar_system__shutdown();

// Later, in a new session:

// Load methods
ar_methodology__load_methods();

// Initialize runtime (will also load persistent agents)
ar_system__init("some_method", some_version);
```

## Method Expressions and Instructions

Agents use a simple expression and instruction language for their methods:

### String Operations

- `parse(template, input)`: Extract values from input based on template placeholders
- `build(template, values)`: Build a string by inserting values into template placeholders

### Messaging

- `send(agent_id, message)`: Send a message to another agent

### Memory Operations

- Read: `memory.key` or nested paths like `memory.user.name`
- Write: `memory.key := value` or nested paths like `memory.user.settings.theme := "dark"`

### Arithmetic Operations

- **Operators**: Standard arithmetic operators: `+`, `-`, `*`, `/`
- **Type Inference**: When parsing strings, values are inferred as integer, double, or string

### Control Flow

- `if(condition, true_value, false_value)`: Conditional evaluation

### Agent Management

- `method(method_name, instructions, version)`: Define a new method with semantic versioning
- `agent(method_name, version, context)`: Create a new agent using a method with specific version
- `destroy(agent_id)`: Destroy an agent

## Code Organization

The AgeRun codebase is organized into modular components, each responsible for a specific aspect of the system:

- **Map Module**: A foundational key-value store with no dependencies
- **List Module**: Doubly-linked list implementation that provides stack and queue operations with an opaque type
- **Data Module**: Type-safe data storage and reference management built on the Map Module
- **Semver Module**: Semantic versioning support for method versioning and compatibility checking
- **Agent Module**: Agent lifecycle and message handling
- **Method Module**: Method definition, versioning, and execution
- **Expression Module**: Expression parsing and evaluation with proper ownership semantics
- **Instruction Module**: Instruction execution engine that processes agent method code
- **System Module**: High-level API for the runtime environment

For more details on the module structure and dependencies, see the [Modules README](modules/README.md).

## Method Files

AgeRun method definitions can be stored as standalone `.method` files in the [methods](methods/) directory. These files provide a way to define and share reusable agent behaviors outside of the runtime environment.

Each method file follows a specific naming convention and format that includes:
- Semantic versioning in the filename (e.g., `echo-1.0.0.method`)
- Plain text instruction code using the AgeRun instruction language
- One instruction per line with proper newline termination

For detailed information about method file format and examples, see the [Methods README](methods/README.md).

## Memory Management

AgeRun implements a comprehensive memory safety system with zero tolerance for memory leaks. The system features:

### Memory Ownership Model
- **Owned Values**: Objects with unique ownership that must be explicitly destroyed
- **Mutable References**: Read-write access to objects without ownership  
- **Borrowed References**: Read-only access to objects without ownership
- **Strict ownership semantics** with explicit prefixes (`own_`, `mut_`, `ref_`) throughout the codebase

### Comprehensive Memory Tracking
- **Heap tracking system** using `AR_HEAP_MALLOC`, `AR_HEAP_FREE`, and `AR_HEAP_STRDUP` macros
- **Zero memory leaks** across all modules confirmed through rigorous testing
- **Automatic memory reporting** via `heap_memory_report.log` after program execution
- **Memory leak detection** catches both actual leaks and intentional test leaks

### Memory Safety Testing
The project includes multiple layers of memory safety verification:
- **Address Sanitizer (ASan)** for runtime memory error detection
- **Static analysis** with Clang Static Analyzer for compile-time checks
- **Comprehensive test coverage** with memory leak verification for every module
- **Agent lifecycle memory management** ensuring proper message queue cleanup

All modules have been converted to use heap tracking and achieve zero memory leaks, making AgeRun suitable for long-running, memory-critical applications.

For comprehensive documentation on memory ownership patterns, see the [Memory Management Model](MMM.md).

## License

[MIT License](LICENSE)