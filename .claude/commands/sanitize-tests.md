Run address sanitizer on all tests for detecting memory issues.

```bash
make sanitize-tests 2>&1
```

Note: Using `2>&1` to capture stderr ensures all sanitizer output and warnings are visible.