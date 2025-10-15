# Checkpoint-Based Workflow Pattern

## Learning
Complex multi-step workflows should use checkpoint tracking with progress files, mandatory gates between phases, and verification steps to ensure all quality standards are met before proceeding.

## Importance
Structured workflows prevent incomplete execution by making progress visible, enforcing dependencies between steps, and blocking advancement when quality gates fail. This ensures consistent, thorough execution of complex operations like commits, releases, or multi-phase builds.

## Example
```c
// Conceptual model of checkpoint workflow
// Real implementation uses bash scripts and make targets

// Step tracking structure (stored in /tmp/command_progress.txt)
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

1. **Progress Tracking File**: `/tmp/command_progress.txt` stores state
   - Command name and start time
   - Total step count
   - STEP_N=complete/pending for each step

2. **Step Management**:
   - `checkpoint-init`: Create tracking file with all steps
   - `checkpoint-update`: Mark step complete, show progress bar
   - `checkpoint-update-verified`: Mark complete with automated verification
   - `checkpoint-status`: Show current progress (verbose mode available)
   - `checkpoint-cleanup`: Remove tracking file when done

3. **Gate Enforcement**:
   - Block progress until required steps complete
   - Exit with error code if gate fails
   - Named gates (e.g., "Build Quality", "Integration")
   - `checkpoint-gate CMD=name GATE="Gate Name" REQUIRED="1,2,3"`

4. **Progress Visualization**:
   ```
   📈 commit: 5/9 steps (55%)
      [███████████░░░░░░░░░] 55%
   → Next: make checkpoint-update CMD=commit STEP=6
   ```

## Implementation
```bash
# Real AgeRun implementation example from /ar:commit command

# 1. Initialize workflow
make checkpoint-init CMD=commit STEPS='"Run Tests" "Check Logs" "Update Docs" "Update TODO" "Update CHANGELOG" "Review Changes" "Stage Files" "Create Commit" "Push and Verify"'

# 2. Execute steps with checkpoints
make checkpoint-update CMD=commit STEP=1
make test  # Run tests
make checkpoint-update-verified CMD=commit STEP=1  # Auto-verifies tests passed

# 3. Enforce gate after phase
make checkpoint-gate CMD=commit GATE="Build Quality" REQUIRED="1,2"
# Blocks if steps 1-2 not complete

# 4. Continue with remaining steps
make checkpoint-update CMD=commit STEP=5
# ... update CHANGELOG ...
make checkpoint-update-verified CMD=commit STEP=5 SUMMARY="CHANGELOG updated"

# 5. Check status anytime
make checkpoint-status CMD=commit VERBOSE=--verbose

# 6. Cleanup when done
make checkpoint-cleanup CMD=commit
```

## Workflow Structure Pattern
Organize complex workflows into phases with gates:

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
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
- [Command Thoroughness Requirements Pattern](command-thoroughness-requirements-pattern.md)
- [Systematic Task Analysis Protocol](systematic-task-analysis-protocol.md)
- [KB Target Compliance Enforcement](kb-target-compliance-enforcement.md)
- [Context Preservation Across Sessions](context-preservation-across-sessions.md)
- [Session Resumption Without Prompting](session-resumption-without-prompting.md)
