Review uncommitted changes for code quality, architectural compliance, and documentation accuracy.


## KB Consultation Required

Before reviewing ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "parnas\|principle\|smell\|architecture\|script\|extraction" kb/README.md`
2. Review relevant principles:
   - Parnas design principles
   - Code smell patterns
   - Architecture patterns
   - Standards over expediency ([details](../../../kb/standards-over-expediency-principle.md))
   - Helper script extraction ([details](../../../kb/command-helper-script-extraction-pattern.md))
   - Script naming conventions ([details](../../../kb/script-domain-naming-convention.md))
3. Apply these standards during review
   - When refactoring commands, extract complex bash logic: one focused script per shell block ([details](../../../kb/command-helper-script-extraction-pattern.md))
   - Use domain-specific naming (action-domain-object.sh) for discoverability ([details](../../../kb/script-domain-naming-convention.md))
   - Command files should orchestrate scripts directly (show all logic inline, not wrap in run-*.sh) ([details](../../../kb/command-orchestrator-pattern.md))

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

After completing each step, you MUST:

1. **Report accomplishments with concrete evidence**
   - Describe what was accomplished (files created/modified, commands executed, outputs produced)
   - Provide **concrete evidence**: actual file paths with line numbers, full command outputs, git diff output, test results with specific test names, grep/search output proving claims
   - **DO NOT** tell step-verifier what to verify - report what was done with evidence
   - **DO NOT** use vague summaries - provide specific details (see [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for examples)

2. **Invoke step-verifier sub-agent**
   - Use `mcp_sub-agents_run_agent` tool with:
     - Agent: `"step-verifier"`
     - Prompt: See format below
     - The step-verifier will independently verify your claims

3. **Handle Verification Results**
  
   **If verification PASSES** (report shows "✅ STEP VERIFIED" or "All requirements met"):
     - Proceed to next step
     -   
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
Prompt: "Verify Step N: [Step Title] completion for review-changes command.

Todo Item: [Description of what the step accomplished]
Command File: .opencode/command/ar/review-changes.md
Step: Step N: [Step Title]

Accomplishment Report:
[Report what was accomplished with CONCRETE EVIDENCE. The step-verifier will independently verify these claims by reading files, checking git status, etc.

**MANDATORY Evidence Requirements:**
- **File Changes**: Include actual file paths, line numbers, and git diff output showing exact changes
- **Command Execution**: Include full command output, exit codes, test results with specific test names
- **Documentation Updates**: Include file paths, section names, actual content snippets, git diff output
- **Git Status**: Include actual `git status` and `git diff` output showing what changed
- **Verification Output**: Include actual grep/search command output proving claims
- **Build/Test Results**: Include full output showing compilation, test execution, memory leak reports

**Examples:**
✅ GOOD: "Updated `.opencode/command/ar/execute-plan.md` line 2356: `git diff` shows lines changed from `### If progress tracking` to `### If step tracking`. Verification: `grep -i 'checkpoint' file.md` returned no matches (exit code 1)"
❌ BAD: "Updated execute-plan.md to remove checkpoint references"

