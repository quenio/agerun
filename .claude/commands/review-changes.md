Review uncommitted changes for code quality, architectural compliance, and documentation accuracy.
## What it does:

This command performs a comprehensive review of all uncommitted changes across multiple quality dimensions:

### 1. Code Quality Review
- **Code Smells Detection**: Long methods (>50 lines), large modules (>850 lines), excessive parameters (>5), duplication
- **Memory Management**: Ownership prefixes (own_, mut_, ref_), heap tracking macros, NULL after transfer
- **Naming Conventions**: ar_module__function pattern, proper prefixes, consistent style
- **Error Handling**: Proper propagation, single print location, graceful degradation
- **Test Coverage**: BDD structure, memory leak verification, one test per behavior

### 2. Architectural Compliance
- **Parnas Principles**: Information hiding, single responsibility, no circular dependencies
- **Module Hierarchy**: Foundation → Data → Core → System layer ordering
- **Interface Design**: Minimal interfaces, opaque types where needed, const-correctness
- **Dependency Management**: No upward dependencies, proper abstraction levels
- **Design Patterns**: Registry for ownership, facade for coordination only

### 3. Documentation Review
- **Real Code Only**: All examples use actual AgeRun types/functions
- **Validation**: Runs make check-docs to verify documentation
- **Cross-References**: Checks for related KB articles and links
- **Completeness**: TODO.md and CHANGELOG.md updates for changes
- **Relative Links**: Ensures markdown links use relative paths

### 4. Pre-Commit Verification
- **Build Status**: Verifies clean build with make clean build
- **Hidden Issues**: Runs make check-logs for concealed problems
- **Test Results**: Checks for memory leaks and test failures
- **File Hygiene**: Identifies backup/temporary files that shouldn't be committed
- **Documentation Sync**: Ensures docs match code changes
- **Task Planning**: For large changes, checks for analysis report ([details](../../kb/report-driven-task-planning.md))

## Usage:

```bash
/review-changes
```

## Review Process:

1. **Diff Analysis**: Reviews git diff for all changes
2. **Pattern Detection**: Scans for known issues and anti-patterns
3. **Compliance Check**: Verifies against architectural principles
4. **Documentation Validation**: Ensures docs are accurate and complete
5. **Quality Metrics**: Reports on code quality indicators with quantitative data ([details](../../kb/quantitative-priority-setting.md))
6. **Memory Leak Check**: Scans memory_report_*.log files for leaks
7. **Test Effectiveness**: Verifies tests actually catch failures
8. **Refactoring Safety**: Uses diff to verify code moves (not rewrites)

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

## Related Commands:
- `/check-logs` - Detailed build log analysis
- `/check-docs` - Documentation validation only
- `/check-naming` - Naming convention verification
- `/commit` - Create commit after review passes

## Related KB Articles:

### Core Review Patterns
- [Pre-Commit Checklist Detailed](../../kb/pre-commit-checklist-detailed.md)
- [Comprehensive Output Review](../../kb/comprehensive-output-review.md)
- [Code Smell Quick Detection](../../kb/code-smell-quick-detection.md)
- [Module Quality Checklist](../../kb/module-quality-checklist.md)

### Memory Management
- [Ownership Naming Conventions](../../kb/ownership-naming-conventions.md)
- [Ownership Pattern Extraction](../../kb/ownership-pattern-extraction.md)
- [Memory Leak Detection Workflow](../../kb/memory-leak-detection-workflow.md)
- [Memory Debugging Comprehensive Guide](../../kb/memory-debugging-comprehensive-guide.md)

### Architecture & Design
- [Architectural Patterns Hierarchy](../../kb/architectural-patterns-hierarchy.md)
- [No Circular Dependencies Principle](../../kb/no-circular-dependencies-principle.md)
- [Single Responsibility Principle](../../kb/single-responsibility-principle.md)
- [Information Hiding Principle](../../kb/information-hiding-principle.md)

### Testing Patterns
- [BDD Test Structure](../../kb/bdd-test-structure.md)
- [Test Effectiveness Verification](../../kb/test-effectiveness-verification.md)
- [Red-Green-Refactor Cycle](../../kb/red-green-refactor-cycle.md)
- [TDD Cycle Detailed Explanation](../../kb/tdd-cycle-detailed-explanation.md)

### Documentation
- [Documentation Standards Integration](../../kb/documentation-standards-integration.md)
- [Documentation Validation Enhancement Patterns](../../kb/documentation-validation-enhancement-patterns.md)
- [Documentation Completion Verification](../../kb/documentation-completion-verification.md)

### Refactoring
- [Refactoring Patterns Detailed](../../kb/refactoring-patterns-detailed.md)
- [Code Movement Verification](../../kb/code-movement-verification.md)
- [Refactoring Key Patterns](../../kb/refactoring-key-patterns.md)

### Error Handling
- [Error Propagation Pattern](../../kb/error-propagation-pattern.md)
- [Graceful Degradation Pattern](../../kb/graceful-degradation-pattern.md)

$ARGUMENTS