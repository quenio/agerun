# Makefile POSIX Shell Compatibility

## Learning
Makefiles use sh (POSIX shell), not bash, regardless of your system's default shell. Using bash-specific features like `[[ ]]` causes mysterious failures with cryptic exit codes.

## Importance
Bash-specific constructs in Makefiles lead to hard-to-debug failures that may manifest as SIGABRT (exit code 134) or other unexpected errors. The error messages often don't indicate the actual problem is shell syntax.

## Example
```makefile
# BAD: Bash-specific test operator
run-tests:
	@for test in $(TESTS); do \
		if [[ "$$test" == *Tests ]]; then \
			./$$test; \
		fi; \
	done

# GOOD: POSIX-compliant case statement
run-tests:
	@for test in $(TESTS); do \
		case "$$test" in \
			*Tests) \
				./$$test; \
				;; \
			*) \
				AGERUN_MEMORY_REPORT="report.log" ./$$test; \
				;; \
		esac; \
	done
```

## Generalization
Always use POSIX-compliant shell constructs in Makefiles:
- Use `case` instead of `[[ ]]` for pattern matching
- Use `[ ]` instead of `[[ ]]` for simple tests
- Use `=` instead of `==` for string comparison
- Avoid bash arrays, process substitution, and other bash-isms

## Implementation
Common POSIX replacements:
```bash
# String pattern matching
case "$var" in
    pattern*) echo "matches" ;;
esac

# String equality
[ "$var" = "value" ] && echo "equal"

# File tests
[ -f "$file" ] && echo "file exists"

# Multiple conditions
[ "$a" = "x" ] && [ "$b" = "y" ] && echo "both true"
```

## Related Patterns
- [Makefile Pattern Rule Management](makefile-pattern-rule-management.md)