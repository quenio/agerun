# Makefile Environment Variable Directory Target Dependencies

## Learning  
When overriding Makefile directory variables via environment variables, manually creating those directories before running make can prevent the target recipes from executing because make treats manually created directories as "up to date" and skips the recipe that creates subdirectories.

## Importance
This gotcha causes subtle build failures where directory creation recipes are silently skipped, leading to missing subdirectories and "No such file or directory" errors during compilation. Understanding this prevents wasted debugging time on seemingly mysterious build failures.

## Example
```makefile
# Makefile target that should create directory with subdirs
$(RUN_EXEC_DIR):
	mkdir -p $@/obj

# Pattern rule that depends on the directory
$(RUN_EXEC_DIR)/obj/%.o: modules/%.c | $(RUN_EXEC_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
```

**Problem scenario:**
```bash
# Override directory via environment variable
export RUN_EXEC_DIR=/tmp/custom_build

# Manually create the directory (THIS IS THE MISTAKE)
mkdir -p /tmp/custom_build

# Run make - the recipe is skipped because directory "exists"
make run_exec_lib
# Result: "No such file or directory" errors for /tmp/custom_build/obj/
```

**Solution:**
```bash
# Let make handle ALL directory creation
export RUN_EXEC_DIR=/tmp/custom_build
# Don't create the directory manually
make run_exec_lib  # Now the recipe runs and creates obj/ subdirectory
```

## Generalization
Make target evaluation rules:
1. **Target exists**: If target file/directory exists, make checks if it's newer than dependencies
2. **Manual creation**: Manually created directories appear "up to date" to make
3. **Recipe skipping**: Make skips recipes for targets it considers current
4. **Environment override**: Environment variables change target paths but not evaluation logic

**Prevention pattern:**
```makefile
# Use .PHONY for setup targets that should always run
.PHONY: setup-dirs
setup-dirs:
	mkdir -p $(BUILD_DIR)/obj $(BUILD_DIR)/lib

# Or use order-only prerequisites with full path specification
$(BUILD_DIR)/obj/%.o: modules/%.c | $(BUILD_DIR)/obj
$(BUILD_DIR)/obj:
	mkdir -p $@
```

## Implementation
```bash
# Correct approach for environment override
export CUSTOM_BUILD_DIR="/tmp/isolated_build_$$"
# Let make create the directory structure
make build-target

# Alternative: Force recreation with .PHONY
make clean  # Remove any existing directories first
export BUILD_DIR="/tmp/custom"
make .PHONY-setup-dirs build-target
```

**Root cause**: Make's timestamp-based evaluation doesn't distinguish between manually created directories and recipe-created directories. When you manually create a directory that a Makefile recipe is supposed to create, make sees the directory as already "built" and skips the recipe.

## Error Patterns Indicating This Issue
```bash
# Build output showing the problem:
clang: error: unable to open output file '/tmp/custom/obj/module.o': 'No such file or directory'
# Directory exists: /tmp/custom
# Directory missing: /tmp/custom/obj
# Make output: "make[1]: '/tmp/custom' is up to date."
```

## Related Patterns
- [Makefile Pattern Rule Management](makefile-pattern-rule-management.md)
- [Makefile POSIX Shell Compatibility](makefile-posix-shell-compatibility.md)
- [Build System Exit Code Verification](build-system-exit-code-verification.md)