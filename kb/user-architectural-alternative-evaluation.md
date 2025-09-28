# User Architectural Alternative Evaluation

## Learning
When users suggest architectural alternatives to proposed plans, evaluate them systematically rather than defending original approaches. User suggestions often reveal simpler, more maintainable solutions.

## Importance
- **Architectural Quality**: User feedback can identify over-engineered solutions
- **Maintainability**: Simpler approaches reduce long-term technical debt
- **User Expertise**: Users understand their domain and maintenance constraints
- **Collaboration**: Shows respect for user input and domain knowledge

## Example
Original plan: Add 104+ individual KB article references to AGENTS.md
```markdown
// ORIGINAL APPROACH: Individual enumeration
- Memory patterns ([details](kb/ownership-naming-conventions.md))
- Build patterns ([details](kb/build-verification-before-commit.md))
- Test patterns ([details](kb/test-assertion-strength-patterns.md))
// ... 101 more lines requiring constant maintenance
```

User alternative: Single guideline + existing index
```markdown
// USER SUGGESTION: Leverage existing infrastructure
**Check KB index FIRST**: Before any task, grep kb/README.md for relevant patterns
```

Result: 95% reduction in maintenance overhead, leverages existing kb/README.md index

## Pattern Recognition
User suggestions that indicate over-engineering:
- "Wouldn't it be more practical to..."
- "Instead of [complex approach], couldn't we..."
- References to existing infrastructure you missed
- Questions about necessity of proposed complexity

## Evaluation Process
1. **Listen carefully** to user feedback without defensive reactions
2. **Identify core problem** the user's alternative addresses
3. **Compare maintenance costs** between approaches
4. **Leverage existing infrastructure** when possible
5. **Acknowledge and implement** superior alternatives

## Implementation Guidelines
- Ask clarifying questions about user suggestions
- Verify existing infrastructure capabilities before creating new solutions
- Document why user alternative was chosen over original plan
- Update plans immediately when user provides better approach

## Anti-Patterns
- Defending original plan without evaluation
- Dismissing user input as "not understanding the technical requirements"
- Proceeding with complex solution when simpler alternative exists
- Missing opportunities to leverage existing infrastructure

## Related Patterns
- [Check Existing Solutions First](check-existing-solutions-first.md)
- [Index-Based Knowledge Discovery Pattern](index-based-knowledge-discovery-pattern.md)
- [User Feedback as QA](user-feedback-as-qa.md)
- [Assumption Verification Before Action](assumption-verification-before-action.md)