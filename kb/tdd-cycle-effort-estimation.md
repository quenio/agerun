# TDD Cycle Effort Estimation Pattern

## Learning
Estimate development effort in TDD cycles rather than hours or days. Each cycle represents one concrete behavior or error condition following Red-Green-Refactor. This provides more accurate estimates because cycles map directly to implementation work and are consistent units regardless of developer experience.

## Importance
Time-based estimates (hours/days) vary wildly between developers and often miss the mark. TDD cycles are atomic units of work that directly correspond to testable behaviors. A "2-cycle task" means exactly 2 behaviors to implement, making estimates more reliable and progress more measurable.

## Example
```c
// Effort estimation for ar_expression_parser error logging:
// Total: 8-10 TDD cycles

// Cycle 1: NULL parameter logging
// RED: Write test expecting error for NULL expression
// GREEN: Add if (!ref_expression) { log_error(...); return NULL; }
// REFACTOR: Extract error message to constant

// Cycle 2-3: Memory allocation failure logging (2 cycles)
// - One for parser allocation failure
// - One for string duplication failure

// Cycle 4-5: Invalid token type logging (2 cycles)
// - One for unexpected token types
// - One for invalid operator tokens

// Cycle 6-7: End of input logging (2 cycles)
// - One for unexpected EOF in expression
// - One for incomplete quoted strings

// Cycle 8-9: Parse state error logging (2 cycles)
// - One for invalid state transitions
// - One for recursive depth exceeded

// Documentation update: Not a cycle, included in effort
```

## Generalization
**Cycle counting guidelines**:

1. **One cycle per error condition type**:
   - NULL parameter validation = 1 cycle
   - Each distinct allocation failure = 1 cycle
   - Each parse error category = 1-2 cycles

2. **Group related behaviors**:
   - Similar NULL checks = 1 cycle total
   - Related validation errors = 1-2 cycles
   - Same error handling pattern = 1 cycle

3. **Effort multipliers**:
   - Simple logging: 1 cycle per condition
   - Complex validation: 2 cycles per condition
   - State-dependent errors: 2-3 cycles
   - Cross-module coordination: 3-4 cycles

4. **Non-cycle work** (included but not counted):
   - Documentation updates
   - Whitelist entries
   - Integration verification

## Implementation
```bash
#!/bin/bash
# Estimate TDD cycles for error logging enhancement

estimate_cycles() {
    local module=$1
    
    # Count different error patterns
    local null_returns=$(grep -c "return NULL" $module)
    local false_returns=$(grep -c "return false" $module)
    local existing_logs=$(grep -c "ar_log__error" $module)
    
    # Calculate cycles needed
    local unlogged=$((null_returns + false_returns - existing_logs))
    
    # Group similar errors (assume 3 errors per cycle on average)
    local cycles=$((unlogged / 3))
    [ $((unlogged % 3)) -ne 0 ] && cycles=$((cycles + 1))
    
    # Add complexity factors
    if grep -q "parse\|token\|ast" $module; then
        cycles=$((cycles + 2))  # Parser complexity
    fi
    
    if grep -q "allocat\|malloc" $module; then
        cycles=$((cycles + 1))  # Memory error handling
    fi
    
    echo "$cycles TDD cycles estimated"
    echo "- Error conditions to log: $unlogged"
    echo "- Base cycles (3 conditions/cycle): $((unlogged / 3))"
    echo "- Additional complexity cycles: $((cycles - unlogged / 3))"
}

# Generate estimates for all parsers
for parser in modules/*_parser.c; do
    echo "=== $(basename $parser) ==="
    estimate_cycles $parser
    echo
done
```

## Cycle Breakdown Template
```markdown
### Module Enhancement - [X] TDD Cycles

**Cycle 1: [Behavior Name]** (Red-Green-Refactor)
- [ ] RED: Write test expecting [specific error message]
- [ ] GREEN: Add minimal error logging code
- [ ] REFACTOR: Extract constants, improve clarity

**Cycle 2-3: [Related Behaviors]** (2 cycles)
- [ ] Cycle 2: [First variant]
- [ ] Cycle 3: [Second variant]

**Final Tasks** (not counted as cycles):
- [ ] Update documentation
- [ ] Add whitelist entries
- [ ] Verify zero memory leaks
```

## Benefits
- **Concrete units**: Cycles map to specific behaviors
- **Consistent sizing**: Each cycle is roughly equal effort
- **Progress visibility**: "3 of 8 cycles complete" is meaningful
- **Better accuracy**: Based on actual work, not time guesses
- **Learning curve agnostic**: Junior/senior do same cycles, just different speeds

## Estimation Accuracy Tips
- Review similar past work for cycle timing
- Add 20% buffer for unexpected complexity
- Count integration testing as 1-2 extra cycles
- Documentation is typically 0.5 cycles worth of effort

## Analysis-Driven Accuracy
Careful upfront analysis dramatically improves estimation accuracy:
- **Verify existing implementations** before estimating new work
- **Question inconsistencies** (e.g., "5 error classes but only 2 cycles?")
- **Map each condition** to specific cycles rather than guessing
- **Example**: ar_compile_instruction_parser seemed to need 2-3 cycles, but thorough analysis revealed only 2 were needed (5 of 7 conditions already handled)

## Related Patterns
- [Report-Driven Task Planning](report-driven-task-planning.md)
- [Quantitative Priority Setting](quantitative-priority-setting.md)
- [Red-Green-Refactor Cycle](red-green-refactor-cycle.md)
- [TDD Cycle Detailed Explanation](tdd-cycle-detailed-explanation.md)
- [Multi-Session TDD Planning](multi-session-tdd-planning.md)
- [Error Coverage Verification Before Enhancement](error-coverage-verification-before-enhancement.md)