See [kb/sub-agent-verification-pattern.md](../../../kb/sub-agent-verification-pattern.md) for complete evidence requirements and examples.]"
```

**CRITICAL**: 
- Report accomplishments with **concrete evidence** (file paths, line numbers, command outputs, git diff, test results), NOT instructions or vague summaries
- The step-verifier independently verifies by reading command files, checking files, git status/diff, etc.
- If step-verifier reports "⚠️ STOP EXECUTION", you MUST fix issues before proceeding
- If accomplishment report lacks concrete evidence, step-verifier will STOP execution and require evidence "⚠️ STOP EXECUTION", you MUST fix issues before proceeding

## MANDATORY: Initialize All Todo Items

**CRITICAL**: Before executing ANY steps, add ALL step and verification todo items to the session todo list using `todo_write`:

**Step and Verification Todo Items:**
- Add todo item: "Step 1: Diff Analysis" - Status: pending
- Add todo item: "Verify Step 1: Diff Analysis" - Status: pending
- Add todo item: "Step 2: Code Smells" - Status: pending
- Add todo item: "Verify Step 2: Code Smells" - Status: pending
- Add todo item: "Step 3: Memory Management" - Status: pending
- Add todo item: "Verify Step 3: Memory Management" - Status: pending
- Add todo item: "Step 4: Naming Conventions" - Status: pending
- Add todo item: "Verify Step 4: Naming Conventions" - Status: pending
- Add todo item: "Step 5: Error Handling" - Status: pending
- Add todo item: "Verify Step 5: Error Handling" - Status: pending
- Add todo item: "Step 6: Test Coverage" - Status: pending
- Add todo item: "Verify Step 6: Test Coverage" - Status: pending
- Add todo item: "Step 7: Parnas Principles" - Status: pending
- Add todo item: "Verify Step 7: Parnas Principles" - Status: pending
- Add todo item: "Step 8: Module Hierarchy" - Status: pending
- Add todo item: "Verify Step 8: Module Hierarchy" - Status: pending
- Add todo item: "Step 9: Interface Design" - Status: pending
- Add todo item: "Verify Step 9: Interface Design" - Status: pending
- Add todo item: "Step 10: Dependency Check" - Status: pending
- Add todo item: "Verify Step 10: Dependency Check" - Status: pending
- Add todo item: "Step 11: Design Patterns" - Status: pending
- Add todo item: "Verify Step 11: Design Patterns" - Status: pending
- Add todo item: "Step 12: Real Code Check" - Status: pending
- Add todo item: "Verify Step 12: Real Code Check" - Status: pending
- Add todo item: "Step 13: Doc Validation" - Status: pending
- Add todo item: "Verify Step 13: Doc Validation" - Status: pending
- Add todo item: "Step 14: Cross-References" - Status: pending
- Add todo item: "Verify Step 14: Cross-References" - Status: pending
- Add todo item: "Step 15: Completeness" - Status: pending
- Add todo item: "Verify Step 15: Completeness" - Status: pending
- Add todo item: "Step 16: Link Validation" - Status: pending
- Add todo item: "Verify Step 16: Link Validation" - Status: pending
- Add todo item: "Step 17: Build Status" - Status: pending
- Add todo item: "Verify Step 17: Build Status" - Status: pending
- Add todo item: "Step 18: Hidden Issues" - Status: pending
- Add todo item: "Verify Step 18: Hidden Issues" - Status: pending
- Add todo item: "Step 19: Test Results" - Status: pending
- Add todo item: "Verify Step 19: Test Results" - Status: pending
- Add todo item: "Step 20: File Hygiene" - Status: pending
- Add todo item: "Verify Step 20: File Hygiene" - Status: pending
- Add todo item: "Step 21: Doc Sync" - Status: pending
- Add todo item: "Verify Step 21: Doc Sync" - Status: pending
- Add todo item: "Step 22: Final Report" - Status: pending
- Add todo item: "Verify Step 22: Final Report" - Status: pending
- Add todo item: "Verify Complete Workflow: review-changes" - Status: pending

**Important**: All todo items are initialized as `pending` and will be updated to `in_progress` when their respective step/verification begins, then to `completed` after verification passes.

### In-Progress Workflow Detection

If a `/review-changes` workflow is already in progress:

# Review Changes

This command uses session todo tracking to ensure thorough review across all quality dimensions. The review process is divided into 4 major phases with 22 steps total.

### Review Process

### Stage 1: Code Quality Review (Steps 1-6)

1. **Diff Analysis**: Reviews git diff for all changes
   ```bash
   # After completing diff analysis
      ```
2. **Code Smells Detection**: Scans for known issues and anti-patterns
   - Long methods (>50 lines)
   - Large modules (>850 lines)
   - Excessive parameters (>5)
   - Code duplication

   **Helper script available:**
   ```bash
   ./scripts/detect-code-smells.sh [path]
   # Returns: Exit 1 if code smells found, 0 if clean
   ```

   ```bash
      ```
3. **Memory Management Check**: Verifies ownership and heap tracking
   - Ownership prefixes (own_, mut_, ref_)
   - Heap tracking macros
   - NULL after transfer
   - Memory leak reports

   **Helper script available:**
   ```bash
   ./scripts/verify-memory-management.sh [files...]
   # Auto-detects changed files from git if no args provided
   # Returns: Exit 1 if violations found, 0 if clean
   ```

   ```bash
      ```

4. **Naming Conventions Check**: Validates consistent naming
   - ar_module__function pattern
   - Proper prefixes
   - Consistent style

   **Helper script available:**
   ```bash
   ./scripts/check-naming-conventions.sh [files...]
   # Auto-detects changed files from git if no args provided
   # Returns: Exit 1 if violations found, 0 if clean
   ```

   ```bash
      ```

5. **Error Handling Check**: Reviews error propagation
   - Proper propagation pattern
   - Single print location
   - Graceful degradation
   ```bash
      ```

6. **Test Coverage Check**: Validates test quality
   - BDD structure
   - Memory leak verification
   - One test per behavior

   **Helper script available:**
   ```bash
   ./scripts/verify-test-coverage.sh [test-files...]
   # Auto-finds all *_tests.c files if no args provided
   # Returns: Exit 1 if quality issues found, 0 if clean
   ```

   ```bash
      ```

**[QUALITY GATE 1: Code Quality Complete]**
```bash
# MANDATORY: Must pass before proceeding to architecture review
```

**Expected gate output:**
```
✅ GATE 'Code Quality' - PASSED
   Verified: Steps 1,2,3,4,5,6
