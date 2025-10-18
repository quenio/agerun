# Cross-Platform Bash Script Patterns

## Learning
Bash scripts that work correctly on macOS may fail on Linux (and vice versa) due to differences between GNU utilities (Linux) and BSD utilities (macOS). The most common issues occur with `sed`, shell pattern matching operators, and command-line option syntax. These differences are silent—scripts may run without errors but execute wrong code paths or produce unexpected results.

## Importance
Cross-platform compatibility issues are particularly dangerous because:
1. **Silent failures**: Scripts don't error—they just do the wrong thing
2. **Platform-specific bugs**: Issues only manifest on specific OS versions
3. **Hidden in production**: Testing on one platform doesn't catch the problems
4. **Affects automation**: CI/CD, deployment scripts, and build systems are affected
5. **Difficult to debug**: Error messages don't indicate the real problem

## Example

### Problem 1: OSTYPE Pattern Matching

```bash
# BAD: Doesn't work on macOS
# REASON: [ ] with = operator doesn't support glob patterns
if [ "$OSTYPE" = "darwin"* ]; then
    echo "macOS"  # This branch never executes!
else
    echo "other"
fi

# GOOD: Works on both macOS and Linux
if [[ "$OSTYPE" == darwin* ]]; then
    echo "macOS"
else
    echo "other"
fi
```

### Problem 2: sed Delimiter and Syntax

```bash
# BAD: May fail with special characters in pattern
# REASON: Using / as delimiter conflicts with paths/special chars
sed -i "s/STEP_${i}=.*/STEP_${i}=complete/" file.txt

# GOOD: Works on both platforms, safer delimiter
# REASON: @ delimiter is standard in sed, no conflicts
sed -i "s@STEP_${i}=.*@STEP_${i}=complete@" file.txt

# BAD: macOS sed requires empty string argument
# REASON: GNU sed (Linux) and BSD sed (macOS) have different syntax
sed -i "s/pattern/replacement/" file.txt  # Works on Linux only
sed -i '' "s/pattern/replacement/" file.txt  # Works on macOS only

# GOOD: Use variable for platform-specific options
if [[ "$OSTYPE" == darwin* ]]; then
    SED_OPTS="-i ''"
else
    SED_OPTS="-i"
fi
sed $SED_OPTS "s@pattern@replacement@" file.txt
```

### Problem 3: grep and awk Differences

```bash
# BAD: GNU grep extended regex syntax
grep -E "^(foo|bar)" file.txt  # Fails on macOS

# GOOD: POSIX-compatible basic regex
grep "^foo\|^bar" file.txt  # Works on both

# Better: Use standard grep without extended regex
grep "^foo" file.txt
grep "^bar" file.txt

# For complex patterns, use awk
awk '/^(foo|bar)/ { print }' file.txt  # Works on both
```

## Generalization

### Key Platform Differences

**BSD sed (macOS) vs GNU sed (Linux):**
- macOS: `sed -i '' "s/pattern/replacement/"` (empty string required)
- Linux: `sed -i "s/pattern/replacement/"` (no argument needed)
- Both: Support `@` as delimiter: `sed -i.bak "s@pattern@replacement@"`

**Shell Pattern Matching:**
- POSIX `[ ]`: Only supports `=` and `!=` operators (no patterns)
- Bash `[[ ]]`: Supports `==` and `!=` with glob patterns like `darwin*`

**bash vs sh:**
- Makefiles use POSIX sh, not bash
- Scripts can use bash-specific features like `[[ ]]`
- Avoid bash-isms in shebang lines that might be invoked as sh

### Safe Cross-Platform Patterns

1. **OSTYPE Detection**:
```bash
if [[ "$OSTYPE" == darwin* ]]; then
    # macOS-specific
else
    # Linux-specific
fi
```

2. **sed with Safe Delimiter**:
```bash
# Use @ or | instead of / to avoid conflicts
sed -i "s@pattern@replacement@" file.txt
sed -i "s|pattern|replacement|" file.txt  # Also works
```

3. **Platform-Specific Options**:
```bash
if [[ "$OSTYPE" == darwin* ]]; then
    SED_OPTS="-i ''"
    TAR_OPTS="jf"  # Different options on macOS
else
    SED_OPTS="-i"
    TAR_OPTS="jf"  # Same as macOS for tar
fi

sed $SED_OPTS "s@pattern@replacement@" file.txt
```

4. **Error Handling**:
```bash
set -e  # Exit on any error
set -o pipefail  # Exit if any pipe command fails
```

## Implementation

### Complete Example: Platform-Aware Script

```bash
#!/bin/bash
set -e
set -o pipefail

# Detect platform and set options
if [[ "$OSTYPE" == darwin* ]]; then
    echo "Running on macOS"
    SED_OPTS="-i ''"
    # macOS-specific code here
else
    echo "Running on Linux"
    SED_OPTS="-i"
    # Linux-specific code here
fi

# Now use variables for platform-independent operations
sed $SED_OPTS "s@pattern@replacement@" myfile.txt

# Exit cleanly
exit 0
```

### Testing for Cross-Platform Compatibility

```bash
# Test on macOS
OSTYPE="darwin24.0" bash script.sh

# Test on Linux
OSTYPE="linux-gnu" bash script.sh

# Test in containers
docker run ubuntu:latest bash script.sh  # Linux
docker run -it --platform linux/amd64 ubuntu bash script.sh
```

### Real AgeRun Example: Checkpoint Updates

```bash
#!/bin/bash
set -e

COMMAND_NAME="${1:-check-docs}"
STEP_NUMBER="${2:-1}"
STATUS="${3:-complete}"
TRACKING_FILE="/tmp/${COMMAND_NAME}_progress.txt"

# Platform-specific sed options
if [[ "$OSTYPE" == darwin* ]]; then
    SED_OPTS="-i ''"
else
    SED_OPTS="-i"
fi

# Extract step description
STEP_DESC=$(grep "STEP_${STEP_NUMBER}=" "$TRACKING_FILE" | sed 's/.*# //')

# Update step status with safe @ delimiter
sed $SED_OPTS "s@STEP_${STEP_NUMBER}=.*@STEP_${STEP_NUMBER}=${STATUS}    # ${STEP_DESC}@" "$TRACKING_FILE"

echo "✅ Step $STEP_NUMBER marked as $STATUS"
```

## Common Gotchas

1. **Hardcoding macOS sed syntax**: Will fail on Linux
2. **Using [ ] for pattern matching**: Use [[ ]] for glob patterns
3. **Assuming grep/awk options work everywhere**: Test extensively
4. **Not handling errors**: Always use `set -e`
5. **Testing on one platform only**: Always test on both platforms
6. **Ignoring variable quoting**: Quote all variables: `"$var"` not `$var`

## Verification Checklist

- [ ] Uses [[ ]] for OSTYPE pattern matching
- [ ] Uses @ or | delimiter for sed, not /
- [ ] Sets platform-specific options in variables
- [ ] Includes `set -e` and `set -o pipefail`
- [ ] Tested on both macOS and Linux
- [ ] No hardcoded paths or platform-specific tools
- [ ] All variables properly quoted
- [ ] Error messages help diagnose platform issues

## Related Patterns
- [Makefile POSIX Shell Compatibility](makefile-posix-shell-compatibility.md)
- [Bash Command Parsing Patterns](bash-command-parsing-patterns.md)
- [Shell Loading Order and Tool Detection](shell-loading-order-tool-detection.md)
