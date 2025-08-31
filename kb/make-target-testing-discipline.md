# Make Target Testing Discipline

## Learning
Always test with `make build` or appropriate make targets, never by running scripts directly. This ensures proper environment setup, dependency handling, and parallel execution.

## Importance
Make targets provide the canonical way to execute project operations. They handle dependencies, set up environments, and coordinate parallel execution. Running scripts directly bypasses these critical orchestration layers.

## Example
```bash
# ❌ WRONG: Running script directly
bash scripts/build.sh  # Misses make environment setup

# ✅ RIGHT: Using make target
make build 2>&1  # Proper execution with all dependencies

# The difference in practice:
# Direct script execution might:
# - Miss environment variables set by make
# - Skip dependency checks
# - Run with wrong working directory
# - Bypass parallel job coordination
```

## Generalization
Always use the project's build system interface (make, npm scripts, cargo commands) rather than executing underlying scripts directly. The build system provides essential context and coordination.

## Implementation
```makefile
# Makefile ensures proper setup
build:
	@echo "=== AgeRun Build ==="
	@mkdir -p logs
	@bash scripts/build.sh
	
# Even simple operations benefit from make
test_name:
	@cd bin && ./test_name  # Ensures correct directory
```

## Related Patterns
- [Absolute Path Navigation](absolute-path-navigation.md)
- [Make-Only Test Execution](make-only-test-execution.md)
- [Generic Make Targets Pattern](generic-make-targets-pattern.md)