```

**Minimum Requirements for Stage 1:**
- [ ] Check at least 3 types of code smells
- [ ] Verify ownership prefixes in all changed files
- [ ] Review memory_report_*.log files for leaks
- [ ] Identify any naming violations
- [ ] Document all issues found with file:line references

### Stage 2: Architectural Compliance (Steps 7-11)

7. **Parnas Principles Check**: Verifies design principles
   - Information hiding
   - Single responsibility
   - No circular dependencies
   ```bash
      ```

8. **Module Hierarchy Check**: Validates layer ordering
   - Foundation → Data → Core → System
   - No upward dependencies
   ```bash
      ```

9. **Interface Design Check**: Reviews API minimality
   - Minimal interfaces
   - Opaque types where needed
   - Const-correctness
   ```bash
      ```

10. **Dependency Management Check**: Analyzes module dependencies
    - No circular dependencies (except heap ↔ io)
    - Proper abstraction levels

    **Helper script available:**
    ```bash
    ./scripts/detect-circular-dependencies.sh [modules-path]
    # Builds dependency graph, detects cycles and upward dependencies
    # Returns: Exit 1 if violations found, 0 if clean
    ```

    ```bash
        ```

11. **Design Patterns Check**: Reviews pattern usage
    - Registry for ownership
    - Facade for coordination only
    - Parser/executor separation
    ```bash
        ```

**[QUALITY GATE 2: Architecture Complete]**
```bash
# MANDATORY: Must pass before proceeding to documentation review
```

**Expected gate output:**
```
✅ GATE 'Architecture' - PASSED
   Verified: Steps 7,8,9,10,11
```

**Minimum Requirements for Stage 2:**
- [ ] Verify no new circular dependencies introduced
- [ ] Check module hierarchy compliance
- [ ] Validate interface minimality
- [ ] Document any architectural violations

### Stage 3: Documentation Review (Steps 12-16)

12. **Real Code Check**: Validates documentation examples
    - All examples use actual AgeRun types/functions
    - No placeholder code
    ```bash
        ```

13. **Documentation Validation**: Runs validation checks
    ```bash
    make check-docs
        ```

14. **Cross-References Check**: Verifies KB article links
    - Related articles linked
    - Bidirectional references
    ```bash
        ```

15. **Completeness Check**: Ensures required docs updated
    - TODO.md updates for changes
    - CHANGELOG.md entries
    ```bash
        ```

16. **Link Validation Check**: Verifies markdown links
    - Relative paths only
    - No broken links
    ```bash
        ```

**[QUALITY GATE 3: Documentation Complete]**
```bash
# MANDATORY: Must pass before final verification
```

**Expected gate output:**
```
✅ GATE 'Documentation' - PASSED
   Verified: Steps 12,13,14,15,16
```

**Minimum Requirements for Stage 3:**
- [ ] Run and pass `make check-docs`
- [ ] Verify TODO.md and CHANGELOG.md updated
- [ ] Check for broken links
- [ ] Ensure all examples use real code

### Stage 4: Pre-Commit Verification (Steps 17-22)

17. **Build Status Check**: Verifies clean build
    ```bash
    make clean build 2>&1
        ```

18. **Hidden Issues Check**: Runs log analysis
    ```bash
    make check-logs
        ```

19. **Test Results Check**: Reviews test outcomes
    - All tests passing
    - No memory leaks in reports
    - Test effectiveness verified
    ```bash
        ```

20. **File Hygiene Check**: Identifies unwanted files
    - No backup files (.bak)
    - No temporary files
    - No debug outputs

    **Helper script available:**
    ```bash
    ./scripts/check-file-hygiene.sh [path]
    # Finds backup, temp, debug, and core dump files
    # Returns: Exit 1 if unwanted files found, 0 if clean
    ```

    ```bash
        ```

21. **Documentation Sync Check**: Final doc verification
    - Docs match code changes
    - All updates synchronized
    ```bash
        ```

**[QUALITY GATE 4: Pre-Commit Complete]**
```bash
# MANDATORY: Must pass before generating final report
```

**Expected gate output:**
```
✅ GATE 'Pre-Commit' - PASSED
   Verified: Steps 17,18,19,20,21
