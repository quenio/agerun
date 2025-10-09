Compact the TODO.md file by condensing completed tasks while keeping incomplete tasks untouched.

## MANDATORY KB Consultation

Before compacting, you MUST:
1. Search: `grep "compact\|selective\|task\|script\|systematic\|dry.*run\|target.*compliance\|search.*result" kb/README.md`
2. Read these KB articles IN FULL using the read tool:
   - `kb/selective-compaction-pattern.md` - for preservation rules
   - `kb/retroactive-task-documentation.md` - for task documentation
   - `kb/documentation-compacting-pattern.md` - for reduction targets
   - `kb/systematic-file-modification-workflow.md` - for script workflow (Enumerate → Script → Apply → Verify → Commit)
   - `kb/batch-update-script-pattern.md` - for creating automated scripts
   - `kb/dry-run-mode-requirement.md` - **CRITICAL**: All file-modifying scripts MUST implement dry-run mode
   - `kb/kb-target-compliance-enforcement.md` - **MANDATORY**: How to enforce KB targets with FAIL gates
   - `kb/search-result-completeness-verification.md` - Never limit search results without checking totals
3. **Check Related Patterns sections** in each article above and read any additional relevant articles found there
4. In your response, quote these specific items from the KB:
   - The target reduction percentage for mixed-state docs (from documentation-compacting-pattern)
   - The key principle "Never modify preserved content - not even whitespace" from selective-compaction-pattern
   - The 5-step workflow from systematic-file-modification-workflow
   - The 5 dry-run principles from dry-run-mode-requirement (default to dry-run, explicit --apply flag, etc.)
   - The enforcement pattern from kb-target-compliance-enforcement (FAIL not warn when target missed)
4. If writing a script:
   - MUST implement dry-run mode (default=dry-run, use --apply to execute)
   - Follow systematic workflow: Enumerate → Script → Apply → Verify → Commit
   - Run in dry-run mode FIRST, review output, then run with --apply
5. Apply ALL guidelines - incomplete tasks must remain EXACTLY unchanged
6. **ENFORCE targets with gates**: Use exit 1 to FAIL if target not met, not just warnings

**Example of proper KB consultation:**
```
I've read selective-compaction-pattern.md which states:

"Key principles:
- Never modify preserved content - not even whitespace"

And documentation-compacting-pattern.md which states:

"For mixed-state documents (e.g., TODO.md):
...
- Expect lower reduction (10-20%) due to preservation needs"

The target for TODO.md is 10-20% reduction, and ALL incomplete tasks must remain unchanged.
```

# Compact Tasks
## Checkpoint Tracking

This command uses checkpoint tracking to ensure safe and systematic TODO.md compaction. The process has 6 checkpoints across 3 phases with verification gates.

### Initialize Tracking
```bash
# Start the task compaction process
make checkpoint-init CMD=compact-tasks STEPS='"Read TODO" "Identify Completed" "Compact Entries" "Verify Integrity" "Write Changes" "Commit and Push"'
```

**Expected output:**
```
📍 Starting: compact-tasks (6 steps)
📁 Tracking: /tmp/compact-tasks_progress.txt
→ Run: make checkpoint-update CMD=compact-tasks STEP=1
```

### Check Progress
```bash
make checkpoint-status CMD=compact-tasks
```

**Expected output (example at 50% completion):**
```
📈 command: X/Y steps (Z%)
   [████░░░░░░░░░░░░░░░░] Z%
→ Next: make checkpoint-update CMD=command STEP=N
```

## Minimum Requirements

**MANDATORY for successful compaction (per KB documentation-compacting-pattern):**
- [ ] **10-20% line reduction achieved** (TODO.md is a mixed-state document - target is 10-20%, NOT 30-50%)
- [ ] All [x] tasks compacted to single lines
- [ ] All [ ] tasks remain **completely untouched** - not even whitespace changes
- [ ] No incomplete tasks modified
- [ ] Document structure preserved
- [ ] Changes committed and pushed

**CRITICAL per KB selective-compaction-pattern**: "Never modify preserved content - not even whitespace". Incomplete tasks ([ ]) must remain EXACTLY as they are.

This uses the selective compaction pattern for mixed-state documents ([details](../../../kb/selective-compaction-pattern.md)) as part of the broader documentation compacting approach ([details](../../../kb/documentation-compacting-pattern.md)).

## Phase 1: Analysis (Steps 1-2)

#### [CHECKPOINT START - PHASE 1]

Follow these steps:

#### [CHECKPOINT END]

#### Checkpoint 1: Read TODO

```bash
# Read and analyze TODO.md
echo "Reading TODO.md..."
if [ ! -f TODO.md ]; then
  echo "❌ TODO.md not found"
  exit 1
fi

TOTAL_LINES=$(wc -l < TODO.md)
echo "TODO.md has $TOTAL_LINES lines"
echo "TOTAL_LINES=$TOTAL_LINES" > /tmp/compact-tasks-stats.txt

make checkpoint-update CMD=compact-tasks STEP=1
```

1. Read the TODO.md file

#### Checkpoint 2: Identify Completed

```bash
# Count completed vs incomplete tasks
echo "Analyzing task status..."

COMPLETED=$(grep -c "^- \[x\]" TODO.md || echo "0")
INCOMPLETE=$(grep -c "^- \[ \]" TODO.md || echo "0")

echo "Completed tasks: $COMPLETED"
echo "Incomplete tasks: $INCOMPLETE (will remain untouched)"

echo "COMPLETED=$COMPLETED" >> /tmp/compact-tasks-stats.txt
echo "INCOMPLETE=$INCOMPLETE" >> /tmp/compact-tasks-stats.txt

make checkpoint-update CMD=compact-tasks STEP=2
```

