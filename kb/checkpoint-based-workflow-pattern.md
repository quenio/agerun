# Checkpoint-Based Workflow Pattern

## Learning
Complex multi-step workflows should use checkpoint tracking with progress files, mandatory gates between stages, and verification steps to ensure all quality standards are met before proceeding.

## Importance
Structured workflows prevent incomplete execution by making progress visible, enforcing dependencies between steps, and blocking advancement when quality gates fail. This ensures consistent, thorough execution of complex operations like commits, releases, or multi-stage builds.

## Example
```c
// Conceptual model of checkpoint workflow
// Real implementation uses bash scripts and make targets

// Step tracking structure (stored in /tmp/command-progress.txt)
typedef struct checkpoint_state_s {  // EXAMPLE: Conceptual model only
    char *command_name;
    int total_steps;
    int completed_steps;
    bool step_status[20];  // complete/pending for each step
} checkpoint_state_t;  // EXAMPLE: Not in actual codebase

// Initialize workflow with all steps
ar_data_t* own_workflow = ar_data__create_map();
ar_data__set_map_string(own_workflow, "command", ar_data__create_string("commit"));
ar_data__set_map_string(own_workflow, "total", ar_data__create_integer(9));

// Track step names
ar_list_t* own_steps = ar_list__create();
ar_list__add_last(own_steps, ar_data__create_string("Run Tests"));
ar_list__add_last(own_steps, ar_data__create_string("Check Logs"));
ar_list__add_last(own_steps, ar_data__create_string("Update CHANGELOG"));
// ... more steps

// Update progress as work completes
ar_data__set_map_string(own_workflow, "step_1", ar_data__create_string("complete"));
ar_data__set_map_string(own_workflow, "step_2", ar_data__create_string("complete"));

// Gate enforcement between stages
bool stage1_complete = check_steps_complete(own_workflow, 1, 2);  // EXAMPLE: Hypothetical function
if (!stage1_complete) {
    printf("❌ GATE BLOCKED: Complete steps 1-2 before Stage 2\n");
    return false;  // Cannot proceed
}

// Cleanup
ar_list__destroy(own_steps);
ar_data__destroy(own_workflow);
```

## Generalization
**Checkpoint Workflow Components:**

1. **Progress Tracking File**: `/tmp/command-progress.txt` stores state
   - Command name and start time
   - Total step count
   - STEP_N=complete/pending for each step

2. **Step Management** (via Makefile targets → bash scripts):
   - `checkpoint-init` → `scripts/checkpoint-init.sh`: Create tracking file with all steps
   - `checkpoint-update` → `scripts/checkpoint-update.sh`: Mark step complete, show progress bar
   - `checkpoint-update-verified` → `scripts/checkpoint-update-enhanced.sh`: Mark complete with automated verification
   - `checkpoint-status` → `scripts/checkpoint-status.sh`: Show current progress (verbose mode available)
   - `checkpoint-cleanup` → `scripts/checkpoint-cleanup.sh`: Remove tracking file when done

3. **Gate Enforcement** (via Makefile targets → bash scripts):
   - `checkpoint-gate` → `scripts/checkpoint-gate.sh`: Block progress until required steps complete
   - Exit with error code if gate fails
   - Named gates (e.g., "Build Quality", "Integration")
   - `checkpoint-gate CMD=name GATE="Gate Name" REQUIRED="1,2,3"`

4. **Script Architecture**:
   - All checkpoint scripts use platform-safe patterns for cross-platform compatibility
   - Scripts employ safe sed delimiters (`@` or `|` instead of `/`)
   - Proper OSTYPE detection (`[[ == ]]` pattern matching) for macOS/Linux differences
   - Comprehensive error handling (`set -e`, `set -o pipefail`)

