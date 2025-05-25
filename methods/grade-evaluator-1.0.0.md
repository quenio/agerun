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

## Implementation Notes

This method showcases:
- Sequential `if()` statements to avoid nesting
- Using intermediate variables to store conditions
- Progressive refinement of results based on conditions
- The pattern of checking conditions and updating values