Create a TDD plan document following strict methodology patterns and best practices.

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for progress tracking ONLY. All verification is done via specialized tools, NOT via checkpoint scripts ([details](../../../kb/checkpoint-tracking-verification-separation.md)).

This section implements the [Checkpoint Workflow Enforcement Pattern](../../../kb/checkpoint-workflow-enforcement-pattern.md) - preventing workflow bypasses through initialization and precondition enforcement.

### In-Progress Workflow Detection

If a `/create-plan` workflow is already in progress:

```bash
# Check current progress
./scripts/checkpoint-status.sh create-plan --verbose
```

Resume from the next pending step, or clean up and start fresh:
```bash
./scripts/checkpoint-cleanup.sh create-plan
./scripts/checkpoint-init.sh create-plan "KB Consultation" "Read Requirements" "Extract Iterations" "Structure Plan" "Validate Plan" "Summary"
```

### First-Time Initialization Check

**MANDATORY**: Before executing ANY steps, verify checkpoint tracking is initialized:

```bash
./scripts/checkpoint-init.sh create-plan "KB Consultation" "Read Requirements" "Extract Iterations" "Structure Plan" "Validate Plan" "Summary"
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

**BEFORE PROCEEDING**: Verify checkpoint tracking initialization:

```bash
./scripts/checkpoint-require.sh create-plan
```

**MANDATORY**: This command MUST use checkpoint tracking. Start by running the checkpoint initialization below. ([details](../../../kb/unmissable-documentation-pattern.md))

## STEP VERIFICATION ENFORCEMENT

**MANDATORY**: After completing each step, you MUST verify step completion using the **step-verifier sub-agent** before proceeding to the next step ([details](../../../kb/sub-agent-verification-pattern.md)).

### About the step-verifier Sub-Agent

The **step-verifier** is a specialized sub-agent that independently verifies step completion:

- **Reads command files** to understand step requirements
- **Checks files, git status/diff, test results, build outputs** to verify accomplishments
- **Compares accomplishments against requirements** systematically
- **Reports verification results with evidence** (what was verified, what's missing)
- **Provides STOP instructions** when failures are detected (blocks execution until fixed)
- **Read-only agent**: Never modifies files, commits changes, or makes autonomous decisions

**CRITICAL**: The step-verifier independently verifies your claims. You report accomplishments with evidence; the step-verifier verifies by reading files and checking outputs.

### Step Verification Process

After completing each step (before calling `checkpoint-update.sh`), you MUST:

1. **Report accomplishments with evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide evidence (file paths, command outputs, git status/diff)
   - **DO NOT** tell step-verifier what to verify - report what was done

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     - Mark checkpoint step as complete (for progress tracking only - verification already done by step-verifier)
  
   **If verification FAILS** (report shows "⚠️ STOP EXECUTION" or missing elements):
     - **STOP execution immediately** - do not proceed to next step
     - Fix all reported issues from verification report
     - Re-invoke step-verifier with updated evidence after fixes
     - Only proceed after verification report shows "✅ STEP VERIFIED"
  
   **If sub-agent CANNOT be executed** (MCP unavailable or tool error):
     - STOP execution immediately
     - Inform user: "⚠️ Step verification sub-agent unavailable. Please manually verify Step N completion before proceeding."
     - Wait for explicit user confirmation before proceeding

### How to Invoke step-verifier

Use the `mcp_sub-agents_run_agent` tool:

```
Agent: "step-verifier"
Prompt: "Verify Step N: [Step Title] completion for create-plan command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/create-plan.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with evidence: files created/modified, commands executed, outputs produced, etc. The step-verifier will independently verify these claims by reading files, checking git status, etc.]"
```

**CRITICAL**: 
- Report accomplishments with evidence, NOT instructions
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

**MANDATORY: Session Todo List Tracking**

Each step MUST be added to the session todo list before execution begins ([details](../../../kb/session-todo-list-tracking-pattern.md)):
- Use `todo_write` to add each step as a todo item with status `in_progress` before starting the step
- Use `todo_write` to mark each step as `completed` after step-verifier verification passes
- This ensures the session maintains track of all steps to be executed

## KB Consultation Required

Before creating any plan ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "plan.*iteration\|TDD.*minimalism\|plan.*refinement" kb/README.md`
2. Read these KB articles IN FULL using the Read tool:
   - `kb/tdd-plan-review-checklist.md` ⭐ **NEW - READ FIRST** - embeds all 14 TDD lessons
   - `kb/red-phase-dual-goals-pattern.md` ⭐ **CRITICAL** - Two independent goals of RED phase
   - `kb/command-pipeline-methodology-enforcement.md` - How all three commands work together
   - `kb/lesson-based-command-design-pattern.md` - Unified 14-lesson verification pattern
   - `kb/tdd-iteration-planning-pattern.md`
   - `kb/tdd-plan-iteration-split-pattern.md`
   - `kb/tdd-green-phase-minimalism.md`
   - `kb/tdd-red-phase-assertion-requirement.md`
   - `kb/temporary-test-cleanup-pattern.md`
   - `kb/plan-review-status-tracking.md`
   - `kb/iterative-plan-review-protocol.md`
   - `kb/iterative-plan-refinement-pattern.md`
   - `kb/bdd-test-structure.md`
   - `kb/ownership-naming-conventions.md`
3. Check Related Patterns sections in each article and read any additional relevant articles found there
4. In your response, quote these specific items from the KB:
   - The 14 critical TDD lessons (from tdd-plan-review-checklist.md)
   - The one-assertion-per-iteration principle
   - When to use hardcoded returns in GREEN phase
   - The temporary cleanup comment format (MANDATORY)
   - The BDD test structure sections (Given/When/Then/Cleanup)
   - The ownership prefix conventions (own_, ref_, mut_)
   - **⭐ CRITICAL**: Assertion validity verification - RED phases MUST document temporary corruption

**Example of proper KB consultation:**
```
I've read tdd-iteration-planning-pattern.md which states:

"Each TDD iteration should test exactly ONE assertion. When planning TDD work,
count the number of assertions needed, not the number of features."

And tdd-green-phase-minimalism.md which requires:

"The GREEN phase should implement ONLY enough code to pass the specific assertion
being tested, even if that means hardcoded return values. Later iterations force
proper implementation by introducing new requirements."

The temporary cleanup format from temporary-test-cleanup-pattern.md is:
// Cleanup (temporary: manually destroy X since not registered yet)

BDD structure from bdd-test-structure.md:
// Given [setup state]
// When [action being tested]
// Then [verification]
// Cleanup
```

**CRITICAL**: If you skip reading these KB articles, the plan will violate TDD methodology and require extensive revision.

## Task Identification

Before creating the plan, identify which task to plan:

### Priority Order (highest to lowest):

1. **User-provided arguments** (supersedes everything)
   - Check if the user provided task name and instructions via command arguments
   - Format: `/create-plan <task-description> [additional-instructions]`
   - Example: `/create-plan "implement message queue" "use FIFO ordering"`

2. **Context from ar:next-priority or ar:next-task**
   - If no user arguments, check conversation history for recent ar:next-priority or ar:next-task output
   - These commands suggest which task to implement next based on priorities
   - Extract the task description from their output

3. **Ask the user**
   - If neither user arguments nor context available, ask the user which task to plan

**Example context extraction:**
```
# From ar:next-priority output:
"Next priority: Implement delegate message queue infrastructure"
→ Task: "delegate message queue infrastructure"

# From ar:next-task output:
"Recommended next task: Add error handling to ar_agent__create"
→ Task: "error handling to ar_agent__create"

# From user arguments:
/create-plan "fixture lifecycle management" "focus on ownership transfer"
→ Task: "fixture lifecycle management"
→ Additional context: "focus on ownership transfer"
```

**Task identification checklist:**
- [ ] Check for user-provided task name in arguments
- [ ] If none, search conversation for ar:next-priority/ar:next-task output
- [ ] Extract task description and any additional context
- [ ] Confirm task understanding before proceeding
- [ ] If unclear, ask user to clarify which task to plan

# Create Plan

## ⚠️ REQUIRED: Initialize Checkpoint Tracking First

**DO NOT PROCEED WITHOUT RUNNING THIS COMMAND:**

```bash
# MANDATORY: Initialize checkpoint tracking (14 steps)
./scripts/checkpoint-init.sh create-plan "KB Consultation" "Gather Requirements" "Identify Behaviors" "Count Assertions" "Define Cycles" "Plan Iterations" "Structure RED Phases" "Structure GREEN Phases" "Add Cleanup" "Add Status Markers" "Add Cross-References" "Validate Plan" "Save Plan" "Summary"
```

This command uses checkpoint tracking to ensure systematic plan creation. The creation process is divided into 4 major stages with 14 checkpoints total.

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: create-plan
Tracking file: /tmp/create-plan-progress.txt
Total steps: 14

Steps to complete:
  1. KB Consultation
  2. Gather Requirements
  3. Identify Behaviors
  4. Count Assertions
  5. Define Cycles
  6. Plan Iterations
  7. Structure RED Phases
  8. Structure GREEN Phases
  9. Add Cleanup
  10. Add Status Markers
  11. Add Cross-References
  12. Validate Plan
  13. Save Plan
  14. Summary
```

### Check Progress
```bash
./scripts/checkpoint-status.sh create-plan
```

**Expected output (example at 50% completion):**
```
📈 create-plan: 7/14 steps (50%)
   [██████████░░░░░░░░░░] 50%
