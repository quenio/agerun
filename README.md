# Agerun

Agerun is a lightweight, message-driven agent system where each agent is defined by a single method responsible for handling all incoming messages. Agents operate within a persistent runtime environment, allowing dynamic creation, versioning, pausing, resumption, and destruction.

## Features

- **Single-Method Agents**: Each agent is defined by a single method that processes all messages.
- **Message-Driven Architecture**: Agents communicate exclusively through asynchronous message passing.
- **Versioning System**: Method definitions are versioned with backward compatibility.
- **Persistence**: Methods and agents (with their state) can be persisted and restored.
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

2. Build the library:
   ```
   make
   ```

3. Build the example application:
   ```
   make example
   ```

4. Run tests:
   ```
   make test
   ```

## Usage Examples

### Basic Usage

```c
#include <system.h>
#include <stdio.h>

int main() {
    // Define a simple echo method
    version_t echo_version = agerun_method("echo", "send(0, message)", 0, true, false);
    
    // Initialize the runtime with the echo method
    agent_id_t initial_agent = agerun_init("echo", echo_version);
    
    // Send a message to the echo agent
    agerun_send(initial_agent, "Hello, Agerun!");
    
    // Process all messages
    agerun_process_all_messages();
    
    // Shutdown the runtime
    agerun_shutdown();
    
    return 0;
}
```

### Creating Agents

```c
// Define a counter method
version_t counter_version = agerun_method("counter", 
    "if(message == \"increment\", memory[\"count\"] := memory[\"count\"] + 1, \"\")\n"
    "if(message == \"get\", send(0, build(\"Count: {}\", memory[\"count\"])), \"\")",
    0, true, true);

// Create a counter agent
agent_id_t counter_id = agerun_create("counter", counter_version, NULL);

// Send messages to the counter agent
agerun_send(counter_id, "increment");
agerun_send(counter_id, "increment");
agerun_send(counter_id, "get");

// Process all messages
agerun_process_all_messages();
```

### Persistence

```c
// Save agents and methods to disk
agerun_save_agents();
agerun_save_methods();

// Shutdown the runtime
agerun_shutdown();

// Later, in a new session:

// Load methods
agerun_load_methods();

// Initialize runtime (will also load persistent agents)
agerun_init("some_method", some_version);
```

## Method Instruction Language

Agents use a simple instruction language for their methods:

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

## License

[MIT License](LICENSE)