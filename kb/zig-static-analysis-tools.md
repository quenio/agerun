# Zig Static Analysis Tools

## Learning
Zig has limited built-in static analysis compared to C compilers. The built-in tools (`zig ast-check` and `zig fmt --check`) only catch syntax errors and formatting issues, not semantic problems or code quality issues.

## Importance
Teams migrating from C to Zig may expect similar static analysis capabilities. Understanding the limitations helps set realistic expectations and plan for additional tooling needs.

## Example
```makefile
# Basic Zig static analysis
analyze-zig:
	@echo "Running Zig AST checks..."
	@for file in $(ZIG_SRC) $(ZIG_TEST_SRC); do \
		echo "Checking $$file..."; \
		$(ZIG) ast-check $$file || exit 1; \
	done
	@echo "Running Zig format check..."
	@for file in $(ZIG_SRC) $(ZIG_TEST_SRC); do \
		$(ZIG) fmt --check $$file || echo "Format issue in $$file"; \
	done

# Third-party tool integration (if zlint installed)
analyze-zig-comprehensive:
	@if command -v zlint >/dev/null 2>&1; then \
		zlint --config .zlintrc $(ZIG_SRC); \
	else \
		echo "zlint not found - install for comprehensive analysis"; \
	fi
```

## Generalization
Static analysis strategy for Zig projects:
1. Use built-in tools for basic checks (syntax, formatting)
2. Rely on Zig's compile-time safety features
3. Consider third-party linters for code quality rules
4. Leverage Zig's built-in safety checks in debug builds
5. Use sanitizers (`-fsanitize-c`) for runtime analysis

## Implementation
Available tools and their capabilities:
- **Built-in**: `zig ast-check` (syntax only), `zig fmt` (formatting)
- **Compiler**: Many checks happen at compile time
- **Third-party**: zlint (comprehensive), ziglint (linting suite), zlinter (build-integrated)
- **Runtime**: `-fsanitize-c` for undefined behavior, `-fsanitize-thread` for races

Integration approach:
```bash
# Add to CI/CD pipeline
make analyze-zig      # Basic checks
make build           # Compile-time checks
make sanitize-tests  # Runtime checks
```

## Related Patterns
- [Zig Test Build Integration](zig-test-build-integration.md)
- [C to Zig Module Migration](c-to-zig-module-migration.md)