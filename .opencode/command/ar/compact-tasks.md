Compact the TODO.md file by condensing completed tasks while keeping incomplete tasks untouched.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/compact-tasks` workflow is already in progress:

```bash
./scripts/status-checkpoint.sh compact-tasks VERBOSE=--verbose
# Resume: ./scripts/update-checkpoint.sh compact-tasks STEP=N
# Or reset: ./scripts/cleanup-checkpoint.sh compact-tasks && ./scripts/init-checkpoint.sh compact-tasks STEPS='"Measure Baseline" "Categorize Tasks" "Manual Compaction" "Verify Preservation" "Add Self-Entry" "Commit Changes" "Final Verification"'
```

### First-Time Initialization Check

```bash
./scripts/init-checkpoint.sh compact-tasks '"Measure Baseline" "Categorize Tasks" "Manual Compaction" "Verify Preservation" "Add Self-Entry" "Commit Changes" "Final Verification"'
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
./scripts/require-checkpoint.sh compact-tasks
```

## CRITICAL: Mixed-State Document Strategy

**Key Learning from Session 2025-10-08**: TODO.md is a **mixed-state document** requiring selective compaction + manual semantic analysis for completed tasks only.

**Mixed-State Document Constraints**:
- TODO.md contains both completed [x] and incomplete [ ] tasks
- **MANDATORY**: ALL incomplete tasks must remain **completely untouched** (not even whitespace)
- Only completed tasks can be compacted
- Target: 10-20% reduction (lower than pure historical records due to preservation needs)

**Selective Compaction + Manual Analysis**:
1. **Categorize first**: Separate completed [x] from incomplete [ ] tasks
2. **Preserve incomplete**: Leave ALL [ ] tasks exactly as they are - structure, sub-items, whitespace
3. **Manual analysis for completed**: Apply semantic grouping/rewriting ONLY to [x] tasks
   - Merge related completed sub-tasks into parent descriptions
   - Combine similar completed efforts with semicolons
   - Add completion dates and preserve metrics
4. **Verify integrity**: Ensure incomplete task count unchanged

**When to Use Scripts vs. Manual**:
- **Scripts**: For mechanical merging of completed task sub-items (helps with 5-10%)
- **Manual**: For semantic grouping of related completed tasks (achieves full 10-20%)
- **Best Practice**: Manual editing with careful preservation of ALL incomplete work

## MANDATORY KB Consultation

Before compacting, you MUST:
1. Search: `grep "compact\|selective\|mixed.*state\|manual.*semantic" kb/README.md`
2. Read these KB articles IN FULL using the read tool:
   - `kb/selective-compaction-pattern.md` - **CRITICAL**: preservation rules for mixed-state docs
   - `kb/documentation-compacting-pattern.md` - **UPDATED**: automation vs. manual for mixed-state docs
   - `kb/retroactive-task-documentation.md` - for task documentation
   - `kb/kb-target-compliance-enforcement.md` - **MANDATORY**: How to enforce KB targets
3. **Check Related Patterns sections** in each article above and read any additional relevant articles found there
4. In your response, quote:
   - The key principle "Never modify preserved content - not even whitespace" from selective-compaction-pattern
   - The target reduction percentage for mixed-state docs (10-20% from documentation-compacting-pattern)
   - The difference between selective compaction (what) vs. manual semantic analysis (how)
5. For 10-20% reduction: **Use selective compaction + manual semantic analysis** (only on completed tasks)
6. **ENFORCE targets with gates**: Verify 10% minimum achieved AND incomplete tasks untouched

**Example of proper approach:**
```
TODO.md has 150 completed [x] tasks and 75 incomplete [ ] tasks. I will:
1. Apply manual semantic analysis ONLY to the 150 completed tasks
2. Leave ALL 75 incomplete tasks completely untouched
3. Merge related completed sub-tasks into parent descriptions
4. Verify that all 75 incomplete tasks remain after compaction

