Review uncommitted changes for code quality, architectural compliance, and documentation accuracy.

**MANDATORY**: This command MUST use checkpoint tracking. Start by running the checkpoint initialization below. ([details](../../../kb/unmissable-documentation-pattern.md))

## KB Consultation Required

Before reviewing ([details](../../../kb/kb-consultation-before-planning-requirement.md)):
1. Search: `grep "parnas\|principle\|smell\|architecture" kb/README.md`
2. Review relevant principles:
   - Parnas design principles
   - Code smell patterns
   - Architecture patterns
   - Standards over expediency ([details](../../../kb/standards-over-expediency-principle.md))
3. Apply these standards during review

## CHECKPOINT WORKFLOW ENFORCEMENT

**CRITICAL**: This command MUST use checkpoint tracking for ALL execution.

### In-Progress Workflow Detection

If a `/review-changes` workflow is already in progress:

```bash
make checkpoint-status CMD=review-changes VERBOSE=--verbose
# Resume: make checkpoint-update CMD=review-changes STEP=N
# Or reset: make checkpoint-cleanup CMD=review-changes && make checkpoint-init CMD=review-changes STEPS='"Diff Analysis" "Code Smells" "Memory Management" "Naming Conventions" "Error Handling" "Test Coverage" "Parnas Principles" "Module Hierarchy" "Interface Design" "Dependency Check" "Design Patterns" "Real Code Check" "Doc Validation" "Cross-References" "Completeness" "Link Validation" "Build Status" "Hidden Issues" "Test Results" "File Hygiene" "Doc Sync" "Final Report"'
```

### First-Time Initialization Check

```bash
if [ ! -f /tmp/review_changes_progress.txt ]; then
  echo "⚠️  Initializing checkpoint tracking..."
  make checkpoint-init CMD=review-changes STEPS='"Diff Analysis" "Code Smells" "Memory Management" "Naming Conventions" "Error Handling" "Test Coverage" "Parnas Principles" "Module Hierarchy" "Interface Design" "Dependency Check" "Design Patterns" "Real Code Check" "Doc Validation" "Cross-References" "Completeness" "Link Validation" "Build Status" "Hidden Issues" "Test Results" "File Hygiene" "Doc Sync" "Final Report"'
else
  make checkpoint-status CMD=review-changes
fi
```

## PRECONDITION: Checkpoint Tracking Must Be Initialized

```bash
if [ ! -f /tmp/review_changes_progress.txt ]; then
  echo "❌ ERROR: Checkpoint tracking not initialized!"
  exit 1
fi
```

# Review Changes

## ⚠️ REQUIRED: Initialize Checkpoint Tracking First

**DO NOT PROCEED WITHOUT RUNNING THIS COMMAND:**

```bash
# MANDATORY: Initialize checkpoint tracking (22 steps)
make checkpoint-init CMD=review-changes STEPS='"Diff Analysis" "Code Smells" "Memory Management" "Naming Conventions" "Error Handling" "Test Coverage" "Parnas Principles" "Module Hierarchy" "Interface Design" "Dependency Check" "Design Patterns" "Real Code Check" "Doc Validation" "Cross-References" "Completeness" "Link Validation" "Build Status" "Hidden Issues" "Test Results" "File Hygiene" "Doc Sync" "Final Report"'
```

This command uses checkpoint tracking to ensure thorough review across all quality dimensions. The review process is divided into 4 major phases with 22 checkpoints total.

**Expected output:**
```
========================================
   CHECKPOINT TRACKING INITIALIZED
========================================

Command: review-changes
Tracking file: /tmp/review-changes_progress.txt
Total steps: 22

Steps to complete:
  1. Diff Analysis
  2. Code Smells
  3. Memory Management
  4. Naming Conventions
  5. Error Handling
  6. Test Coverage
  7. Parnas Principles
  8. Module Hierarchy
  9. Interface Design
  10. Dependency Check
  11. Design Patterns
  12. Real Code Check
  13. Doc Validation
  14. Cross-References
  15. Completeness
  16. Link Validation
  17. Build Status
  18. Hidden Issues
  19. Test Results
  20. File Hygiene
  21. Doc Sync
  22. Final Report
```

### Check Progress
```bash
make checkpoint-status CMD=review-changes
```

**Expected output (example at 27% completion):**
```
📈 review-changes: 6/22 steps (27%)
   [█████░░░░░░░░░░░░░░░] 27%
→ Next: make checkpoint-update CMD=review-changes STEP=7
```