```

**Minimum Requirements for Stage 4:**
- [ ] Clean build with no errors
- [ ] Pass `make check-logs`
- [ ] No memory leaks detected
- [ ] No temporary files to commit

22. **Final Report Generation**: Compile review results
    ```bash
        ```

**Expected completion output:**
```
========================================
   WORKFLOW COMPLETION SUMMARY
========================================

📈 review-changes: X/Y steps (Z%)
   [████████████████████] 100%

```
```

```bash
# Clean up tracking file
```

## Review Metrics and Quality Tracking

The enhanced review process tracks quantitative metrics:

### Quality Metrics ([details](../../../kb/quantitative-priority-setting.md))
- **Code Quality Score**: Percentage of checks passed
- **Issue Density**: Issues per 100 lines changed
- **Memory Safety**: Percentage of proper ownership usage
- **Test Coverage**: Percentage of behaviors with tests
- **Documentation Compliance**: Percentage passing validation

### Progress Tracking Benefits
- **Resumable Reviews**: Can pause and resume review sessions
- **Systematic Coverage**: Ensures nothing gets skipped
- **Quality Gates**: Prevents rushing through critical checks
- **Visible Progress**: Shows exactly where you are in the process
- **Accountability**: Documents what was checked and when

## Output Format:

The review provides:
- **Summary**: Overall health of changes
- **Issues Found**: Categorized by severity (critical/warning/suggestion)
- **Specific Locations**: File:line references for each issue
- **Recommendations**: Actionable fixes for each problem
- **Checklist**: Final pre-commit verification items

## Review Categories:

### Critical Issues (Must Fix)
- Memory leaks or missing ownership management (check memory_report_*.log files)
- Ownership naming violations (own_, mut_, ref_ prefixes required)
- Missing heap tracking macros (AR__HEAP__MALLOC, AR__HEAP__FREE)
- Circular dependencies between modules (except heap ↔ io)
- Build failures or test failures (make clean build 2>&1)
- Invalid documentation examples (not using real AgeRun types)
- Missing required documentation updates (TODO.md, CHANGELOG.md)
- Tests without BDD structure (Given/When/Then comments)
- Missing AR_ASSERT macros in tests
- Untested error conditions (test effectiveness not verified)

### Warnings (Should Fix)
- Code smells: Long methods (>50 lines), large modules (>850 lines)
- Functions with >5 parameters (use structs instead)
- Duplicate code patterns (should be extracted)
- Naming convention violations (ar_module__function pattern)
- Missing test coverage for new behaviors
- Incomplete error handling (not following propagation pattern)
- Architectural principle violations (Parnas principles)
- Mixed abstraction levels in same module
- God modules with multiple responsibilities
- Missing opaque types for complex structures

### Suggestions (Consider)
- Opportunities for ownership pattern extraction (ar_data__claim_or_copy)
- Better design patterns available (registry, facade, parser/executor split)
- Documentation enhancements (cross-references to KB articles)
- Performance improvements (memory-efficient streaming)
- Code simplification opportunities (removing non-functional code)
- Graceful degradation for non-critical operations
- Test simplification (use literals not complex expressions)
- Refactoring to single responsibility modules

## Integration with Commit Workflow:

Run this command before `/commit` to ensure:
1. All critical issues are resolved
2. Documentation is complete and valid
3. Tests pass without memory leaks
4. Architecture remains clean
5. Code quality standards are met

## Troubleshooting

### If progress tracking gets stuck:
```bash
# Check current status

# If needed, reset and start over
```

### If a gate is blocking incorrectly:
```bash
# Manually check which steps are pending
cat /tmp/review-changes-progress.txt

# Update a specific step if it was completed
```

### To skip progress tracking (emergency only):
The original review process still works without session todo tracking, but you lose progress tracking and quality gates. This should only be used in emergencies.

