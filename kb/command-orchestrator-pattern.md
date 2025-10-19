# Command File as Orchestrator Pattern

## Learning

Complex multi-step commands should use the command file itself as the orchestrator, showing all steps, gates, and orchestration logic directly. The command file should NOT delegate orchestration to a wrapper script like `run-*.sh`.

## Importance

**Without orchestrator visibility:**
- All workflow logic is hidden in wrapper scripts
- Operators cannot see gates and conditional flow
- Manual intervention between steps is difficult
- Debugging requires reading wrapper script code
- Changes to orchestration require script modifications

**With command file as orchestrator:**
- All workflow logic is visible in the markdown documentation
- Operators understand the complete flow
- Conditional steps and gates are clearly documented
- Manual intervention is straightforward
- Changes to orchestration stay in documentation

## Pattern: Command File is the Orchestrator

### Structure

Command files should show the orchestration logic directly:

```markdown
#### Step 1: Do First Thing
```bash
./scripts/do-first-thing.sh
```

#### Decision: Error Gate
If no errors, skip to Step 5:
```bash
if [ $ERROR_COUNT -eq 0 ]; then
  skip steps 2-4
else
  proceed with steps 2-4
fi
```

#### Step 2: Do Second Thing
```bash
./scripts/do-second-thing.sh
```

[more steps...]

#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh command-name
```
```

### Key Principles

1. **Show all orchestration logic in the command file**
   - Gates and conditions are visible
   - Step sequence is documented
   - Decision points are clear

2. **Scripts do one thing only**
   - Each focused script executes a single step
   - No script makes orchestration decisions
   - Scripts don't call other scripts (no chaining)

3. **Wrapper scripts are anti-pattern**
   - ❌ WRONG: Command calls `run-something.sh` which then calls multiple step scripts
   - ✅ CORRECT: Command calls each step script directly with orchestration logic inline

## Anti-Pattern: Wrapper Scripts

### Before (Anti-Pattern)

**Command file:**
```markdown
#### Workflow Execution
Just run this script:
```bash
./scripts/run-check-docs.sh
```
```

**Wrapper script (run-check-docs.sh - 180+ lines):**
```bash
#!/bin/bash
# Initialize checkpoint
./scripts/checkpoint-init.sh check-docs ...

# Step 1
./scripts/validate-docs.sh

# Step 2 - Conditional logic
if [ $ERROR_COUNT -eq 0 ]; then
  skip steps...
else
  gate logic...
fi

# Step 3-5
./scripts/preview-doc-fixes.sh
./scripts/apply-doc-fixes.sh
# ... more steps ...
```

**Problems:**
- All orchestration hidden in wrapper script
- Command documentation doesn't show what actually happens
- Cannot intervene between steps without reading wrapper script
- Operator must trust the wrapper script logic

### After (Correct Pattern)

**Command file (check-docs.md):**
```markdown
#### Initialize Checkpoint
```bash
./scripts/checkpoint-init.sh check-docs [steps]
./scripts/checkpoint-require.sh check-docs
```

#### Step 1: Validate Docs
```bash
./scripts/validate-docs.sh
```

#### Step 2: Conditional Flow (Error Gate)
If no errors found, skip to Step 5:
```bash
source /tmp/check-docs-stats.txt
if [ $ERROR_COUNT -eq 0 ]; then
  ./scripts/checkpoint-update.sh check-docs STEP=2
  ./scripts/checkpoint-update.sh check-docs STEP=3
  ./scripts/checkpoint-update.sh check-docs STEP=4
else
  ./scripts/checkpoint-gate.sh check-docs "Errors Found" "1"
fi
```

#### Step 3: Preview Fixes
```bash
./scripts/preview-doc-fixes.sh
```

[more steps shown directly...]
```

**Benefits:**
- All orchestration logic is visible in documentation
- Operator can read markdown to understand flow
- Easy to intervene between steps
- Can manually execute specific steps
- Clear what happens at each gate

## Real-World Example: Check-Docs Refactoring

This pattern emerged from refactoring the check-docs command:

**Initial (Anti-Pattern)**:
- check-docs.md delegated to run-check-docs.sh
- run-check-docs.sh was 180+ lines combining all logic
- Tried to also have 5 focused scripts but wrapped them in run-check-docs.sh
- Made it hard to see orchestration from command documentation

**Corrected (Correct Pattern)**:
- Deleted run-check-docs.sh entirely
- check-docs.md shows all 6 steps directly
- Conditional flow (error gate) is inline
- Each step calls its focused script
- All gates are visible
- Operator can manually execute any step

## Real-World Example: Check-Naming Refactoring

Session 2f (2025-10-18) applied this pattern to check-naming:

**Problem**: Command had embedded multi-line bash blocks (10+ lines) directly in markdown:
- Naming check execution logic was embedded
- Conditional flow logic was embedded
- Violated extraction discipline from command-helper-script-extraction-pattern.md

**Solution**: Extracted to focused helper scripts:
- `run-naming-check.sh` - Execute check and capture violations (single responsibility)
- `check-naming-conditional-flow.sh` - Handle conditional step skipping (single responsibility)
- check-naming.md now shows orchestration directly with script references
- All checkpoint markers and conditional logic visible in command documentation
- Verified end-to-end checkpoint workflow with conditional step skipping

**Key lesson**: Each 10+ line shell block gets its own focused script - never combine multiple blocks into one wrapper. Command file remains the orchestrator.

## Comparison with Check-Commands

The check-commands command exemplifies the correct pattern:
- Command file shows all 5 steps
- Each step calls its focused script
- No wrapper script between command and focused scripts
- Orchestration logic visible in markdown

## When to Create Wrapper Scripts

Wrapper scripts ARE appropriate for:
1. **Simple convenience entry points** (single-purpose automation)
2. **Integrations with external tools** (CI/CD pipeline adapters)
3. **Build system integration** (Makefile helpers)

Wrapper scripts are NOT appropriate for:
1. **Multi-step workflow orchestration** (use command file)
2. **Complex conditional logic** (document in command file)
3. **Manual intervention points** (show in command file)

## Related Patterns

- [Command Helper Script Extraction Pattern](command-helper-script-extraction-pattern.md) - When to extract and how to name scripts
- [Script Domain Naming Convention](script-domain-naming-convention.md) - Naming focused scripts for discoverability
- [Single Responsibility Principle](single-responsibility-principle.md) - Each script does one thing
- [Checkpoint Implementation Guide](checkpoint-implementation-guide.md) - Multi-step workflow patterns
- [Command Orchestrator and Checkpoint Separation](command-orchestrator-checkpoint-separation.md) - Separation of checkpoint coordination from script responsibilities
- [Checkpoint Operations and Steps Hierarchy](checkpoint-operations-and-steps-hierarchy.md) - Two-level hierarchy for checkpoint tracking
- [Helper Script Reusability Pattern](helper-script-reusability-pattern.md) - Decoupling scripts from orchestration context