### What it does

This command performs a comprehensive review of all uncommitted changes across multiple quality dimensions:

#### 1. Code Quality Review
- **Code Smells Detection**: Long methods (>50 lines), large modules (>850 lines), excessive parameters (>5), duplication
- **Memory Management**: Ownership prefixes (own_, mut_, ref_), heap tracking macros, NULL after transfer
- **Naming Conventions**: ar_module__function pattern, proper prefixes, consistent style, take_ vs get_ distinction ([details](../../../kb/function-naming-state-change-convention.md))
- **Error Handling**: Proper propagation, single print location, graceful degradation
- **Test Coverage**: BDD structure, memory leak verification, one test per behavior
- **Feature Value**: Validate features provide functional value, not just decoration ([details](../../../kb/functional-value-validation-pattern.md))

#### 2. Architectural Compliance
- **Parnas Principles**: Information hiding, single responsibility, no circular dependencies
- **Module Hierarchy**: Foundation → Data → Core → System layer ordering
- **Interface Design**: Minimal interfaces, opaque types where needed, const-correctness
- **Dependency Management**: No upward dependencies, proper abstraction levels
- **Design Patterns**: Registry for ownership, facade for coordination only

#### 3. Documentation Review
- **Real Code Only**: All examples use actual AgeRun types/functions
- **Validation**: Runs make check-docs to verify documentation
- **Cross-References**: Checks for related KB articles and links
- **Completeness**: TODO.md and CHANGELOG.md updates for changes
- **Relative Links**: Ensures markdown links use relative paths

#### 4. Pre-Commit Verification
- **Build Status**: Verifies clean build with make clean build
- **Hidden Issues**: Runs make check-logs for concealed problems
- **Test Results**: Checks for memory leaks and test failures
- **File Hygiene**: Identifies backup/temporary files that shouldn't be committed
- **Documentation Sync**: Ensures docs match code changes
- **Task Planning**: For large changes, checks for analysis report ([details](../../../kb/report-driven-task-planning.md))

### Execution Order (MANDATORY)

1. **FIRST**: Run the checkpoint initialization command above
2. **SECOND**: Follow the review process below, updating checkpoints after each step
3. **THIRD**: Check progress with `make checkpoint-status CMD=review-changes`
4. **FOURTH**: Complete all 22 steps before generating final report
5. **LAST**: Clean up with `make checkpoint-cleanup CMD=review-changes`

### Usage

```bash
/review-changes
```

**IMPORTANT**: Running `/review-changes` alone is NOT sufficient. You MUST initialize checkpoints first as shown above.

### Review Process

### Stage 1: Code Quality Review (Steps 1-6)

#### [CHECKPOINT START - STAGE 1]

1. **Diff Analysis**: Reviews git diff for all changes
   ```bash
   # After completing diff analysis
   make checkpoint-update CMD=review-changes STEP=1
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
   make checkpoint-update CMD=review-changes STEP=2
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
   make checkpoint-update CMD=review-changes STEP=3
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
   make checkpoint-update CMD=review-changes STEP=4
   ```

5. **Error Handling Check**: Reviews error propagation
   - Proper propagation pattern
   - Single print location
   - Graceful degradation
   ```bash
   make checkpoint-update CMD=review-changes STEP=5
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
   make checkpoint-update CMD=review-changes STEP=6
   ```

**[QUALITY GATE 1: Code Quality Complete]**
```bash
# MANDATORY: Must pass before proceeding to architecture review
make checkpoint-gate CMD=review-changes GATE="Code Quality" REQUIRED="1,2,3,4,5,6"
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

#### [CHECKPOINT END]

### Stage 2: Architectural Compliance (Steps 7-11)

#### [CHECKPOINT START - STAGE 2]

7. **Parnas Principles Check**: Verifies design principles
   - Information hiding
   - Single responsibility
   - No circular dependencies
   ```bash
   make checkpoint-update CMD=review-changes STEP=7
   ```

8. **Module Hierarchy Check**: Validates layer ordering
   - Foundation → Data → Core → System
   - No upward dependencies
   ```bash
   make checkpoint-update CMD=review-changes STEP=8
   ```

9. **Interface Design Check**: Reviews API minimality
   - Minimal interfaces
   - Opaque types where needed
   - Const-correctness
   ```bash
   make checkpoint-update CMD=review-changes STEP=9
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
    make checkpoint-update CMD=review-changes STEP=10
    ```

11. **Design Patterns Check**: Reviews pattern usage
    - Registry for ownership
    - Facade for coordination only
    - Parser/executor separation
    ```bash
    make checkpoint-update CMD=review-changes STEP=11
    ```

**[QUALITY GATE 2: Architecture Complete]**
```bash
# MANDATORY: Must pass before proceeding to documentation review
make checkpoint-gate CMD=review-changes GATE="Architecture" REQUIRED="7,8,9,10,11"
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