→ Next: ./scripts/checkpoint-update.sh create-plan STEP=8
```

### What it does

This command creates TDD plan documents following strict methodology:

#### 1. Requirements Analysis
- **Behavior identification**: List all expected behaviors
- **Assertion counting**: One assertion per iteration
- **Cycle organization**: Group related behaviors (3-5 iterations per cycle)
- **API design**: Ensure return types enable verification

#### 2. Iteration Structure
- **One assertion per iteration**: Never bundle multiple behaviors ([details](../../../kb/tdd-iteration-planning-pattern.md))
- **Decimal numbering**: Use N.1, N.2 for splits, not renumbering ([details](../../../kb/tdd-plan-iteration-split-pattern.md))
- **RED phase clarity**: Every iteration shows explicit failure (// ← FAILS)
- **GREEN minimalism**: Hardcoded returns valid ([details](../../../kb/tdd-green-phase-minimalism.md))

#### 3. Memory Management
- **Temporary cleanup**: .1 iterations need manual cleanup ([details](../../../kb/temporary-test-cleanup-pattern.md))
- **Zero leak policy**: Every iteration verified for leaks
- **Ownership semantics**: Proper own_, ref_, mut_ prefixes ([details](../../../kb/ownership-naming-conventions.md))

#### 4. Documentation and Tracking
- **Status markers**: PENDING REVIEW for all iterations initially ([details](../../../kb/plan-review-status-tracking.md))
- **BDD structure**: Given/When/Then/Cleanup throughout ([details](../../../kb/bdd-test-structure.md))
- **Real types**: Use actual AgeRun types (ar_*_t) not placeholders
- **Cross-references**: Link to relevant KB articles

### Status Marker Lifecycle

This command creates plans with iterations marked `PENDING REVIEW`. These markers track progress through the complete TDD workflow:

| Status | Used By | Meaning | Next Step |
|--------|---------|---------|-----------|
| `PENDING REVIEW` | create-plan | Newly created iteration awaiting review | Review with ar:review-plan |
| `REVIEWED` | review-plan | Iteration approved, ready for implementation | Execute with ar:execute-plan |
| `REVISED` | review-plan | Iteration updated after review, ready for implementation | Execute with ar:execute-plan |
| `IMPLEMENTED` | execute-plan | RED-GREEN-REFACTOR complete, awaiting commit | Commit preparation |
| `✅ COMMITTED` | execute-plan | Iteration committed to git | Done (or continue with next iteration) |
| `✅ COMPLETE` | execute-plan | Full plan complete (plan-level marker) | Documentation only |

**Important Notes:**
- **Iterations only**: Status markers appear ONLY on iteration headings (not phase/cycle headings)
- **REVISED meaning**: Changes applied and ready for implementation (ar:execute-plan processes REVISED same as REVIEWED)
- **Two-phase updates**: During execution, iterations update REVIEWED/REVISED → IMPLEMENTED immediately; before commit, all IMPLEMENTED → ✅ COMMITTED in batch
- **Complete vs Committed**: ✅ COMPLETE is optional plan-level header; ✅ COMMITTED marks individual iterations in git

### Execution Order (MANDATORY)

1. **FIRST**: Run the checkpoint initialization command above
2. **SECOND**: Follow the creation process below:
   - Add each step to session todo list before starting
   - Complete the step work
   - **VERIFY step completion via step-verifier sub-agent** (MANDATORY)
   - Mark step complete in session todo list after verification passes
   - Update checkpoint (for progress tracking only)
3. **THIRD**: Check progress with `./scripts/checkpoint-status.sh create-plan`
4. **FOURTH**: Complete all 14 steps before saving plan (each verified via step-verifier)
5. **LAST**: Verify complete workflow via step-verifier, then clean up with `./scripts/checkpoint-cleanup.sh create-plan`

### Usage

```bash
/create-plan <feature-name> <output-file>
```

**Example:**
```bash
/create-plan "message queue infrastructure" plans/tdd_cycle_7_plan.md
```

**IMPORTANT**: Running `/create-plan` alone is NOT sufficient. You MUST initialize checkpoints first as shown above.

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: KB Consultation & 14 Lesson Verification" - Status: pending
- Add todo item: "Verify Step 1: KB Consultation & 14 Lesson Verification" - Status: pending
- Add todo item: "Step 2: Gather Requirements" - Status: pending
- Add todo item: "Verify Step 2: Gather Requirements" - Status: pending
- Add todo item: "Step 3: Identify Behaviors" - Status: pending
- Add todo item: "Verify Step 3: Identify Behaviors" - Status: pending
- Add todo item: "Step 4: Count Assertions" - Status: pending
- Add todo item: "Verify Step 4: Count Assertions" - Status: pending
- Add todo item: "Step 5: Define Cycles" - Status: pending
- Add todo item: "Verify Step 5: Define Cycles" - Status: pending
- Add todo item: "Step 6: Plan Iterations" - Status: pending
- Add todo item: "Verify Step 6: Plan Iterations" - Status: pending
- Add todo item: "Step 7: Structure RED Phases" - Status: pending
- Add todo item: "Verify Step 7: Structure RED Phases" - Status: pending
- Add todo item: "Step 8: Structure GREEN Phases" - Status: pending
- Add todo item: "Verify Step 8: Structure GREEN Phases" - Status: pending
- Add todo item: "Step 9: Add Cleanup" - Status: pending
- Add todo item: "Verify Step 9: Add Cleanup" - Status: pending
- Add todo item: "Step 10: Add Status Markers" - Status: pending
- Add todo item: "Verify Step 10: Add Status Markers" - Status: pending
- Add todo item: "Step 11: Add Cross-References" - Status: pending
- Add todo item: "Verify Step 11: Add Cross-References" - Status: pending
- Add todo item: "Step 12: Validate Plan" - Status: pending
- Add todo item: "Verify Step 12: Validate Plan" - Status: pending
- Add todo item: "Step 13: Save Plan" - Status: pending
- Add todo item: "Verify Step 13: Save Plan" - Status: pending
- Add todo item: "Step 14: Summary" - Status: pending
- Add todo item: "Verify Step 14: Summary" - Status: pending
- Add todo item: "Verify Complete Workflow: create-plan" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

## Plan Creation Process

### Stage 1: Requirements and Analysis (Steps 1-5)

#### [CHECKPOINT START - STAGE 1]

#### Step 1: KB Consultation & 14 Lesson Verification

**Mandatory KB Reading:**
Read all 11 KB articles listed above (with tdd-plan-review-checklist.md first) and quote all 7 specific items.

**CRITICAL: Verify All 14 TDD Lessons Before Plan Creation**

Before proceeding, confirm understanding of these 14 critical lessons from kb/tdd-plan-review-checklist.md:

- [ ] **Lesson 1**: Iteration numbering clarity prevents confusion (sequential, no gaps)
- [ ] **Lesson 2**: One assertion per iteration creates reliable pace
- [ ] **Lesson 3**: Hardcoded returns are acceptable for minimalism
- [ ] **Lesson 4**: Minimal implementations must still clean up resources
- [ ] **Lesson 5**: Tests drive implementation through dependency chains
- [ ] **Lesson 6**: Integration tests catch module seam bugs
- [ ] **Lesson 7** ⭐ **CRITICAL**: Assertion validity MUST be verified via temporary corruption
  - RED phases MUST document temporary code that BREAKS the feature
  - This PROVES the assertion catches real bugs (not just always-passing tests)
  - Example: "Don't queue message so assertion fails" or "Corrupt type field so assertion fails"
- [ ] **Lesson 8**: Temporary code in tests is valid TDD technique (not a smell)
- [ ] **Lesson 9**: Property validation through independent assertions
- [ ] **Lesson 10**: Clear test type distinction (TDD vs Validation vs Hybrid)
- [ ] **Lesson 11**: Over-implementation in GREEN violates minimalism
- [ ] **Lesson 12**: Commit messages should document methodology
- [ ] **Lesson 13**: Forward dependencies disable sequential reading
- [ ] **Lesson 14**: Resource ownership naming conventions matter (own_, ref_, mut_)

**Lesson 7 is MOST CRITICAL**: Every RED phase in the plan MUST include documentation of how the test will fail before GREEN code is written. This proves assertions actually catch bugs.

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 1: KB Consultation & 14 Lesson Verification"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 2, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 1: KB Consultation & 14 Lesson Verification"
- Status: in_progress

Before proceeding to Step 2, you MUST verify Step 1 completion via **step-verifier sub-agent**:

1. **Report accomplishments with evidence**:
   - All 11 KB articles were read (list file paths)
   - All 7 specific items were quoted (provide quotes)
   - All 14 TDD lessons were verified (describe verification)
   - KB consultation was completed before proceeding (describe completion)
   - Step objectives were met (describe what was accomplished)

2. **Invoke step-verifier sub-agent** using `mcp_sub-agents_run_agent`:
   - Agent: `"step-verifier"`
   - Prompt: "Verify Step 1: KB Consultation & 14 Lesson Verification completion for create-plan command. Todo Item: [what was accomplished]. Command File: .opencode/command/ar/create-plan.md. Step: Step 1: KB Consultation & 14 Lesson Verification. Accomplishment Report: [evidence of what was accomplished]"
   - The step-verifier will independently verify by reading KB articles, checking quotes, verifying TDD lessons, etc.

3. **Handle verification results**:
   - **If "✅ STEP VERIFIED"**: Proceed to Step 2
   - **If "⚠️ STOP EXECUTION"**: Fix reported issues, re-invoke step-verifier, only proceed after verification passes
   - **If sub-agent unavailable**: STOP and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 1: KB Consultation & 14 Lesson Verification"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 1: KB Consultation & 14 Lesson Verification"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=1
```

