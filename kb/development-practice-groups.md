# Development Practice Groups

## Learning

This pattern organizes development practices into logical groups for easier reference and application, consolidating related practices that were previously scattered.

## Importance

Scattered practices are hard to find and apply consistently. Grouping them by category makes it easier to apply all relevant practices for a given task type, reducing errors and improving consistency.

## Example

### Scattered Practices (Hard to Find)
```
- Use absolute paths
- Check syscall returns  
- Never use .bak files
- Redirect stderr
- Use make targets
- Check existing methods
- Use proper variable assignment
```

### Grouped Practices (Easy to Apply)
```
Navigation & Files:
- Use absolute paths only
- Git stash/diff not .bak files  
- ar_io creates backups automatically

Build & Debug:
- make sanitize-tests 2>&1
- Redirect stderr, check syscalls
- Always use make targets not direct scripts

Pattern Discovery:
- Check existing methods for solutions
- grep -r "pattern" modules/
- Extract common functions
```

## Generalization

### Major Practice Categories

**Navigation & File Management**:
- Absolute paths only ([details](absolute-path-navigation.md))
- Git for backups, never .bak ([details](file-io-backup-mechanism.md))
- Stub unready features with notes ([details](stub-and-revisit-pattern.md))

**Build System**:
- Parallel jobs for efficiency ([details](parallel-build-job-integration.md))
- Always use make targets ([details](make-target-testing-discipline.md))
- Shell variable assignment ([details](shell-script-command-substitution.md))

**Debug & Analysis**:
- `make sanitize-tests 2>&1` ([details](development-debug-tools.md))
- Static analyzer compliance ([details](static-analyzer-stream-compliance.md))
- Ownership debugging at transfer points ([details](debug-logging-ownership-tracing.md))

**Error & Log Management**:
- Context-aware error filtering ([details](intentional-test-errors-filtering.md))
- Precise grep patterns for metrics ([details](build-log-extraction-patterns.md))
- Whitelist reduction strategies ([details](systematic-whitelist-error-resolution.md))

**YAML & Persistence**:
- Quote escaping issues ([details](yaml-string-matching-pitfalls.md))
- Direct I/O for ar_data_t ([details](yaml-serialization-direct-io-pattern.md))
- 2-space indentation ([details](yaml-indentation-consistency-pattern.md))
- Container stack parsing ([details](container-stack-parsing-pattern.md))
- Multi-line escaping ([details](multi-line-data-persistence-format.md))

**Pattern Discovery**:
- Check existing methods ([details](cross-method-pattern-discovery.md))
- Fix root causes not symptoms ([details](systematic-error-whitelist-reduction.md))
- Filter at source vs whitelisting ([details](whitelist-vs-pattern-filtering.md))

**Process Management**:
- Checkpoint tracking for complex tasks ([details](multi-step-checkpoint-tracking-pattern.md))
- Gates for phase validation ([details](gate-enforcement-exit-codes-pattern.md))
- Progress visualization ([details](progress-visualization-ascii-pattern.md))

**CI/CD**:
- gh CLI for debugging ([details](github-actions-debugging-workflow.md))
- Local/CI discrepancy investigation ([details](local-ci-discrepancy-investigation.md))
- Tool version verification ([details](tool-version-selection-due-diligence.md))

**Quality Practices**:
- Stop after 3 failed attempts ([details](struggling-detection-pattern.md))
- Staged cleanup approach ([details](staged-cleanup-pattern.md))
- Comprehensive impact analysis ([details](comprehensive-impact-analysis.md))

## Implementation

### Quick Reference by Task Type

**Starting New Feature**:
1. Check existing patterns: `grep -r "feature" kb/*.md`
2. Use absolute paths in navigation
3. Create stubs with dependency notes
4. Use checkpoint tracking for complex tasks

**Debugging Issues**:
1. `make sanitize-tests 2>&1`
2. Add ownership logging at transfer points
3. Use precise grep for log extraction
4. Check CI with gh CLI if local works

**File Operations**:
1. Always absolute paths
2. Let ar_io handle backups
3. Direct YAML I/O for ar_data_t
4. Escape newlines in multi-line data

**Build Problems**:
1. Always use make targets
2. Check shell variable assignments
3. Verify parallel job configuration
4. Review static analyzer compliance

### Script Enhancement Patterns

**Creating Scripts**:
- Add to `/scripts/` directory
- Create make target wrapper
- Make reusable with parameters ([details](script-reusability-parameters-pattern.md))
- Support batch operations ([details](batch-update-script-pattern.md))

**Progressive Enhancement**:
- Start simple, enhance iteratively ([details](progressive-tool-enhancement.md))
- Add checkpoint support for complex scripts
- Include progress visualization

## Related Patterns

- [Development Debug Tools](development-debug-tools.md)
- [Make Target Testing Discipline](make-target-testing-discipline.md)
- [Comprehensive Impact Analysis](comprehensive-impact-analysis.md)
- [Multi-Step Checkpoint Tracking Pattern](multi-step-checkpoint-tracking-pattern.md)