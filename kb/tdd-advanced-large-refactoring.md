# TDD for Large Refactoring (Advanced Pattern)

## Learning
How to apply TDD principles to large architectural changes by breaking them into multiple sequential cycles, each addressing one specific behavior.

## Importance
Large refactoring tasks can be overwhelming and error-prone without a systematic approach. TDD provides structure and safety through incremental changes.

## Example
```c
// Example: Refactoring facade to handle 9 different instruction types
// Instead of one massive change, break into 9 TDD cycles:

// Cycle 1: Assignment instructions
// Red: Write test for assignment evaluation
void test_facade__evaluates_assignment() {
    // Given an assignment instruction AST
    ar_instruction_ast_t *own_ast = ar_instruction_ast__create_assignment("x", "42");
    
    // When evaluated through facade
    ar_data_t *own_result = ar_instruction_facade__evaluate(own_ast, memory);
    
    // Then assignment succeeds
    assert(ar_data__get_integer(own_result) == 1);  // 1 = true in AgeRun
    
    ar_data__destroy(own_result);
    ar_instruction_ast__destroy(own_ast);
}

// Green: Implement minimal code for assignment
// Refactor: Extract common patterns

// Cycle 2: Send instructions  
// Red: Write test for send evaluation
// Green: Implement minimal code
// Refactor: Reuse patterns from Cycle 1

// Continue for all 9 instruction types...
```

## Generalization
- Breaking down architectural changes: Divide large refactoring into multiple sequential TDD cycles
- Each cycle addresses one behavior: Don't try to implement multiple instruction types in one cycle
- Build on previous cycles: Later cycles can assume earlier cycles work correctly
- Refactor phase is critical: Use refactor phase to eliminate duplication and extract common patterns
- Accept partial improvements: Better implementation with some duplication > wrong abstraction
- Verify integration points: Check that related cycles properly connect (e.g., log propagation through hierarchies)
- All cycles before commit: Complete ALL planned cycles before documentation and commit
- MANDATORY facade updates: Always include facade/integration updates as separate TDD cycles in same plan

## Multi-Session Tracking

For refactorings spanning multiple work sessions, add tracking:

```markdown
### TDD Cycle 1: Remove wake messages
**Session tracking**: [ ] Started [ ] Completed

### TDD Cycle 2: Remove sleep messages  
**Session tracking**: [ ] Started [ ] Completed
```

Benefits:
- Resume work with clear context
- Track progress across days/weeks
- Enable handoff between team members
- Prevent losing work state

## Implementation
1. Identify all behaviors that need to change
2. Create a plan listing each behavior as a separate TDD cycle
3. Add session tracking checkboxes for multi-session work
4. For each cycle:
   - Mark as started when beginning work
   - Write failing test for ONE behavior
   - Implement minimal code to pass
   - Refactor to improve design
   - Mark as completed when done
5. After all cycles complete:
   - Update documentation
   - Update TODO.md and CHANGELOG.md
   - Commit all changes together

Example pattern: 9 cycles for facade refactoring (assignment, send, if, parse, build, method, agent, destroy, error handling)

## Related Patterns
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
- [Facade Pattern Coordination](facade-pattern-coordination.md)
- [Ownership Pattern Extraction](ownership-pattern-extraction.md)
- [TDD Feature Completion Before Commit](tdd-feature-completion-before-commit.md)
- [Multi-Session TDD Planning](multi-session-tdd-planning.md)
- [Lifecycle Event Separation TDD](lifecycle-event-separation-tdd.md)
- [Progressive System Decoupling TDD](progressive-system-decoupling-tdd.md)
- [Compilation-Based TDD Approach](compilation-based-tdd-approach.md)