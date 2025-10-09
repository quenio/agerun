# Systematic Task Analysis Protocol

## Overview

This protocol prevents incomplete task analysis by ensuring comprehensive data gathering before making priority recommendations. Based on session learning from 2025-09-27 where initial analysis found "1 pending task" when actually 71 tasks existed.

## Core Problem

**Analytical Error Pattern**: Making priority recommendations based on partial data due to:
- Narrow search focus (searching only priority headers vs all incomplete tasks)
- Confirmation bias (not questioning unexpectedly limited results)
- Surface-level analysis (rushing to conclusions vs thorough verification)

## Mandatory Protocol Steps

### 1. Complete Data Gathering (ALWAYS FIRST)

```bash
# Get total incomplete task count
grep "^- \[ \]" TODO.md | wc -l

# Get all incomplete tasks with line numbers
grep -n "^- \[ \]" TODO.md

# Cross-verify with different patterns
grep -n "pending\|incomplete\|TODO\|FIXME" TODO.md
```

**Rule**: Never analyze priorities without knowing the total scope.

### 2. Categorization Before Prioritization

Group tasks by:
- **High-level sections** (System Module, Agent Store, etc.)
- **Complexity level** (single TDD cycle vs multi-phase projects)
- **Dependencies** (prerequisite relationships)
- **Impact level** (user-facing vs internal improvements)

### 3. Evidence-Based Verification

Apply AGENTS.md principles:
- **"Always verify with concrete evidence, not assumptions"**
- **Show quantitative metrics** where available
- **Question unexpected results** (e.g., very few/many tasks)

### 4. Sanity Checks

Before presenting analysis:
- **Does task count seem reasonable?** (Active projects rarely have just 1-2 tasks)
- **Do priorities align with project context?** (Check recent completion patterns)
- **Are search methods comprehensive?** (Multiple grep patterns, different approaches)

### 5. Transparent Process

Present methodology:
- Show data gathering commands and results
- Explain categorization logic
- Provide quantitative justification for recommendations
- Admit uncertainty when appropriate

## Example Application

**Good Analysis Process**:
```
1. Total tasks: grep "^- \[ \]" TODO.md | wc -l → 71 tasks
2. Categories: Agent Store (14), System Module (5), YAML (9), etc.
3. Priority metrics: Agent Store has most detailed TDD cycles
4. Verification: Cross-check with section headers and recent completions
5. Recommendation: Agent Store Load Implementation (14 specific cycles ready)
```

**Bad Analysis Process**:
```
1. Search: grep "HIGHEST PRIORITY" → 3 results
2. Assumption: Few high-priority items means low activity
3. Conclusion: Pick the first available task
4. Error: Missed 70 other pending tasks
```

## Session Integration

### Session Start
- Check KB index for analysis protocols
- Apply systematic methodology to any priority questions
- Reference this article when doing task analysis

### Analysis Tasks
- Use TodoWrite to track analysis steps
- Document methodology in responses
- Show quantitative evidence

### Quality Assurance
- User feedback indicating analytical errors should trigger protocol review
- Update this article with new patterns as discovered

## Related Guidelines

- [Evidence-Based Debugging](evidence-based-debugging.md) - Apply same rigor to analysis
- [Quantitative Priority Setting](quantitative-priority-setting.md) - Use metrics for decisions
- [Knowledge Base Consultation Protocol](knowledge-base-consultation-protocol.md) - Check KB first

## Anti-Patterns to Avoid

1. **Priority Header Tunnel Vision** - Searching only section headers vs all tasks
2. **Confirmation Bias** - Not questioning limited search results
3. **Analysis Rush** - Jumping to conclusions without verification
4. **Single Search Method** - Using only one grep pattern
5. **Scope Minimization** - Assuming simple explanations without evidence

This protocol ensures comprehensive, evidence-based task analysis that reflects actual project state rather than partial perspectives.

## Related Patterns
- [KB Consultation Before Planning Requirement](kb-consultation-before-planning-requirement.md)
- [Task Verification Before Execution](task-verification-before-execution.md)
- [Quantitative Priority Setting](quantitative-priority-setting.md)
- [Report-Driven Task Planning](report-driven-task-planning.md)
- [Knowledge Base Consultation Protocol](knowledge-base-consultation-protocol.md)
- [Search Result Completeness Verification](search-result-completeness-verification.md) - Never limit searches without checking totals