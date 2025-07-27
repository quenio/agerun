# Zig Test Build Integration

## Learning
Zig's built-in test framework can be integrated into existing C-based build systems by using `zig test` command directly and differentiating test types through naming patterns.

## Importance
Enables unified test execution across mixed C/Zig codebases without requiring separate test commands or build systems. Maintains consistency in CI/CD pipelines.

## Example
```makefile
# Zig test source files
ZIG_TEST_SRC = $(wildcard modules/*Tests.zig)
ZIG_TEST_BIN = $(patsubst modules/%Tests.zig,bin/%Tests,$(ZIG_TEST_SRC))

# Combined test list
ALL_TEST_BIN_NAMES = $(C_TEST_NAMES) $(ZIG_TEST_BIN_NAMES)

# Unified test execution
run-tests:
	@for test in $(ALL_TEST_BIN_NAMES); do \
		case "$$test" in \
			*Tests) \
				./$$test || echo "ERROR: $$test failed"; \
				;; \
			*) \
				AGERUN_MEMORY_REPORT="report.log" ./$$test || echo "ERROR: $$test failed"; \
				;; \
		esac; \
	done

# Build rule for Zig tests
$(BIN_DIR)/%Tests: modules/%Tests.zig
	$(ZIG) test $< -femit-bin=$@

# With sanitizers
$(SANITIZE_DIR)/%Tests: modules/%Tests.zig
	$(ZIG) test $< -femit-bin=$@ -fsanitize-c
```

## Generalization
Pattern-based differentiation allows multiple test frameworks to coexist:
- Use naming conventions to identify test types
- Apply framework-specific execution patterns
- Maintain unified reporting and error handling
- Support sanitizers and other build variants

## Implementation
Integration checklist:
1. Define test discovery patterns (wildcard rules)
2. Create unified test lists combining all frameworks
3. Use case statements to apply framework-specific logic
4. Ensure all build variants (debug, sanitize, etc.) support all test types
5. Maintain consistent error reporting across frameworks

## Related Patterns
- [Zig Struct Modules Pattern](zig-struct-modules-pattern.md)
- [Makefile POSIX Shell Compatibility](makefile-posix-shell-compatibility.md)