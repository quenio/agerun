# AgeRun Executable Module

## Overview

The executable module serves as the main entry point for the AgeRun runtime system. It provides the
`agerun` executable, which loads methods, restores persisted state when present, and on fresh runs
creates the boot agent that starts the bundled workflow demo. The executable also accepts an
optional `--boot-method <name-version>` override for fresh startup selection and an optional
`--no-persistence` flag for clean non-persistent runs.

## Purpose

The executable module has multiple purposes:

1. **Primary Entry Point**: Provides the `main()` function that initializes and runs the system
2. **Example Application**: Demonstrates proper usage of the AgeRun API
3. **Integration Test**: Tests the complete system including persistence across sessions
4. **Reference Implementation**: Shows best practices for memory management and error handling

## Key Functions

### Main Entry Point

```c
int main(int argc, char *argv[]);
```

The main function that:
- Parses optional process arguments such as `--boot-method <name-version>` and `--no-persistence`
- Initializes the AgeRun system and loads methods from persisted storage or the methods directory
- Creates the default `bootstrap` boot agent on fresh startup unless a valid boot override was supplied
- Processes queued messages and saves runtime state before shutdown unless persistence was disabled
- Demonstrates proper cleanup and memory management

## Implementation Details

### Program Flow

1. **Argument Parsing**
   - Accepts optional `--boot-method <name-version>` for fresh startup override
   - Accepts optional `--no-persistence` to suppress persisted methodology/agency load and save
   - Uses the default `bootstrap-1.0.0` startup path when no override is supplied

2. **Method and State Loading**
   - Loads persisted methods when available unless `--no-persistence` is active, otherwise loads `.method` files from the methods directory
   - Loads persisted agents from `agerun.agency` when available unless `--no-persistence` is active

3. **Fresh-Start Boot Selection**
   - If no agents were restored, creates a boot agent from either:
     - the default `bootstrap-1.0.0` method, or
     - the requested override method identifier
   - Queues the standard `"__boot__"` message for the selected boot agent
   - Requires any override method to handle the raw startup string safely (for example,
     `boot-echo-1.0.0` is valid, while `echo-1.0.0` by itself is not)

4. **Persistence-Aware Behavior**
   - If agents were restored, skips fresh boot-agent creation
   - If an override was requested while agents were restored, reports that the override was skipped
   - If an override is malformed or the selected method cannot be created, startup fails clearly
     without falling back to the default boot method

5. **Cleanup and Persistence**
   - Processes remaining messages
   - Saves methods and agents before shutdown unless `--no-persistence` is active
   - Leaves existing persisted files untouched when persistence is disabled
   - Ensures orderly cleanup with zero-leak expectations

### Message Examples

The executable sends several demonstration messages:

```c
// String builder demonstration
// Create the message map
ar_data_t *msg = ar_data__create_map();
ar_data_t *values = ar_data__create_list();
ar_data__list_add_last_string(values, "Hello");
ar_data__list_add_last_string(values, "World");
ar_data__set_map_string(msg, "op", "concat");
ar_data__set_map_data(msg, "values", values);
ar_agent__send(/* to */ 2, /* from */ 0, msg);
ar_data__destroy(msg);

// Message routing demonstration
ar_data_t *route_msg = ar_data__create_map();
ar_data__set_map_string(route_msg, "type", "urgent");
ar_data__set_map_string(route_msg, "content", "Alert!");
ar_agent__send(/* to */ 3, /* from */ 0, route_msg);
ar_data__destroy(route_msg);

// Agent management demonstration
ar_data_t *mgmt_msg = ar_data__create_map();
ar_data__set_map_string(mgmt_msg, "action", "create");
ar_data__set_map_string(mgmt_msg, "method", "string-builder");
ar_data__set_map_string(mgmt_msg, "version", "1.0.0");
ar_agent__send(/* to */ 4, /* from */ 0, mgmt_msg);
ar_data__destroy(mgmt_msg);
```

## Usage Example

```bash
# First run - creates the default bootstrap agent and saves state
./bin/agerun

# Fresh run with an alternate boot-safe method
./bin/agerun --boot-method boot-echo-1.0.0

# Clean one-off run that ignores persisted state and does not save it back
./bin/agerun --no-persistence

# Repository wrappers for the same behavior
make run-exec BOOT_METHOD=boot-echo-1.0.0
make run-exec NO_PERSISTENCE=1
make run-exec NO_PERSISTENCE=1 BOOT_METHOD=boot-echo-1.0.0

# Malformed or unavailable overrides fail instead of falling back
./bin/agerun --boot-method invalid
./bin/agerun --boot-method does-not-exist-1.0.0
```

## Memory Management

The executable demonstrates proper memory management:

- **Message Ownership**: Messages are owned by the agent module after sending
- **Temporary Data**: All temporary data objects are properly destroyed
- **System Cleanup**: Proper shutdown ensures all resources are freed
- **Zero Leaks**: The implementation achieves zero memory leaks

## Design Principles

The executable module follows these principles:

1. **Demonstration Focus**: Code is written to be educational and clear
2. **Error Handling**: Shows graceful handling of missing files
3. **Memory Safety**: Demonstrates proper ownership and cleanup patterns
4. **Real-World Usage**: Provides a realistic example of system usage

## Dependencies

The executable module depends on:
- `ar_system`: For system initialization and message processing
- `ar_agent`: For agent creation and message sending
- `ar_data`: For creating message data structures
- Standard C library: For printf output

## Testing

While the executable itself is not unit tested (being a main entry point), it serves as an integration test for the entire system. The `make build` target runs the executable and verifies:
- Successful execution
- No memory leaks
- Proper persistence functionality

## Implementation Notes

### Initial Agent

On a fresh run, the executable creates exactly one boot agent. By default this is the
`bootstrap-1.0.0` method. If `--boot-method <name-version>` is supplied and no persisted agents are
restored, the executable creates the requested boot agent instead and still queues the standard
`"__boot__"` startup message. Override methods therefore must be boot-capable handlers for the raw
startup string, such as `boot-echo-1.0.0`.

### Persistence Behavior

The executable supports three execution modes:
1. **Fresh Run**: Loads methods, creates a boot agent, queues `"__boot__"`, and saves state
2. **Restored Run**: Loads saved state and skips fresh boot-agent creation
3. **No-Persistence Run**: Skips persisted methodology and agency load/save, follows the fresh-start path, and leaves any existing persisted files untouched

If a boot override is requested during a restored run, the executable reports that the override was
skipped because persisted agents took precedence.

If `--no-persistence` is combined with `--boot-method <name-version>`, the executable still uses the
requested boot method because the run stays on the fresh-start path.

If a requested override is malformed or its method/version cannot be instantiated, the executable
stops startup with an explicit error and does not fall back to `bootstrap-1.0.0`.

### Message Processing

The executable shows different message processing patterns:
- Immediate processing after each send
- Batch processing of multiple messages
- Processing until queue is empty

## See Also

- [ar_system.md](ar_system.md) - Core system runtime
- [ar_agent.md](ar_agent.md) - Agent management
- [ar_agency.md](ar_agency.md) - Agent persistence
- [ar_methodology.md](ar_methodology.md) - Method management