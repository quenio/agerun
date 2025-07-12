# Memory Leak Detection Workflow

## Learning
Each test generates its own memory report file at `bin/memory_report_<test_name>.log` for precise leak tracking

## Importance
Enables pinpointing memory leaks to specific tests rather than searching through generic reports

## Example
```bash
# Run specific test
make ar_string_tests

# Check its specific memory report
cat bin/memory_report_ar_string_tests.log

# Check for any leaks across all tests
grep "Actual memory leaks:" bin/memory_report_*.log | grep -v "0 (0 bytes)"
```

## Generalization
Generate specific diagnostic outputs per test/component rather than aggregated reports

## Implementation
- Individual test memory reports: `bin/memory_report_<test_name>.log`
- Full test suite: Check console for "WARNING: X memory leaks detected"
- Complete verification: `grep "Actual memory leaks:" bin/memory_report_*.log | grep -v "0 (0 bytes)"`
- CI visibility: full_build.sh prints leak reports to stdout
- Use `AGERUN_MEMORY_REPORT` environment variable for custom report paths

## Related Patterns
- Zero memory leak tolerance
- Per-component diagnostics
- Automated testing workflows
- Debug-friendly tooling design