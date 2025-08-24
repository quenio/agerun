# Quantitative Priority Setting Pattern

## Learning
Use specific metrics and percentages to set task priorities objectively rather than relying on subjective assessments. Quantifying technical debt with numbers like "97.6% silent failures" or "41 unlogged error conditions" makes prioritization decisions data-driven and indisputable.

## Importance
Subjective priority labels like "important" or "critical" lead to debates and poor decisions. Quantitative metrics eliminate ambiguity - everyone agrees that 97.6% silent failures is worse than 12.5% silent failures. This enables rational resource allocation and clear communication with stakeholders.

## Example
```c
// Quantitative analysis revealing priority
// ar_expression_parser analysis:
// - Total error conditions: 41 (counted via grep "return NULL")
// - Logged errors: 1 (counted via grep "ar_log__error")
// - Silent failure rate: 97.6%
// - Priority: MOST CRITICAL

// ar_instruction_parser analysis:
// - Total error conditions: 8
// - Logged errors: 1  
// - Silent failure rate: 87.5%
// - Priority: HIGH

// ar_agency analysis:
// - Total error conditions: 10
// - Logged errors: 5
// - Silent failure rate: 50%
// - Priority: ADEQUATE (no action needed)
```

## Generalization
**Metrics for different technical debt types**:

1. **Error handling gaps**:
   - Silent failure percentage
   - Unlogged error count
   - Error conditions per KLOC

2. **Code duplication**:
   - Lines of duplicated code
   - Number of duplicate instances
   - Maintenance cost multiplier

3. **Performance issues**:
   - Response time degradation %
   - Memory usage increase
   - CPU cycles wasted

4. **Test coverage gaps**:
   - Uncovered lines percentage
   - Untested error paths count
   - Critical paths without tests

**Priority thresholds**:
- **CRITICAL**: > 90% gap or > 100 instances
- **HIGH**: 70-90% gap or 50-100 instances
- **MEDIUM**: 40-70% gap or 20-50 instances
- **LOW**: < 40% gap or < 20 instances

## Implementation
```bash
# Automated priority calculation script
#!/bin/bash
analyze_module_priority() {
    local module=$1
    
    # Count metrics
    local error_conditions=$(grep -c "return NULL\|return false" $module)
    local logged_errors=$(grep -c "ar_log__error" $module)
    
    # Calculate percentage
    if [ $error_conditions -gt 0 ]; then
        local silent_rate=$((100 * ($error_conditions - $logged_errors) / $error_conditions))
        
        # Assign priority based on metrics
        if [ $silent_rate -gt 90 ]; then
            echo "CRITICAL - $silent_rate% silent failures ($error_conditions conditions, $logged_errors logged)"
        elif [ $silent_rate -gt 70 ]; then
            echo "HIGH - $silent_rate% silent failures"
        elif [ $silent_rate -gt 40 ]; then
            echo "MEDIUM - $silent_rate% silent failures"
        else
            echo "LOW - $silent_rate% silent failures"
        fi
    else
        echo "N/A - No error conditions found"
    fi
}

# Generate priority report
for module in modules/*_parser.c; do
    echo "$(basename $module): $(analyze_module_priority $module)"
done | sort -t: -k2 -r  # Sort by priority
```

## Benefits
- **Objective decisions**: Numbers don't lie or play politics
- **Clear communication**: "97.6% silent failures" is unambiguous
- **Progress tracking**: Can measure improvement quantitatively
- **Resource justification**: Easy to explain why X needs fixing before Y
- **Prevents bike-shedding**: Hard to argue against data

## Anti-patterns to Avoid
- **Vague priorities**: "This seems important"
- **Opinion-based ranking**: "I think X is more critical"
- **Feature-based priority**: Prioritizing by what's visible vs what's broken
- **Recency bias**: Latest issue becomes highest priority

## Related Patterns
- [Report-Driven Task Planning](report-driven-task-planning.md)
- [TDD Cycle Effort Estimation](tdd-cycle-effort-estimation.md)
- [Quantitative Documentation Metrics](quantitative-documentation-metrics.md)
- [Systematic Error Whitelist Reduction](systematic-error-whitelist-reduction.md)
- [Module Consistency Verification](module-consistency-verification.md)