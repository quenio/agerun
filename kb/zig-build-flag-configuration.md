# Zig Build Flag Configuration

## Learning
Zig modules in mixed Zig/C projects need specific build flags to ensure compatibility with C modules. Key flags include optimization level (`-O Debug`), preprocessor definitions (`-DDEBUG`, `-D__ZIG__`), and C interop flags (`-lc`, `-fno-stack-check`).

## Importance
Incorrect build flags cause subtle bugs like memory tracking mismatches, optimization inconsistencies, and missing preprocessor definitions. These issues only manifest at runtime and are difficult to debug.

## Example
```makefile
# WRONG: Zig with different optimization and no DEBUG
$(ZIG) build-obj -O ReleaseSafe -target native -mcpu=native -fno-stack-check -lc -I./modules $< -femit-bin=$@

# CORRECT: Zig matching C debug build
$(ZIG) build-obj -O Debug -DDEBUG -D__ZIG__ -target native -mcpu=native -fno-stack-check -lc -I./modules $< -femit-bin=$@
```

Key flag meanings:
- `-O Debug`: No optimizations (matches C's `-O0`)
- `-DDEBUG`: Define DEBUG macro (matches C's `-DDEBUG`)
- `-D__ZIG__`: Custom flag to identify Zig compilation in headers
- `-fno-stack-check`: Disable stack protector for C compatibility
- `-lc`: Link with C standard library

## Generalization
When configuring Zig builds in mixed projects:
1. Match optimization levels between Zig and C
2. Define all preprocessor macros that C code expects
3. Add Zig-specific markers for conditional compilation
4. Ensure all build variants use consistent flags
5. Document flag purposes in Makefile comments

## Implementation
```makefile
# Define consistent flags for all Zig builds
ZIG_DEBUG_FLAGS = -O Debug -DDEBUG -D__ZIG__
ZIG_BASE_FLAGS = -target native -mcpu=native -fno-stack-check -lc -I./modules

# Apply to all pattern rules
$(RUN_TESTS_DIR)/obj/%.o: modules/%.zig | $(RUN_TESTS_DIR)
	$(ZIG) build-obj $(ZIG_DEBUG_FLAGS) $(ZIG_BASE_FLAGS) $< -femit-bin=$@
```

## Related Patterns
- [Zig-C Memory Tracking Consistency](kb/zig-c-memory-tracking-consistency.md)
- [Makefile Pattern Rule Management](kb/makefile-pattern-rule-management.md)