#### Step 2: Gather Requirements

**Collect feature requirements:**
- [ ] What is the feature name?
- [ ] What is the main objective?
- [ ] What existing modules/types will be used?
- [ ] What new modules/types will be created?
- [ ] What patterns does this follow? (e.g., similar to ar_agent/ar_agency)

**Document requirements:**
```markdown
## Feature: [Name]

### Objective
[1-2 sentence description]

### Context
- Existing types: ar_delegation_t, ar_delegate_t
- Pattern: Similar to ar_agent/ar_agency message passing
- Module: ar_delegation.h (new functions)

### Success Criteria
- Zero memory leaks
- BDD test structure
- One assertion per iteration
- All tests pass
```

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 2: Gather Requirements"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 3, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 2: Gather Requirements"
- Status: in_progress

Before proceeding to Step 3, you MUST verify Step 2 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Feature name was identified
   - Main objective was documented
   - Existing modules/types were listed
   - New modules/types were identified
   - Patterns were identified (similar implementations)
   - Requirements document was created
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 2: Gather Requirements"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 2: Gather Requirements"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=2
```

#### Step 3: Identify Behaviors

**List ALL expected behaviors:**

For each function to be implemented, list what it should do:
```markdown
## Behaviors to Test

### ar_delegate__send()
1. Returns true when successful
2. Queues the message
3. Transfers ownership to delegate
4. Returns false when delegate is NULL
5. Returns false when message is NULL

### ar_delegate__has_messages()
1. Returns false when queue is empty
2. Returns true after send()
3. Returns false after take_message() empties queue

### ar_delegate__take_message()
1. Returns NULL when queue is empty
2. Returns message when available
3. Removes message from queue
4. Transfers ownership to caller
```

**Critical: Each behavior becomes ONE iteration**

**⭐ MANDATORY: NULL Parameter Validation (from session 2025-10-18)**

For EVERY function in your plan, systematically verify NULL parameter handling coverage:

**Systematic NULL Parameter Coverage Check:**

For each function introduced in the plan:

1. **Identify function signature**
   ```c
   // Example: create function with 2 parameters
   ar_file_delegate_t* ar_file_delegate__create(ar_log_t *ref_log, const char *ref_path);  // EXAMPLE: Hypothetical type
   ```

2. **Count parameters**
   - ref_log (parameter 1)
   - ref_path (parameter 2)

3. **Verify NULL handling iterations exist:**
   - [ ] Iteration X.Y.1: create() handles NULL log
   - [ ] Iteration X.Y.2: create() handles NULL path

4. **Pattern for each parameter:**
   ```markdown
   #### Iteration X.Y.1: create() handles NULL log - PENDING REVIEW

   **Objective**: Verify function handles NULL log parameter gracefully

   **RED Phase:**
   - Call function with NULL log parameter
   - Assert function returns NULL (or appropriate error value)

   **GREEN Phase:**
   - Add NULL check for log parameter
   - Return NULL if log is NULL
   ```

5. **If missing NULL iterations:**
   - STOP behavior listing
   - Add NULL parameter iterations immediately
   - Pattern: One iteration per NULL-able parameter

**Common Functions Needing NULL Checks:**

```markdown
✅ CORRECT: Comprehensive NULL parameter coverage

### ar_foo_t* ar_foo__create(ar_log_t *ref_log, const char *ref_path)  // EXAMPLE: Hypothetical function
Behaviors:
1. create() returns non-NULL (main success case)
2. create() handles NULL log         ← NULL parameter 1
3. create() handles NULL path         ← NULL parameter 2
4. create() handles malloc failure    ← Allocation failure (use dlsym technique)

### void ar_foo__destroy(ar_foo_t *own_foo)  // EXAMPLE: Hypothetical function
Behaviors:
1. destroy() handles NULL foo         ← NULL parameter 1

### bool ar_foo__set_max_size(ar_foo_t *mut_foo, size_t max_size)  // EXAMPLE: Hypothetical function
Behaviors:
1. set_max_size() returns true (success case)
2. set_max_size() handles NULL foo    ← NULL parameter 1

### bool ar_foo__write(ar_foo_t *mut_foo, const char *ref_path, ar_data_t *ref_content)  // EXAMPLE: Hypothetical function
Behaviors:
1. write() returns true (success case)
2. write() handles NULL foo           ← NULL parameter 1
3. write() handles NULL path          ← NULL parameter 2
4. write() handles NULL content       ← NULL parameter 3
```

**❌ WRONG: Missing NULL parameter coverage**

```markdown
### ar_foo_t* ar_foo__create(ar_log_t *ref_log, const char *ref_path)  // EXAMPLE: Hypothetical function
Behaviors:
1. create() returns non-NULL

Problem: Missing NULL checks for both ref_log and ref_path parameters!
Required: Add iterations for NULL log, NULL path, and malloc failure
```

**NULL Parameter Coverage Checklist:**

For EVERY function in the plan, verify:
- [ ] Function signature identified
- [ ] Parameter count determined
- [ ] NULL handling iteration exists for EACH parameter
- [ ] Allocation failure iteration exists if function calls malloc/STRDUP
- [ ] destroy() functions have NULL parameter check
- [ ] setter functions have NULL object parameter check
- [ ] operation functions have NULL checks for all pointer parameters

**Allocation Failure Testing:**

For functions that allocate memory (malloc/STRDUP), add allocation failure iterations:

```markdown
#### Iteration X.Y.3: create() handles malloc failure - PENDING REVIEW

**Objective**: Verify function handles memory allocation failure gracefully

**Note**: This test uses the dlsym technique to intercept malloc calls.
See [dlsym-test-interception-technique.md](../kb/dlsym-test-interception-technique.md)

**RED Phase:**
```c
// Control which malloc call should fail
static int fail_at_malloc = -1;
static int current_malloc = 0;

void* malloc(size_t size) {
    typedef void* (*malloc_fn)(size_t);
    static malloc_fn real_malloc = NULL;

    if (!real_malloc) {
        union { void* obj; malloc_fn func; } converter;
        converter.obj = dlsym(RTLD_NEXT, "malloc");
        real_malloc = converter.func;
    }

    current_malloc++;
    if (current_malloc == fail_at_malloc) {
        return NULL;  // Simulate failure
    }

    return real_malloc(size);
}

static void test_foo__create_handles_malloc_failure(void) {
    // Test malloc failure at each allocation point
    fail_at_malloc = 1;  // Fail first malloc
    ar_foo_t *own_foo = ar_foo__create(ref_log, "test");  // EXAMPLE: Hypothetical function
    AR_ASSERT(own_foo == NULL, "Should return NULL on malloc failure");
}
```

**GREEN Phase:**
```c
ar_foo_t* ar_foo__create(ar_log_t *ref_log, const char *ref_path) {  // EXAMPLE: Hypothetical function
    // Check malloc result
    ar_foo_t *own_foo = AR__HEAP__MALLOC(sizeof(ar_foo_t));  // EXAMPLE: Using hypothetical type
    if (!own_foo) return NULL;  // Handle malloc failure

    // Continue with initialization...
    return own_foo;
}
```
```

**MANDATORY**: Before proceeding to Step 4, verify ALL functions have complete NULL parameter coverage.

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 3: Identify Behaviors"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 4, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 3: Identify Behaviors"
- Status: in_progress

Before proceeding to Step 4, you MUST verify Step 3 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - All expected behaviors were listed for each function
   - NULL parameter coverage was verified for ALL functions
   - Allocation failure iterations were added where needed
   - Each behavior maps to one iteration
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 3: Identify Behaviors"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 3: Identify Behaviors"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=3
```

#### Step 4: Count Assertions

**Convert behaviors to assertions:**

For each behavior, write the exact assertion:
```markdown
## Iteration Mapping

Iteration 1.1: ar_delegate__send() returns true
  Assertion: AR_ASSERT(result, "Send should return true");

Iteration 1.2: ar_delegate__has_messages() returns false initially
  Assertion: AR_ASSERT(!has_messages, "Should have no messages initially");

Iteration 1.3: ar_delegate__has_messages() returns true after send
  Assertion: AR_ASSERT(has_messages, "Should have messages after send");

[Continue for all behaviors...]

Total iterations: 12
```

**Check for multi-assertion violations:**
- ❌ WRONG: Test both creation AND registration in one iteration
- ✅ CORRECT: Split into .1 (creation) and .2 (registration)

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 4: Count Assertions"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 5, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 4: Count Assertions"
- Status: in_progress

Before proceeding to Step 5, you MUST verify Step 4 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Each behavior was converted to an exact assertion
   - Total iteration count was determined
   - Multi-assertion violations were identified and split
   - One assertion per iteration principle was followed
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 4: Count Assertions"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 4: Count Assertions"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=4
```

#### Step 5: Define Cycles

**Group iterations into logical cycles (3-5 iterations each):**

```markdown
## Cycle Organization

### Cycle 1: Basic Functionality (3 iterations)
- 1.1: send() returns true
- 1.2: has_messages() returns false initially
- 1.3: has_messages() returns true after send