## Related Commands:
- `/check-logs` - Detailed build log analysis
- `/check-docs` - Documentation validation only
- `/check-naming` - Naming convention verification
- `/commit` - Create commit after review passes
- `/new-learnings` - Extract patterns from review findings

## Related KB Articles:

### Tracking Patterns
- [Session Todo List Tracking Pattern](../../../kb/session-todo-list-tracking-pattern.md)
- [Gate Enforcement Exit Codes Pattern](../../../kb/gate-enforcement-exit-codes-pattern.md)
- [Progress Visualization ASCII Pattern](../../../kb/progress-visualization-ascii-pattern.md)
- [Command Thoroughness Requirements Pattern](../../../kb/command-thoroughness-requirements-pattern.md)
- [Command Output Documentation Pattern](../../../kb/command-output-documentation-pattern.md)

### Core Review Patterns
- [Pre-Commit Checklist Detailed](../../../kb/pre-commit-checklist-detailed.md)
- [Comprehensive Output Review](../../../kb/comprehensive-output-review.md)
- [Code Smell Quick Detection](../../../kb/code-smell-quick-detection.md)
- [Module Quality Checklist](../../../kb/module-quality-checklist.md)
- [Quantitative Priority Setting](../../../kb/quantitative-priority-setting.md)
- [Plan Review Status Tracking Pattern](../../../kb/plan-review-status-tracking.md)
- [Iterative Plan Review Protocol](../../../kb/iterative-plan-review-protocol.md)
- [Iterative Plan Refinement Pattern](../../../kb/iterative-plan-refinement-pattern.md)

### Memory Management
- [Ownership Naming Conventions](../../../kb/ownership-naming-conventions.md)
- [Ownership Pattern Extraction](../../../kb/ownership-pattern-extraction.md)
- [Memory Leak Detection Workflow](../../../kb/memory-leak-detection-workflow.md)
- [Memory Debugging Comprehensive Guide](../../../kb/memory-debugging-comprehensive-guide.md)

### Architecture & Design
- [Architectural Patterns Hierarchy](../../../kb/architectural-patterns-hierarchy.md)
- [No Circular Dependencies Principle](../../../kb/no-circular-dependencies-principle.md)
- [Single Responsibility Principle](../../../kb/single-responsibility-principle.md)
- [Information Hiding Principle](../../../kb/information-hiding-principle.md)

### Testing Patterns
- [BDD Test Structure](../../../kb/bdd-test-structure.md)
- [Test Effectiveness Verification](../../../kb/test-effectiveness-verification.md)
- [Red-Green-Refactor Cycle](../../../kb/red-green-refactor-cycle.md)
- [TDD Cycle Detailed Explanation](../../../kb/tdd-cycle-detailed-explanation.md)
- [TDD Plan Iteration Split Pattern](../../../kb/tdd-plan-iteration-split-pattern.md)
- [TDD GREEN Phase Minimalism](../../../kb/tdd-green-phase-minimalism.md)
- [TDD Iteration Planning Pattern](../../../kb/tdd-iteration-planning-pattern.md)
- [Temporary Test Cleanup Pattern](../../../kb/temporary-test-cleanup-pattern.md)
- [Mock at Right Level Pattern](../../../kb/mock-at-right-level-pattern.md)
- [Test Complexity as Code Smell](../../../kb/test-complexity-as-code-smell.md)
- [Check Existing Solutions First](../../../kb/check-existing-solutions-first.md)
- [Test Assertion Strength Patterns](../../../kb/test-assertion-strength-patterns.md)
- [DLSym Test Interception Technique](../../../kb/dlsym-test-interception-technique.md)

### Documentation
- [Documentation Standards Integration](../../../kb/documentation-standards-integration.md)
- [Documentation Validation Enhancement Patterns](../../../kb/documentation-validation-enhancement-patterns.md)
- [Documentation Completion Verification](../../../kb/documentation-completion-verification.md)
- [Documentation Index Consistency Pattern](../../../kb/documentation-index-consistency-pattern.md)

### Refactoring
- [Refactoring Patterns Detailed](../../../kb/refactoring-patterns-detailed.md)
- [Code Movement Verification](../../../kb/code-movement-verification.md)
- [Refactoring Key Patterns](../../../kb/refactoring-key-patterns.md)

### Error Handling
- [Error Propagation Pattern](../../../kb/error-propagation-pattern.md)
- [Graceful Degradation Pattern](../../../kb/graceful-degradation-pattern.md)

$ARGUMENTS