# ar_event Module

## Overview

The `ar_event` module provides a representation of individual events (errors, warnings, info messages) that can be logged and inspected. Events are immutable once created and capture essential information including message, type, optional position, and timestamp.

## Key Features

- **Event Types**: Support for ERROR, WARNING, and INFO event types
- **Immutable Design**: Events cannot be modified after creation
- **Timestamp**: Automatic ISO 8601 timestamp capture on creation
- **Optional Position**: Support for source position tracking (for parsers)
- **Memory Safe**: Proper ownership management with heap tracking

## Public API

### Types

```c
typedef struct ar_event_s ar_event_t;

typedef enum {
    AR_EVENT_ERROR,
    AR_EVENT_WARNING,
    AR_EVENT_INFO
} ar_event_type_t;
```

### Core Functions

#### Creation
- `ar_event_t* ar_event__create(void)` - Creates a basic event
- `ar_event_t* ar_event__create_with_message(const char *message)` - Creates event with message
- `ar_event_t* ar_event__create_typed(ar_event_type_t type, const char *message)` - Creates typed event
- `ar_event_t* ar_event__create_with_position(ar_event_type_t type, const char *message, int position)` - Creates event with position info

#### Destruction
- `void ar_event__destroy(ar_event_t *own_event)` - Destroys event and frees resources

#### Accessors
- `const char* ar_event__get_message(const ar_event_t *ref_event)` - Gets event message
- `ar_event_type_t ar_event__get_type(const ar_event_t *ref_event)` - Gets event type
- `int ar_event__get_position(const ar_event_t *ref_event)` - Gets position (-1 if not set)
- `bool ar_event__has_position(const ar_event_t *ref_event)` - Checks if position is set
- `const char* ar_event__get_timestamp(const ar_event_t *ref_event)` - Gets ISO 8601 timestamp

## Usage Example

```c
// Create an error event with message
ar_event_t *own_error = ar_event__create_typed(AR_EVENT_ERROR, "File not found");

// Create an error with position info (for parsers)
ar_event_t *own_parse_error = ar_event__create_with_position(
    AR_EVENT_ERROR, 
    "Unexpected token", 
    42
);

// Access event properties
const char *message = ar_event__get_message(own_error);
ar_event_type_t type = ar_event__get_type(own_error);
const char *timestamp = ar_event__get_timestamp(own_error);

// Clean up
ar_event__destroy(own_error);
ar_event__destroy(own_parse_error);
```

## Implementation Details

- Events store messages as owned copies (using heap tracking)
- Timestamps are generated automatically using system time
- Position tracking is optional (-1 indicates no position)
- All creation functions return NULL on allocation failure

## Dependencies

- `ar_heap` - For memory management
- Standard C libraries: `stdlib.h`, `string.h`, `time.h`, `stdbool.h`

## Design Rationale

The event module was created as part of the error handling refactoring to eliminate code duplication across evaluators and parsers. It provides a consistent way to represent diagnostic events that can be collected and analyzed by the upcoming `ar_log` module.