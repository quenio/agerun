# Commented Code Accumulation Antipattern

## Learning
Code that is commented out "temporarily" while waiting for future features or fixes tends to become permanent dead weight. The 77-line commented test in bootstrap_tests.c waiting for "Cycle 5" demonstrates how these blocks accumulate and never get cleaned up, adding confusion and maintenance burden.

## Importance
Commented code blocks create multiple problems: they confuse readers about whether the code is important, they don't get updated when APIs change (leading to broken code if uncommented), they pollute searches and diffs, and they suggest the codebase is unfinished or poorly maintained. Version control makes commenting unnecessary - code can be deleted and retrieved from history if needed.

## Example
```c
// bootstrap_tests.c - 77 lines of dead weight
// TODO: Re-enable this test once system can load methods from files (Cycle 5)
// This test verifies that bootstrap can spawn echo, but that requires
// the system to be able to load methods from the filesystem first.
/*
static void test_bootstrap_spawns_echo(void) {
    printf("Testing bootstrap spawns echo agent...\n");
    
    // Given a bootstrap agent with echo message as context
    ar_method_fixture_t *own_fixture = ar_method_fixture__create("bootstrap_spawns_echo");
    AR_ASSERT(own_fixture != NULL, "Fixture should be created");
    
    // ... 70+ more lines of commented code ...
    
    printf("PASS\n");
}
*/

// Problems with this:
// 1. "Cycle 5" may never come or priorities may change
// 2. APIs referenced in the code have likely changed
// 3. Confuses readers - is this planned work or abandoned?
// 4. If needed later, better to rewrite with current patterns
```

## Generalization
Signs of commented code accumulation:
1. **TODO/FIXME comments** with no timeline or ownership
2. **Version references** ("Re-enable after v2.0")
3. **Conditional comments** ("Uncomment when X is ready")
4. **Alternative implementations** ("Old way, keeping for reference")
5. **Debugging code** ("Uncomment to debug")

Better alternatives:
- **Delete it** - Version control preserves history
- **Create an issue** - Track future work properly
- **Use feature flags** - For code that needs conditional activation
- **Move to documentation** - If it's a useful example
- **Create a separate branch** - For experimental work

## Implementation
```bash
# Detecting and cleaning commented code blocks

# 1. Find large commented blocks (>10 consecutive lines)
awk '/\/\*/{flag=1; count=0} flag{count++} /\*\//{if(count>10) print FILENAME":"NR-count+1"-"NR; flag=0}' **/*.c

# 2. Find TODO/FIXME comments older than 30 days
for file in $(grep -l "TODO\|FIXME" **/*.c); do
  grep -n "TODO\|FIXME" "$file" | while read -r line; do
    line_num=$(echo "$line" | cut -d: -f1)
    last_change=$(git blame -L "$line_num,$line_num" "$file" | head -1)
    echo "$file:$line_num - $last_change"
  done
done

# 3. Find commented function definitions
grep -n "^[[:space:]]*/\*.*static.*void\|^[[:space:]]*/\*.*int\|^//.*static.*void\|^//.*int" **/*.c

# 4. Clean up process
# - Review each block for historical value
# - Check if referenced work is still planned
# - Search git history if you need to understand why it was commented
# - Delete blocks that are:
#   * Older than 6 months
#   * Reference cancelled features
#   * Have no associated issue/ticket
#   * Would need complete rewrite anyway

# 5. Prevent accumulation
# Add to code review checklist:
# [ ] No new commented code blocks without issue numbers
# [ ] No commented code without expiration date
# [ ] Consider deletion instead of commenting
```

## Related Patterns
- [Stub and Revisit Pattern](stub-and-revisit-pattern.md)
- [Feature Remnant Cleanup Pattern](feature-remnant-cleanup-pattern.md)
- [Non-Functional Code Detection Pattern](non-functional-code-detection-pattern.md)
- [Regression Test Removal Criteria](regression-test-removal-criteria.md)