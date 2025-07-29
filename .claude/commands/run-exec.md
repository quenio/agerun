Build and run the agerun executable.

```bash
make run-exec 2>&1
```

Note: Using `2>&1` to capture stderr ensures memory leak warnings and error messages are visible.