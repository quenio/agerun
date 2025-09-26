# Development Debug Tools

## Learning
Comprehensive debugging tools and techniques for AgeRun development, from memory analysis to crash debugging.

## Importance
Having the right debugging tools and knowing how to use them saves hours of development time and helps maintain code quality.

## Example
```bash
# Memory debugging
make sanitize-tests  # Run with AddressSanitizer
# Check individual test reports:
cat bin/memory_report_ar_string_tests.log

# Static analysis
make analyze-exec  # Requires scan-build (brew install llvm)

# Crash debugging with abort traps
lldb -o "run" -o "bt" -o "quit" ./bin/ar_string_tests

# Verify debug build
strings bin/*/agerun | grep DEBUG

# Test failures - often directory issues
# 4-step check: pwd → cd /path → pwd → run
cd /Users/quenio/Repos/agerun/bin && ./ar_string_tests

# Pattern testing before use
echo "test string" | sed 's/test/TEST/' # Test sed pattern
echo "ar_foo__bar" | grep -o "ar_[^_]*"  # Test grep pattern

# Environment variables for memory debugging
ASAN_OPTIONS=halt_on_error=0 ./bin/ar_string_tests  # Continue after first error
ASAN_OPTIONS=detect_leaks=1:leak_check_at_exit=1 ./bin/ar_string_tests  # Complex leaks
```

## Generalization
- **Memory**: Use ASan and per-test memory reports for leak detection
- **Static Analysis**: Use scan-build for compile-time bug detection
- **Crashes**: Use lldb for backtraces and debugging
- **Build verification**: Check binary contents to confirm build flags
- **Test failures**: Verify working directory as first debugging step
- **Pattern testing**: Always test regex/sed/awk patterns in isolation first
- **Doc validation**: `make build` catches documentation errors early
- **CI debugging**: Add log capture for better visibility in automated builds

## Implementation
1. **Memory leak workflow**:
   - Run `make sanitize-tests`
   - Check `bin/memory_report_*.log` files
   - Use `grep "Actual memory leaks:" bin/memory_report_*.log | grep -v "0 (0 bytes)"`

2. **Crash debugging workflow**:
   - Reproduce crash consistently
   - Run under lldb with backtrace
   - Check for memory corruption with ASan
   - Verify ownership patterns

3. **Static analysis workflow**:
   - Install scan-build if needed
   - Run `make analyze-exec`
   - Fix high-priority issues first
   - Re-run to verify fixes

4. **CI debugging enhancement**:
   ```bash
   # In build.sh or CI scripts:
   make analyze-exec 2>&1 | tee analyze.log || {
       echo "Static analysis failed, showing log:"
       cat analyze.log
       exit 1
   }
   ```

## Related Patterns
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Absolute Path Navigation](absolute-path-navigation.md)
- [Shell Configuration Diagnostic Troubleshooting](shell-configuration-diagnostic-troubleshooting.md)
- [Configuration Migration Troubleshooting Strategy](configuration-migration-troubleshooting-strategy.md)