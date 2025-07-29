Run static analysis on the test code.

```bash
make analyze-tests 2>&1
```

Note: Using `2>&1` to capture stderr ensures all analyzer warnings are visible.