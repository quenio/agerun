# Script Debugging Through Isolation

## Learning
When debugging complex shell scripts, reproducing the error in isolation is far more effective than studying the code. Creating a minimal test case that reproduces the problem reveals the root cause quickly. This technique is especially useful for sed, awk, and other text processing tools where the actual error messages can be cryptic or misleading.

## Importance
Debugging script errors without isolation leads to:
1. **Wasted time**: Reading code without understanding the actual failure
2. **Missed patterns**: Similar bugs hiding in other scripts go unfound
3. **Incomplete fixes**: Fixing the symptom rather than the root cause
4. **Fragile solutions**: Fixes that work in one context but not others

Isolation-based debugging leads to:
1. **Root cause discovery**: Understanding the actual problem
2. **Comprehensive fixes**: Finding all instances of the pattern
3. **Reusable solutions**: Fixes that work across the entire codebase
4. **Confidence**: Verification that the fix actually works

## Example

### Bad: Debug by Code Reading

```bash
# Studying checkpoint-update.sh
# Line 36: sed -i '' "s/STEP_${STEP_NUMBER}=.*/STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}/" "$TRACKING_FILE"
# Looks correct to me... Let me add debug output
```

### Good: Debug by Isolation

```bash
# Reproduce the error in isolation
cat > /tmp/test_checkpoint.sh << 'EOF'
#!/bin/bash
COMMAND_NAME="check-docs"
TRACKING_FILE="/tmp/${COMMAND_NAME}-test-progress.txt"
STEP_NUMBER=1
STATUS="complete"

# Create minimal tracking file
cat > "$TRACKING_FILE" << 'TRACK'
STEP_1=pending    # Initial Check
STEP_2=pending    # Preview Fixes
TRACK

# Extract step description
STEP_DESC=$(grep "STEP_${STEP_NUMBER}=" "$TRACKING_FILE" | sed 's/.*# //')
echo "STEP_DESC: '$STEP_DESC'"

# Try the sed command
if [ "$OSTYPE" = "darwin"*  ]; then
    sed -i '' "s/STEP_${STEP_NUMBER}=.*/STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}/" "$TRACKING_FILE"
else
    sed -i "s/STEP_${STEP_NUMBER}=.*/STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}/" "$TRACKING_FILE"
fi

echo "Result:"
cat "$TRACKING_FILE"
rm "$TRACKING_FILE"
EOF

bash /tmp/test_checkpoint.sh
# Output: sed: 1: "/tmp/check-docs_test ...": command c expects \ followed by text
# NOW: I can see the error clearly! The problem is... let me test further
```

## Generalization

### Isolation-Based Debugging Process

1. **Identify the failing command**: Pinpoint which exact line causes the problem
2. **Extract the essential parts**: Remove everything not needed to reproduce the error
3. **Create a minimal test case**: Smallest possible script that shows the error
4. **Test systematically**: Change one thing at a time
5. **Verify the fix**: Confirm the fix actually works
6. **Search for patterns**: Find all similar issues across the codebase
7. **Apply comprehensive fix**: Fix all instances, not just the first one

### Debugging Sed Errors

```bash
# When you see: sed: command c expects \ followed by text

# Step 1: Isolate the sed command
PATTERN="to_find"
REPLACEMENT="to_replace_with"
FILE="test.txt"

# Step 2: Test with simple delimiter (/)
sed "s/$PATTERN/$REPLACEMENT/" "$FILE"  # May fail

# Step 3: Test with alternative delimiter (@)
sed "s@$PATTERN@$REPLACEMENT@" "$FILE"  # May work

# Step 4: Test with OSTYPE detection
if [[ "$OSTYPE" == darwin* ]]; then
    echo "macOS: Need to test sed -i '' syntax"
    sed -i '' "s@$PATTERN@$REPLACEMENT@" "$FILE"
else
    echo "Linux: Use sed -i syntax"
    sed -i "s@$PATTERN@$REPLACEMENT@" "$FILE"
fi

# Step 5: Verify the fix with a test file
cat > /tmp/test.txt << 'EOF'
STEP_1=pending    # Initial
STEP_2=pending    # Preview
EOF

sed "s@STEP_1=.*@STEP_1=complete    # Initial@" /tmp/test.txt
# Success! The @ delimiter and updated sed command works
```

### Debugging Pattern Matching Issues

```bash
# When pattern matching doesn't work as expected

# Step 1: Test the pattern directly
OSTYPE="darwin24.0"

# Step 2: Try different operators
[ "$OSTYPE" = "darwin"* ] && echo "Match with =" || echo "No match with ="
# Output: No match with = (because = doesn't support patterns!)

[ "$OSTYPE" = "darwin24.0" ] && echo "Exact match works" || echo "No exact match"
# Output: Exact match works

# Step 3: Use proper operator for patterns
[[ "$OSTYPE" == darwin* ]] && echo "Match with [[ ==" || echo "No match"
# Output: Match with [[ == (works!)

# Lesson: [ ] with = operator doesn't support glob patterns
#         Use [[ ]] with == operator for pattern matching
```

