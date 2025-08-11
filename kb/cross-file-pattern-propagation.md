# Cross-File Pattern Propagation

## Learning
Technical debt patterns, especially unnecessary code, propagate across multiple files through copy-paste programming and consistent but obsolete conventions. When developers create new test files or modules, they often copy existing ones as templates, inadvertently spreading outdated patterns throughout the codebase.

## Importance
A single obsolete pattern can multiply across dozens of files, creating widespread technical debt. Understanding how patterns propagate helps identify all instances during cleanup and prevents future spread through better template selection and code review practices.

## Example
```c
// The "Process wake message" pattern appeared in 10 method test files:

// calculator_tests.c:50
// Process wake message
ar_method_fixture__process_next_message(own_fixture);

// echo_tests.c:39  
// Process wake message
ar_method_fixture__process_next_message(own_fixture);

// grade_evaluator_tests.c:51
// Process wake message
printf("DEBUG: About to process wake message for agent %lld\n", (long long)evaluator_agent);
bool wake_processed = ar_method_fixture__process_next_message(own_fixture);

// message_router_tests.c:55-58
// Process wake messages
ar_method_fixture__process_next_message(own_fixture); // router wake
ar_method_fixture__process_next_message(own_fixture); // echo wake  
ar_method_fixture__process_next_message(own_fixture); // calc wake

// Pattern clearly copied from a template, even though wake messages no longer existed
```

## Generalization
Common propagation vectors:
1. **Copy-paste templates** - New files created by copying existing ones
2. **Consistent conventions** - "All tests must process initial messages"
3. **Defensive programming** - "Better to process even if not needed"
4. **Incomplete cleanup** - Partial removal leaves examples for copying
5. **Documentation examples** - Outdated examples in guides or comments

Pattern characteristics that promote spread:
- **Appears to work** - Doesn't cause immediate failures
- **Looks intentional** - Has comments explaining its purpose
- **Seems defensive** - Appears to handle edge cases
- **Matches context** - Fits the surrounding code structure

## Implementation
```bash
# Detecting cross-file patterns

# 1. Identify a suspicious pattern in one file
PATTERN="Process wake message"

# 2. Search for all instances across codebase
grep -r "$PATTERN" --include="*.c" --include="*.h" . | wc -l
# If count > 5, likely a propagated pattern

# 3. Analyze distribution
grep -r "$PATTERN" --include="*.c" . | cut -d: -f1 | sort | uniq -c
# Look for clusters in similar file types (all test files, all modules, etc.)

# 4. Check file creation dates to trace propagation
for file in $(grep -l "$PATTERN" *.c); do
  echo "$(git log --format="%ai" -- "$file" | tail -1) $file"
done | sort
# Earlier files are likely the source templates

# 5. Fix systematically
# Create a script to fix all instances consistently
cat > fix_pattern.sh << 'EOF'
#!/bin/bash
for file in $(grep -l "Process wake message" methods/*_tests.c); do
  sed -i.bak '/Process wake message/,+1d' "$file"
done
EOF

# 6. Prevent future propagation
# - Update template files first
# - Add checks to code review
# - Document the correct pattern prominently
```

## Related Patterns
- [Code Smell Duplicate Code](code-smell-duplicate-code.md)
- [Test Standardization Retroactive](test-standardization-retroactive.md)
- [Feature Remnant Cleanup Pattern](feature-remnant-cleanup-pattern.md)
- [Systematic Whitelist Error Resolution](systematic-whitelist-error-resolution.md)