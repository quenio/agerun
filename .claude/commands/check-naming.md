Run naming convention validation to check for typedef and function naming issues.

## Command
```bash
make check-naming
```

## Expected Output

### Success State
```
Checking naming conventions...
  Checking typedef naming (ar_*_t pattern)...
  Checking function naming (ar_*__ pattern)...
  Checking static function naming (_* pattern)...
  Checking test function naming (test_*__ pattern)...
  Checking heap macro naming (AR__HEAP__* pattern)...

All naming conventions are correct!
```

### Failure States

**Typedef Naming Violation:**
```
Checking naming conventions...
  Checking typedef naming (ar_*_t pattern)...
    ERROR: Invalid typedef name: AgentData (should be ar_agent_data_t)
    ERROR: Invalid typedef name: string_buffer_t (should be ar_string_buffer_t)
  Found 2 typedef naming violations

make: *** [check-naming] Error 1
```

**Function Naming Violation:**
```
Checking naming conventions...
  Checking function naming (ar_*__ pattern)...
    ERROR: Invalid function name: ar_data_create (should be ar_data__create)
    ERROR: Invalid function name: ar__string__trim (should be ar_string__trim)
  Found 2 function naming violations

make: *** [check-naming] Error 1
```

**Static Function Violation:**
```
Checking naming conventions...
  Checking static function naming (_* pattern)...
    ERROR: Static function without underscore: validate_input (should be _validate_input)
    ERROR: Non-static with underscore: _ar_data__internal (should not start with _)
  Found 2 static function naming violations

make: *** [check-naming] Error 1
```

## Key Points

- **Enforces strict naming conventions** across entire codebase
- **Exit code 1** on any violation - blocks commits
- **Patterns checked**:
  - Typedefs: `ar_<module>_t`
  - Functions: `ar_<module>__<function>`
  - Static functions: `_<function>`
  - Test functions: `test_<module>__<test>`
  - Heap macros: `AR__HEAP__<OPERATION>`