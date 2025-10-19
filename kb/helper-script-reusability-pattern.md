# Helper Script Reusability Pattern

## Learning

Helper scripts must be completely decoupled from their calling context (checkpoint tracking, orchestration logic, workflow assumptions) to remain reusable across different workflows. Scripts that embed orchestrator-specific logic become coupled to a single use case and cannot be reused.

## Importance

Reusable helper scripts provide:
1. **Flexibility**: Same script can be used in multiple workflows
2. **Maintainability**: Domain logic changes only in one place
3. **Testing**: Scripts can be tested independently
4. **Consistency**: Same validation/operation works everywhere

## The Coupling Problem

When scripts are tightly coupled to a specific workflow:

```bash
# ❌ WRONG: validate-docs.sh coupled to check-docs workflow
#!/bin/bash
set -e

echo "========== STAGE 1: Initial Check =========="

make check-docs > /tmp/check-docs-output.txt 2>&1
ERROR_COUNT=$(grep -E "ERROR|FAIL" /tmp/check-docs-output.txt | wc -l)
echo "ERROR_COUNT=$ERROR_COUNT" > /tmp/check-docs-stats.txt

./scripts/checkpoint-update.sh check-docs STEP=1  # ← COUPLING!
```

Problems:
- Script assumes "check-docs" workflow context
- Script calls checkpoint update (orchestrator responsibility)
- Script format is specific to this workflow
- Can't reuse in other documentation validation workflows

## The Decoupling Solution

Separate domain work from orchestration:

```bash
# ✅ RIGHT: validate-docs.sh (reusable, independent)
#!/bin/bash
set -e

echo "Running documentation validation..."

if make check-docs > /tmp/check-docs-output.txt 2>&1; then
  cat /tmp/check-docs-output.txt
  echo "✅ No documentation errors found!"
  echo "ERROR_COUNT=0" > /tmp/check-docs-stats.txt
else
  cat /tmp/check-docs-output.txt
  ERROR_COUNT=$(grep -E "ERROR|FAIL" /tmp/check-docs-output.txt | wc -l || echo "0")
  echo "⚠️ Found $ERROR_COUNT documentation errors"
  echo "ERROR_COUNT=$ERROR_COUNT" > /tmp/check-docs-stats.txt
fi

# NO checkpoint calls - script is independent!
```

Benefits:
- Works in any workflow context
- Can be called directly without wrapper
- Produces results for downstream processing
- No workflow assumptions

## Reusability Patterns

### Pattern 1: Output Results for Downstream Processing

```bash
# Script does work and outputs results
./scripts/validate-docs.sh

# Results stored in predictable location
ERROR_COUNT=$(cat /tmp/check-docs-stats.txt | grep ERROR_COUNT | cut -d= -f2)

# Orchestrator (e.g., check-docs.md) uses results
if [ $ERROR_COUNT -gt 0 ]; then
  ./scripts/preview-doc-fixes.sh
fi
```

### Pattern 2: Independent Execution

```bash
# Script can run standalone without orchestration
$ ./scripts/validate-docs.sh
Running documentation validation...
✅ No documentation errors found!
ERROR_COUNT=0

# Or within workflow
$ /ar:check-docs
# Uses same script within orchestrator
```

### Pattern 3: Conditional Wrapper

When a script needs to be workflow-aware, use a wrapper:

```bash
# ✅ validate-docs.sh (reusable, independent)
#!/bin/bash
make check-docs && echo "PASS" || echo "FAIL"

# ✅ check-docs-conditional-flow.sh (workflow-specific wrapper)
#!/bin/bash
source /tmp/check-docs-stats.txt
if [ $ERROR_COUNT -gt 0 ]; then
  echo "Errors found, continuing to fix stage..."
else
  echo "No errors, skipping fix stage..."
fi
```

## Decoupling Checklist

For a helper script to be truly reusable:

- [ ] **No checkpoint calls** - Leave orchestration to caller
- [ ] **No workflow assumptions** - Script doesn't assume specific context
- [ ] **Produces outputs** - Results stored in predictable locations or stdout
- [ ] **Clear responsibility** - Single domain task (validate, preview, apply, etc.)
- [ ] **Independent executable** - Can run standalone with `./script.sh`
- [ ] **No embedded logic** - Orchestration/workflow decisions happen in caller
- [ ] **Exit codes meaningful** - 0=success, 1=failure, with clear semantics

## Real Example: Documentation Validation Scripts

### Before Refactoring (Coupled)
```bash
# validate-docs.sh
make check-docs && echo "PASS"
./scripts/checkpoint-update.sh check-docs 1  # ← COUPLED

# preview-doc-fixes.sh
python3 scripts/batch_fix_docs.py --dry-run
./scripts/checkpoint-update.sh check-docs 2  # ← COUPLED

# apply-doc-fixes.sh
python3 scripts/batch_fix_docs.py
./scripts/checkpoint-update.sh check-docs 3  # ← COUPLED
```

Usage: Only works with check-docs workflow

### After Refactoring (Decoupled)
```bash
# validate-docs.sh (reusable)
make check-docs > /tmp/check-docs-output.txt 2>&1
# Stores results, no checkpoint calls

# preview-doc-fixes.sh (reusable)
python3 scripts/batch_fix_docs.py --dry-run
# Shows preview, no checkpoint calls

# apply-doc-fixes.sh (reusable)
python3 scripts/batch_fix_docs.py
# Applies fixes, no checkpoint calls

# check-docs.md (orchestrator)
./scripts/validate-docs.sh
./scripts/checkpoint-update.sh check-docs 1  # Orchestrator handles checkpoint
./scripts/preview-doc-fixes.sh
./scripts/checkpoint-update.sh check-docs 2  # Orchestrator handles checkpoint
```

Usage: Scripts work anywhere; orchestrator handles workflow coordination

## Benefits Realized

With decoupled scripts:
1. **Same scripts used in multiple workflows**
2. **Scripts focus on single responsibility**
3. **Orchestrator handles all coordination**
4. **Easier testing and debugging**
5. **Simpler maintenance and updates**

## Related Patterns

- [Command Orchestrator and Checkpoint Separation](command-orchestrator-checkpoint-separation.md) - Separation principle
- [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) - Script extraction discipline
- [Command Orchestrator Pattern](command-orchestrator-pattern.md) - Orchestrator responsibilities
