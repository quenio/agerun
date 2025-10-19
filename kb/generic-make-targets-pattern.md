# Generic Make Targets Pattern

## Learning
Make targets should be designed as generic, parameterized rules rather than creating multiple specific targets for similar operations. This approach uses variables passed via command line to customize behavior, following the DRY (Don't Repeat Yourself) principle.

## Importance
Creating specific targets for each variation leads to Makefile bloat, maintenance burden, and inconsistency. Generic targets with parameters reduce duplication, improve maintainability, and ensure consistent behavior across similar operations.

## Modern Approach: Direct Script Calls
In modern projects, it's often more flexible to call scripts **directly** rather than through Makefile wrapper targets. This decouples build automation from script execution and simplifies documentation.

## Example: Anti-Pattern vs. Better Approach

**Anti-Pattern: Specific targets for each variation**
```makefile
# BAD: Creates bloat and maintenance burden
new-learnings-init:
	@bash scripts/process.sh new-learnings step1 step2

commit-init:
	@bash scripts/process.sh commit stage review push

build-init:
	@bash scripts/process.sh build clean compile link
```

**Better: Use Makefile for primary build tasks only**
```makefile
# Good for primary build commands in Makefile
build: install-scan-build
	@if [ -x ./scripts/build.sh ]; then \
		./scripts/build.sh; \
	else \
		echo "ERROR: scripts/build.sh not found"; \
		exit 1; \
	fi

run-tests: run_tests_lib
	$(MAKE) $(TEST_BINARIES)
	@cd bin/run-tests && ./run_all_tests.sh

# Delegate secondary operations to direct script calls
# Usage: ./scripts/process.sh new-learnings step1 step2
# Usage: ./scripts/process.sh commit stage review push
```

## When to Use Makefile Targets
- **Core build operations**: compilation, linking, testing
- **Frequently used sequences**: build → test → analyze
- **OS-specific logic**: compiler selection, platform detection
- **Complex dependency chains**: object file management, parallel builds

## When to Use Direct Script Calls
- **Sequential multi-step workflows**: step tracking, conditional logic
- **Maintenance-heavy operations**: tasks that change frequently
- **Independent utilities**: operations that stand alone
- **Documentation workflows**: checkpoint tracking, KB generation

## Generalization Principles
1. **Use variables for customization**: `$(CMD)`, `$(OPTS)`, `$(ARGS)`
2. **Quote parameters properly**: Use `"$(VAR)"` for parameters with spaces
3. **Provide sensible defaults**: `VAR ?= default_value` for optional parameters
4. **Group related targets**: Keep similar operations together in Makefile
5. **Limit wrapper bloat**: Avoid targets that simply pass through to scripts

## Implementation Example: Generic Target with Validation
```makefile
# Generic operation target with parameter validation
generic-operation:
	@if [ -z "$(CMD)" ]; then \
		echo "Error: CMD parameter required"; \
		echo "Usage: make generic-operation CMD=command [OPTS=options]"; \
		exit 1; \
	fi
	@echo "Executing $(CMD) with options: $(OPTS)"
	@bash scripts/generic_script.sh $(CMD) $(OPTS)

# Optional parameters with defaults
VERBOSE ?=
DEBUG ?= false

status:
	@bash scripts/status.sh $(CMD) $(VERBOSE)

# Multiple parameter support
process:
	@bash scripts/process.sh --input=$(INPUT) --output=$(OUTPUT) --format=$(FORMAT)
```

## Related Patterns
- [Makefile Filtered Variables Pattern](makefile-filtered-variables-pattern.md)
- [Makefile Pattern Rule Management](makefile-pattern-rule-management.md)
- [Code Smell Duplicate Code](code-smell-duplicate-code.md)