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
memory.is_grade := if(message.type = "grade", 1, 0)
memory.is_status := if(message.type = "status", 1, 0)
memory.grade_a := if(message.value >= 90, 1, 0)
memory.grade_b := if(message.value >= 80, 1, 0)
memory.grade_c := if(message.value >= 70, 1, 0)
memory.grade := "F"
memory.grade := if(memory.grade_c = 1, "C", memory.grade)
memory.grade := if(memory.grade_b = 1, "B", memory.grade)
memory.grade := if(memory.grade_a = 1, "A", memory.grade)
memory.status := if(message.value > 0, "active", "inactive")
memory.result := if(memory.is_grade = 1, memory.grade, "unknown")
memory.result := if(memory.is_status = 1, memory.status, memory.result)
send(message.sender, memory.result)
```

The implementation evaluates the type field to determine whether to perform grade or status evaluation. It then checks the value against appropriate thresholds and returns the corresponding result.

## Notes

The grade evaluator demonstrates extensive use of conditional logic with the if() function to handle multiple evaluation criteria.

## Implementation Notes

This method showcases:
- Sequential `if()` statements to avoid nesting
- Using intermediate variables to store conditions
- Progressive refinement of results based on conditions
- The pattern of checking conditions and updating values