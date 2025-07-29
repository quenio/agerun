# CI Debugging Artifact Upload

## Learning
When CI builds fail with issues that can't be reproduced locally, adding artifact upload to the workflow enables downloading and analyzing detailed logs from the CI environment.

## Importance
CI environments often have different configurations, library versions, or system settings that make local reproduction impossible. Artifact upload provides access to the actual CI output for debugging.

## Example
```yaml
# .github/workflows/ci.yml
- name: Full build
  run: |
    make clean build || BUILD_EXIT=$?
    make check-logs || LOGS_EXIT=$?
    exit $((BUILD_EXIT + LOGS_EXIT))

- name: Upload build logs on failure
  if: failure()
  uses: actions/upload-artifact@v4
  with:
    name: build-logs
    path: logs/
    retention-days: 7
```

Download and analyze artifacts:
```bash
# Download artifacts locally
gh run download <run-id> --name build-logs

# Analyze specific test failure
grep -B10 -A10 "Segmentation fault" build-logs/sanitize-tests.log

# Check for environment differences
grep "Running test:" build-logs/sanitize-tests.log | head -20

# Compare with local logs
diff -u logs/sanitize-tests.log build-logs/sanitize-tests.log
```

## Generalization
1. **Strategic Artifact Collection**:
   - Capture logs directory for build issues
   - Capture core dumps for segmentation faults
   - Capture generated files for compilation issues

2. **Conditional Upload**:
   - Use `if: failure()` to only upload on failures
   - Use `if: always()` for debugging intermittent issues
   - Set appropriate retention days based on debugging needs

3. **Artifact Organization**:
   ```yaml
   - name: Upload test artifacts
     if: failure()
     uses: actions/upload-artifact@v4
     with:
       name: test-results-${{ matrix.os }}-${{ matrix.compiler }}
       path: |
         logs/
         bin/*/memory_report*.log
         core.*
   ```

## Implementation
```bash
# Helper script to analyze CI artifacts
#!/bin/bash
# scripts/analyze-ci-failure.sh

RUN_ID=$1
if [ -z "$RUN_ID" ]; then
    echo "Usage: $0 <github-run-id>"
    exit 1
fi

# Download artifacts
gh run download "$RUN_ID" --name build-logs

# Common analysis
echo "=== Checking for crashes ==="
grep -r "Segmentation fault\|core dumped" build-logs/

echo "=== Checking for test failures ==="
grep -r "FAILED\|ERROR:" build-logs/ | grep -v "0 failed"

echo "=== Comparing test counts ==="
for log in build-logs/*-tests.log; do
    count=$(grep -c "Running test:" "$log" || echo 0)
    echo "$(basename "$log"): $count tests"
done
```

## Related Patterns
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Evidence Based Debugging](evidence-based-debugging.md)