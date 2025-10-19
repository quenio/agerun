# Documentation Implementation Sync

## Learning
Documentation must be systematically updated whenever implementation changes. Drift between documentation and implementation causes confusion, wasted time, and errors. Changes to shared components can require updates to dozens of related documents.

## Importance
Out-of-sync documentation is worse than no documentation - it actively misleads users. Systematic updates ensure documentation remains a reliable source of truth and prevents accumulation of technical debt.

## Example
```bash
# When checkpoint scripts changed to 3-line output:
# 26 command files needed updates

# Systematic update approach:
for file in .opencode/command/ar/*.md; do
    if grep -q "checkpoint-init" "$file"; then
        # Update expected output sections
        sed -i 's/20+ lines of output/3 lines of output/g' "$file"
        # Update example outputs to match new format
        update_checkpoint_examples "$file"
    fi
done

# Verification after updates
python3 verify_documentation_sync.py
```

## Generalization
Maintain sync through:
1. Impact analysis before changes (what docs need updating?)
2. Automated verification scripts to detect drift
3. Batch update scripts for systematic changes
4. Documentation tests that fail when implementation changes
5. CI checks that verify documentation matches implementation

## Implementation
```python
# Verification script to detect drift
def verify_sync():
    """Check if documentation matches implementation."""
    issues = []
    
    # Extract actual output from implementation
    actual_output = run_command("./scripts/checkpoint-status.sh command-name --compact")
    
    # Check each documentation file
    for doc_file in Path(".opencode/command/ar").glob("*.md"):
        content = doc_file.read_text()
        if "Expected output:" in content:
            # Extract expected output from docs
            expected = extract_expected_output(content)
            if not outputs_match(expected, actual_output):
                issues.append(f"{doc_file}: Output mismatch")
    
    return issues

# Run in CI to prevent drift
if verify_sync():
    print("Documentation out of sync with implementation!")
    sys.exit(1)
```

## Related Patterns
- [Command Output Documentation Pattern](command-output-documentation-pattern.md)
- [Documentation Standards Integration](documentation-standards-integration.md)
- [Comprehensive Impact Analysis](comprehensive-impact-analysis.md)
- [Module Consistency Verification](module-consistency-verification.md)
- [Global Function Removal Script Pattern](global-function-removal-script-pattern.md)
- [Documentation Index Consistency Pattern](documentation-index-consistency-pattern.md)