This achieves 10-20% reduction while preserving all active work.
```

# Compact Tasks - Selective Compaction with Manual Semantic Analysis

## Checkpoint Tracking

This command uses checkpoint tracking to ensure safe and systematic TODO.md compaction through selective compaction + manual semantic analysis. The process has 7 checkpoints across 3 phases with verification gates.

### Initialize Tracking
```bash
# Start the task compaction process
./scripts/init-checkpoint.sh compact-tasks STEPS='"Measure Baseline" "Categorize Tasks" "Manual Compaction" "Verify Preservation" "Add Self-Entry" "Commit Changes" "Final Verification"'
```

**Expected output:**
```
📍 Starting: compact-tasks (7 steps)
📁 Tracking: /tmp/compact-tasks-progress.txt
→ Run: ./scripts/update-checkpoint.sh compact-tasks STEP=1
```

### Check Progress
```bash
./scripts/status-checkpoint.sh compact-tasks
```

**Expected output (example at 43% completion):**
```
📈 compact-tasks: 3/7 steps (43%)
   [████████░░░░░░░░░░░░] 43%
→ Next: ./scripts/update-checkpoint.sh compact-tasks STEP=4
```

## Minimum Requirements

**MANDATORY for successful compaction (per KB documentation-compacting-pattern):**
- [ ] **10-20% line reduction achieved** (TODO.md is a mixed-state document)
- [ ] All [x] tasks compacted using manual semantic analysis
- [ ] All [ ] tasks remain **completely untouched** - not even whitespace changes
- [ ] Incomplete task count verified unchanged
- [ ] Document structure preserved
- [ ] Changes committed

**CRITICAL per KB selective-compaction-pattern**: "Never modify preserved content - not even whitespace". Incomplete tasks ([ ]) must remain EXACTLY as they are. Only apply manual semantic analysis to completed tasks.

This uses selective compaction for mixed-state documents ([details](../../../kb/selective-compaction-pattern.md)) combined with manual semantic analysis ([details](../../../kb/documentation-compacting-pattern.md)).

### Stage 1: Analysis (Steps 1-2)

#### [CHECKPOINT START - STAGE 1]

#### Step 1: Measure Baseline

```bash
# Measure initial state
echo "Measuring TODO.md baseline..."

ORIGINAL_LINES=$(wc -l < TODO.md)
ORIGINAL_BYTES=$(wc -c < TODO.md)

echo "Original: $ORIGINAL_LINES lines, $ORIGINAL_BYTES bytes"

# Save for later verification
echo "ORIGINAL_LINES=$ORIGINAL_LINES" > /tmp/compact-tasks-stats.txt
echo "ORIGINAL_BYTES=$ORIGINAL_BYTES" >> /tmp/compact-tasks-stats.txt

./scripts/update-checkpoint.sh compact-tasks STEP=1
```

**Expected output:**
```
Measuring TODO.md baseline...
Original: 1324 lines, 85432 bytes
✓ Updated checkpoint 1/7 for compact-tasks
```

#### Step 2: Categorize Tasks

```bash
# Categorize completed vs incomplete tasks
echo "Categorizing tasks..."

COMPLETED=$(grep "^- \[x\]" TODO.md | wc -l || echo "0")
INCOMPLETE=$(grep "^- \[ \]" TODO.md | wc -l || echo "0")

echo "Completed tasks: $COMPLETED (can be compacted)"
echo "Incomplete tasks: $INCOMPLETE (MUST remain untouched)"

echo "COMPLETED=$COMPLETED" >> /tmp/compact-tasks-stats.txt
echo "INCOMPLETE=$INCOMPLETE" >> /tmp/compact-tasks-stats.txt

# Identify patterns in completed tasks
echo ""
echo "Analysis questions for completed tasks:"
echo "1. Which completed tasks have verbose sub-items that can be merged?"
echo "2. Are there related completed tasks that can be combined?"
echo "3. Do all completed tasks have completion dates?"
echo "4. Which sections contain only completed tasks?"

./scripts/update-checkpoint.sh compact-tasks STEP=2
```

**Expected output:**
```
Categorizing tasks...
Completed tasks: 150 (can be compacted)
Incomplete tasks: 75 (MUST remain untouched)

Analysis questions for completed tasks:
[...]
✓ Updated checkpoint 2/7 for compact-tasks
```

#### [CATEGORIZATION GATE]
```bash
# Verify categorization before proceeding to manual work
./scripts/gate-checkpoint.sh compact-tasks "Categorization Complete" "1,2"
```

**Expected gate output:**
```
✅ GATE 'Categorization Complete' - PASSED
   Verified: Steps 1,2
