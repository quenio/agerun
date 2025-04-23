# AgeRun Expression Module

## Overview

The Expression module is responsible for evaluating expressions in the AgeRun agent system according to the BNF grammar defined in the specification. It provides a recursive descent parser that can handle string literals, number literals, memory access paths, arithmetic expressions, and comparison expressions.

## Key Features

- Recursive descent parsing implementation for expressions
- Support for all data types: INTEGER, DOUBLE, STRING, LIST, and MAP
- Memory access using dot notation for message, memory, and context
- Arithmetic operations: addition, subtraction, multiplication, division
- Comparison operations: equals, not equals, less than, greater than, etc.

## BNF Grammar

The expression module implements the following BNF grammar as defined in the specification:

```
<expression> ::= <string-literal>
               | <number-literal>
               | <memory-access>
               | <arithmetic-expression>
               | <comparison-expression>

<string-literal> ::= '"' <characters> '"'

<number-literal> ::= <integer>
                   | <double>

<integer> ::= ['-'] <digit> {<digit>}
<double>  ::= <integer> '.' <digit> {<digit>}

<memory-access> ::= 'message' {'.' <identifier>}
                  | 'memory' {'.' <identifier>}
                  | 'context' {'.' <identifier>}

<arithmetic-expression> ::= <expression> <arithmetic-operator> <expression>
<arithmetic-operator> ::= '+' | '-' | '*' | '/'

<comparison-expression> ::= <expression> <comparison-operator> <expression>
<comparison-operator> ::= '=' | '<>' | '<' | '<=' | '>' | '>='

<identifier> ::= <letter> {<letter> | <digit> | '_'}
<characters> ::= {<any-character-except-quote>}
<digit> ::= '0' | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8' | '9'
<letter> ::= 'a' | 'b' | ... | 'z' | 'A' | 'B' | ... | 'Z'
```

Note that function calls are not part of the expression grammar. Function calls are handled at the instruction level as defined in the specification.

## API Reference

### ar_expression_evaluate

```c
data_t* ar_expression_evaluate(agent_t *agent, const data_t *message, const char *expr, int *offset);
```

Evaluates an expression in the agent's context.

**Parameters:**
- `agent`: The agent executing the expression
- `message`: The message being processed
- `expr`: The expression string to evaluate
- `offset`: Pointer to the current position in the expression string

**Returns:**
- A pointer to a new data_t containing the result of the evaluation
- NULL on syntax error (such as encountering a function call in an expression)

The offset is updated to point to the position after the evaluated expression on success, or to the position where the syntax error was detected on failure.

## Usage Examples

### Evaluating a String Literal

```c
int offset = 0;
data_t *result = ar_expression_evaluate(agent, message, "\"Hello, World!\"", &offset);
// result will contain a STRING data with value "Hello, World!"
```

### Evaluating a Number Literal

```c
int offset = 0;
data_t *result = ar_expression_evaluate(agent, message, "42", &offset);
// result will contain an INTEGER data with value 42

offset = 0;
result = ar_expression_evaluate(agent, message, "3.14159", &offset);
// result will contain a DOUBLE data with value 3.14159
```

### Evaluating Memory Access

```c
int offset = 0;
data_t *result = ar_expression_evaluate(agent, message, "memory.user.name", &offset);
// result will contain the value stored in memory.user.name, or 0 if not found
```

### Evaluating Arithmetic Expression

```c
int offset = 0;
data_t *result = ar_expression_evaluate(agent, message, "2 + 3 * 4", &offset);
// result will contain an INTEGER data with value 14
```

### Evaluating Comparison Expression

```c
int offset = 0;
data_t *result = ar_expression_evaluate(agent, message, "memory.count > 5", &offset);
// result will contain an INTEGER data with value 1 (true) or 0 (false)
```


## Implementation Notes

- The expression evaluator uses recursive descent parsing to handle nested expressions
- It properly handles precedence of operators (e.g., multiplication before addition)
- Memory access with dot notation is supported for message, memory, and context
- Type conversions are performed automatically where appropriate
- Function calls are detected and treated as syntax errors in expressions
- When a syntax error is encountered, NULL is returned and the offset points to the error location
- This ensures clear distinction between expressions and function calls as specified in the BNF grammar