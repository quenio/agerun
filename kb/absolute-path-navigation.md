# Absolute Path Navigation

## Learning
Always use absolute paths in development commands rather than relative paths to avoid directory-dependent failures

## Importance
Prevents script failures and command errors when working directory changes unexpectedly

## Example
```bash
# Correct: Absolute paths
cd /Users/quenio/Repos/agerun/bin
pytest /foo/bar/tests

# Incorrect: Relative paths  
cd bin  # Depends on current working directory
cd /foo/bar && pytest tests  # Changes directory unnecessarily
```

## Generalization
Make commands location-independent by using absolute references

## Implementation
- Use absolute paths for all directory navigation
- Avoid relative paths like `cd bin` or `../scripts`
- Use absolute paths in test execution
- Makefile handles directory changes automatically
- Scripts should verify they run from repo root

## Related Patterns
- Location-independent commands
- Script robustness
- Environment reliability
- Directory verification in scripts
- Makefile automation patterns