```

#### [CHECKPOINT END]

### Stage 2: Manual Selective Compaction (Steps 3-4)

#### [CHECKPOINT START - STAGE 2]

#### Step 3: Manual Semantic Compaction

**MANUAL WORK REQUIRED**: Edit TODO.md using your text editor. Apply these strategies to **COMPLETED TASKS ONLY**:

**Merging Completed Sub-Tasks**:
```markdown
# Before:
- [x] Agent Store Load Implementation (Completed 2025-10-07)
  - [x] TDD Cycle 9: Create agents with method lookup
  - [x] TDD Cycle 10: Restore agent memory from map
  - [x] TDD Cycle 11: Handle multiple agents with memory
  - [x] Test with 3+ agents with different methods
  - [x] Ensure proper resource management

# After (single line):
- [x] Agent Store Load Implementation: Completed TDD Cycles 9-11 with method lookup, memory restoration, and multi-agent handling; verified proper resource management with 3+ agents; zero memory leaks (Completed 2025-10-07)
```

**Combining Related Completed Tasks**:
```markdown
# Before (multiple completed tasks):
- [x] Fixed memory leaks in agent module
- [x] Updated documentation for new API
- [x] Added comprehensive test coverage

# After (combined if closely related):
- [x] Agent Module Cleanup: Fixed memory leaks; updated documentation for new API; added comprehensive test coverage (Completed 2025-MM-DD)
```

**CRITICAL - Incomplete Tasks**:
```markdown
# NEVER change these - leave EXACTLY as they are:
- [ ] Implement new feature X
  - [ ] Design interface
  - [ ] Write implementation  
  - [ ] Add tests

# Not even whitespace - preserve structure, formatting, everything
```

**Compaction Guidelines for Completed Tasks**:
- Merge sub-items into parent description with semicolons
- Preserve completion dates: `(Completed YYYY-MM-DD)`
- Keep all metrics: file counts, test counts, reduction percentages
- Remove verbose sub-bullets but preserve key outcomes
- Add context if the parent task name alone isn't clear

```bash
# After completing manual edits
echo "✅ Manual semantic compaction of completed tasks complete"
echo "Review changes with: git diff TODO.md"
./scripts/update-checkpoint.sh compact-tasks STEP=3
```

#### Step 4: Verify Preservation

```bash
# Verify compaction using helper script
./scripts/verify-compaction.sh /tmp/compact-tasks-stats.txt | tee -a /tmp/compact-tasks-stats.txt

# Only proceed if verification passes
if [ $? -eq 0 ]; then
  ./scripts/update-checkpoint.sh compact-tasks STEP=4
else
  echo "❌ Verification failed - do not proceed"
  exit 1
fi
```

The script verifies:
1. **Size Comparison** - Original vs new lines/bytes
2. **Reduction Achieved** - Percentage reductions calculated
3. **Task Preservation** - All incomplete tasks preserved (CRITICAL)

**Expected output:**
```
Verifying preservation...
New: 1120 lines, 72548 bytes
Incomplete tasks: 75 (expected: 75)
Completed tasks: 95 (some may have been merged)
Line reduction: 15%
File size reduction: 15%
✅ All 75 incomplete tasks preserved
✓ Updated checkpoint 4/7 for compact-tasks
```

#### [INTEGRITY GATE]
```bash
# ⚠️ CRITICAL: Verify incomplete tasks untouched and minimum reduction
source /tmp/compact-tasks-stats.txt

if [ "$INTEGRITY" != "PASS" ]; then
  echo "❌ CRITICAL: Incomplete tasks were modified!"
  echo "Per selective-compaction-pattern.md: 'Never modify preserved content - not even whitespace'"
  exit 1
fi

if [ $LINE_REDUCTION -lt 10 ]; then
  echo "❌ FAILURE: Only ${LINE_REDUCTION}% line reduction (KB target: 10-20%)"
  echo "Per documentation-compacting-pattern.md, TODO.md requires 10-20% reduction"
  exit 1
fi

echo "✅ Integrity verified and reduction target met"
./scripts/gate-checkpoint.sh compact-tasks "Selective Compaction Quality" "3,4"
```

**Expected gate output:**
```
✅ Integrity verified and reduction target met
✅ GATE 'Selective Compaction Quality' - PASSED
   Verified: Steps 3,4
```

#### [CHECKPOINT END]

### Stage 3: Documentation and Commit (Steps 5-7)

#### [CHECKPOINT START - STAGE 3]

#### Step 5: Add Self-Entry

```bash
# Add self-documenting entry
echo "Adding self-documenting entry to TODO.md..."
source /tmp/compact-tasks-stats.txt

