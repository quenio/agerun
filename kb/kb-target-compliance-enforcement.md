# KB Target Compliance Enforcement

## Learning
When KB articles specify quantitative targets (e.g., "40-50% reduction"), achieving significantly less (e.g., 3%) means the KB guidance was not followed, regardless of effort expended. Commands must enforce KB targets with gates that FAIL, not just warn, when targets are missed.

## Importance
KB articles represent established best practices and proven targets. Accepting lower achievement undermines the knowledge base and creates precedent for partial compliance. In this session, achieving 3% CHANGELOG reduction when KB required 40-50% meant the work was incomplete, not that the target was unrealistic.

## Example
```bash
# BAD: Warning that can be ignored
if [ $REDUCTION -lt 30 ]; then
    echo "⚠️ WARNING: Only $REDUCTION% reduction achieved (target: 30%+)"
fi
# Proceeds anyway - target violated

# GOOD: Enforcing gate that blocks progress
if [ $REDUCTION -lt 40 ]; then
    echo "❌ FAILURE: Only $REDUCTION% reduction achieved"
    echo "Per kb/documentation-compacting-pattern.md:"
    echo "  'For historical records (e.g., CHANGELOG.md):"
    echo "   5. Target 40-50% reduction while keeping all information'"
    echo ""
    echo "Current work is INCOMPLETE - continue until 40% minimum is reached"
    exit 1
fi
# Blocks commit - must meet target
```

## Generalization
**KB Target Enforcement Pattern**:

1. **Read the target**: Quote exact percentage/number from KB article
2. **Measure actual**: Calculate what was achieved  
3. **Compare strictly**: Use >= for minimums, not approximate ranges
4. **Fail explicitly**: Exit with error code if target missed
5. **Reference KB**: Show user which article and section defines the target

**Types of KB targets**:
- **Reduction percentages**: 40-50% for historical records, 10-20% for mixed-state docs
- **Coverage thresholds**: 90%+ command excellence, 100% test coverage for critical paths
- **Quality metrics**: Zero memory leaks, zero circular dependencies
- **Completeness**: 3+ cross-references, 3+ command updates

**When targets can be adjusted**:
- Never unilaterally - consult user
- Document why in KB if target proves wrong
- Update KB article with new target
- Update all commands that reference it

## Implementation
```bash
# Target enforcement template
enforce_kb_target() {
    local actual=$1
    local min_target=$2
    local kb_article=$3
    local metric_name=$4
    
    if [ $actual -lt $min_target ]; then
        echo "❌ KB TARGET NOT MET"
        echo ""
        echo "Metric: $metric_name"
        echo "Achieved: $actual%"
        echo "Required: $min_target%+ (per $kb_article)"
        echo ""
        echo "KB Article Section:"
        grep -A 5 "Target.*$min_target" "kb/$kb_article" || \
        grep -A 5 "$metric_name" "kb/$kb_article"
        echo ""
        echo "STATUS: Work is INCOMPLETE"
        echo "ACTION: Continue until minimum target is reached"
        exit 1
    fi
    
    echo "✓ KB target met: $actual% (required: $min_target%+)"
}

# Usage
REDUCTION=35
enforce_kb_target $REDUCTION 40 "documentation-compacting-pattern.md" "CHANGELOG reduction"
```

## Common Compliance Failures

1. **Misreading targets**: Seeing "40-50%" as "30%+" due to wishful thinking
2. **Premature stopping**: Citing "complexity" when target is clear
3. **Approximation acceptance**: Treating 35% as "close enough" to 40%
4. **Warning demotion**: Using warnings instead of hard failures
5. **Scope creep**: Changing what's measured to hit target easier

## Prevention Strategy

**In Commands**:
- Quote exact target from KB in command documentation
- Implement automated gates at target thresholds
- Make gates FAIL (exit 1) not warn when target missed
- Show KB reference when gate triggers

**In Execution**:
- Measure target metric early and often
- Don't proceed to next step if target missed
- Ask user if target seems wrong, don't just ignore it
- Update KB if target proves incorrect, then follow new target

## Quality Indicators

Signs of proper target compliance:
- Actual achievement meets or exceeds KB minimum
- Gates enforce targets automatically
- Commit messages reference KB targets met
- No partial compliance accepted without explicit user approval

## Related Patterns
- [Gate Enforcement Exit Codes Pattern](gate-enforcement-exit-codes-pattern.md)
- [Command KB Consultation Enforcement](command-kb-consultation-enforcement.md)
- [Quantitative Documentation Metrics](quantitative-documentation-metrics.md)
- [Documentation Compacting Pattern](documentation-compacting-pattern.md)
- [Standards Over Expediency Principle](standards-over-expediency-principle.md)
