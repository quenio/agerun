# Checkpoint Operations and Steps Hierarchy Pattern

## Learning

The checkpoint system uses a two-level hierarchy: **Checkpoint Steps** are logical workflow units tracked by the checkpoint system, while **Operations** are individual command invocations that compose those steps. Understanding this distinction is essential for properly structuring command orchestrators.

## Importance

This hierarchy provides:
1. **Coarse-grained Tracking**: Checkpoint system tracks logical workflow units (5-10 steps)
2. **Fine-grained Documentation**: Command file shows all operations (10-20+ operations)
3. **Clear Workflow Boundaries**: Markers delineate logical workflow units
4. **Explicit Progression**: Users understand workflow structure and current position

## Definitions

**Checkpoint Step**: A logical workflow unit that:
- Has a descriptive name: "Validate Docs", "Apply Fixes", etc.
- Is tracked with a number (1, 2, 3, etc.) by the checkpoint system
- Contains one or more operations
- Has [CHECKPOINT START] and [CHECKPOINT END] markers
- Ends with a checkpoint-update call that records its completion

**Operation**: An individual command invocation that:
- Is numbered within documentation: "Operation 1", "Operation 2"
- Performs a specific task (domain work, conditional checks, checkpoint updates)
- Is NOT individually tracked by checkpoint system
- Lives between the START and END markers of a checkpoint step

## Pattern Example

### Checkpoint Step Hierarchy

```
Checkpoint System: 5 tracked steps
├─ Step 1: Validate Docs
├─ Step 2: Preview Fixes
├─ Step 3: Apply Fixes
├─ Step 4: Verify Resolution
└─ Step 5: Commit and Push

Command Documentation: 14+ operations
├─ Initialization (pre-checkpoint)
│  ├─ Operation 1: init-checkpoint.sh
│  └─ Operation 2: require-checkpoint.sh
├─ Checkpoint Step 1 Operations
│  ├─ Operation 3: validate-docs.sh
│  └─ Operation 4: checkpoint-update.sh 1
├─ Checkpoint Step 2 Operations
│  ├─ Operation 5: check-docs-conditional-flow.sh
│  ├─ Operation 6: preview-doc-fixes.sh
│  └─ Operation 7: checkpoint-update.sh 2
├─ Checkpoint Step 3 Operations
│  ├─ Operation 8: apply-doc-fixes.sh
│  └─ Operation 9: checkpoint-update.sh 3
├─ Checkpoint Step 4 Operations
│  ├─ Operation 10: verify-docs.sh
│  ├─ Operation 11: checkpoint-update.sh 4
│  └─ Operation 12: gate-checkpoint.sh
├─ Checkpoint Step 5 Operations
│  ├─ Operation 13: commit-docs.sh
│  └─ Operation 14: checkpoint-update.sh 5
└─ Completion
   └─ Operation 15: complete-checkpoint.sh
```

## Documentation Structure

### Correct Pattern with Hierarchy

```markdown
### Initialization (Pre-Checkpoint)

#### Operation 1: Initialize Checkpoint Tracking
./scripts/init-checkpoint.sh check-docs "..."

#### Operation 2: Verify Checkpoint Ready
./scripts/require-checkpoint.sh check-docs

#### [CHECKPOINT START - STEP 1: Validate Docs]

#### Operation 3: Validate Documentation
./scripts/validate-docs.sh

#### Operation 4: Update Checkpoint
./scripts/checkpoint-update.sh check-docs 1

#### [CHECKPOINT END - STEP 1]

#### [CHECKPOINT START - STEP 2: Preview Fixes]

#### Operation 5: Conditional Flow Check
./scripts/check-docs-conditional-flow.sh

#### Operation 6: Preview Fixes
./scripts/preview-doc-fixes.sh

#### Operation 7: Update Checkpoint
./scripts/checkpoint-update.sh check-docs 2

#### [CHECKPOINT END - STEP 2]
```

## Key Alignment Rules

1. **Checkpoint-update calls** must be inside [CHECKPOINT START/END] markers
2. **Checkpoint-update number** must match checkpoint step number (1, 2, 3, etc.)
3. **Step names** must match names given in init-checkpoint.sh
4. **Operations** are documented but not individually tracked
5. **Markers wrap complete logical units** including the checkpoint-update call

## Incorrect Patterns to Avoid

### ❌ Checkpoint-update outside markers
```markdown
#### [CHECKPOINT START - STEP 1]
./scripts/validate-docs.sh
#### [CHECKPOINT END - STEP 1]

./scripts/checkpoint-update.sh check-docs 1  # Wrong: outside markers!
```

### ❌ Wrong checkpoint number
```markdown
#### [CHECKPOINT START - STEP 1]
./scripts/validate-docs.sh
./scripts/checkpoint-update.sh check-docs 5  # Wrong: should be 1!
#### [CHECKPOINT END - STEP 1]
```

### ❌ Operations at wrong level
```markdown
#### [CHECKPOINT START - STEP 1: Validate Docs]

#### Operation 1: Preview Fixes  # Wrong: this is Step 2 content!
./scripts/preview-doc-fixes.sh

#### [CHECKPOINT END - STEP 1]
```

## Benefits of This Hierarchy

### For Users
- Clear workflow structure: "I'm on step 3 of 5"
- Checkpoints provide coarse-grained progress tracking
- Can resume from checkpoint without re-running earlier steps

### For Developers
- Documentation shows all operations with full transparency
- Checkpoint system remains simple (5-10 steps)
- Clear responsibility boundaries

### For Maintenance
- Changing operations doesn't affect checkpoint system
- Adding new operations only requires updating documentation
- Checkpoint numbering stays stable and predictable

## Related Patterns

- [Command Orchestrator and Checkpoint Separation](command-orchestrator-checkpoint-separation.md) - Separation of concerns
- [Checkpoint Workflow Enforcement Pattern](checkpoint-workflow-enforcement-pattern.md) - Complete enforcement pattern
- [Checkpoint Sequential Execution Discipline](checkpoint-sequential-execution-discipline.md) - Sequential execution
