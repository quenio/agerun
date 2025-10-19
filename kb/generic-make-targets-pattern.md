# Generic Make Targets Pattern

## Learning
Make targets should be designed as generic, parameterized rules rather than creating multiple specific targets for similar operations. This approach uses variables passed via command line to customize behavior, following the DRY (Don't Repeat Yourself) principle.

## Importance
Creating specific targets for each variation leads to Makefile bloat, maintenance burden, and inconsistency. Generic targets with parameters reduce duplication, improve maintainability, and ensure consistent behavior across similar operations.

## Example
```makefile
# BAD: Specific targets for each command
new-learnings-init:
	@bash scripts/checkpoint-init.sh new-learnings "Step 1" "Step 2"

commit-init:
	@bash scripts/checkpoint-init.sh commit "Stage" "Review" "Push"

build-init:
	@bash scripts/checkpoint-init.sh build "Clean" "Compile" "Link"

# GOOD: Generic parameterized target
checkpoint-init:
	@bash scripts/checkpoint-init.sh $(CMD) $(STEPS)

# Usage examples in comments
# ./scripts/checkpoint-init.sh new-learnings ""Step 1" "Step 2""
# ./scripts/checkpoint-init.sh commit ""Stage" "Review" "Push""
# ./scripts/checkpoint-init.sh build ""Clean" "Compile" "Link""
```

## Generalization
1. **Use variables for customization**: `$(CMD)`, `$(OPTS)`, `$(ARGS)`
2. **Document parameter usage**: Include usage examples in comments
3. **Quote parameters properly**: Use `"$(VAR)"` for parameters with spaces
4. **Provide sensible defaults**: `VAR ?= default_value` for optional parameters
5. **Group related targets**: Keep generic utilities together in Makefile

## Implementation
```makefile
# Generic target pattern with defaults and validation
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