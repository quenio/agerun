# User Feedback as Quality Assurance

## Learning
User feedback during development planning serves as critical quality assurance, often revealing missing verification steps, overlooked constraints, and gaps in implementation plans that prevent bugs.

## Importance
Leverages domain expertise to catch planning gaps early, ensures guidelines remain practical and actionable, and prevents implementation errors by incorporating real-world constraints into plans.

## Example
User feedback improved development quality throughout session:
- **Missing verification**: User pointed out need for diff verification when moving code between modules
- **Module constraints**: User identified that different parsers have different requirements (method parser: all quoted args, build parser: expression args)
- **Documentation gaps**: User caught missing interface documentation updates
- **Plan completeness**: User suggested generic solutions (ar_path) instead of specific modules (ar_memory_path)

## Generalization
Integrate user-as-QA pattern into development workflow:
1. **Present plans before implementation**: Share detailed implementation plans with domain experts
2. **Listen for constraint corrections**: Users often know module-specific requirements
3. **Ask about missing steps**: "What verification steps am I missing?"
4. **Validate assumptions**: "Does this approach fit the existing architecture?"
5. **Check completeness**: "Are there other similar cases I should consider?"
6. **Update plans immediately**: Incorporate feedback before starting implementation

## Implementation
```markdown
# Plan presentation template
## Proposed Approach
[Detailed implementation plan]

## Verification Steps
[What will be checked to ensure correctness]

## Questions for Review
- Are there similar patterns in the codebase I should consider?
- What verification steps might I be missing?
- Are there module-specific constraints I should know about?
- Would a more generic approach be better?

# Feedback integration
## User Feedback Received
- [Specific feedback item]
- [Another feedback item]

## Plan Updates
- [How plan was modified based on feedback]
- [Additional verification steps added]
```

## Related Patterns