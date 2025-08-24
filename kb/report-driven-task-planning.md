# Report-Driven Task Planning Pattern

## Learning
Before adding large sets of tasks to TODO.md, create comprehensive analysis reports that document the problem, quantify the impact, and provide detailed recommendations. These reports serve as permanent references that explain WHY tasks exist, not just WHAT needs to be done.

## Importance
Reports provide context that would otherwise be lost. Future developers (including yourself) can understand the reasoning behind task prioritization, see the full scope of the problem, and make informed decisions about implementation approach. This prevents redundant analysis and ensures work is based on data, not assumptions.

## Example
```bash
# 1. Create comprehensive analysis report
reports/module-consistency-analysis-2025-08-24.md
├── Executive Summary (key findings)
├── Analysis Methodology (how we discovered the issues)
├── Key Findings (quantitative data)
├── Impact Analysis (why it matters)
├── Recommendations (what to do)
└── Appendix (supporting data)

# 2. Reference report sections in TODO.md tasks
### 2. HIGH PRIORITY - Parser Module Error Logging Enhancement

**Rationale**: Module consistency analysis (reports/module-consistency-analysis-2025-08-24.md) 
revealed severe underutilization...

**Success Criteria** (per reports/module-consistency-analysis-2025-08-24.md Section 4):
- All NULL returns have associated error logging...

**Estimated Benefits** (per report Section 7):
- Reduce debugging time by 50-70%...
```

## Generalization
1. **Trigger for report creation**:
   - Multiple modules need similar changes
   - Technical debt exceeds 10+ tasks
   - Architectural decisions need documentation
   - Performance issues need quantification

2. **Report structure**:
   - Executive summary with key metrics
   - Methodology (how issues were found)
   - Quantitative findings with tables
   - Impact analysis (cost of not fixing)
   - Prioritized recommendations
   - Effort estimates

3. **TODO.md integration**:
   - Reference report in task rationale
   - Link specific sections for criteria
   - Use report metrics for priority
   - Quote benefits for motivation

## Implementation
```bash
# 1. Run analysis and gather metrics
for module in modules/*.c; do
    echo "=== $(basename $module) ==="
    echo "Error conditions: $(grep -c "return NULL" $module)"
    echo "Error logs: $(grep -c "ar_log__error" $module)"
done > analysis_data.txt

# 2. Create report from template
cat > reports/analysis-$(date +%Y-%m-%d).md << 'EOF'
# [Analysis Title] Report
**Date**: $(date +%Y-%m-%d)
**Focus**: [What was analyzed]

## Executive Summary
[Key findings with metrics]

## Analysis Methodology
[How the analysis was conducted]

## Key Findings
| Module | Metric 1 | Metric 2 | Impact |
|--------|----------|----------|--------|
| [data] | [data]   | [data]   | [data] |

## Recommendations
[Prioritized action items]
EOF

# 3. Update TODO.md with references
## [Priority] - [Task Category]
**Rationale**: [Analysis type] (reports/[report-name]) revealed [key finding]
**Impact**: [Quantified impact from report]
**Execution Plan** ([effort] from report Section X):
- [ ] Task referencing report section
```

## Benefits
- **Permanent context**: Reports preserve analysis reasoning
- **Data-driven decisions**: Quantitative metrics guide priority
- **Reduced re-analysis**: Future developers don't repeat work
- **Clear justification**: Stakeholders understand why work is needed
- **Progress tracking**: Can measure against original metrics

## Related Patterns
- [Quantitative Priority Setting](quantitative-priority-setting.md)
- [TDD Cycle Effort Estimation](tdd-cycle-effort-estimation.md)
- [Quantitative Documentation Metrics](quantitative-documentation-metrics.md)
- [Comprehensive Impact Analysis](comprehensive-impact-analysis.md)
- [Documentation Completion Verification](documentation-completion-verification.md)