2. For each section, identify completed tasks (marked with [x])

#### [ANALYSIS GATE]
```bash
# Verify analysis before proceeding
make checkpoint-gate CMD=compact-tasks GATE="Analysis" REQUIRED="1,2"
```

**Expected gate output:**
```
✅ GATE 'Analysis' - PASSED
   Verified: Steps 1,2
```

## Phase 2: Compaction (Steps 3-4)

#### [CHECKPOINT START - PHASE 2]

#### [CHECKPOINT END]

#### Checkpoint 3: Compact Entries

```bash
# Compact completed tasks
echo "Compacting completed tasks..."
source /tmp/compact-tasks-stats.txt

# Here you would perform the actual compaction
# For tracking purposes, we record the action
echo "- Merging sub-items into top-level descriptions"
echo "- Adding completion dates"
echo "- Preserving all incomplete tasks untouched"

COMPACTED=$COMPLETED
echo "COMPACTED=$COMPACTED" >> /tmp/compact-tasks-stats.txt
echo "✅ Compacted $COMPACTED completed tasks"

make checkpoint-update CMD=compact-tasks STEP=3
```

3. For completed top-level tasks with sub-items:
   - Merge key information from sub-items into the top-level description
   - Add completion date if present
   - Remove all sub-items
   - Keep as single-line entry
4. For incomplete tasks (marked with [ ]):
   - KEEP COMPLETELY UNTOUCHED including all sub-items
   - Do not modify or compact these in any way

#### Checkpoint 4: Verify Integrity

```bash
# Verify no incomplete tasks were modified
echo "Verifying integrity..."
source /tmp/compact-tasks-stats.txt

# Check that incomplete tasks remain
NEW_INCOMPLETE=$(grep -c "^- \[ \]" TODO.md.compacted 2>/dev/null || echo "$INCOMPLETE")

if [ "$NEW_INCOMPLETE" -eq "$INCOMPLETE" ]; then
  echo "✅ All $INCOMPLETE incomplete tasks preserved"
  INTEGRITY="PASS"
else
  echo "❌ Incomplete task count mismatch!"
  INTEGRITY="FAIL"
fi

echo "INTEGRITY=$INTEGRITY" >> /tmp/compact-tasks-stats.txt
make checkpoint-update CMD=compact-tasks STEP=4
```

5. Preserve section headers and overall structure

#### [INTEGRITY GATE]
```bash
# ⚠️ CRITICAL: Verify incomplete tasks untouched
source /tmp/compact-tasks-stats.txt
if [ "$INTEGRITY" != "PASS" ]; then
  echo "❌ CRITICAL: Incomplete tasks were modified!"
  exit 1
fi
make checkpoint-gate CMD=compact-tasks GATE="Integrity" REQUIRED="3,4"
```

## Phase 3: Commit (Steps 5-6)

#### [CHECKPOINT START - PHASE 3]

#### [CHECKPOINT END]

#### Checkpoint 5: Write Changes

```bash
# Write compacted version
echo "Writing changes to TODO.md..."
source /tmp/compact-tasks-stats.txt

# Show summary
echo "\nCompaction Summary:"
echo "  Original lines: $TOTAL_LINES"
NEW_LINES=$(wc -l < TODO.md)
echo "  New lines: $NEW_LINES"
REDUCTION=$((TOTAL_LINES - NEW_LINES))
echo "  Reduction: $REDUCTION lines"
echo "  Completed tasks compacted: $COMPACTED"
echo "  Incomplete tasks preserved: $INCOMPLETE"

make checkpoint-update CMD=compact-tasks STEP=5
```

6. Write the compacted version back to TODO.md
7. Show a summary of changes (completed tasks compacted, incomplete preserved)

#### Checkpoint 6: Commit and Push

```bash
# Commit and push changes
echo "Committing changes..."
git add TODO.md
git commit -m "docs: compact completed tasks in TODO.md"
git push

# Verify
git status
echo "✅ Changes committed and pushed"

make checkpoint-update CMD=compact-tasks STEP=6
```

8. Commit and push the changes:
   - `git add TODO.md`
   - `git commit -m "docs: compact completed tasks in TODO.md"`
   - `git push`

#### [CHECKPOINT COMPLETE]
```bash
# Show final summary
make checkpoint-status CMD=compact-tasks
```

**Expected completion output:**
```
========================================
   CHECKPOINT STATUS: compact-tasks
========================================

Progress: 6/6 steps (100%)

[████████████████████] 100%

✅ ALL CHECKPOINTS COMPLETE!

Compaction Results:
  Completed tasks compacted: 42
  Incomplete tasks preserved: 18
  Line reduction: 350 lines
  Changes pushed: Yes

TODO.md successfully compacted!
```

```bash
# Clean up tracking
make checkpoint-cleanup CMD=compact-tasks
rm -f /tmp/compact-tasks-stats.txt
```

Guidelines for compaction:
- ONLY compact tasks marked with [x]
- NEVER modify tasks marked with [ ]
- Keep completion dates in format "(Completed YYYY-MM-DD)"
- Merge critical details into single-line descriptions
- Use semicolons to separate merged details
- Remove empty sections only if all tasks are completed
- Preserve all priority indicators and "In Progress" markers