#### [CHECKPOINT END]

### Stage 3: Documentation Review (Steps 12-16)

#### [CHECKPOINT START - STAGE 3]

12. **Real Code Check**: Validates documentation examples
    - All examples use actual AgeRun types/functions
    - No placeholder code
    ```bash
    make checkpoint-update CMD=review-changes STEP=12
    ```

13. **Documentation Validation**: Runs validation checks
    ```bash
    make check-docs
    make checkpoint-update CMD=review-changes STEP=13
    ```

14. **Cross-References Check**: Verifies KB article links
    - Related articles linked
    - Bidirectional references
    ```bash
    make checkpoint-update CMD=review-changes STEP=14
    ```

15. **Completeness Check**: Ensures required docs updated
    - TODO.md updates for changes
    - CHANGELOG.md entries
    ```bash
    make checkpoint-update CMD=review-changes STEP=15
    ```

16. **Link Validation Check**: Verifies markdown links
    - Relative paths only
    - No broken links
    ```bash
    make checkpoint-update CMD=review-changes STEP=16
    ```

**[QUALITY GATE 3: Documentation Complete]**
```bash
# MANDATORY: Must pass before final verification
make checkpoint-gate CMD=review-changes GATE="Documentation" REQUIRED="12,13,14,15,16"
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

#### [CHECKPOINT END]

### Stage 4: Pre-Commit Verification (Steps 17-22)

#### [CHECKPOINT START - STAGE 4]

17. **Build Status Check**: Verifies clean build
    ```bash
    make clean build 2>&1
    make checkpoint-update CMD=review-changes STEP=17
    ```

18. **Hidden Issues Check**: Runs log analysis
    ```bash
    make check-logs
    make checkpoint-update CMD=review-changes STEP=18
    ```

19. **Test Results Check**: Reviews test outcomes
    - All tests passing
    - No memory leaks in reports
    - Test effectiveness verified
    ```bash
    make checkpoint-update CMD=review-changes STEP=19
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
    make checkpoint-update CMD=review-changes STEP=20
    ```

21. **Documentation Sync Check**: Final doc verification
    - Docs match code changes
    - All updates synchronized
    ```bash
    make checkpoint-update CMD=review-changes STEP=21
    ```

**[QUALITY GATE 4: Pre-Commit Complete]**
```bash
# MANDATORY: Must pass before generating final report
make checkpoint-gate CMD=review-changes GATE="Pre-Commit" REQUIRED="17,18,19,20,21"
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
    make checkpoint-update CMD=review-changes STEP=22
    ```

#### [CHECKPOINT END]

#### [CHECKPOINT COMPLETE]
```bash
./scripts/complete-checkpoint.sh review-changes
```

**Expected completion output:**
```
========================================
   CHECKPOINT COMPLETION SUMMARY
========================================

📈 review-changes: X/Y steps (Z%)
   [████████████████████] 100%

✅ Checkpoint workflow complete
```
```

```bash
# Clean up tracking file
make checkpoint-cleanup CMD=review-changes
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

### If checkpoint tracking gets stuck:
```bash
# Check current status
make checkpoint-status CMD=review-changes

# If needed, reset and start over
make checkpoint-cleanup CMD=review-changes
make checkpoint-init CMD=review-changes STEPS='...'
```

### If a gate is blocking incorrectly:
```bash
# Manually check which steps are pending
cat /tmp/review-changes_progress.txt

# Update a specific step if it was completed
make checkpoint-update CMD=review-changes STEP=N
```

### To skip checkpoint tracking (emergency only):
The original review process still works without checkpoints, but you lose progress tracking and quality gates. This should only be used in emergencies.

## Related Commands:
- `/check-logs` - Detailed build log analysis
- `/check-docs` - Documentation validation only
- `/check-naming` - Naming convention verification
- `/commit` - Create commit after review passes
- `/new-learnings` - Extract patterns from review findings

## Related KB Articles:

### Checkpoint and Progress Patterns
- [Multi-Step Checkpoint Tracking Pattern](../../../kb/multi-step-checkpoint-tracking-pattern.md)
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