## Implementation

### Debugging Template

```bash
#!/bin/bash
# Script debugging template using isolation

# 1. Define what we're testing
TARGET_COMMAND="sed -i '' \"s/PATTERN/REPLACEMENT/\""
TEST_NAME="Checkpoint sed command on macOS"

# 2. Create minimal test case
setup_test() {
    mkdir -p /tmp/debug-test
    cat > /tmp/debug-test/input.txt << 'EOF'
STEP_1=pending    # Initial Check
STEP_2=pending    # Preview Fixes
EOF
    echo "Created test file: /tmp/debug-test/input.txt"
}

# 3. Run the test
run_test() {
    echo "Testing: $TARGET_COMMAND"
    # Your problematic command here
    OSTYPE="darwin24.0"

    # Test different approaches
    echo "Attempt 1: Using / delimiter"
    sed -i '' "s/STEP_1=.*/STEP_1=complete/" /tmp/debug-test/input.txt 2>&1 || echo "FAILED"

    echo "Attempt 2: Using @ delimiter"
    sed -i '' "s@STEP_1=.*@STEP_1=complete@" /tmp/debug-test/input.txt 2>&1 && echo "SUCCESS"
}

# 4. Verify the results
verify_test() {
    echo "File contents after test:"
    cat /tmp/debug-test/input.txt
}

# 5. Clean up
cleanup_test() {
    rm -rf /tmp/debug-test
    echo "Cleaned up test files"
}

# Execute the test sequence
setup_test
run_test
verify_test
cleanup_test
```

### Real AgeRun Example 1: Checkpoint Script Debugging

The checkpoint scripts use sed for step tracking. When debugging issues:

```bash
# Test checkpoint update in isolation
COMMAND="test-cmd"
STEP=1
DESC="Step 1 Description"
TRACKING_FILE="/tmp/${COMMAND}-progress.txt"

# Create minimal tracking file
cat > "$TRACKING_FILE" << 'EOF'
STEP_1=pending    # Step 1 Description
STEP_2=pending    # Step 2 Description
EOF

# Test the sed pattern used in checkpoint-update.sh
echo "Testing checkpoint sed pattern..."
sed "s@STEP_${STEP}=.*@STEP_${STEP}=complete    # ${DESC}@" "$TRACKING_FILE"
# Output: STEP_1=complete    # Step 1 Description

# Verify the pattern works with platform-specific sed
if [[ "$OSTYPE" == darwin* ]]; then
    echo "Running on macOS - test with sed -i ''"
    sed -i '' "s@STEP_${STEP}=.*@STEP_${STEP}=complete    # ${DESC}@" "$TRACKING_FILE"
else
    echo "Running on Linux - test with sed -i"
    sed -i "s@STEP_${STEP}=.*@STEP_${STEP}=complete    # ${DESC}@" "$TRACKING_FILE"
fi

cat "$TRACKING_FILE"
rm "$TRACKING_FILE"
```

### Real AgeRun Example 2: Shell Script Analysis

When we encountered the checkpoint sed error:

```bash
# Step 1: Identified failing command in checkpoint-update.sh:36
sed -i '' "s/STEP_${STEP_NUMBER}=.*/STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}/" "$TRACKING_FILE"

# Step 2: Created isolation test with actual values
STEP_NUMBER=1
STEP_DESC="Initial Check"
STATUS="complete"
TRACKING_FILE="/tmp/test-tracking.txt"

# Step 3: Tested systematically
# Test 1: Does the basic pattern work?
echo "STEP_1=pending" | sed "s/STEP_${STEP_NUMBER}=.*/STEP_${STEP_NUMBER}=${STATUS}/"
# Result: Works!

# Test 2: Does the full pattern work?
echo "STEP_1=pending    # Initial Check" | sed "s/STEP_${STEP_NUMBER}=.*/STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}/"
# Result: ERROR - special chars in replacement causing issues

# Test 3: Different delimiter?
echo "STEP_1=pending    # Initial Check" | sed "s@STEP_${STEP_NUMBER}=.*@STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}@"
# Result: Works perfectly!

# Lesson learned: @ delimiter is safer for complex replacements
```

## Benefits of Isolation

1. **Fast diagnosis**: Root cause found in minutes, not hours
2. **Confident fixes**: Verification before applying to production code
3. **Reusable patterns**: Solution applies to all similar code
4. **Educational**: Team learns the actual problem, not speculation
5. **Testable**: Can add as permanent test case to prevent regression

## Verification Checklist

- [ ] Created minimal test case that reproduces the error
- [ ] Tested with one variable changed at a time
- [ ] Verified fix works in isolation
- [ ] Searched for same pattern in other scripts
- [ ] Applied fix comprehensively across codebase
- [ ] Tested fix on target platforms
- [ ] Added verification to catch regressions

## Related Patterns
- [Evidence-Based Debugging](evidence-based-debugging.md)
- [Bash Command Parsing Patterns](bash-command-parsing-patterns.md)
- [Cross-Platform Bash Script Patterns](cross-platform-bash-script-patterns.md)
