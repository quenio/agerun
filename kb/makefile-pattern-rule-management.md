# Makefile Pattern Rule Management

## Learning
When updating compilation flags in Makefiles with multiple build targets, all pattern rules must be updated consistently. Missing even one target creates inconsistent builds that fail mysteriously.

## Importance
Build systems often have multiple targets (debug, release, sanitizer, etc.) each with their own pattern rules. Inconsistent updates cause different behavior across targets, making bugs appear and disappear based on which target is used.

## Example
```makefile
# Multiple pattern rules for different build directories
$(RUN_TESTS_DIR)/obj/%.o: modules/%.zig | $(RUN_TESTS_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ ...

$(RUN_EXEC_DIR)/obj/%.o: modules/%.zig | $(RUN_EXEC_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ ...

$(SANITIZE_TESTS_DIR)/obj/%.o: modules/%.zig | $(SANITIZE_TESTS_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ ...

$(SANITIZE_EXEC_DIR)/obj/%.o: modules/%.zig | $(SANITIZE_EXEC_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ ...

$(TSAN_TESTS_DIR)/obj/%.o: modules/%.zig | $(TSAN_TESTS_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ ...

$(TSAN_EXEC_DIR)/obj/%.o: modules/%.zig | $(TSAN_EXEC_DIR)
	$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ ...
```

All 6 pattern rules needed the same update!

## Generalization
Best practices for Makefile maintenance:
1. Use variables for common flag sets
2. Document why each flag exists
3. Group related pattern rules together
4. Use make functions to reduce duplication
5. Test all targets after changes

Refactoring approach:
```makefile
# Define common flags once
ZIG_DEBUG_FLAGS = -O Debug -DDEBUG -D__ZIG__
ZIG_COMMON_FLAGS = -target native -mcpu=native -fno-stack-check -lc -I./modules

# Use in all rules
$(RUN_TESTS_DIR)/obj/%.o: modules/%.zig | $(RUN_TESTS_DIR)
	$(ZIG) build-obj $(ZIG_DEBUG_FLAGS) $(ZIG_COMMON_FLAGS) $< -femit-bin=$@
```

## Implementation
To update pattern rules systematically:
```bash
# Find all pattern rules for a file type
grep -n "\.zig.*:" Makefile

# Count occurrences to ensure all are updated
grep "build-obj.*-O.*ReleaseSafe" Makefile | wc -l

# Use sed with careful patterns
sed -i 's/-O ReleaseSafe/-O Debug -DDEBUG -D__ZIG__/g' Makefile

# Verify changes
git diff Makefile
```

## Related Patterns
- [Zig Build Flag Configuration](zig-build-flag-configuration.md)