### Cycle 2: Message Retrieval (3 iterations)
- 2.1: take_message() returns NULL when empty
- 2.2: take_message() returns message when available
- 2.3: take_message() removes from queue

### Cycle 3: Error Handling (2 iterations)
- 3.1: send() returns false with NULL delegate
- 3.2: send() returns false with NULL message

### Cycle 4: Ownership Management (2 iterations)
- 4.1: send() transfers ownership to delegate
- 4.2: take_message() transfers ownership to caller
```

**Verify cycle sizes:**
- ⚠️ Too small: 1-2 iterations (excessive overhead)
- ✅ Optimal: 3-5 iterations (focused review)
- ⚠️ Too large: 8+ iterations (reviewer fatigue)

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 5: Define Cycles"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 6, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 5: Define Cycles"
- Status: in_progress

Before proceeding to Step 6, you MUST verify Step 5 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Iterations were grouped into logical cycles
   - Each cycle contains 3-5 iterations (optimal size)
   - Cycle organization makes sense for review
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 5: Define Cycles"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 5: Define Cycles"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=5
```

#### [CHECKPOINT END]

**[QUALITY GATE 1: Requirements Complete]**
```bash
# MANDATORY: Must pass before proceeding to iteration planning
./scripts/checkpoint-gate.sh create-plan "Requirements" "1,2,3,4,5"
```

**Expected gate output:**
```
✅ GATE 'Requirements' - PASSED
   Verified: Steps 1,2,3,4,5
```

**Minimum Requirements for Stage 1:**
- [ ] All 10 KB articles read and quoted
- [ ] Requirements documented
- [ ] All behaviors identified and listed
- [ ] Assertions counted (one per iteration)
- [ ] Cycles defined (3-5 iterations each)

### Stage 2: Iteration Planning (Steps 6-8)

#### [CHECKPOINT START - STAGE 2]

#### Step 6: Plan Iterations

**CRITICAL: Per-Iteration Progress Tracking**

This checkpoint involves creating multiple iterations. To ensure the command doesn't lose track of progress, initialize a separate nested checkpoint instance for iteration tracking.

**Initialize Iteration Tracking:**
```bash
# Initialize nested checkpoint for iteration tracking
# After determining iteration count from Checkpoint 4
./scripts/checkpoint-init.sh create-plan-iterations "Iteration 0.1" "Iteration 0.2" "Iteration 0.3" "Iteration 1.1" "Iteration 1.2" ... [all iteration names]'

# Check iteration tracking status anytime
./scripts/checkpoint-status.sh create-plan-iterations
```

**Write complete iteration structure:**

For EACH iteration, create:
```markdown
#### Iteration 1.1: send() returns true - PENDING REVIEW

**Objective**: Verify ar_delegate__send() returns success value

**RED Phase:**
```c
static void test_delegate__send_returns_true(void) {
    // Given a delegate instance
    ar_log_t *ref_log = ar_log__create();
    ar_delegate_t *own_delegate = ar_delegate__create(ref_log, "test");

    // When sending a message
    ar_data_t *own_message = ar_data__create_string("hello");
    bool result = ar_delegate__send(own_delegate, own_message);

    // Then send should return true
    AR_ASSERT(result, "Send should return true");  // ← FAILS (stub returns false)

    // Cleanup
    ar_delegate__destroy(own_delegate);
    ar_log__destroy(ref_log);
}
```

**GREEN Phase:**
```c
bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
    if (own_message) ar_data__destroy(own_message);
    return true;  // Hardcoded! Minimal implementation
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks
```

**For iterations with .1/.2 splits:**
```markdown
#### Iteration 1.6.1: create_and_register_agent() returns valid ID - PENDING REVIEW

**Objective**: Verify function returns non-zero agent ID (minimal)

**RED Phase:**
```c
static void test_fixture__create_and_register_agent(void) {
    // Given empty fixture
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();

    // When creating agent
    int64_t agent_id = ar_agent_store_fixture__create_agent(fixture, "echo", "1.0");

    // Then should return valid ID
    AR_ASSERT(agent_id > 0, "Should return agent ID");  // ← FAILS (stub returns 0)

    // Cleanup (temporary: manually destroy agent since not registered yet)
    ar_agent_store_fixture__destroy_agent(fixture, agent_id);
    ar_agent_store_fixture__destroy(fixture);
}
```

**GREEN Phase:**
```c
int64_t ar_agent_store_fixture__create_agent(...) {
    // Minimal: just allocate ID, no registration yet
    return ar_agent_store_fixture__get_next_agent_id(mut_fixture);
}
```

#### Iteration 1.6.2: Agent is actually registered - PENDING REVIEW

**Objective**: Verify registration happens (ownership transfer)

**RED Phase:**
```c
static void test_fixture__create_and_register_agent(void) {
    // Given empty fixture
    ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();

    // When creating agent
    int64_t agent_id = ar_agent_store_fixture__create_agent(fixture, "echo", "1.0");
    AR_ASSERT(agent_id > 0, "Should return agent ID");

    // Then agent should be registered
    AR_ASSERT(ar_agent_store_fixture__verify_agent(fixture, agent_id, "echo"),
              "Should be registered");  // ← FAILS (not registered in 0.6.1)

    // Cleanup (removed manual destroy_agent - fixture now owns it)
    ar_agent_store_fixture__destroy(fixture);
}
```

**GREEN Phase:**
```c
int64_t ar_agent_store_fixture__create_agent(...) {
    int64_t agent_id = ar_agent_store_fixture__get_next_agent_id(mut_fixture);

    ar_agent_t *own_agent = ar_agent__create(ref_method_name, ref_method_version);
    if (!own_agent) return 0;

    // NOW: Register (ownership transfer)
    bool registered = ar_agent_registry__register(get_registry(mut_fixture), agent_id, own_agent);
    if (!registered) {
        ar_agent__destroy(own_agent);
        return 0;
    }

    return agent_id;
}
```
```

**MANDATORY: After creating EACH iteration:**

```bash
# Mark iteration as complete immediately after creation
./scripts/checkpoint-update.sh create-plan-iterations STEP=1  # After Iteration 1.1
./scripts/checkpoint-update.sh create-plan-iterations STEP=2  # After Iteration 1.2
./scripts/checkpoint-update.sh create-plan-iterations STEP=3  # After Iteration 1.3
# ... continue for each iteration
```

**Progress Tracking Pattern:**

For each iteration created:
1. **Create the iteration** (RED phase, GREEN phase, verification)
2. **Immediately update checkpoint** using `./scripts/checkpoint-update.sh create-plan-iterations STEP=N`
3. **Continue to next iteration**
4. **DO NOT batch updates** - update after each iteration

**Check Progress Anytime:**
```bash
# See current iteration progress
./scripts/checkpoint-status.sh create-plan-iterations

# Example output:
# 📈 create-plan-iterations: 5/12 steps (42%)
#    [████████░░░░░░░░░░░░] 42%
# → Next: ./scripts/checkpoint-update.sh create-plan-iterations STEP=6
```

**Resuming After Interruption:**
```bash
# Check which iterations are complete
./scripts/checkpoint-status.sh create-plan-iterations

# The status will show which step to continue from
# Resume creating iterations starting from the indicated step
```

**CRITICAL**: This iteration tracking uses a nested checkpoint instance (`create-plan-iterations`) independent of the main checkpoint tracking (`create-plan`). This allows resuming within Checkpoint 6 if interrupted mid-iteration creation.

**After ALL iterations created:**
```bash
# The nested checkpoint system will show 100% when all iterations are done
./scripts/checkpoint-status.sh create-plan-iterations
# Should show: 🎆 All 12 steps complete!

# Clean up iteration tracking
./scripts/checkpoint-cleanup.sh create-plan-iterations

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 6: Plan Iterations"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 7, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 6: Plan Iterations"
- Status: in_progress

Before proceeding to Step 7, you MUST verify Step 6 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - All iterations were written with complete structure
   - Each iteration has RED phase, GREEN phase, and verification sections
   - BDD structure (Given/When/Then/Cleanup) is present
   - Real AgeRun types are used (not placeholders)
   - Proper ownership prefixes are used
   - One assertion per iteration
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 6: Plan Iterations"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 6: Plan Iterations"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=6
```

**[QUALITY GATE 2: Plan Structure Validation]**

**MANDATORY VALIDATION**: Before proceeding to Step 7, run structural validation:

```bash
# Validate plan structure (checks all iterations have required sections)
./scripts/validate-plan-structure.sh plans/[plan-file].md [expected-iteration-count]

# Example:
# ./scripts/validate-plan-structure.sh plans/file_delegate_plan.md 15
```

**Expected output:**
```
🔍 Validating plan structure: plans/file_delegate_plan.md
Expected iterations: 15
Found iterations: 15
✅ Iteration count matches

Checking iteration structure...
  Iteration 8.1: ✅
  Iteration 8.2: ✅
  ...
  Iteration 12.3: ✅

