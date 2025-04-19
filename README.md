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

- C compiler (gcc or clang)
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
   make test
   ```

4. Run the executable:
   ```
   make run
   ```

## Usage Examples

### Basic Usage

```c
#include "agerun_system.h"
#include <stdio.h>

int main(void) {
    // Define a simple echo method
    version_t echo_version = ar_method_create("echo", "send(0, message)", 0, true, false);
    
    // Initialize the runtime with the echo method
    agent_id_t initial_agent = ar_system_init("echo", echo_version);
    
    // Send a message to the echo agent
    ar_agent_send(initial_agent, "Hello, AgeRun!");
    
    // Process all messages
    ar_system_process_all_messages();
    
    // Shutdown the runtime
    ar_system_shutdown();
    
    return 0;
}
```

### Creating Agents

```c
// Define a counter method
version_t counter_version = ar_method_create("counter", 
    "if(message == \"increment\", memory[\"count\"] := memory[\"count\"] + 1, \"\")\n"
    "if(message == \"get\", send(0, build(\"Count: {}\", memory[\"count\"])), \"\")",
    0, true, true);

// Create a counter agent
agent_id_t counter_id = ar_agent_create("counter", counter_version, NULL);

// Send messages to the counter agent
ar_agent_send(counter_id, "increment");
ar_agent_send(counter_id, "increment");
ar_agent_send(counter_id, "get");

// Process all messages
ar_system_process_all_messages();
```

### Persistence

```c
// Save agents and methods to disk
ar_agency_save_agents();
ar_methodology_save_methods();

// Shutdown the runtime
ar_system_shutdown();

// Later, in a new session:

// Load methods
ar_methodology_load_methods();

// Initialize runtime (will also load persistent agents)
ar_system_init("some_method", some_version);
```

## Method Expressions and Instructions

Agents use a simple expression and instruction language for their methods:

### String Operations

- `parse(template, input)`: Extract values from input based on template placeholders
- `build(template, values)`: Build a string by inserting values into template placeholders

### Messaging

- `send(agent_id, message)`: Send a message to another agent

### Memory Operations

- Read: `memory["key"]` or using shorthand `key`
- Write: `memory["key"] := value` or using shorthand `key := value`

### Control Flow

- `if(condition, true_value, false_value)`: Conditional evaluation

### Agent Management

- `method(name, instructions, previous_version, backward_compatible, persist)`: Define a new method
- `create(method_name, version, context)`: Create a new agent
- `destroy(agent_id)`: Destroy an agent

## Code Organization

The AgeRun codebase is organized into modular components, each responsible for a specific aspect of the system:

- **Map Module**: A foundational key-value store with no dependencies
- **Data Module**: Type-safe data storage and reference management built on the Map Module
- **Queue Module**: Message queue implementation for agent communication
- **Agent Module**: Agent lifecycle and message handling
- **Method Module**: Method definition, versioning, and execution
- **System Module**: High-level API for the runtime environment

For more details on the module structure and dependencies, see the [Modules README](modules/README.md).

## License

[MIT License](LICENSE)