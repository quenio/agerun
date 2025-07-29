Run thread sanitizer on all tests for detecting data races.

```bash
make tsan-tests 2>&1
```

Note: Using `2>&1` to capture stderr ensures all sanitizer output and warnings are visible.