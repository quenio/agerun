# Script Archaeology Pattern

## Learning
When understanding what work was actually completed, examine the scripts that performed the work rather than relying solely on documentation. Scripts contain the actual implementation details and often reveal that more was accomplished than originally planned.

## Importance
Documentation can become outdated or incomplete, especially when implementation exceeds planning. Scripts provide ground truth about what changes were actually made to the codebase. This prevents duplicating already-completed work.

## Example
Investigating Phase 2b completion by examining scripts:
```bash
# Check what the removal script actually did
grep "FUNCTIONS_TO_REMOVE" scripts/remove_global_functions.py
# Shows 25 functions removed, not just 20 as planned

# Verify script execution in git history
git log --oneline -S "remove_global_functions.py" | head -5
# Shows when script was run

# Check script output if captured
git show HEAD:scripts/remove_global_functions.py | grep -A 30 "FUNCTIONS_TO_REMOVE"
# Lists all functions that were removed
```

Script revealed Phase 2b removed ALL functions:
```python
FUNCTIONS_TO_REMOVE = [
    'ar_agency__create_agent_with_instance',           # Phase 2b
    'ar_agency__destroy_agent_with_instance',          # Phase 2b
    # ... 18 more from Phase 2b ...
    'ar_agency__is_agent_active',        # Phase 2c (already done!)
    'ar_agency__set_agent_active',       # Phase 2c (already done!)
    'ar_agency__set_agent_id',           # Phase 2c (already done!)
    'ar_agency__get_agent_method_info',  # Phase 2c (already done!)
    'ar_agency__update_agent_method',    # Phase 2c (already done!)
]
```

## Generalization
Script archaeology workflow:
1. **Find relevant scripts**: Search for scripts related to the task
2. **Examine script content**: Look at what it actually does, not just its name
3. **Check execution history**: Use git to see when and how it was run
4. **Verify results**: Check that script's changes are in the codebase
5. **Compare with documentation**: Update docs if script did more than planned

## Implementation
```bash
# Script archaeology helper function
investigate_script_work() {
    local script_name="$1"
    local module="$2"

    echo "=== Script Investigation: $script_name ==="

    # Check if script exists
    if [ -f "$script_name" ]; then
        echo "Script content summary:"
        grep -E "def |class |REMOVE|DELETE|FUNCTIONS" "$script_name" | head -10
    fi

    # Check git history
    echo -e "\nExecution history:"
    git log --oneline -S "$(basename $script_name)" | head -5

    # Check impact on module
    echo -e "\nModule changes:"
    git diff HEAD~5 HEAD -- "$module" | grep "^-.*function" | wc -l
    echo "Functions removed from $module"

    # Check for script outputs
    echo -e "\nRelated commits:"
    git log --oneline --grep="$(basename $script_name .py)" | head -3
}

# Usage
investigate_script_work scripts/remove_global_functions.py modules/ar_agency.c
```

## Common Patterns
```bash
# Scripts often do more than their names suggest
scripts/fix_one_issue.py     # May fix multiple related issues
scripts/partial_cleanup.py   # May do complete cleanup
scripts/phase1_only.py       # May include phase 2 work

# Check what scripts actually modify
for script in scripts/*.py; do
    echo "=== $script ==="
    grep -l "with open\|\.write\|\.replace" "$script" 2>/dev/null
done

# Find all scripts that touched a module
git log --format="" --name-only modules/ar_agency.c |
    grep "^scripts/" | sort -u
```

## Related Patterns
- [Task Verification Before Execution](task-verification-before-execution.md)
- [Comprehensive Impact Analysis](comprehensive-impact-analysis.md)
- [Script Enhancement Over One-Off](script-enhancement-over-one-off.md)
- [Global Function Removal Script Pattern](global-function-removal-script-pattern.md)