echo ""
echo "Add this entry under 'Completed Tasks' section in TODO.md:"
echo ""
echo "- [x] TODO.md Selective Compaction: Reduced from $ORIGINAL_LINES to $NEW_LINES lines (${LINE_REDUCTION}% reduction) through manual semantic analysis of ${COMPLETED} completed tasks; preserved all ${INCOMPLETE} incomplete tasks untouched per selective-compaction-pattern.md; merged sub-tasks into parent descriptions with semicolons; maintained completion dates and metrics (Completed $(date +%Y-%m-%d))"
echo ""

read -p "Press Enter after adding the entry..."

./scripts/update-checkpoint.sh compact-tasks STEP=5
```

#### Step 6: Commit Changes

```bash
# Commit documentation updates
echo "Committing changes..."
source /tmp/compact-tasks-stats.txt

git add TODO.md
git commit -m "Compact completed tasks in TODO.md using selective compaction

Applied manual semantic analysis to ${COMPLETED} completed tasks while preserving all ${INCOMPLETE} incomplete tasks completely untouched. Merged sub-items into parent descriptions, maintained completion dates and metrics.

Reduction: $ORIGINAL_LINES→$NEW_LINES lines (${LINE_REDUCTION}%)
Completed tasks compacted: ${COMPLETED}
Incomplete tasks preserved: ${INCOMPLETE}

Per selective-compaction-pattern.md: 'Never modify preserved content - not even whitespace'"

git status
echo ""
echo "✅ Changes committed"

./scripts/update-checkpoint.sh compact-tasks STEP=6
```

**Expected output:**
```
[main abc1234] Compact completed tasks in TODO.md using selective compaction
 1 file changed, 95 insertions(+), 204 deletions(-)
On branch main
Your branch is ahead of 'origin/main' by 1 commit.
[...]
✅ Changes committed
✓ Updated checkpoint 6/7 for compact-tasks
```

#### Step 7: Final Verification

```bash
# Verify commit and incomplete tasks one final time
echo "Final verification..."
source /tmp/compact-tasks-stats.txt

# Check incomplete tasks after commit
FINAL_INCOMPLETE=$(grep "^- \[ \]" TODO.md | wc -l || echo "0")

if [ "$FINAL_INCOMPLETE" -ne "$INCOMPLETE" ]; then
  echo "❌ CRITICAL: Incomplete tasks changed after commit!"
  echo "   Expected: $INCOMPLETE, Found: $FINAL_INCOMPLETE"
  exit 1
fi

echo "✅ Final verification passed"
echo "   Incomplete tasks: $FINAL_INCOMPLETE (unchanged)"
echo "   Line reduction: ${LINE_REDUCTION}%"
echo "   Commit status: Clean"

./scripts/update-checkpoint.sh compact-tasks STEP=7
```

#### [CHECKPOINT COMPLETE]
```bash
./scripts/complete-checkpoint.sh compact-tasks
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 compact-tasks: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
rm -f /tmp/compact-tasks-stats.txt
```

#### [CHECKPOINT END]

## Key Compaction Techniques for Mixed-State Documents

**Selective Compaction (What to Compact)**:
- ONLY compact completed tasks marked with [x]
- NEVER modify incomplete tasks marked with [ ]
- Preserve ALL structure, sub-items, whitespace for incomplete tasks
- Verify incomplete task count remains unchanged

**Manual Semantic Analysis (How to Compact Completed Tasks)**:
- Merge sub-items into parent descriptions using semicolons
- Combine closely related completed tasks if appropriate
- Preserve completion dates: `(Completed YYYY-MM-DD)`
- Keep all metrics: file counts, test counts, percentages
- Ensure parent task description is clear and contextual

**Preservation Strategy**:
- Incomplete tasks: 100% untouched (structure, whitespace, everything)
- Completed tasks: Compact while preserving dates, metrics, key outcomes
- Section headers: Keep all, even if section becomes smaller
- Priority indicators: Preserve in both completed and incomplete tasks

## Why Selective Compaction + Manual Analysis?

**Mixed-state constraint**:
- TODO.md contains active work (incomplete tasks) that must remain fully detailed
- Cannot extract to external files - this IS the active work tracker
- Lower reduction target (10-20%) reflects this preservation requirement

**Manual analysis adds value**:
- Identifies which completed sub-tasks can be meaningfully merged
- Preserves context while reducing verbosity
- Ensures completed task descriptions remain clear and useful
- Achieves target reduction without compromising incomplete work