5. **Wrapper Scripts** (simplify checkpoint integration in commands):
   - `scripts/checkpoint-init.sh`: Initialize checkpoint or show current status
     - Replaces 7-8 line if/then/else initialization pattern
     - Usage: `./scripts/checkpoint-init.sh command '\"Step 1\" \"Step 2\"'`
   - `scripts/checkpoint-require.sh`: Verify checkpoint is initialized before proceeding
     - Replaces 5 line precondition check pattern
     - Usage: `./scripts/checkpoint-require.sh command` (exits 1 if not initialized)
   - `scripts/checkpoint-gate.sh`: Verify gate conditions and provide clear feedback
     - Replaces 3-4 line gate verification pattern
     - Usage: `./scripts/checkpoint-gate.sh command 'Gate Name' '1,2,3'` (exits 1 on failure)
   - `scripts/checkpoint-complete.sh`: Show final status and cleanup
     - Replaces 4-5 line completion pattern (status + cleanup)
     - Usage: `./scripts/checkpoint-complete.sh command`

   **Why These Wrappers Exist:**
   - Reduce boilerplate: Commands had 7-8 line patterns repeated across multiple implementations
   - Consistent API: Unified interface for all checkpoint operations across different commands
   - Easier integration: Command authors simply call wrapper scripts instead of managing checkpoint details
   - Better UX: Wrappers provide clear error messages and status output

6. **Progress Visualization**:
   ```
   📈 commit: 5/9 steps (55%)
      [███████████░░░░░░░░░] 55%
   → Next: ./scripts/checkpoint-update.sh commit 6
   ```

## Implementation

### Recommended: Direct Script Calls
```bash
# Real AgeRun implementation example using direct script calls

# 1. Initialize workflow (idempotent - safe to call multiple times)
./scripts/checkpoint-init.sh commit "Run Tests" "Check Logs" "Update Docs" "Update TODO" "Update CHANGELOG" "Review Changes" "Stage Files" "Create Commit" "Push and Verify"

# 2. Execute steps with checkpoints
./scripts/checkpoint-update.sh commit 1
make test  # Run tests

# 3. Verify precondition before stage 2
./scripts/checkpoint-require.sh commit || exit 1

# 4. Enforce gate after stage
./scripts/checkpoint-gate.sh commit "Build Quality" "1,2" || exit 1
# Exits 1 and shows error if steps 1-2 not complete

# 5. Continue with remaining steps
./scripts/checkpoint-update.sh commit 5

# 6. Check status anytime
./scripts/checkpoint-status.sh commit --verbose

# 7. Cleanup when done
./scripts/checkpoint-complete.sh commit
```

## Workflow Structure Pattern
Organize complex workflows into stages with gates:

**Stage 1: Pre-Flight Checks**
- Steps: Validation, prerequisites, environment setup
- Gate: "Ready to Execute"

**Stage 2: Core Execution**
- Steps: Main work, intermediate verification
- Gate: "Quality Standards Met"

**Stage 3: Finalization**
- Steps: Documentation, commit, publish
- Gate: "Complete and Verified"

## Benefits
1. **Visibility**: Progress bars show completion percentage
2. **Resumability**: Can see which step failed and resume
3. **Enforcement**: Gates block progress until requirements met
4. **Documentation**: Step names document the workflow
5. **Auditability**: Tracking files show what was completed

## Common Applications
- **Commit workflows**: Test → docs → review → commit → push
- **Release workflows**: Build → test → tag → publish → announce
- **Migration workflows**: Backup → transform → validate → deploy → verify
- **Analysis workflows**: Collect → process → analyze → report → archive

## Related Patterns
- [Checkpoint Workflow Enforcement Pattern](checkpoint-workflow-enforcement-pattern.md) - Prevention mechanism for bypasses (initialization & precondition enforcement)
- [Checkpoint Sequential Execution Discipline](checkpoint-sequential-execution-discipline.md) - Execution discipline and sequential ordering requirements
- [Checkpoint Tracking Verification Separation](checkpoint-tracking-verification-separation.md) - Separating checkpoint tracking from verification
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](command-thoroughness-requirements-pattern.md)
- [Systematic Task Analysis Protocol](systematic-task-analysis-protocol.md)
- [KB Target Compliance Enforcement](kb-target-compliance-enforcement.md)
- [Context Preservation Across Sessions](context-preservation-across-sessions.md)
- [Session Resumption Without Prompting](session-resumption-without-prompting.md)
- [Cross-Platform Bash Script Patterns](cross-platform-bash-script-patterns.md) - Checkpoint scripts implement platform-safe patterns
- [Script Debugging Through Isolation](script-debugging-through-isolation.md) - Debugging checkpoint script issues
- [Sub-Agent Verification Pattern](sub-agent-verification-pattern.md) - Using sub-agents for verification instead of checkpoint scripts
