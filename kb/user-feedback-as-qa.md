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

### Real-Time Implementation Feedback
During ar_compile_instruction_evaluator Zig migration, user interrupted multiple times to correct patterns:
- **Unnecessary includes**: "Is @cInclude("ar_compile_instruction_evaluator.h") really necessary?" → Removed
- **Redundant variables**: "Is `const evaluator = ref_evaluator.?` really necessary?" → Direct usage
- **API misunderstanding**: "If you had read the docs, you would know" → ar_data__claim_or_copy handles null
- **Variable naming**: "Instead of: const method_name Should be: const ref_method_name" → Ownership clarity
- **Direct instruction**: "I refuse to accept your code until you follow my instructions" → Simplified approach

During ar_method_resolver implementation:
- **Unclear integration**: "Not clear which functions of method_registry and semver will be used" → Detailed plan
- **Missing test**: "You did not do the following: Now let me add a test specifically for partial version resolution" → Added test
- **Spec compliance**: "Does the spec say anything about wildcard support in version numbers?" → Check SPEC.md first

During slash command role clarification:
- **Feature utility question**: "Are any of these names actually helping the agent?" → Revealed decorative nature
- **Simplification insight**: User question led to complete removal instead of improvement
- **Functional validation**: Roles provided no execution value, just perceived clarity

## Generalization
Integrate user-as-QA pattern into development workflow:
1. **Present plans before implementation**: Share detailed implementation plans with domain experts
2. **Listen for constraint corrections**: Users often know module-specific requirements
3. **Ask about missing steps**: "What verification steps am I missing?"
4. **Validate assumptions**: "Does this approach fit the existing architecture?"
5. **Check completeness**: "Are there other similar cases I should consider?"
6. **Update plans immediately**: Incorporate feedback before starting implementation
7. **Question feature utility**: When users ask "Is this actually helping?", validate functional value
8. **Consider removal**: User questions about utility often reveal unnecessary complexity

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
- [Plan Verification and Review](plan-verification-and-review.md)
- [User Feedback Debugging Pattern](user-feedback-debugging-pattern.md)
- [Specification Consistency Maintenance](specification-consistency-maintenance.md)
- [Comprehensive Output Review](comprehensive-output-review.md)
- [Architectural Review in Feedback](architectural-review-in-feedback.md)
- [Task Authorization Pattern](task-authorization-pattern.md)
- [Resource-Aware API Design](resource-aware-api-design.md)
- [Functional Value Validation Pattern](functional-value-validation-pattern.md)