✅ All iterations have required structure
✅ PLAN STRUCTURE VALIDATION PASSED
```

**What this validates:**
- Correct number of iterations exist in the file
- Each iteration has Objective, RED Phase, GREEN Phase, Verification sections
- Each iteration has GOAL 1 and GOAL 2 markers in RED phase
- Each iteration has PENDING REVIEW status marker

**If validation fails:**
- DO NOT proceed to Step 7
- Review and fix structural issues in the plan
- Re-run validation until it passes

This gate prevents proceeding with incomplete or structurally invalid plans.

#### Step 7: Structure RED Phases

**⭐ CRITICAL: Lesson 7 - Assertion Validity Verification Via Temporary Corruption**

This step ensures generated plans follow the most critical lesson: Every RED phase MUST document temporary code/corruption that makes the assertion fail, proving the assertion actually catches bugs.

**Initialize RED Phase Verification Tracking:**
```bash
# Initialize nested checkpoint for RED phase verification
# Use same iteration list as Checkpoint 6
./scripts/checkpoint-init.sh create-plan-red-phases "Iteration 0.1" "Iteration 0.2" "Iteration 0.3" ... [all iteration names]'
```

**Verify all RED phases have:**

For EACH iteration:
- [ ] BDD structure (Given/When/Then/Cleanup comments)
- [ ] Explicit failure indicator (// ← FAILS comment)
- [ ] **⭐ LESSON 7 - CRITICAL**: RED phase MUST document temporary corruption/code-break
  - **REQUIRED**: Include comment describing what code is temporarily broken
  - **Example**: "Temporary: Don't queue the message (destroy it instead) so assertion fails"
  - **Example**: "Temporary: Corrupt type field to INT so assertion fails"
  - **Purpose**: This PROVES the assertion catches real bugs, not just always-passing tests
- [ ] Real AgeRun types (ar_*_t) not placeholders
- [ ] Proper ownership prefixes (own_, ref_, mut_)
- [ ] Exactly ONE assertion (not multiple)

**RED phase checklist per iteration:**
```markdown
✅ Has Given comment explaining setup
✅ Has When comment explaining action
✅ Has Then comment explaining verification
✅ Has // ← FAILS (reason) on assertion line
✅ **CRITICAL**: RED phase documents temporary corruption/code-break that will make assertion fail
✅ Cleanup section with proper destroy calls
✅ Uses real AgeRun types with ar_ prefix (not generic placeholders)
✅ Uses ownership prefixes (own_message not message)
✅ One assertion only (not multiple AR_ASSERT calls)
```

**Lesson 7 Documentation Template with Dual Goals (MANDATORY):**

Every plan iteration MUST use this template structure:

```markdown
#### RED Phase

**RED Phase has TWO independent goals (both must be completed):**

**GOAL 1: Prove Test Validity** (ALWAYS REQUIRED - even for verification iterations)
- **Purpose**: Prove this test can actually catch [specific type of] bugs
- **Method**: Apply temporary corruption, verify test FAILS
- **Status**: [For NEW iterations: "Not yet proven" | For VERIFICATION iterations: "Implementation exists from Iteration X, but validity must still be proven"]

**GOAL 2: Identify What to Implement** (CONDITIONAL)
- **Purpose**: Determine what code needs to be written
- **Method**: Observe what failing test expects
- **Status**: [For NEW iterations: "Needs implementation" | For VERIFICATION iterations: "✅ ALREADY SATISFIED - Implemented in Iteration X"]

**CRITICAL**: Both goals are independent. Even if Goal 2 is satisfied (implementation exists), Goal 1 is STILL REQUIRED to prove test validity.

---

**Step 1: Write the Test**

