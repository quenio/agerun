# Memory Leak Detection Workflow

## Learning
Each test generates its own memory report file at `bin/run-tests/memory_report_<test_name>.log` for precise leak tracking

## Importance
Enables pinpointing memory leaks to specific tests rather than searching through generic reports

## Example
```bash
# Run specific test
make ar_string_tests

# Check its specific memory report
cat bin/run-tests/memory_report_ar_string_tests.log

# Check for any leaks across all tests
grep "Actual memory leaks:" bin/run-tests/memory_report_*.log | grep -v "0 (0 bytes)"
```

## Generalization
Generate specific diagnostic outputs per test/component rather than aggregated reports

## Implementation
- Individual test memory reports: `bin/run-tests/memory_report_<test_name>.log`
- **Note**: Reports are in subdirectories based on build target:
  - `bin/run-tests/memory_report_*.log` for normal tests
  - `bin/sanitize-tests/memory_report_*.log` for sanitizer tests
- Full test suite: Check console for "WARNING: X memory leaks detected"
- Complete verification: `grep "Actual memory leaks:" bin/run-tests/memory_report_*.log | grep -v "0 (0 bytes)"`
- **Individual verification (CRITICAL)**: Check each report separately, not just counts:
  ```bash
  # List all reports with their leak status
  for report in bin/run-tests/memory_report_*.log; do
    echo -n "$report: "
    grep "Actual memory leaks:" "$report"
  done
  ```
- CI visibility: build.sh prints leak reports to stdout
- Use `AGERUN_MEMORY_REPORT` environment variable for custom report paths

## Related Patterns
- Zero memory leak tolerance
- Per-component diagnostics
- Automated testing workflows
- Debug-friendly tooling design