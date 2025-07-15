# Knowledge Base

This directory contains development learnings captured from Claude Code sessions. Each file documents a specific pattern, technique, or insight that emerged during development.

## Purpose
- Preserve institutional knowledge from development sessions
- Provide searchable reference for common patterns
- Document solutions to recurring problems
- Share insights between development sessions

## Format
Each learning file contains:
- **Learning**: What was discovered
- **Importance**: Why it matters
- **Example**: Specific instance from development
- **Generalization**: How to apply broadly
- **Implementation**: Code/commands when applicable
- **Related Patterns**: Connected concepts

## Current Learnings

### Development Workflow
- [API Migration Completion Verification](api-migration-completion-verification.md)
- [Build Verification Before Commit](build-verification-before-commit.md)
- [Build Time Reporting](build-time-reporting.md)
- [Documentation Language Migration Updates](documentation-language-migration-updates.md)
- [Git Push Verification](git-push-verification.md)
- [Language Instruction Renaming Pattern](language-instruction-renaming-pattern.md)
- [Red Green Refactor Cycle](red-green-refactor-cycle.md)
- [Search Replace Precision](search-replace-precision.md)
- [Specification Consistency Maintenance](specification-consistency-maintenance.md)

### Memory Management
- [Memory Leak Detection Workflow](memory-leak-detection-workflow.md)
- [Ownership Naming Conventions](ownership-naming-conventions.md)
- [Memory Debugging Comprehensive Guide](memory-debugging-comprehensive-guide.md)
- [Expression Ownership Rules](expression-ownership-rules.md)

### Code Quality & Testing
- [BDD Test Structure](bdd-test-structure.md)
- [Test Function Naming Accuracy](test-function-naming-accuracy.md)
- [Code Movement Verification](code-movement-verification.md)
- [TDD Advanced Large Refactoring](tdd-advanced-large-refactoring.md)
- [Code Smell Quick Detection](code-smell-quick-detection.md)
- [Module Quality Checklist](module-quality-checklist.md)
- [Method Test Template](method-test-template.md)

### Architecture & Design
- [Opaque Types Pattern](opaque-types-pattern.md)
- [Facade Pattern Coordination](facade-pattern-coordination.md)
- [Error Propagation Pattern](error-propagation-pattern.md)
- [Refactoring Key Patterns](refactoring-key-patterns.md)
- [Architectural Patterns Hierarchy](architectural-patterns-hierarchy.md)

### Parnas Design Principles
- [Information Hiding Principle](information-hiding-principle.md)
- [Single Responsibility Principle](single-responsibility-principle.md)
- [Separation of Concerns Principle](separation-of-concerns-principle.md)
- [No Circular Dependencies Principle](no-circular-dependencies-principle.md)
- [Opaque Types Principle](opaque-types-principle.md)
- [Minimal Interfaces Principle](minimal-interfaces-principle.md)
- [Complete Documentation Principle](complete-documentation-principle.md)
- [Const Correctness Principle](const-correctness-principle.md)
- [No Parallel Implementations Principle](no-parallel-implementations-principle.md)
- [Composition Over Inheritance Principle](composition-over-inheritance-principle.md)
- [Design for Change Principle](design-for-change-principle.md)
- [Program Families Principle](program-families-principle.md)

### Tools & Automation
- [Automated Batch Documentation Fixes](automated-batch-documentation-fixes.md)
- [Progressive Tool Enhancement](progressive-tool-enhancement.md)
- [Module Renaming Script Enhancement](module-renaming-script-enhancement.md)
- [Script Enhancement Over One Off](script-enhancement-over-one-off.md)
- [Settings File Merging Pattern](settings-file-merging-pattern.md)
- [String Parsing Quote Tracking](string-parsing-quote-tracking.md)

### Development Practices
- [Documentation Validation Enhancement Patterns](documentation-validation-enhancement-patterns.md)
- [Example Marker Granular Control](example-marker-granular-control.md)
- [Multi-Language Documentation Validation](multi-language-documentation-validation.md)
- [Documentation Standards Integration](documentation-standards-integration.md)
- [Validated Documentation Examples](validated-documentation-examples.md)
- [Absolute Path Navigation](absolute-path-navigation.md)
- [Systematic Guideline Enhancement](systematic-guideline-enhancement.md)
- [Cross-Referencing Knowledge Base](cross-referencing-knowledge-base.md)
- [Research-Driven Development](research-driven-development.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [OO to C Adaptation](oo-to-c-adaptation.md)
- [C to Zig Module Migration](c-to-zig-module-migration.md)
- [Zig Memory Allocation with ar_allocator](zig-memory-allocation-with-ar-allocator.md)
- [Zig Migration Memory Debugging](zig-migration-memory-debugging.md)
- [Zig-C Memory Tracking Consistency](zig-c-memory-tracking-consistency.md)
- [Zig Build Flag Configuration](zig-build-flag-configuration.md)
- [User Feedback Debugging Pattern](user-feedback-debugging-pattern.md)
- [Makefile Pattern Rule Management](makefile-pattern-rule-management.md)
- [Development Debug Tools](development-debug-tools.md)
- [Zig Integration Comprehensive](zig-integration-comprehensive.md)
- [Documentation Compacting Pattern](documentation-compacting-pattern.md)
- [Just-In-Time KB Reading](just-in-time-kb-reading.md)
- [Documentation Validation Error Patterns](documentation-validation-error-patterns.md)

### Code Quality & Smells
- [Code Smell: Long Method](code-smell-long-method.md)
- [Code Smell: Large Class](code-smell-large-class.md)
- [Code Smell: Long Parameter List](code-smell-long-parameter-list.md)
- [Code Smell: Data Clumps](code-smell-data-clumps.md)
- [Code Smell: Duplicate Code](code-smell-duplicate-code.md)
- [Code Smell: Feature Envy](code-smell-feature-envy.md)
- [Code Smell: Primitive Obsession](code-smell-primitive-obsession.md)

## Usage
Search for relevant patterns when facing similar challenges:
```bash
grep -r "keyword" ./kb/
```