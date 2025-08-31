Run static analysis on the test code.

## Command
```bash
make analyze-tests 2>&1
```

## Expected Output

### Success State
```
Running static analysis on tests...
Analyzing ar_agent_tests.c...
Analyzing ar_agency_tests.c...
Analyzing ar_data_tests.c...
... (65 more test files)

Static analysis complete!
No issues found in test code.
```

### Warning States

**Unused Variables:**
```
Running static analysis on tests...
Analyzing ar_data_tests.c...
ar_data_tests.c:45:10: warning: unused variable 'result' [-Wunused-variable]
    int result = 42;
        ^~~~~~

Analyzing ar_string_tests.c...
ar_string_tests.c:123:15: warning: unused variable 'buffer' [-Wunused-variable]
    char buffer[256];
         ^~~~~~

Static analysis complete!
Found 2 warnings in test code.
```

**Potential NULL Dereference:**
```
Running static analysis on tests...
Analyzing ar_agent_tests.c...
ar_agent_tests.c:78:5: warning: Access to field 'id' results in a dereference of a null pointer
    agent->id = 123;
    ^~~~~~~~~

Static analysis complete!
Found 1 potential bug in test code.
make: *** [analyze-tests] Error 1
```

**Resource Leak:**
```
Running static analysis on tests...
Analyzing ar_data_tests.c...
ar_data_tests.c:156:1: warning: Potential leak of memory pointed to by 'data'
}
^

Static analysis complete!
Found 1 potential leak in test code.
```

## Key Points

- **Clang Static Analyzer** checks for bugs without running code
- **Common issues**: NULL dereferences, resource leaks, dead code
- **Test-specific checks**: Ensures test quality and correctness
- **Non-blocking warnings** but bugs should be fixed
- **Always use `2>&1`** to capture all analyzer output