# Code Movement Verification

## Learning
Use diff to verify code is moved (not rewritten) when refactoring functions between modules

## Importance
Ensures behavior preservation and prevents introduction of subtle bugs during refactoring

## Example
```bash
# Verify code movement between files
diff -u <(sed -n '130,148p' original.c) <(sed -n '11,29p' new.c)

# Should show minimal differences (just context changes)
# Large differences indicate rewriting rather than moving
```

## Generalization
Verify mechanical transformations are truly mechanical, not reimplementation

## Implementation
```bash
# Extract function from original location
sed -n 'START,ENDp' original_file.c > /tmp/original_function

# Extract function from new location  
sed -n 'START,ENDp' new_file.c > /tmp/moved_function

# Compare - should be nearly identical
diff -u /tmp/original_function /tmp/moved_function
```

## Related Patterns
- Refactoring safety practices
- Behavior preservation
- Mechanical transformations
- Risk reduction in code changes
- Systematic verification methods