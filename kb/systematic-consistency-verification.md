# Systematic Consistency Verification

## Learning
Automated verification scripts are essential for maintaining consistency across multiple related files. Manual checking is error-prone and doesn't scale. Systematic verification catches issues that would otherwise accumulate into technical debt.

## Importance
Inconsistencies multiply over time through copy-paste and independent edits. Automated verification provides rapid feedback, prevents regression, and ensures systematic quality across the codebase.

## Example
```python
#!/usr/bin/env python3
"""Verify checkpoint consistency across all commands."""

def check_checkpoint_consistency(filepath):
    """Check that checkpoint declarations match usage."""
    content = filepath.read_text()
    
    # Extract checkpoint initialization
    init_match = re.search(
        r'\./scripts/checkpoint-init\.sh (\w+) "([^"]+)"',
        content
    )
    if not init_match:
        return None

    cmd_name = init_match.group(1)
    steps = init_match.group(2).split('" "')
    num_steps = len(steps)

    # Find all checkpoint-update calls
    updates = re.findall(
        r'\./scripts/checkpoint-update\.sh (\w+) (\d+)',
        content
    )
    
    # Verify consistency
    issues = []
    for update_cmd, step_num in updates:
        if update_cmd != cmd_name:
            issues.append(f"CMD mismatch: {update_cmd} != {cmd_name}")
        if int(step_num) > num_steps:
            issues.append(f"Step {step_num} exceeds {num_steps} declared")
    
    return issues

# Run across all files
for md_file in Path(".opencode/command/ar").glob("*.md"):
    issues = check_checkpoint_consistency(md_file)
    if issues:
        print(f"❌ {md_file.name}: {len(issues)} issues")
        for issue in issues:
            print(f"   - {issue}")
```

## Generalization
Create verification scripts that:
1. Parse declarations and usage patterns
2. Compare expected vs actual across files
3. Report specific issues with context
4. Support dry-run and fix modes
5. Integrate with CI for continuous verification

## Implementation
```bash
# Makefile target for verification
check-consistency:
	@echo "Checking consistency across files..."
	@python3 scripts/verify_consistency.py || exit 1
	@echo "✓ All files consistent"

# Git pre-commit hook
if ! make check-consistency; then
    echo "Consistency check failed. Fix issues before committing."
    exit 1
fi
```

## Related Patterns
- [Module Consistency Verification](module-consistency-verification.md)
- [Comprehensive Impact Analysis](comprehensive-impact-analysis.md)
- [Cross File Pattern Propagation](cross-file-pattern-propagation.md)
- [Documentation Implementation Sync](documentation-implementation-sync.md)