[Test code with Given/When/Then/Cleanup structure]
[Include // ← FAILS comment on assertion]

---

**Step 2: Prove Test Validity (GOAL 1 - MANDATORY)**

**This step is REQUIRED even if implementation exists. It proves the test can catch bugs.**

[Temporary corruption code]
Temporary corruption: [describe exactly what code is broken and why]
Expected RED: "Test FAILS because [specific failure reason]"
Verify: `make test` → assertion fails with [expected message]

**Evidence of Goal 1 completion**: Test output showing FAILURE

---

#### GREEN Phase

**GREEN Phase Goal: Make Test Pass**

For **NEW implementation iterations**:
- **Goal 2** needs satisfaction (write minimal implementation)
- Use test failure to guide what to implement
- Implement ONLY what's needed to pass this specific assertion

For **VERIFICATION iterations**:
- **Goal 2** already satisfied (implementation exists from previous iteration)
- Simply remove temporary corruption from Goal 1
- Test should immediately PASS

**This iteration**: [State which case applies and what to do]

[Implementation or corruption removal code]
Expected GREEN: "Test PASSES because [reason]"
Verify: `make test` → assertion passes
```

**MANDATORY EVIDENCE REQUIREMENT**: As you verify each RED phase, document the corruption in an evidence file:

```bash
# Create evidence file before starting RED phase verification
cat > /tmp/red-corruptions-evidence.txt << 'EOF'
# RED Phase Corruptions Evidence
# One entry per iteration documenting how assertion will fail
# Format: N.N: Description of temporary corruption

EOF

# After verifying each iteration's RED phase, append to evidence file:
echo "8.1: Stub returns NULL to prove test catches creation failures" >> /tmp/red-corruptions-evidence.txt
echo "8.2: Skip freeing own_allowed_path to cause memory leak" >> /tmp/red-corruptions-evidence.txt
echo "8.3: Return 'network' instead of 'file' to prove type check" >> /tmp/red-corruptions-evidence.txt
# ... continue for each iteration
```

**MANDATORY: After verifying EACH iteration's RED phase:**
```bash
# 1. Document corruption in evidence file (see above)
# 2. Mark RED phase verification complete
./scripts/checkpoint-update.sh create-plan-red-phases STEP=1  # After verifying Iteration 1.1
./scripts/checkpoint-update.sh create-plan-red-phases STEP=2  # After verifying Iteration 1.2
# ... continue for each iteration
```

**CRITICAL**: Update checkpoint immediately after verifying each iteration's RED phase. DO NOT batch updates.

**After ALL RED phases verified:**
```bash
# Verify all complete
./scripts/checkpoint-status.sh create-plan-red-phases
# Should show: 🎆 All steps complete!

# MANDATORY: Validate evidence file before proceeding
./scripts/validate-red-corruptions.sh /tmp/red-corruptions-evidence.txt [expected-iteration-count]

# Example:
# ./scripts/validate-red-corruptions.sh /tmp/red-corruptions-evidence.txt 15
```

**Expected validation output:**
```
🔍 Validating RED corruption evidence: /tmp/red-corruptions-evidence.txt
Expected entries: 15
Found entries: 15
✅ Corruption entry count matches

Checking corruption entries...
  ✅ 8.1
  ✅ 8.2
  ...
  ✅ 12.3

✅ All corruption entries are properly formatted
✅ RED CORRUPTION EVIDENCE VALIDATION PASSED
```

**What this validates:**
- Evidence file contains correct number of corruption entries
- Each entry has iteration number and meaningful description
- Descriptions are substantial (not just placeholders)

**If validation fails:**
- DO NOT proceed to Step 8
- Add missing or fix invalid corruption entries
- Re-run validation until it passes

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 7: Structure RED Phases"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 8, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 7: Structure RED Phases"
- Status: in_progress

Before proceeding to Step 8, you MUST verify Step 7 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - All RED phases have BDD structure (Given/When/Then/Cleanup)
   - All RED phases have explicit failure indicators (// ← FAILS comments)
   - **CRITICAL**: All RED phases document temporary corruption/code-break (Lesson 7)
   - Real AgeRun types are used (not placeholders)
   - Proper ownership prefixes are present
   - Exactly ONE assertion per iteration
   - RED corruption evidence file was created and validated
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 7: Structure RED Phases"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 7: Structure RED Phases"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
# Clean up RED phase tracking
./scripts/checkpoint-cleanup.sh create-plan-red-phases

# Mark main Checkpoint 7 as complete
./scripts/checkpoint-update.sh create-plan STEP=7
```

#### Step 8: Structure GREEN Phases

**CRITICAL: Per-Iteration Verification Tracking**

**Initialize GREEN Phase Verification Tracking:**
```bash
# Initialize nested checkpoint for GREEN phase verification
./scripts/checkpoint-init.sh create-plan-green-phases "Iteration 0.1" "Iteration 0.2" "Iteration 0.3" ... [all iteration names]'
```

**Verify all GREEN phases follow minimalism:**

For EACH iteration:
- [ ] Implements ONLY what's needed to pass this iteration's assertion
- [ ] Uses hardcoded returns when valid (return false;, return NULL;)
- [ ] No "future-proofing" or untested features
- [ ] No error handling before test demands it
- [ ] Proper ownership transfer documented

**GREEN phase minimalism checklist:**
```markdown
Iteration 1: Send returns true
  ✅ return true;  // Hardcoded - minimal!
  ❌ return queue_message(...);  // Too much - not tested yet

Iteration 2: Has messages returns false
  ✅ return false;  // Hardcoded - minimal!
  ❌ return check_queue(...);  // Too much - not needed yet

Iteration 3: Has messages returns true after send
  ✅ NOW implement queue logic // Forced by this test
```

**Progression pattern verification:**
```markdown
Iteration N.1: Stub implementation
  → return NULL; or return false;

Iteration N.2: Hardcoded success
  → return true; or create minimal object

Iteration N.3: Actual logic (forced by test)
  → Implement real behavior because test demands it
```

**MANDATORY: After verifying EACH iteration's GREEN phase:**
```bash
# Mark GREEN phase verification complete
./scripts/checkpoint-update.sh create-plan-green-phases STEP=1  # After verifying Iteration 1.1
./scripts/checkpoint-update.sh create-plan-green-phases STEP=2  # After verifying Iteration 1.2
# ... continue for each iteration
```

**CRITICAL**: Update checkpoint immediately after verifying each iteration's GREEN phase. DO NOT batch updates.

**After ALL GREEN phases verified:**
```bash
# Verify all complete
./scripts/checkpoint-status.sh create-plan-green-phases
# Should show: 🎆 All steps complete!

# Clean up GREEN phase tracking
./scripts/checkpoint-cleanup.sh create-plan-green-phases
```

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 8: Structure GREEN Phases"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 9, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 8: Structure GREEN Phases"
- Status: in_progress

Before proceeding to Step 9, you MUST verify Step 8 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - All GREEN phases follow minimalism (hardcoded returns when valid)
   - No over-implementation or untested features
   - No error handling before tests demand it
   - Proper ownership transfer is documented
   - Progression pattern is correct (stub → hardcoded → real logic)
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 8: Structure GREEN Phases"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 8: Structure GREEN Phases"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
# Mark main Checkpoint 8 as complete
./scripts/checkpoint-update.sh create-plan STEP=8
```

#### [CHECKPOINT END]

**[QUALITY GATE 2: Iterations Complete]**
```bash
# MANDATORY: Must pass before proceeding to cleanup/markers
./scripts/checkpoint-gate.sh create-plan "Iterations" "6,7,8"
```

**Expected gate output:**
```
✅ GATE 'Iterations' - PASSED
   Verified: Steps 6,7,8
```

**Minimum Requirements for Stage 2:**
- [ ] All iterations written with RED and GREEN phases
- [ ] All RED phases have BDD structure and failure comments
- [ ] All GREEN phases follow minimalism (hardcoded when valid)
- [ ] Decimal numbering used for splits (N.1, N.2)

### Stage 3: Cleanup and Documentation (Steps 9-11)

#### [CHECKPOINT START - STAGE 3]

#### Step 9: Add Cleanup

**Add proper cleanup to all iterations:**

For .1 iterations (create but don't register):
```c
// Cleanup (temporary: manually destroy agent since not registered yet)
ar_agent_store_fixture__destroy_agent(fixture, agent_id);
ar_agent_store_fixture__destroy(fixture);
```

For .2 iterations (ownership transferred):
```c
// Cleanup (removed manual destroy_agent - fixture now owns it)
ar_agent_store_fixture__destroy(fixture);
```

For normal iterations:
```c
// Cleanup
ar_delegate__destroy(own_delegate);
ar_log__destroy(ref_log);
```

**Cleanup verification checklist:**
- [ ] Every test has Cleanup section
- [ ] .1 iterations have "temporary:" comment (MANDATORY format)
- [ ] .2 iterations have "removed manual..." comment
- [ ] All owned objects destroyed (own_ prefix)
- [ ] Borrowed objects NOT destroyed (ref_ prefix)
- [ ] Destruction order is reverse of creation

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 9: Add Cleanup"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 10, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 9: Add Cleanup"
- Status: in_progress

Before proceeding to Step 10, you MUST verify Step 9 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Every test has Cleanup section
   - .1 iterations have "temporary:" comment with MANDATORY format
   - .2 iterations have "removed manual..." comment
   - All owned objects are destroyed (own_ prefix)
   - Borrowed objects are NOT destroyed (ref_ prefix)
   - Destruction order is reverse of creation
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 9: Add Cleanup"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 9: Add Cleanup"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=9
```

#### Step 10: Add Status Markers

**MANDATORY: Add PENDING REVIEW markers to all iterations:**

**CRITICAL REQUIREMENT**: Every single iteration heading MUST end with " - PENDING REVIEW" suffix. This is non-negotiable and required for proper review tracking.

**Note**: Status markers appear ONLY on iteration headings, NOT on cycle headings.

```markdown
### Cycle 1: Basic Functionality

#### Iteration 1.1: send() returns true - PENDING REVIEW
[...]

#### Iteration 1.2: has_messages() returns false - PENDING REVIEW
[...]

### Cycle 2: Message Retrieval

#### Iteration 2.1: take_message() returns NULL - PENDING REVIEW
[...]
```

**Status marker rules:**
- All new plan iterations start with `PENDING REVIEW`
- Iterations will be updated to `REVIEWED` or `REVISED` during review process (ar:review-plan)
- Iterations will be updated to `IMPLEMENTED` during execution (ar:execute-plan)
- Iterations will be updated to `✅ COMMITTED` before git commit

**CRITICAL**: Every iteration created must have "- PENDING REVIEW" suffix in the heading. This is MANDATORY and must not be omitted.

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 10: Add Status Markers"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 11, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 10: Add Status Markers"
- Status: in_progress

Before proceeding to Step 11, you MUST verify Step 10 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - All iterations have "- PENDING REVIEW" suffix in heading
   - Status markers appear ONLY on iteration headings (not cycle headings)
   - No iterations are missing status markers
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 10: Add Status Markers"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 10: Add Status Markers"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=10
```

#### Step 11: Add Cross-References

**Add Related Patterns section:**

```markdown
## Related Patterns
- [TDD Iteration Planning Pattern](../../../kb/tdd-iteration-planning-pattern.md)
- [TDD GREEN Phase Minimalism](../../../kb/tdd-green-phase-minimalism.md)
- [TDD RED Phase Assertion Requirement](../../../kb/tdd-red-phase-assertion-requirement.md)
- [BDD Test Structure](../../../kb/bdd-test-structure.md)
- [Ownership Naming Conventions](../../../kb/ownership-naming-conventions.md)
- [Temporary Test Cleanup Pattern](../../../kb/temporary-test-cleanup-pattern.md)
- [Red-Green-Refactor Cycle](../../../kb/red-green-refactor-cycle.md)
```

**Add pattern notes where relevant:**
```markdown
**Note**: This plan follows the ar_agent/ar_agency message passing pattern.
See ar_agent.c and ar_agency.c for reference implementations.
```

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 11: Add Cross-References"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 12, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 11: Add Cross-References"
- Status: in_progress

Before proceeding to Step 12, you MUST verify Step 11 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Related Patterns section was added with relevant KB article links
   - Pattern notes were added where relevant
   - Cross-references use correct relative paths
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 11: Add Cross-References"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 11: Add Cross-References"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=11
```

#### [CHECKPOINT END]

**[QUALITY GATE 3: Documentation Complete]**
```bash
# MANDATORY: Must pass before validation
./scripts/checkpoint-gate.sh create-plan "Documentation" "9,10,11"
```

**Expected gate output:**
```
✅ GATE 'Documentation' - PASSED
   Verified: Steps 9,10,11
```

**Minimum Requirements for Stage 3:**
- [ ] All iterations have proper cleanup
- [ ] Temporary cleanup uses MANDATORY comment format
- [ ] All iterations marked PENDING REVIEW
- [ ] Related Patterns section added

### Stage 4: Validation and Saving (Steps 12-14)

#### [CHECKPOINT START - STAGE 4]

#### Step 12: Validate Plan

**⭐ LESSON 7 CRITICAL CHECK**: Run validation to ensure plan will pass review-plan:

```bash
# Quick sanity check - fast pattern verification
./scripts/plan-sanity-check.sh <plan-file>
# Shows: assertion counts, FAILS comments, Lesson 7 corruption docs, etc.

# Comprehensive automated validator - catches all issues
./scripts/validate-tdd-plan.sh <plan-file>
# Expected: ✅ Plan validation PASSED
```

**Pre-review validation checklist (including all 14 lessons):**
- [ ] No iteration has multiple assertions (except .2 keeping .1's assertion) - **Lesson 2**
- [ ] All RED phases have // ← FAILS comments - **Lesson 2**
- [ ] **⭐ LESSON 7 CRITICAL**: All RED phases document temporary corruption/failure
  - Check: Does each RED phase explain how assertion will FAIL?
  - Check: Is temporary code/break clearly documented?
  - Check: Will GREEN phase remove it?
- [ ] All .1 iterations have temporary cleanup with correct format - **Lesson 4**
- [ ] All .2 iterations removed temporary cleanup - **Lesson 4, 8**
- [ ] All GREEN phases use hardcoded returns when valid - **Lesson 3, 11**
- [ ] All tests have BDD structure (Given/When/Then/Cleanup) - **Lesson 2**
- [ ] All types are real AgeRun types (ar_*_t) - **Lesson 2**
- [ ] All ownership prefixes present (own_, ref_, mut_) - **Lesson 14**
- [ ] Sections are 3-5 iterations each - **Lesson 1**
- [ ] Decimal numbering used for splits - **Lesson 1**
- [ ] Iteration numbering sequential, no gaps - **Lesson 1, 13**
- [ ] No forward references to unreviewed iterations - **Lesson 13**

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 12: Validate Plan"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 13, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 12: Validate Plan"
- Status: in_progress

Before proceeding to Step 13, you MUST verify Step 12 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Plan validation scripts were executed
   - All 14 TDD lessons were verified
   - Automated validator passed
   - Pre-review validation checklist was completed
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 12: Validate Plan"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 12: Validate Plan"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=12
```

#### Step 13: Save Plan

**Write plan to file:**

```markdown
# Plan: [Feature Name]

## Overview
[Feature description]

**Pattern**: Similar to [reference pattern]
**Modules**: [modules involved]
**Status**: DRAFT - PENDING REVIEW

## Plan Status
- Total iterations: [count]
- Cycles: [count]
- Review status: 0% (all PENDING REVIEW)

[... all iterations ...]

## Related Patterns
[... cross-references ...]
```

**File location:**
- Save to: `plans/[feature_name]_plan.md`
- Use descriptive task-based name (e.g., agent_store_fixture_plan.md, message_queue_plan.md)
- Follow naming convention

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 13: Save Plan"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before proceeding to Step 14, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 13: Save Plan"
- Status: in_progress

Before proceeding to Step 14, you MUST verify Step 13 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Plan was written to file
   - File location follows naming convention
   - Plan includes all required sections (Overview, Objective, Context, Success Criteria, Plan Status, iterations, Related Patterns)
   - File was saved successfully
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after step-verifier verification passes** (checkpoint-update is for progress tracking only, NOT verification):

1. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 13: Save Plan"
   - Status: completed

2. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 13: Save Plan"
   - Status: completed

3. **Update checkpoint** (for progress tracking only):
```bash
# Save the plan
# <use Write tool with plan content>

./scripts/checkpoint-update.sh create-plan STEP=13
```

#### Step 14: Summary

**Generate creation summary:**

```markdown
## Plan Creation Summary

**Plan**: message_queue_plan.md
**Created**: 2025-10-15
**Feature**: Message queue infrastructure

### Statistics
- Total iterations: 12
- Cycles: 3
- .1/.2 splits: 2
- Temporary cleanup locations: 2

### Methodology Compliance (14 TDD Lessons)
✅ **Lesson 1**: Numbering sequential (1.1, 1.2, 1.3, etc. - no gaps)
✅ **Lesson 2**: One assertion per iteration (12/12)
✅ **Lesson 3**: GREEN minimalism (hardcoded returns used)
✅ **Lesson 4**: Resource cleanup in all iterations (2/2 .1 cleanups)
✅ **Lesson 5**: Iteration dependencies force real implementation
✅ **Lesson 6**: Integration tests at module seams
✅ **⭐ Lesson 7 CRITICAL**: RED phases document temporary corruption (12/12)
✅ **Lesson 8**: Temporary code marked and documented
✅ **Lesson 9**: Property validation through independent assertions
✅ **Lesson 10**: Test type distinction (TDD vs Validation)
✅ **Lesson 11**: No over-implementation in GREEN
✅ **Lesson 12**: Commit messages (documented in methodology)
✅ **Lesson 13**: No forward dependencies
✅ **Lesson 14**: Ownership naming (own_, ref_, mut_)

### Validation Results
✅ Automated validator: `./scripts/validate-tdd-plan.sh` passed
✅ BDD structure throughout (12/12)
✅ Real AgeRun types (no placeholders)
✅ Status markers (PENDING REVIEW)

### Next Steps
1. **Verify plan quality**: `./scripts/validate-tdd-plan.sh plans/message_queue_plan.md`
2. **Review plan**: `/review-plan plans/message_queue_plan.md`
3. **Apply feedback** from review (if needed)
4. **Get approval** (all iterations REVIEWED)
5. **Begin implementation** following plan
6. **Update plan** with completion status when done

### Validation Checklist
- ✅ All 14 TDD lessons verified
- ✅ Automated validator passed
- ✅ Ready for /review-plan
```

**MANDATORY: Update step todo item status**

Before starting this step, update the step todo item status to `in_progress`:
- Update todo item: "Step 14: Summary"
- Status: in_progress

**⚠️ MANDATORY STEP VERIFICATION**

**MANDATORY: Update verification todo item status**

Before completing the workflow, update the verification todo item status to `in_progress`:
- Update todo item: "Verify Step 14: Summary"
- Status: in_progress

Before completing the workflow, you MUST verify Step 14 completion via step-verifier sub-agent:

1. **Invoke step-verifier sub-agent** to verify:
   - Creation summary was generated
   - Statistics were included (total iterations, cycles, splits, temporary cleanup locations)
   - All 14 TDD lessons compliance was documented
   - Validation results were included
   - Next steps were documented
   - Step objectives were met

2. **If verification fails**: Fix issues and re-verify before proceeding

3. **If sub-agent unavailable**: Stop and request user manual verification

**⚠️ MANDATORY FINAL VERIFICATION**

**MANDATORY: Update final verification todo item status**

Before completing the workflow, update the final verification todo item status to `in_progress`:
- Update todo item: "Verify Complete Workflow: create-plan"
- Status: in_progress

Before completing the workflow, you MUST verify ALL steps were completed correctly:

1. **Invoke step-verifier sub-agent** to verify complete workflow:
   - Verify all 14 steps were completed correctly
   - Verify all step objectives were met
   - Verify plan was created successfully
   - Verify plan is ready for review

2. **If verification fails**: Fix issues and re-verify before completing

3. **If sub-agent unavailable**: Stop and request user manual verification

**Only after ALL steps verified:**

1. **Mark final verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Complete Workflow: create-plan"
   - Status: completed

2. **Mark verification complete in session todo list** using `todo_write`:
   - Update todo item: "Verify Step 14: Summary"
   - Status: completed

3. **Mark step complete in session todo list** using `todo_write`:
   - Update todo item: "Step 14: Summary"
   - Status: completed

4. **Update checkpoint** (for progress tracking only):
```bash
./scripts/checkpoint-update.sh create-plan STEP=14
```

#### [CHECKPOINT END]

#### [CHECKPOINT COMPLETE]
```bash
./scripts/checkpoint-complete.sh create-plan
```

**Note**: `checkpoint-complete.sh` is used ONLY for progress tracking cleanup. All verification is done via step-verifier sub-agent, NOT via checkpoint scripts.

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 create-plan: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
```

```bash
# Clean up tracking
./scripts/checkpoint-cleanup.sh create-plan
```

## Plan Template Structure

### Complete Plan Template

```markdown
# Plan: [Feature Name]

## Overview
[1-2 sentence description of what will be implemented]

**Pattern**: Similar to [ar_agent/ar_agency | other reference]
**Modules**: [ar_delegate.h | new/modified modules]
**Status**: DRAFT - PENDING REVIEW

## Objective
[Detailed objective statement]

## Context
- Existing types: [list relevant types]
- New functions: [list functions to implement]
- Pattern reference: [point to similar implementation]

## Success Criteria
- All tests pass
- Zero memory leaks
- BDD test structure throughout
- One assertion per iteration
- GREEN minimalism followed

## Plan Status
- Total iterations: [count]
- Cycles: [count]
- Review status: 0% (all PENDING REVIEW)

### Cycle 1: [Cycle Name]

#### Iteration 1.1: [Behavior description] - PENDING REVIEW

**Objective**: [What this iteration tests]

**RED Phase:**
⭐ **LESSON 7 CRITICAL**: Document temporary corruption that makes assertion fail
```c
static void test_[function]__[behavior](void) {
    // Given [setup description]
    ar_[type]_t *ref_[name] = ar_[type]__create([params]);
    ar_[type]_t *own_[name] = ar_[type]__create([params]);

    // When [action description]
    [type] result = ar_[function]__[operation]([params]);

    // Then [expected outcome]
    AR_ASSERT([condition], "[message]");  // ← FAILS ([reason])
    // Temporary: [describe code break that makes this assertion fail]
    // Example: "Don't queue message so take_message() returns NULL"
    // Example: "Corrupt type field to INT so type check fails"

    // Cleanup
    ar_[type]__destroy(own_[name]);
    ar_[type]__destroy(ref_[name]);
}
```

**GREEN Phase:**
```c
[return_type] ar_[function]__[operation]([params]) {
    // Minimal implementation
    return [hardcoded_value];  // Hardcoded - next iteration will force real impl
    // Remove temporary corruption from RED phase
}
```

**Verification:**
- Test passes
- Memory leak check: 0 leaks

[... more iterations ...]

## Related Patterns
- [TDD Iteration Planning Pattern](../../../kb/tdd-iteration-planning-pattern.md)
- [TDD GREEN Phase Minimalism](../../../kb/tdd-green-phase-minimalism.md)
- [Temporary Test Cleanup Pattern](../../../kb/temporary-test-cleanup-pattern.md)
- [BDD Test Structure](../../../kb/bdd-test-structure.md)
- [Ownership Naming Conventions](../../../kb/ownership-naming-conventions.md)
```

## Quality Indicators (All 14 TDD Lessons)

A well-created plan demonstrates compliance with all 14 critical TDD lessons:

- ✅ **Lesson 1**: Iteration numbering sequential (N.1, N.2, N.3 - no gaps)
- ✅ **Lesson 2**: One assertion per iteration (no bundling)
- ✅ **Lesson 3**: Hardcoded returns in early iterations (minimalism)
- ✅ **Lesson 4**: Temporary cleanup in .1 iterations with correct format
- ✅ **Lesson 5**: Iteration N+1 RED would fail with iteration N GREEN
- ✅ **Lesson 6**: Integration tests at module seams (not just unit tests)
- ✅ **⭐ Lesson 7 CRITICAL**: RED phases document temporary corruption/failure
  - Each RED phase explains how assertion will fail before GREEN code
  - Proves assertion catches real bugs, not just always-passing tests
- ✅ **Lesson 8**: Temporary code marked and clearly documented
- ✅ **Lesson 9**: Property validation through independent assertions
- ✅ **Lesson 10**: Clear distinction between TDD vs Validation vs Hybrid
- ✅ **Lesson 11**: GREEN implements ONLY what RED assertion tests
- ✅ **Lesson 12**: Documentation explains methodology ("why", not just "what")
- ✅ **Lesson 13**: No forward references to unreviewed iterations
- ✅ **Lesson 14**: Ownership prefixes (own_, ref_, mut_) throughout
- ✅ Cycles sized 3-5 iterations (optimal for review)
- ✅ BDD structure throughout (Given/When/Then/Cleanup)
- ✅ Real AgeRun types (ar_*_t) not placeholders
- ✅ Status markers (PENDING REVIEW)
- ✅ Cross-references to KB articles
- ✅ Passes `./scripts/validate-tdd-plan.sh` validator

## Common Planning Mistakes to Avoid

### 0. ⭐ MOST CRITICAL - Missing Temporary Corruption Documentation (Lesson 7)

**This is the MOST CRITICAL mistake.** RED phases must document temporary code/corruption that makes assertions fail.

❌ **WRONG** - No documentation of test failure:
```c
// RED Phase - Missing how assertion will fail
static void test_delegate__send_returns_true(void) {
    ar_delegate_t *own_delegate = ar_delegate__create(...);
    ar_data_t *own_message = ar_data__create_string("hello");
    bool result = ar_delegate__send(own_delegate, own_message);

    AR_ASSERT(result, "Send should return true");  // ← How does this fail? Unknown!

    ar_delegate__destroy(own_delegate);
}
```

✅ **CORRECT** - Documents temporary corruption that causes failure:
```c
// RED Phase - With temporary corruption documentation
static void test_delegate__send_returns_true(void) {
    ar_delegate_t *own_delegate = ar_delegate__create(...);
    ar_data_t *own_message = ar_data__create_string("hello");
    bool result = ar_delegate__send(own_delegate, own_message);

    // Temporary: send() returns false (not implemented yet)
    AR_ASSERT(result, "Send should return true");  // ← FAILS (stub returns false)

    ar_delegate__destroy(own_delegate);
}

// GREEN Phase
bool ar_delegate__send(ar_delegate_t *mut_delegate, ar_data_t *own_message) {
    if (own_message) ar_data__destroy(own_message);
    return true;  // Remove temporary corruption - now test passes
}
```

**Why this matters**: This proves the assertion actually catches bugs. Without it, you have "always-passing tests" that don't prove anything.

### 1. Multiple Assertions Per Iteration
❌ **WRONG**:
```c
// Iteration 1: create_and_register works
AR_ASSERT(obj != NULL, "Should create");
AR_ASSERT(registered, "Should register");  // TWO assertions!
```

✅ **CORRECT**:
```c
// Iteration 1.1: Returns non-NULL
AR_ASSERT(obj != NULL, "Should create");

// Iteration 1.2: Actually registers
AR_ASSERT(registered, "Should register");
```

### 2. GREEN Phase Over-Implementation
❌ **WRONG**:
```c
// Iteration 1 GREEN - implementing more than tested
bool ar_delegate__send(ar_delegate_t *mut_d, ar_data_t *own_m) {
    if (!mut_d || !own_m) return false;  // Not tested yet!
    queue_message(mut_d->queue, own_m);   // Not tested yet!
    return true;
}
```

✅ **CORRECT**:
```c
// Iteration 1 GREEN - minimal
bool ar_delegate__send(ar_delegate_t *mut_d, ar_data_t *own_m) {
    if (own_m) ar_data__destroy(own_m);
    return true;  // Hardcoded - next iteration will force more
}
```

### 3. Missing Temporary Cleanup
❌ **WRONG**:
```c
// Iteration 1.1 - creates but doesn't register
int64_t agent_id = ar_agent_store_fixture__create_agent(fixture, "echo", "1.0");
AR_ASSERT(agent_id > 0, "Should create");
// Missing cleanup - will leak!
ar_agent_store_fixture__destroy(fixture);
```

✅ **CORRECT**:
```c
// Iteration 1.1 - creates but doesn't register
int64_t agent_id = ar_agent_store_fixture__create_agent(fixture, "echo", "1.0");
AR_ASSERT(agent_id > 0, "Should create");
// Cleanup (temporary: manually destroy agent since not registered yet)
ar_agent_store_fixture__destroy_agent(fixture, agent_id);
ar_agent_store_fixture__destroy(fixture);
```

### 4. Wrong Temporary Cleanup Format
❌ **WRONG**:
```c
// Cleanup - temporary
destroy_agent(fixture, agent_id);
```

✅ **CORRECT**:
```c
// Cleanup (temporary: manually destroy agent since not registered yet)
ar_agent_store_fixture__destroy_agent(fixture, agent_id);
```

### 5. Placeholder Types Instead of Real Types
❌ **WRONG**: Using generic types without ar_ prefix

Don't use placeholders like "Thing", "Object", or types without the ar_ prefix. These are not real AgeRun types and will fail validation.

✅ **CORRECT**: Using real AgeRun types
```c
// Always use actual AgeRun types with ar_ prefix
ar_agent_store_fixture_t *fixture = ar_agent_store_fixture__create_empty();
ar_agent_t *agent = ar_agent__create("echo", "1.0");
```

## Integration with Workflow

### Complete TDD Workflow

```bash
# 1. Create plan
/create-plan "message queue infrastructure" plans/message_queue_plan.md

# 2. Review plan
/review-plan plans/message_queue_plan.md

# 3. Apply feedback (iterative refinement)
# <update plan based on review findings>

# 4. Re-review until approved
/review-plan plans/message_queue_plan.md
# Continue until all iterations marked REVIEWED

# 5. Implement following plan
# <execute RED-GREEN-REFACTOR for each iteration>

# 6. Update plan with completion status
# <add completion status header>

# 7. Commit
/commit
```

## Troubleshooting

### If checkpoint tracking gets stuck:
```bash
# Check current status
./scripts/checkpoint-status.sh create-plan

# If needed, reset and start over
./scripts/checkpoint-cleanup.sh create-plan
./scripts/checkpoint-init.sh create-plan '...'
```

### If unsure about iteration splitting:
Ask yourself:
- Does this test multiple behaviors? → Split it
- Would GREEN implement untested features? → Split it
- Does function name have "and" in it? → Likely needs split

### If unclear about GREEN minimalism:
Use the simplest possible implementation:
- Return hardcoded values (return false;, return NULL;)
- Don't add error handling until test demands it
- Don't add features until test demands them

## Related Commands
- `/review-plan` - Review created plan for methodology compliance
- `/commit` - Create commit after plan is approved and implemented

## Related KB Articles

### TDD Planning Patterns
- [TDD Plan Review Checklist](../../../kb/tdd-plan-review-checklist.md) ⭐ **NEW - Embeds all 14 TDD lessons**
- [TDD Iteration Planning Pattern](../../../kb/tdd-iteration-planning-pattern.md)
- [TDD Plan Iteration Split Pattern](../../../kb/tdd-plan-iteration-split-pattern.md)
- [TDD GREEN Phase Minimalism](../../../kb/tdd-green-phase-minimalism.md)
- [TDD RED Phase Assertion Requirement](../../../kb/tdd-red-phase-assertion-requirement.md)
- [Iterative Plan Refinement Pattern](../../../kb/iterative-plan-refinement-pattern.md)

### Testing and Memory Patterns
- [Temporary Test Cleanup Pattern](../../../kb/temporary-test-cleanup-pattern.md)
- [BDD Test Structure](../../../kb/bdd-test-structure.md)
- [Ownership Naming Conventions](../../../kb/ownership-naming-conventions.md)
- [Memory Leak Detection Workflow](../../../kb/memory-leak-detection-workflow.md)
- [Red-Green-Refactor Cycle](../../../kb/red-green-refactor-cycle.md)

### Review and Documentation Patterns
- [Plan Review Status Tracking Pattern](../../../kb/plan-review-status-tracking.md)
- [Iterative Plan Review Protocol](../../../kb/iterative-plan-review-protocol.md)
- [Plan Verification and Review](../../../kb/plan-verification-and-review.md)
- [Plan Document Completion Status Pattern](../../../kb/plan-document-completion-status-pattern.md)

### Command Implementation Patterns
- [Checkpoint Step Consolidation Pattern](../../../kb/checkpoint-step-consolidation-pattern.md) ⭐ Pattern for consolidating per-item verification steps
- [Checkpoint Implementation Guide](../../../kb/checkpoint-implementation-guide.md)
- [Command KB Consultation Enforcement](../../../kb/command-kb-consultation-enforcement.md)
- [Command Output Documentation Pattern](../../../kb/command-output-documentation-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md)

---

## Command Arguments

**IMPORTANT**: The text following this command execution is passed as arguments.

When you invoke this command with `/create-plan <arguments>`, everything after `/create-plan` is treated as command arguments and will be available for processing.

**Argument format:**
```
/create-plan <task-description> [additional-instructions]
```

**Examples:**
```
/create-plan "implement message queue infrastructure"
→ Arguments: "implement message queue infrastructure"

/create-plan "fixture lifecycle management" "focus on ownership transfer"
→ Arguments: "fixture lifecycle management" "focus on ownership transfer"

/create-plan
→ Arguments: (empty - will use context from ar:next-priority/ar:next-task)
```

**How arguments are used:**
1. Arguments are checked FIRST in task identification priority order
2. If arguments provided, they supersede context from ar:next-priority/ar:next-task
3. If no arguments, the command falls back to context extraction
4. Arguments can include task name and additional instructions/constraints

$ARGUMENTS