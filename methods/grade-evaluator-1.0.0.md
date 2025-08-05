# Grade Evaluator Method

Version: 1.0.0

## Overview

The grade-evaluator method demonstrates extensive use of the `if()` function to evaluate numeric values and return appropriate grades or status indicators.

## Message Format

The method expects a message with the following fields:
- `type`: The type of evaluation ("grade" or "status")
- `value`: The numeric value to evaluate
- `sender`: The agent ID to send the result back to

## Behavior

For type "grade":
- 90 or above: "A"
- 80-89: "B"
- 70-79: "C"
- Below 70: "F"

For type "status":
- Greater than 0: "active"
- 0 or less: "inactive"

For unknown types:
- Returns "unknown"

## Example Usage

Grade evaluation:
```
{
  "type": "grade",
  "value": 85,
  "sender": 100
}
```
Output: "B"

Status evaluation:
```
{
  "type": "status",
  "value": 5,
  "sender": 100
}
```
Output: "active"

## Implementation

```
memory.is_wake := if(message = "__wake__", 1, 0)
memory.is_sleep := if(message = "__sleep__", 1, 0)
memory.is_special := memory.is_wake + memory.is_sleep
memory.type := if(memory.is_special > 0, "none", message.type)
memory.value := if(memory.is_special > 0, 0, message.value)
memory.sender := if(memory.is_special > 0, 0, message.sender)
memory.is_grade := if(memory.type = "grade", 1, 0)
memory.is_status := if(memory.type = "status", 1, 0)
memory.grade_a := if(memory.value >= 90, 1, 0)
memory.grade_b := if(memory.value >= 80, 1, 0)
memory.grade_c := if(memory.value >= 70, 1, 0)
memory.grade := "F"
memory.grade := if(memory.grade_c = 1, "C", memory.grade)
memory.grade := if(memory.grade_b = 1, "B", memory.grade)
memory.grade := if(memory.grade_a = 1, "A", memory.grade)
memory.status := if(memory.value > 0, "active", "inactive")
memory.result := if(memory.is_grade = 1, memory.grade, "unknown")
memory.result := if(memory.is_status = 1, memory.status, memory.result)
send(memory.sender, memory.result)
```

The implementation handles special lifecycle messages (`__wake__` and `__sleep__`) which are strings, not maps. For these messages:
- The type is set to "none"
- The value is set to 0
- The sender is set to 0 (system)
- Result "unknown" is sent back
- This prevents errors when trying to access fields that don't exist on string messages

## Special Messages

The grade evaluator method handles lifecycle messages specially:
- `__wake__`: Sent back to agent 0 (system) with result "unknown"
- `__sleep__`: Sent back to agent 0 (system) with result "unknown"

These special messages are strings, not maps, so the method detects them and provides default values to avoid field access errors.

## Implementation Notes

This method showcases:
- Sequential `if()` statements to avoid nesting
- Using intermediate variables to store conditions
- Progressive refinement of results based on conditions
- The pattern of checking conditions and updating values