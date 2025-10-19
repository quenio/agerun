# Command Orchestrator and Checkpoint Separation Pattern

## Learning

The command orchestrator pattern requires strict separation of responsibilities between helper scripts and checkpoint tracking. Helper scripts should never contain checkpoint-update calls; instead, the orchestrator (command file) handles all checkpoint state management while scripts focus on their single domain responsibility.

## Importance

This separation is critical for:
1. **Script Reusability**: Scripts can be used in multiple workflows, not just those using checkpoints
2. **Orchestrator Clarity**: All workflow coordination logic is in one place (the command file)
3. **Maintenance**: Checkpoint changes don't require script updates
4. **Testing**: Scripts can be tested independently without checkpoint context

## Problem Pattern

When scripts embed checkpoint tracking:
```bash
# ❌ WRONG: validate-docs.sh
./scripts/validate-docs.sh
./scripts/checkpoint-update.sh check-docs 1  # Script calls checkpoint update
```

This couples the script to checkpoint tracking and prevents reuse.

## Solution Pattern

Separate concerns into two layers:
```bash
# ✅ RIGHT: validate-docs.sh (script - domain work only)
./scripts/validate-docs.sh  # Just validates, produces output/side effects

# ✅ RIGHT: check-docs.md (orchestrator - checkpoint coordination)
./scripts/validate-docs.sh
./scripts/checkpoint-update.sh check-docs 1  # Command handles checkpoint
```

## Implementation

### Helper Script Structure

Helper scripts should:
1. Accept domain-specific parameters
2. Perform their single responsibility
3. Output results or create side effects (files, environment)
4. Exit cleanly with appropriate exit codes
5. Have ZERO knowledge of checkpoint tracking

**Example**: `validate-docs.sh`
```bash
#!/bin/bash
set -e

# Do the domain work
make check-docs > /tmp/check-docs-output.txt 2>&1

# Save results for downstream processing
ERROR_COUNT=$(grep -E "ERROR|FAIL" /tmp/check-docs-output.txt | wc -l)
echo "ERROR_COUNT=$ERROR_COUNT" > /tmp/check-docs-stats.txt

# No checkpoint calls - orchestrator handles that
```

### Orchestrator Structure

The command orchestrator should:
1. Call helper scripts in sequence
2. Manage all checkpoint updates
3. Handle conditional flow based on script outputs
4. Maintain clear separation between operations and checkpoint steps

**Example**: `check-docs.md` workflow structure
```
#### [CHECKPOINT START - STEP 1]

#### Operation 1: Validate Documentation
./scripts/validate-docs.sh

#### Operation 2: Update Checkpoint
./scripts/checkpoint-update.sh check-docs 1

#### [CHECKPOINT END - STEP 1]
```

### Two-Level Hierarchy

**Checkpoint Steps** (tracked, numbered 1-5): Logical workflow units
- "Validate Docs"
- "Preview Fixes"
- "Apply Fixes"
- "Verify Resolution"
- "Commit and Push"

**Operations** (not tracked): Individual command invocations
- validate-docs.sh
- checkpoint-update.sh check-docs 1
- conditional-flow.sh
- preview-doc-fixes.sh
- etc.

## Markers Usage

Checkpoint markers must wrap complete logical steps including their checkpoint-update call:

```markdown
#### [CHECKPOINT START - STEP N: Step Name]

Operations that do work...
./scripts/some-work.sh

Operation that updates checkpoint...
./scripts/checkpoint-update.sh command N

#### [CHECKPOINT END - STEP N]
```

## Related Patterns

- [Command Orchestrator Pattern](command-orchestrator-pattern.md) - Core orchestrator responsibilities
- [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) - How to extract scripts properly
- [Checkpoint Sequential Execution Discipline](checkpoint-sequential-execution-discipline.md) - Sequential execution requirements
